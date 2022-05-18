// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INVENTORY_H
#define INVENTORY_H

#include "inventoried.h"
#include "assert_macro.h"
#include <ostream>

/// Attributes and remember serial-numbers to Inventoried
/**
A Inventory assigns serial-numbers (of type ObjectID) to Inventoried, 
and it records a pointer to these objects.
 
The pointers can be recovered from their 'number' in constant time.

\author Nedelec, August 2003. EMBL Heidelberg. nedelec@embl.de
*/
class Inventory
{
private:
    
    /// array of objects, stored at the index of their ID
    /**
     This stores pointers to the objects, such that
     byNames[i]->identity() == i
     for any i > 0. 
     */
    Inventoried ** byNames;
    
    
    /// size of memory allocated
    size_t    allocated_;
    
    /// lowest i > 0 for which byNames[i] == 0
    ObjectID  lowest_;
    
    /// highest i > 0 for which byNames[i] != 0
    ObjectID  highest_;
    
    /// memory allocation function
    void      allocate(size_t size);
    
    ///update available when the spot has been taken
    void      updateFirstFree(ObjectID start = 0);
    
    /// Disabled copy constructor
    Inventory(Inventory const&);
    
    /// Disabled copy assignment
    Inventory& operator=(Inventory const&);
    
public:
        
    /// Constructor
    Inventory();
    
    /// Destructor
    ~Inventory();
    
    /// the smallest assigned number
    ObjectID       first_assigned() const;

    /// the largest assigned number
    ObjectID       last_assigned() const;
    
    /// lowest assigned number strictly greater than `n`
    ObjectID       next_assigned(ObjectID n) const;
    
    /// the smallest unassigned number
    ObjectID       first_unassigned();

    /// current size of array
    size_t         capacity() const { return allocated_; }
    
    /// remember `obj`, assign a new ObjectID if necessary
    void           assign(Inventoried * obj);
    
    /// forget the object and release its serial number
    void           unassign(const Inventoried * obj);
    
    /// return the object with given serial number, or 0 if not found
    Inventoried*   get(ObjectID number) const;
    
    /// object with the smallest inventory number
    Inventoried*   first() const;
    
    /// object with the largest inventory number
    Inventoried*   last() const;
    
    /// return object just before in the inventory
    Inventoried*   previous(Inventoried const*) const;
    
    /// return object just after in the inventory
    Inventoried*   next(Inventoried const*) const;

    /// return object with given number
    Inventoried*   operator[](ObjectID n) const { assert_true(n<allocated_); return byNames[n]; }

    /// number of non-zero entries in the registry
    unsigned int   count() const;

    /// reattribute all serial numbers consecutively and pack the array
    void           reassign();
    
    /// clear all entries
    void           clear();
    
    /// Human friendly ouput
    void           print(std::ostream&) const;
};


/// output of all values
std::ostream& operator << (std::ostream&, Inventory const&);


#endif
