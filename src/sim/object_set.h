// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef OBJECT_SET_H
#define OBJECT_SET_H

#include "node.h"
#include "object.h"
#include "node_list.h"
#include "inventory.h"
#include <vector>

class Outputter;
class Property;
class PropertyList;
class Glossary;
class Simul;

/// A set of Object
/**
 Encapsulates the different functions used to manage Objects.
 Pointers to the Objects are stored in two lists:
 - a doubly linked list: nodes
 - an array: inventory
 .
 The NodeList nodes is mixed at every time step,
 and thus it can be used to access the objects in a random order,
 as necessary for Monte-Carlo. 
 
 The Inventory can be used to access objects directly.
 
 Functions are used to manage:
 - object creation: newProperty(), newObjects().
 - object lists: size(), add(), remove(), erase().
 - object access: first(), find().
 - simulation: step(), shuffle().
 - I/O: loadObject(), read(), write(), freeze(), thaw().
 .
 */
class ObjectSet
{
private:
    
    ObjectSet();

public:

    /// holds pointers to the Objects organized by ObjectID
    Inventory         inventory;
    
    /// holds pointers to the Objects in a doubly linked list
    NodeList          nodes;
    
    /// the Simul containing this set
    Simul&            simul;
    
protected:
    
    /// mark all objects from given list with value `f`
    static void       flag(NodeList const&, ObjectFlag f);
    
    /// delete objects which are marked as `f` from given list, and mark objects with `s`
    static void       prune(NodeList const&, ObjectFlag f, ObjectFlag g);
    
    /// collect objects from NodeList for which func(obj, val) == true
    static unsigned   count(NodeList const&, bool (*func)(Object const*, void const*), void const*);

    /// collect all objects
    static ObjectList collect(NodeList const&);

    /// collect objects from NodeList for which func(obj, val) == true
    static ObjectList collect(NodeList const&, bool (*func)(Object const*, void const*), void const*);

    /// write Object in NodeList to file
    static void       writeNodes(Outputter&, NodeList const&);
    
    /// print a list of the content (nb of objects, class)
    void              writeAssets(std::ostream&, const std::string& title) const;

public:
    
    /// mark objects before import
    virtual void      freeze(ObjectFlag f) { flag(nodes, f); }
    
    /// delete marked objects
    virtual void      prune(ObjectFlag f)  { prune(nodes, f, 0); }
    
    /// unmark objects after import
    virtual void      thaw()               { flag(nodes, 0); }
    
    /// apply translation to all Objects in ObjectList
    static void       translateObjects(ObjectList const&, Vector const&);
    
    /// apply rotation to all Objects in ObjectList
    static void       rotateObjects(ObjectList const&, Rotation const&);
    
    /// apply Isometry to all Objects in ObjectList
    static void       moveObjects(ObjectList const&, Isometry const&);

    /// flag all Objects in ObjectList
    static void       flagObjects(ObjectList const&, ObjectFlag f);

    /// apply translation to unflagged Objects in list
    static void       translateObjects(ObjectList const&, Vector const&, ObjectFlag f);
    
    /// apply rotation to unflagged Objects in list
    static void       rotateObjects(ObjectList const&, Rotation const&, ObjectFlag f);

    /// apply Isometry to unflagged Objects in list
    static void       moveObjects(ObjectList const&, Isometry const&, ObjectFlag f);

public:
    
    /// creator
    ObjectSet(Simul& s) : simul(s) { }
    
    /// destructor
    virtual ~ObjectSet() { erase(); }    
    
    //--------------------------

    /// create a new property of category `cat` for a class `name`
    virtual Property * newProperty(const std::string& cat, const std::string& name, Glossary&) const = 0;
    
    /// create objects of class `name`, given the options provided in `opt`
    virtual ObjectList newObjects(const std::string& name, Glossary& opt) = 0;
   
    /// create new Object with given Tag and Property `num` (used for reading trajectory file)
    virtual Object *   newObject(ObjectTag, unsigned num) = 0;

    //--------------------------
    
    /// register Object, and add it at the end of the list
    virtual void       add(Object *);
    
    /// add multiple Objects
    void               add(ObjectList const&);
    
    /// remove Object
    virtual void       remove(Object *);

    /// remove all Objects in list
    void               remove(ObjectList const&);
    
    /// link the object last in the list
    virtual void       link(Object *);
    
    /// link the object last in the list
    virtual void       unlink(Object *);

    /// remove Object, and delete it
    void               erase(Object *);
    
    /// delete  Objects specified in given list
    void               erase(NodeList&);

    /// delete all Objects in list and forget all serial numbers
    virtual void       erase();
    
    /// number of elements
    virtual size_t     size()             const { return nodes.size(); }

    /// mix the order of elements in the doubly linked list nodes
    virtual void       shuffle()                { nodes.shuffle(); }
    
    /// first Object in the list
    Object *           first()            const { return static_cast<Object*>(nodes.front()); }
    
    /// last Object
    Object *           last()             const { return static_cast<Object*>(nodes.back()); }
    
    /// find Object of given serial-number (see Inventory)
    Object *           findID(ObjectID n) const { return static_cast<Object*>(inventory.get(n)); }
    
    /// return an Object which has this property
    Object *           findObject(Property const*) const;

    /// return Object corresponding to specifications
    Object *           findObject(std::string spec, long identity, const std::string&) const;
    
    /// return Object corresponding to a certain criteria (eg. 'first' or 'last')
    Object *           findObject(std::string spec, const std::string&) const;
    
    //--------------------------
    
    /// number of objects for which ( func(obj, val) == true )
    virtual unsigned   count(bool (*func)(Object const*, void const*), void const*) const;

    /// collect all objects
    virtual ObjectList collect() const;
 
    /// collect objects for which ( func(obj, val) == true )
    virtual ObjectList collect(bool (*func)(Object const*, void const*), void const*) const;

    /// collect objects that have given Property
    ObjectList         collect(Property const*) const;
    
    /// load one Object from file, or skip it if `skip==true`
    void               loadObject(Inputter&, ObjectTag tag, bool fat, bool skip, bool update);
    
    /// write all Objects to file
    virtual void       write(Outputter&) const = 0;
    
    /// print a summary of the content (nb of objects, class)
    virtual void       report(std::ostream&) const = 0;

};

#endif
