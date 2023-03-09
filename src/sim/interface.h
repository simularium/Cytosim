// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INTERFACE_H
#define INTERFACE_H

#include <iostream>
#include "isometry.h"
#include "object.h"

class Glossary;
class Property;
class Simul;

/// Cytosim Application Programming Interface
/*
 A reduced set of commands to control and simulate
 a system of objects within cytosim.
 */
class Interface
{
private:
    
    /// disabled default constructor
    Interface();

protected:
    
    /// associated Simul
    Simul& simul;
    
public:
    
    /// construct and associates with given Simul
    Interface(Simul&);
    
    //-------------------------------------------------------------------------------
    
    /// this is called between commands during the execution process
    /**
     It provides an opportunity to stop or to display the simulation world
     */
    virtual void hold() {}
    
    //-------------------------------------------------------------------------------
    
    /// create a new Property of category `cat` from values set in Glossary
    Property*  execute_set(std::string const& cat, std::string const& name, Glossary&);

    /// change values in Property as specified in Glossary
    void       execute_change(Property*, Glossary&);

    /// change values in Property called `name` as specified in Glossary
    Property*  execute_change(std::string const& name, Glossary&, bool strict=true);
    
    /// change values of all Property of category `cat`
    void       execute_change_all(std::string const& cat, Glossary&);

    /// read the specification of position and orientation of an object
    Isometry   read_placement(Glossary&);
    
    /// return position and orientation of an object, with verification of 'placement'
    Isometry   find_placement(Glossary&, int);
    
    /// create 1 object of type `name`, following options in Glossary
    ObjectList execute_new(std::string const& name, Glossary&);
    
    /// create `cnt` objects of type `name`, randomly placed in space (no option)
    void       execute_new(std::string const& name, unsigned cnt);
    
    /// delete `cnt` objects of type `name`, following options in Glossary
    void       execute_delete(std::string const& name, Glossary&, unsigned cnt);
    
    /// move object of type `name`, following options in Glossary
    void       execute_move(std::string const& name, Glossary&, size_t cnt);

    /// mark `cnt`  objects of type `name`, following options in Glossary
    void       execute_mark(std::string const& name, Glossary&, unsigned cnt);

    /// cut fibers of type `name`, following different options in Glossary
    void       execute_cut(std::string const& name, Glossary&);
    
    /// import objects (or `what`) from the file with specified name
    void       execute_import(std::string const& file, std::string const& what, Glossary&);

    /// export objects (or `what`) to a file with specified name
    void       execute_export(std::string& file, std::string const& what, Glossary&);

    /// write information (specified in `what`) to a file with specified name
    void       execute_report(std::string& file, std::string const& what, Glossary&);
    
    /// perform `cnt` simulation steps
    void       execute_run(unsigned cnt, Glossary&, bool write_permission);
    
    /// perform `cnt` simulation steps, with no option
    void       execute_run(unsigned cnt);

    /// execute miscellaneous functions
    void       execute_call(std::string& func, Glossary&);

};

#endif

