// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SOLID_SET_H
#define SOLID_SET_H

#include "object_set.h"
#include "solid.h"

class Simul;

/// a list of Solid
class SolidSet : public ObjectSet
{
public:
    
    /// creator
    SolidSet(Simul& s) : ObjectSet(s) {}
    
    //--------------------------
    
    /// identifies the class
    static std::string title() { return "solid"; }
    
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
    
    /// register a Solid into the list
    void        add(Object *);
    
    /// remove from the list
    void        remove(Object *);
    
    /// first Solid
    Solid *     first() const
    {
        return static_cast<Solid*>(nodes.front());
    }
    
    /// last Solid
    Solid *     last() const
    {
        return static_cast<Solid*>(nodes.back());
    }
    
    /// first Solid in inventory
    Solid *     firstID() const
    {
        return static_cast<Solid*>(inventory.first());
    }

    /// next Solid in inventory
    Solid *     nextID(Solid const* obj) const
    {
        return static_cast<Solid*>(inventory.next(obj));
    }

    /// return pointer to the Object of given ID, or zero if not found
    Solid *     findID(ObjectID n) const
    {
        return static_cast<Solid*>(inventory.get(n));
    }
    
    /// bring all objects to centered image using periodic boundary conditions
    void        foldPositions(Modulo const*) const;
    
    /// Monte-Carlo simulation step for every Object
    void        step() {}
};


#endif

