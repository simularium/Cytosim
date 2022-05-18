// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIELD_SET_H
#define FIELD_SET_H

#include "object_set.h"

class Simul;
class Field;

/// a list of Field
/**
 FieldSet contains all the 'Field' in the simulation

 */
class FieldSet : public ObjectSet
{
public:
    
    /// creator
    FieldSet(Simul& s) : ObjectSet(s) {}
    
    //--------------------------
    
    /// identifies the class
    static std::string title() { return "field"; }
    
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
    
    /// first object
    Field *     first() const;
    
    /// find object
    Field *     findObject(Property const* p) const;
    
    /// return pointer to the Object of given ID, or zero if not found
    Field *     findID(ObjectID n) const;
    
    /// get ready to do a step()
    void        prepare();
    
    /// Monte-Carlo simulation step for every Object
    void        step();

};


#endif
