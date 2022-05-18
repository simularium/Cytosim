// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INVENTORIED_H
#define INVENTORIED_H


/// type for the serial number of an Object
typedef unsigned int ObjectID;


/// Class that can be registered in the Inventory
/**
 Inventoried provides a serial-number of type ObjectID, used to identify objects in the simulation.
 A serial-number is strictly positive, and it is given only once in each class.
 
 Inventoried [and any derived class] can be registered in a Inventory.
 The Inventory keeps track of all assigned serial-numbers, 
 and can be used to retrieve back an object given its serial-number.
*/
class Inventoried
{
protected:
    
    /// object identifier, unique within each class
    ObjectID   ID_;
    
public:
    
    /// set identity to 0
    Inventoried() : ID_(0) {}
    
    /// passive destructor
    ~Inventoried() {}
    
    
    /// change the identity of this object
    void      identity(ObjectID n) { ID_ = n; }
    
    /// returns identity (strictly positive integer, unique within each class)
    ObjectID  identity()     const { return ID_; }
    
    /// returns modifiable identity
    ObjectID& identity()           { return ID_; }

};


#endif
