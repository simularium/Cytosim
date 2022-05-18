// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "assert_macro.h"
#include "nucleus.h"
#include "exceptions.h"
#include "sphere_prop.h"
#include "bundle_prop.h"
#include "mecapoint.h"
#include "fiber_set.h"
#include "glossary.h"
#include "bundle.h"
#include "simul.h"
#include "meca.h"


void Nucleus::step()
{
}


void Nucleus::setInteractions(Meca & meca) const
{
    Sphere * sph = sphere();
    
    if ( sph )
    {
        unsigned nix = sphere()->nbPoints() - Sphere::nbRefPoints;
        
        for ( unsigned ix = 0; ix < nix; ++ix )
        {
            const Fiber * fib = fiber(ix);
            if ( fib )
                meca.addLink(Mecapoint(sph, ix+Sphere::nbRefPoints),
                               fib->exactEnd(MINUS_END),
                               prop->stiffness );
        }
    }
}


//------------------------------------------------------------------------------
ObjectList Nucleus::build(Glossary& opt, Simul& sim)
{
    std::string str, spec;
    assert_true(prop);
    ObjectList res;
    size_t cnt = 0;
    
    real rad = -1;
    if ( !opt.set(rad, "radius" ) || rad <= 0 )
        throw InvalidParameter("parameter `radius` should be specified and > 0");
   
    if ( !opt.set(str, "sphere") )
        throw InvalidParameter("parameter `sphere` should be specified");

    SphereProp * sp = sim.findProperty<SphereProp>("sphere", str);
    Sphere * sph = new Sphere(sp, rad);
    grasp(sph);
    res.push_back(sph);
    
    // get the center of the sphere
    Vector c = sph->posP(0);
    
    if ( opt.set(cnt, "fibers") && cnt > 0 )
    {
        if ( !opt.set(str, "fibers", 1) )
            throw InvalidParameter("fiber type (fiber[1]) should be specified");
        opt.set(spec, "fibers", 2);

        // create points and clamps and add fiber attached to them
        for ( size_t ii = 0; ii < cnt; ++ii )
        {
            Glossary fiber_opt(spec);
            ObjectList objs = sim.fibers.newObjects(str, fiber_opt);
            if ( objs.size() )
            {
                Fiber * fib = Fiber::toFiber(objs[0]);
                Vector pos = c + Vector::randU(rad);
                Vector dir = Vector::randU();
                fib->setStraight(pos, dir, fib->length());
                sph->addPoint(pos);
                res.append(objs);
                grasp(fib);
            }
        }
    }
    
    if ( opt.set(cnt, "bundles") && cnt > 0 )
    {
        if ( !opt.set(str, "bundles", 1) )
            throw InvalidParameter("bundle type (bundles[1]) should be specified");
        opt.set(spec, "bundles", 2);

        BundleProp * bp = sim.findProperty<BundleProp>("bundle", str);

        Rotation rot;
        // add bundles
        const real len = 0.5 * bp->overlap;
        for ( size_t ii = 0; ii < cnt; ++ii  )
        {
            Glossary bundle_opt(spec);
            rot = Rotation::randomRotation();
            //a random position on the sphere:
            Vector pos = rot * Vector(0, rad, 0);
            //a direction tangent to the sphere:
            Vector dir = rot * Vector(RNG.sflip(), 0, 0);
            
            Bundle * bu = new Bundle(bp);
            ObjectList objs = bu->build(bundle_opt, sim);
            res.append(objs);
            res.push_back(bu);
            
            //position the bundle (initially aligned with X) tangentially:
            ObjectSet::moveObjects(objs, Isometry(pos, rot));
            
            sph->addPoint( c + (pos-len*dir).normalized(rad) );
            grasp(bu->organized(0));
            
            sph->addPoint( c + (pos+len*dir).normalized(rad) );
            grasp(bu->organized(1));
        }
    }
    
    return res;
}

//------------------------------------------------------------------------------


/**
 This sets the ends of the link number `inx`
 or returns zero if the link does not exist
 */
bool Nucleus::getLink(size_t inx, Vector& pos1, Vector& pos2) const
{
    size_t i = inx + Sphere::nbRefPoints;
    if ( sphere() && i < sphere()->nbPoints() )
    {
        pos1 = sphere()->posP(i);
        
        Fiber const* fib = Fiber::toFiber(organized(inx+1));
        if ( fib )
            pos2 = fib->posEnd(MINUS_END);
        else
            pos2 = pos1;
        return true;
    }
    return false;
}

