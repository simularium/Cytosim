// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_site.h"
#include "iowrapper.h"
#include "simul.h"
#include "sim.h"


FiberSite::FiberSite(Fiber* f, real a)
: fbFiber(f), fbAbs(a)
{
    assert_true(f);
#if FIBER_HAS_LATTICE
    fbLattice = nullptr;
    fbSite = 0;
#endif
    inter = f->interpolate(a);
}


void FiberSite::relocateM()
{
    assert_true(fbFiber);
    fbAbs = fbFiber->abscissaM();
    inter = fbFiber->interpolateEndM();
#if FIBER_HAS_LATTICE
    assert_true(!fbLattice);
#endif
}


void FiberSite::relocateP()
{
    assert_true(fbFiber);
    fbAbs = fbFiber->abscissaP();
    inter = fbFiber->interpolateEndP();
#if FIBER_HAS_LATTICE
    assert_true(!fbLattice);
#endif
}


//------------------------------------------------------------------------------
#pragma mark -


FiberEnd FiberSite::nearestEnd() const
{
    assert_true(fbFiber);
    if ( fbAbs > fbFiber->abscissaC() )
        return PLUS_END;
    else
        return MINUS_END;
}


real FiberSite::distanceToEnd(FiberEnd end) const
{
    assert_true(fbFiber);
    if ( end == PLUS_END )
        return fbFiber->abscissaP() - fbAbs;
    else
    {
        assert_true(end == MINUS_END);
        return fbAbs - fbFiber->abscissaM();
    }
}


real  FiberSite::abscissaFrom(const FiberEnd ref) const
{
    assert_true(fbFiber);
    switch( ref )
    {
        case MINUS_END:  return fbAbs - fbFiber->abscissaM();
        case PLUS_END:   return fbFiber->abscissaP() - fbAbs;
        case ORIGIN:     return fbAbs;
        case CENTER:     return fbAbs - fbFiber->abscissaC();
        default:         ABORT_NOW("invalid argument value");
    }
    return 0;
}


//------------------------------------------------------------------------------
#pragma mark - I/O

void FiberSite::write(Outputter& out) const
{
    out.writeSoftSpace();
    if ( fbFiber )
    {
        checkAbscissa();
#if FIBER_HAS_LATTICE
        if ( fbLattice )
        {
            Object::writeReference(out, Fiber::TAG_LATTICE, fbFiber->identity());
            // in older format, `fbAbs` was written here
            out.writeInt32(fbSite);
        }
        else
#endif
        {
            Object::writeReference(out, fbFiber);
            out.writeFloat(fbAbs);
        }
    }
    else
    {
        Object::writeNullReference(out);
    }
}


void FiberSite::read(Inputter& in, Simul& sim)
{
    ObjectTag tag = 0;
    Object * w = sim.readReference(in, tag);
    fbFiber = static_cast<Fiber*>(w);

    if ( w )
    {
        //std::clog << "FiberSite::read() " << (char)tag << std::endl;
        if ( tag == Fiber::TAG )
        {
            fbAbs  = in.readFloat();
#if FIBER_HAS_LATTICE
            // set mSite to closest integral position
            if ( fbLattice )
                fbSite = fbLattice->index(fbAbs);
#endif
        }
        else if ( tag == Fiber::TAG_LATTICE )
        {
#ifdef BACKWARD_COMPATIBILITY
            if ( in.formatID() < 49 )
                fbAbs = in.readFloat();
#endif
#if FIBER_HAS_LATTICE
            fbSite = in.readInt32();
            fbLattice = &fbFiber->lattice();
            // put in the middle of the site:
            ///@todo: we should use digit:site_shift here:
            fbAbs = ( fbSite + 0.5 ) * fbLattice->unit();
#else
            lati_t t = in.readUInt32();
            fbAbs = ( t + 0.5 ) * fbFiber->prop->lattice_unit;
            //throw InvalidIO("Cannot import Digit without fiber's lattice");
#endif
        }
#ifdef BACKWARD_COMPATIBILITY
        else if ( tag == 'm' )
        {
            fbAbs = in.readFloat();
        } 
#endif
        else
        {
            ///\todo: we should allow binder to refer to any Mecable
            throw InvalidIO("unexpected class in FiberSite");
        }

        reinterpolate();
        checkAbscissa();
    }
}

void FiberSite::print(std::ostream& os) const
{
    if ( fiber() )
    {
        int p = os.precision();
        os.precision(3);
        os << "(f" << fiber()->identity();
#if FIBER_HAS_LATTICE
        if ( fbLattice )
            os << " s " << fbSite;
        else
#endif
            os << " @ " << std::fixed << abscissa() << ")";
        os.precision(p);
    } else
        os << "(null)";
}

std::ostream& operator << (std::ostream& os, FiberSite const& obj)
{
    obj.print(os);
    return os;
}


//------------------------------------------------------------------------------
#pragma mark -


int FiberSite::checkAbscissa() const
{
    assert_true(fbFiber);
    
    real a = fbFiber->abscissaM() - fbAbs;
    if ( a > 1e-3 )
    {
        std::cerr << "FiberSite:abscissa < fiber:abscissa(MINUS_END) : " << a << '\n';
        return 2;
    }
    
    real b = fbAbs - fbFiber->abscissaP();
    if ( b > 1e-3 )
    {
        std::cerr << "FiberSite:abscissa > fiber:abscissa(PLUS_END)  : " << b << '\n';
        return 1;
    }
    return 0;
}


int FiberSite::bad() const
{
    if ( fbFiber != inter.mecable() )
    {
        std::cerr << "Interpolation mismatch " << fbFiber << " " << inter.mecable() << std::endl;
        return 7;
    }
    
    if ( fbFiber->betweenMP(fbAbs) )
    {
        const real e = fbAbs - abscissaInter();
        
        //std::clog << "Interpolation " << std::scientific << e << '\n';
        if ( std::abs(e) > 1e-3 )
        {
            std::cerr << "FiberSite::Interpolation error " << std::scientific << e << "\n";
            std::cerr << " abscissa:\n";
            std::cerr << "    binder       " << fbAbs << "\n";
            std::cerr << "    interpolated " << abscissaInter() << "\n";
            Interpolation pi = fbFiber->interpolate(fbAbs);
            std::cerr << "    updated      " << fbFiber->abscissaPoint(pi.point1()+pi.coef1()) << "\n";
            return 8;
        }
    }
    return 0;
}


