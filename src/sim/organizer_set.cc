// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "organizer_set.h"
#include "organizer.h"
#include "mecapoint.h"
#include "glossary.h"
#include "nucleus.h"
#include "bundle.h"
#include "aster.h"
#include "fake.h"
#include "solid.h"
#include "simul.h"


// first Organizer
Organizer * OrganizerSet::first() const
{
    return static_cast<Organizer*>(nodes.front());
}

// find object with given ID
Organizer * OrganizerSet::findID(ObjectID n) const
{
    return static_cast<Organizer*>(inventory.get(n));
}

//------------------------------------------------------------------------------

void OrganizerSet::step()
{
    Organizer * obj = first(), * nxt;
    while ( obj )
    {
        nxt = obj->next();
        obj->step();
        obj = nxt;
    }
}

//------------------------------------------------------------------------------

Property* OrganizerSet::newProperty(const std::string& cat, const std::string& nom, Glossary&) const
{
    if ( cat == "aster" )   return new AsterProp(nom);
    if ( cat == "bundle" )  return new BundleProp(nom);
    if ( cat == "nucleus" ) return new NucleusProp(nom);
    if ( cat == "fake" )    return new FakeProp(nom);
    return nullptr;
}


Object * OrganizerSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Aster::TAG )
    {
        AsterProp * p = simul.findProperty<AsterProp>("aster", num);
        return new Aster(p);
    }
    
    if ( tag == Bundle::TAG )
    {
        BundleProp * p = simul.findProperty<BundleProp>("bundle", num);
        return new Bundle(p);
    }
    
    if ( tag == Nucleus::TAG )
    {
        NucleusProp * p = simul.findProperty<NucleusProp>("nucleus", num);
        return new Nucleus(p);
    }
    
    if ( tag == Fake::TAG )
    {
        FakeProp * p = simul.findProperty<FakeProp>("fake", num);
        return new Fake(p);
    }
    
    return nullptr;
}


ObjectList OrganizerSet::newObjects(const std::string& nom, Glossary& opt)
{
    Organizer * obj = nullptr;
    Property * p = simul.properties.find_or_die(nom);
    
    if ( p->category() == "aster" )
        obj = new Aster(static_cast<AsterProp*>(p));
    else if ( p->category() == "bundle" )
        obj = new Bundle(static_cast<BundleProp*>(p));
    else if ( p->category() == "nucleus" )
        obj = new Nucleus(static_cast<NucleusProp*>(p));
    else if ( p->category() == "fake" )
        obj = new Fake(static_cast<FakeProp*>(p));

    ObjectList res;
    if ( obj )
    {
        res = obj->build(opt, simul);
        res.push_back(obj);
    }
    
    return res;
}


void OrganizerSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}

//------------------------------------------------------------------------------

void OrganizerSet::add(Object * obj)
{
    ObjectSet::add(obj);
    // we also link all dependent objects:
    static_cast<Organizer*>(obj)->addOrganized(simul);
}


ObjectID OrganizerSet::findOrganizerID(const Mecable * m) const
{
    ObjectID res = 0;
    for ( Organizer * o=first(); o; o=o->next() )
        for ( size_t i = 0; i < o->nbOrganized(); ++i )
            if ( m == o->organized(i) )
                res = std::max(res, o->identity());

    return res;
}


void OrganizerSet::report(std::ostream& os) const
{
    if ( size() > 0 )
    {
        unsigned total = 0;
        os << '\n' << title();
        for ( Property * i : simul.properties.find_all("aster", "bundle") )
        {
            unsigned cnt = count(match_property, i);
            os << '\n' << std::setw(10) << cnt << " " << i->name();
            ++total;
        }
        for ( Property * i : simul.properties.find_all("nucleus", "fake") )
        {
            unsigned cnt = count(match_property, i);
            os << '\n' << std::setw(10) << cnt << " " << i->name();
            ++total;
        }
        if ( total > 1 )
            os << '\n' << std::setw(10) << size() << " total";
    }
}


