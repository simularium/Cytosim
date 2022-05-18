// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_SET_H
#define SPACE_SET_H

#include "object_set.h"
#include "space.h"
class Simul;

///a list of Space
class SpaceSet : public ObjectSet
{
    /// the master space
    static Space const* master_;

public:

    /// return master
    static Space const* master() { return master_; }

    /// change master
    void setMaster(Space const* s);

    /// constructor
    SpaceSet(Simul& s) : ObjectSet(s) {}
    
    //--------------------------
    
    /// identifies the property
    static std::string title() { return "space"; }
    
    /// create a new property of category `cat` for a class `name`
    Property *  newProperty(const std::string& cat, const std::string& name, Glossary&) const;
    
    /// create objects of class `name`, given the options provided in `opt`
    ObjectList  newObjects(const std::string& name, Glossary& opt);
    
    /// create a new object (used for reading trajectory file)
    Object *    newObject(ObjectTag, unsigned);
    
    /// write all Objects to file
    void        write(Outputter& out) const;
        
    /// print a summary of the content (nb of objects, class)
    void        report(std::ostream& out) const { writeAssets(out, title()); }

    //--------------------------
    
    /// add Object
    void add(Object *);
    
    /// remove Object
    void remove(Object *);

    /// erase all Object and all Property
    void erase();
    
    /// Monte-Carlo step for every Space
    void step();
    
    /// first Space
    Space * first() const
    {
        return static_cast<Space*>(nodes.front());
    }

    /// first Space with this Property
    Space * findObject(const Property * prop) const
    {
        return static_cast<Space*>(ObjectSet::findObject(prop));
    }
    
    /// last Space
    Space * last() const
    {
        return static_cast<Space*>(nodes.back());
    }

    /// return pointer to the Object of given ID, or zero if not found
    Space * findID(ObjectID n) const
    {
        return static_cast<Space*>(inventory.get(n));
    }

};


#endif

