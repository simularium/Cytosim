// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "nucleator.h"
#include "nucleator_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "fiber_prop.h"
#include "fiber_set.h"
#include "hand_monitor.h"
#include "simul.h"


//------------------------------------------------------------------------------

Nucleator::Nucleator(NucleatorProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
    gspTime = RNG.exponential();
}

//------------------------------------------------------------------------------

void Nucleator::makeFiber(Simul& sim, Vector pos, std::string const& fiber_type, Glossary& opt)
{
    ObjectList objs = sim.fibers.newObjects(fiber_type, opt);
    if ( objs.empty() )
        return;
    Fiber * fib = Fiber::toFiber(objs[0]);
    sim.add(objs);

    ObjectMark mk = 0;
    if ( opt.set(mk, "mark") )
        Simul::mark(objs, mk);
    else
        Simul::mark(objs, haMonitor->nucleatorID());

    Rotation rot(0, 1);

    Hand const* h = haMonitor->otherHand(this);

    if ( h && h->attached() )
    {
        // nucleating on the side of a 'mother' fiber:
        Vector dir = h->dirFiber();
        // select rotation to align with direction of 'mother' fiber:
        rot = Rotation::randomRotationToVector(dir);
        real A = prop->nucleation_angle;
        // add deviation 'nucleation_angle' in branching
        real L = haMonitor->linkRestingLength();
#if ( DIM == 2 )
        real F = RNG.sflip();
        // shift position by the length of the interaction:
        pos += rot * Vector(0, L*F, 0);
        rot = rot * Rotation::rotation(std::cos(A), std::sin(A)*F);
#elif ( DIM == 3 )
        // shift position by the length of the interaction:
        pos += rot * Vector(0, L, 0);
        rot = rot * Rotation::rotationAroundZ(A);
#endif
        // equalize marks to highlight amplification:
        mk = h->fiber()->mark();
        if ( mk == 0 )
        {
            mk = h->fiber()->identity();
            h->fiber()->mark(mk);
        }
        // remove key to avoid unused warning:
        opt.clear("direction");
    }
    else
    {
        // nucleating in the bulk:
        std::string str;
        if ( opt.set(str, "direction") )
        {
            std::istringstream ss(str);
            Vector dir = Movable::readDirection(ss, pos, fib->prop->confine_space_ptr);
            rot = Rotation::randomRotationToVector(dir);
        }
        else
            rot = Rotation::randomRotation();
    }
    
    ObjectSet::rotateObjects(objs, rot);
    /*
     We translate Fiber to match the Nucleator's position,
     and if prop->hold_end, the Hand is attached to the new fiber
     */
    if ( prop->hold_end == MINUS_END )
    {
        attachEnd(fib, MINUS_END);
        pos -= fib->posEndM();
    }
    else if ( prop->hold_end == PLUS_END )
    {
        attachEnd(fib, PLUS_END);
        pos -= fib->posEndP();
    }
    else
        pos -= fib->position();
    
    ObjectSet::translateObjects(objs, pos);
    //std::clog << "nucleated fiber in direction " << fib->dirEndM() << "\n";

    opt.print_warning(std::cerr, 1, "in nucleator's spec\n");
}


//------------------------------------------------------------------------------
/**
 Does not attach nearby Fiber, but can nucleate
 */
void Nucleator::stepUnattached(Simul& sim, Vector const& pos)
{
    assert_false( attached() );
    
    gspTime -= prop->rate_dt;
    
    if ( gspTime < 0 )
    {
        gspTime = RNG.exponential();
        try {
            Glossary opt(prop->fiber_spec);
            makeFiber(sim, pos, prop->fiber_type, opt);
        }
        catch( Exception & e )
        {
            e << "\nException occurred while executing nucleator:code";
            throw;
        }
    }
}


void Nucleator::stepUnloaded()
{
    assert_true( attached() );
    
    if ( testDetachment() )
        return;
    
    /// OPTION 1: delete entire fiber
    if ( prop->addictive == 2 )
    {
        delete(fiber());
        return;
    }
    
    // may track the end of the Fiber:
    if ( prop->track_end == MINUS_END )
        relocateM();
    else if ( prop->track_end == PLUS_END )
        relocateP();
}


void Nucleator::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    if ( testKramersDetachment(force_norm) )
        return;
    
    // may track the end of the Fiber:
    if ( prop->track_end == MINUS_END )
        relocateM();
    else if ( prop->track_end == PLUS_END )
        relocateP();
}


//------------------------------------------------------------------------------
/**
 If prop->addictive, this gives a poisonous goodbye-kiss to the fiber
 */
void Nucleator::detach()
{
    if ( prop->addictive )
        fiber()->setDynamicState(nearestEnd(), STATE_RED);
    
    Hand::detach();
}

