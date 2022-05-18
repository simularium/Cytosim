// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ORGANIZER_SET_H
#define ORGANIZER_SET_H

#include "object_set.h"

class Mecable;
class Organizer;
class Aster;
class Simul;

/// a list of Organizer (Aster, Nucleus, Bundle)
class OrganizerSet : public ObjectSet
{
public:
    
    ///creator
    OrganizerSet(Simul& s) : ObjectSet(s) {}
    
    //--------------------------
    
    /// identifies the class
    static std::string title() { return "organizer"; }
    
    /// create a new property of category `cat` for a class `name`
    Property *  newProperty(const std::string& cat, const std::string& name, Glossary&) const;
    
    /// create objects of class `name`, given the options provided in `opt`
    ObjectList  newObjects(const std::string& name, Glossary& opt);
    
    /// create a new object (used for reading trajectory file)
    Object *    newObject(ObjectTag, unsigned);
    
    /// write all Objects to file
    void        write(Outputter& out) const;
        
    /// print a summary of the content (nb of objects, class)
    void        report(std::ostream& out) const;

    //--------------------------

    /// register Organizer
    void        add(Object *);
    
    /// first Organizer
    Organizer * first() const;
    
    /// find object with given ID
    Organizer * findID(ObjectID n) const;

    /// find highest ObjectID among Organizers containing given Mecable
    ObjectID    findOrganizerID(Mecable const*) const;

    /// Monte-Carlo simulation step for every Object
    void        step();

};


#endif


