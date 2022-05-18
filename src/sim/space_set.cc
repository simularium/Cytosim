// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_set.h"
#include "space_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"

//---------------------------- GLOBAL VARIABLES --------------------------------

/**
 This is a global variable that is initialized in Simul
 It is used to implement periodic boundary conditions
 */
Modulo const* modulo = nullptr;


/// static variable of SpaceSet:
Space const* SpaceSet::master_ = nullptr;

/**
 set current Space to `spc`. (spc==NULL is a valid argument).
 */
void SpaceSet::setMaster(Space const* spc)
{
    if ( spc != master_ )
    {
        master_ = spc;
        
#if ( 0 )
        if ( spc )
            std::clog << "setMaster(" << spc->prop->name() << ")" << std::endl;
        else
            std::clog << "setMaster(NULL)" << std::endl;
#endif
    }
    
    modulo = nullptr;

    if ( master_ )
        modulo = master_->getModulo();
}

//------------------------------------------------------------------------------

Property * SpaceSet::newProperty(const std::string& cat,const std::string& nom, Glossary&) const
{
    if ( cat == "space" )
        return new SpaceProp(nom);
    return nullptr;
}


void SpaceSet::step()
{
    for ( Space * sp = first(); sp; sp=sp->next() )
        sp->step();
}


void SpaceSet::erase()
{
    ObjectSet::erase();
    
    // simul has lost its current Space:
    setMaster(nullptr);
}

/**
 This will change the Simul current Space if it was not set
*/
void SpaceSet::add(Object * obj)
{
    assert_true(obj->tag() == Space::TAG);
    //std::clog << "SpaceSet::add " << obj << std::endl;
    ObjectSet::add(obj);
    
    if ( !master() || obj->identity() < master()->identity() )
        setMaster(static_cast<Space*>(obj));
}

/**
 If the Simulation current Space is deleted,
 the 'oldest' remaining Space is chosen to replace it.
 */
void SpaceSet::remove(Object * obj)
{
    //std::clog << "SpaceSet::remove " << obj << std::endl;
    ObjectSet::remove(obj);

    if ( obj == master() )
    {
        /*
         if the current space was deleted, use the oldest Space available
         */
        Space * spc = first();
        
        for ( Space * s=spc; s; s=s->next() )
            if ( s->identity() < spc->identity() )
                spc = s;
        
        setMaster(spc);
    }
}

//------------------------------------------------------------------------------

Object * SpaceSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Space::TAG )
    {
        SpaceProp * p = simul.findProperty<SpaceProp>("space", num);
        return p->newSpace();
    }
    return nullptr;
}

/**
 The dimensions of a Space can be specified when it is created
 
     new cell
     {
        length = 3, 4
     }
 
 */
ObjectList SpaceSet::newObjects(const std::string& name, Glossary& opt)
{
    SpaceProp * p = simul.findProperty<SpaceProp>("space", name);
    Space * obj = p->newSpace(opt);

    ObjectList res(2);
    if ( obj )
        res.push_back(obj);
        
    return res;
}


void SpaceSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}
