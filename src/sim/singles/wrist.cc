// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "wrist.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo const* modulo;


Wrist::Wrist(SingleProp const* sp, Mecable const* mec, const unsigned pti)
: Single(sp)
{
    anchor.set(mec, pti);

#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=anchored");
#endif
}


Wrist::Wrist(SingleProp const* sp, Mecable const* mec, unsigned a, unsigned b, real c)
: Single(sp)
{
    assert_true(mec);
    anchor.set(mec, a, b, c);
    
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=anchored");
#endif
}


Wrist::Wrist(SingleProp const* sp, Mecable const* mec, unsigned ref, Vector pos)
: Single(sp)
{
    assert_true(mec);
    anchor.set(mec, ref, pos);
    
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=anchored");
#endif
}


Wrist::~Wrist()
{
}


Vector Wrist::force() const
{
    assert_true( sHand->attached() );
    Vector d = posFoot() - sHand->pos();
    
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}


void Wrist::stepF(Simul& sim)
{
    assert_false( sHand->attached() );

    sHand->stepUnattached(sim, posFoot());
}


void Wrist::stepA()
{
    assert_true( sHand->attached() );
    Vector f = force();
    sHand->stepLoaded(f, f.norm());
}


void Wrist::setInteractions(Meca & meca) const
{
    anchor.addLink(meca, sHand->interpolation(), prop->stiffness);
}


void Wrist::write(Outputter& out) const
{
    sHand->write(out);
    out.writeSoftSpace();
    anchor.write(out);
}


void Wrist::read(Inputter& in, Simul& sim, ObjectTag tag)
{
    const int s = state();

    sHand->read(in, sim);
    
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 47 )
    {
        Mecapoint base;
        base.read(in, sim);
        anchor.set(base.mecable(), base.point());
    }
    else
#endif
        anchor.read(in, sim);
    
    /*
     Because the SingleSet has 2 lists where Single are stored depending
     on their bound/unbound state, we need to unlink and relink here, in
     case the state stored on file is different from the current state.
     */
    if ( s != state() )
    {
        SingleSet * set = static_cast<SingleSet*>(objset());
        if ( set )
        {
            if ( s )
                set->relinkD(this);
            else
                set->relinkA(this);
        }
    }
}

