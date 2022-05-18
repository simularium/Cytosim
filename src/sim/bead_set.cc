// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bead_set.h"
#include "bead_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "wrist.h"
#include "simul.h"


Property* BeadSet::newProperty(const std::string& cat, const std::string& nom, Glossary&) const
{
    if ( cat == "bead" )
        return new BeadProp(cat, nom);
    return nullptr;
}


Object * BeadSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Bead::TAG )
    {
        BeadProp * p = simul.findProperty<BeadProp>("bead", num);
        return new Bead(p, Vector(0,0,0), 0);
    }
    return nullptr;
}

/**
 @ingroup NewObject

 By definition, a Bead has one point, and one can only set the radius of the Bead:

     new bead NAME
     {
       radius = REAL
     }

 <h3> How to add Single </h3>

 Singles can only be attached at the center of the Bead:

     new bead NAME
     {
       radius = REAL
       attach = SINGLE_SPEC [, SINGLE_SPEC] ...
     }
 
 Where `SINGLE_SPEC` is string containing at most 3 words: `[INTEGER] NAME`,
 where the `INTEGER` specifies the number of Singles, `NAME` specifies their name.
 
 For example if `grafted` is the name of a Single, one can use:
 
     new bead NAME
     {
       attach = 10 grafted
     }

 */

ObjectList BeadSet::newObjects(const std::string& name, Glossary& opt)
{
    ObjectList res;
    // get sphere radius:
    real rad = -1;
    unsigned inx = 2;

    std::string var = "point1";
    if ( opt.has_key(var) )
    {
        if ( opt.value(var, 0) != "center" )
            throw InvalidParameter("the position of `point1` must be `center'");
        opt.set(rad, var, 1);
    }
    else
    {
        inx = 0;
        var = "attach";
        opt.set(rad, "radius");
        
        // possibly add some variability in the radius:
        real dev = 0;
        if ( opt.set(dev, "radius", 1) )
        {
            real r;
            do
                r = rad + dev * RNG.gauss();
            while ( r < REAL_EPSILON );
            rad = r;
        }
    }
    
    if ( rad <= 0 )
        throw InvalidParameter("bead:radius must be specified and > 0");

    BeadProp * p = simul.findProperty<BeadProp>("bead", name);
    Bead * obj = new Bead(p, Vector(0,0,0), rad);
    
    res.push_back(obj);
    
    std::string str;
    // attach different kinds of SINGLE
    while ( opt.set(str, var, inx++) )
        res.append(simul.singles.makeWrists(obj, 0, 1, str));

    return res;
}


void BeadSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}


void BeadSet::remove(Object * obj)
{
    ObjectSet::remove(obj);
    simul.singles.removeWrists(obj);
}


void BeadSet::foldPositions(Modulo const* m) const
{
    for ( Bead * o=first(); o; o=o->next() )
        o->foldPosition(m);
}
