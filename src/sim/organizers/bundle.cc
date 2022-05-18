// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "bundle.h"
#include "exceptions.h"
#include "mecapoint.h"
#include "interpolation.h"
#include "fiber_prop.h"
#include "glossary.h"
#include "simul.h"
#include "meca.h"


void Bundle::step()
{
    Simul & sim = simul();
    
    for ( size_t ii = 0; ii < nbOrganized(); ++ii )
    {
        if ( !organized(ii)  &&  RNG.test(prop->fiber_prob) )
        {
            Glossary opt(prop->fiber_spec);
            ObjectList objs = sim.fibers.newObjects(prop->fiber_type, opt);
            if ( objs.size() )
            {
                Fiber * fib = Fiber::toFiber(objs[0]);
                fib->adjustLength(prop->overlap, prop->focus);
                ///\todo: we should orient the new Fiber in bundle direction
                sim.add(objs);
                grasp(fib, ii);
            }
        }
    }
}


/*
 Parallel connection near the 'prop->focus' end of the fibers
*/
void Bundle::linkParallel(Meca & meca, Fiber * mt1, Fiber * mt2) const
{
    const real stiff = prop->stiffness;
    const real dis = prop->overlap;
    
    meca.addLink(mt1->interpolate(dis, prop->focus), mt2->interpolate(dis, prop->focus), stiff);
    meca.addLink(mt1->exactEnd(prop->focus), mt2->exactEnd(prop->focus), stiff);
}


/**
 Antiparallel connection near the 'prop->focus' end of the fibers
*/
void Bundle::linkAntiparallel(Meca & meca, Fiber * mt1, Fiber * mt2) const
{
    const real stiff = prop->stiffness;
    const real dis = prop->overlap;

    if ( dis < REAL_EPSILON )
        meca.addLink(mt1->exactEnd(prop->focus), mt2->exactEnd(prop->focus), stiff+stiff);
    else {
        meca.addLink(mt2->exactEnd(prop->focus), mt1->interpolate(dis, prop->focus), stiff);
        meca.addLink(mt1->exactEnd(prop->focus), mt2->interpolate(dis, prop->focus), stiff);
    }
}


/**
 Connect the fibers near their ends, to form a ring:
 1. connect fibers with their neighbors,
 2. close the ring by connecting first and last fibers.
 */
void Bundle::setInteractions(Meca & meca) const
{
    assert_true( linked() );
    
    if ( nbOrganized() > 0 )
    {
        Fiber * mt0 = Fiber::toFiber(organized(0));
        Fiber * mt1 = mt0, * mt2 = nullptr;
        
        for ( unsigned ii = 1 ; ii < nbOrganized(); ++ii )
        {
            mt2 = Fiber::toFiber(organized(ii));
            if ( mt1 && mt2 )
                linkAntiparallel(meca, mt1, mt2);
            mt1 = mt2;
        }
        
        // connect first and last fibers:
        mt1 = mt0;
        
        if ( mt1 && mt2 )
        {
            if ( nbOrganized() & 1 )
                linkParallel(meca, mt1, mt2);
            else
                linkAntiparallel(meca, mt1, mt2);
        }
    }
}


//------------------------------------------------------------------------------

Vector Bundle::position() const
{
    Vector res(0,0,0);
    for ( unsigned ii = 1 ; ii < nbOrganized(); ++ii )
    {
        Fiber const* fib = Fiber::toFiber(organized(ii));
        res += fib->posEnd(prop->focus);
    }
    return res / nbOrganized();
}


/**
 It is possible to specify the lengths of individual fibers:

 new bundle bundle
 {
    length = 3.0, 4.2
 }

 */
ObjectList Bundle::build(Glossary& opt, Simul& sim)
{
    assert_true(prop);
    ObjectList res;
    
    unsigned cnt = 0;
    std::string type, spec;
    opt.set(cnt,  "fibers");
    opt.set(type, "fibers", 1);
    opt.set(spec, "fibers", 2);
    
    if ( cnt <= 0 )
        throw InvalidParameter("number of fibers (fibers[0]) must be specified and >= 1");
    
    nbOrganized(cnt);
    
    for ( unsigned inx = 0; inx < cnt; ++inx )
    {
        Glossary fiber_opt(spec);
        ObjectList objs = sim.fibers.newObjects(type, fiber_opt);
        if ( objs.size() > 0 )
        {
            Fiber * fib = Fiber::toFiber(objs[0]);

            if ( fib )
            {
                // rotate odd fibers by 180 degrees to make an anti-parallel overlap:
                if ( inx & 1 )
                    ObjectSet::rotateObjects(objs, Rotation::rotation180());
                
                // translate to adjust the overlap:
                ObjectSet::translateObjects(objs, fib->posMiddle()-fib->posFrom(0.5*prop->overlap, prop->focus));
                
                real len;
                if ( opt.set(len, "length", inx) )
                    fib->adjustLength(len, prop->focus== PLUS_END?MINUS_END:PLUS_END);
                
                grasp(fib, inx);
            }
            res.append(objs);
        }
    }
    
    return res;
}


Bundle::~Bundle()
{
    prop = nullptr;
}


