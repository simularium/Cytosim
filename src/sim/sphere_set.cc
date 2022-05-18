// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "sphere_set.h"
#include "sphere_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "wrist.h"
#include "simul.h"


void SphereSet::remove(Object * obj)
{
    ObjectSet::remove(obj);
    simul.singles.removeWrists(obj);
}


Property* SphereSet::newProperty(const std::string& cat, const std::string& nom, Glossary&) const
{
    if ( cat == "sphere" )
        return new SphereProp(nom);
    return nullptr;
}


Object * SphereSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Sphere::TAG )
    {
        SphereProp * p = simul.findProperty<SphereProp>("sphere", num);
        return new Sphere(p);
    }
    return nullptr;
}

/**
 @copydetails Sphere::build
 */
ObjectList SphereSet::newObjects(const std::string& name, Glossary& opt)
{
    SphereProp * p = simul.findProperty<SphereProp>("sphere", name);
        
    // set radius if provided as argument
    real rad = -1;
    if ( !opt.set(rad, "radius" ) || rad <= 0 )
        throw InvalidParameter("parameter `radius` should be specified and > 0");
    
    // possibly add some variability
    real dev = 0;
    if ( opt.set(dev, "radius", 1) )
    {
        real r;
        do
            r = rad + dev * RNG.gauss();
        while ( r < REAL_EPSILON );
        rad = r;
    }
    
    Sphere * obj = new Sphere(p, rad);

    ObjectList res;
    res.push_back(obj);
    res.append(obj->build(opt, simul));
    return res;
}


void SphereSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}


void SphereSet::foldPositions(Modulo const* m) const
{
    for ( Sphere * o=SphereSet::first(); o; o=o->next() )
        o->foldPosition(m);
}

