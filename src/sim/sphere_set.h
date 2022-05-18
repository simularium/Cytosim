// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPHERE_SET_H
#define SPHERE_SET_H

#include "object_set.h"
#include "sphere.h"
class Simul;

///a list of Sphere
class SphereSet : public ObjectSet
{
public:
    
    ///creator
    SphereSet(Simul& s) : ObjectSet(s) {}
    
    //--------------------------
    
    /// identifies the class
    static std::string title() { return "sphere"; }
    
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
   
    /// remove object
    void        remove(Object *);

    /// first Object
    Sphere *    first() const
    {
        return static_cast<Sphere*>(nodes.front());
    }
    
    /// first Sphere in inventory
    Sphere * firstID() const
    {
        return static_cast<Sphere*>(inventory.first());
    }

    /// return pointer to the Object of given ID, or zero if not found
    Sphere *    findID(ObjectID n) const
    {
        return static_cast<Sphere*>(inventory.get(n));
    }
    
    /// bring all objects to centered image using periodic boundary conditions
    void        foldPositions(Modulo const*) const;
    
    /// Monte-Carlo simulation step for every Object
    void        step() {}
 };

#endif
