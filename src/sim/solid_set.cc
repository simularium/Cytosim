// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "solid_set.h"
#include "solid_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"
#include "wrist.h"


#if ( 0 )
void SolidSet::step()
{
    for ( Solid * o = first(); o; o=o->next() )
        o->step();
}
#endif


//------------------------------------------------------------------------------

Property* SolidSet::newProperty(const std::string& cat, const std::string& nom, Glossary&) const
{
    if ( cat == "solid" )
        return new SolidProp(cat, nom);
    return nullptr;
}


Object * SolidSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Solid::TAG )
    {
        Property * p = simul.properties.find("solid", num);
#ifdef BACKWARD_COMPATIBILITY
        // prior to 04.2016, "bead" and "solid" were used interchangeably
        if ( !p )
             p = simul.properties.find("bead", num);
#endif
        if ( !p )
            throw InvalidIO("could not find `solid' class with id "+std::to_string(num));
        return new Solid(static_cast<SolidProp*>(p));
   }
    return nullptr;
}


/**
@ref Solid::build
 */
ObjectList SolidSet::newObjects(const std::string& name, Glossary& opt)
{
    SolidProp * p = simul.findProperty<SolidProp>("solid", name);
    Solid * obj = new Solid(p);
    
    ObjectList res;
    res.push_back(obj);
    res.append(obj->build(opt, simul));
    obj->fixShape();
    return res;
}


void SolidSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}

//------------------------------------------------------------------------------

void SolidSet::add(Object * obj)
{
    assert_true(obj->tag() == Solid::TAG);
    ObjectSet::add(obj);
}


void SolidSet::remove(Object * obj)
{
    ObjectSet::remove(obj);
    simul.singles.removeWrists(obj);
}


void SolidSet::foldPositions(Modulo const* m) const
{
    for ( Solid * o=first(); o; o=o->next() )
        o->foldPosition(m);
}

