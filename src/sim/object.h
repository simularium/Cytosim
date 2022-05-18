// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef OBJECT_H
#define OBJECT_H

#include "inventoried.h"
#include "movable.h"
#include "random.h"
#include "array.h"
#include "node.h"

class Simul;
class Property;
class Inputter;
class Outputter;
class ObjectSet;
class Display;

/// Type for unique class identifier used to read/write objects from file
typedef int ObjectTag;

/// Type used to mark objects
typedef unsigned long ObjectMark;

/// Type used to flag objects
typedef unsigned long ObjectFlag;

/// Type used for signature
typedef unsigned ObjectSignature;

/// Parent class for all simulated objects
/**
 This is the interface used for writing / reading from a file.
 
 Three functions identify an Object:
 - tag() [ASCII character] identifies the class of Object.
 - property->number() [integer] identifies its Property.
 - identity() [serial-number] derived from Inventoried identifies unique instantiations.
 .
 These three qualities are concatenated in reference() and writeReference().
 
 Objects are stored in ObjectSet.
 */
class Object : public Node, public Inventoried, public Movable
{
private:
    
    /// integer used for custom tasks, which is recorded to file
    ObjectMark        mark_;

    /// another integer used for various tasks, not saved to file
    ObjectFlag        flag_;
    
    /// a random number associated with this object
    unsigned     signature_;
    
    /// upstream pointer to container class
    ObjectSet *        set_;
    
public:
    
    /// Object::TAG = 'v' represents the 'void' pointer
    static const ObjectTag TAG = 'v';
    
    /// build a reference string by concatenating (tag, property_number, ObjectID)
    static std::string reference(ObjectTag, unsigned, ObjectID);
    
    /// write a reference, but using the provided Tag
    static void writeReference(Outputter&, ObjectTag, ObjectID);
    
    /// write a reference that does not refer to any Object
    static void writeNullReference(Outputter&);

    /// write a reference that identifies the Object uniquely
    static void writeReference(Outputter&, Object const*);
    
    /// write header to object data, using provided tag
    void        writeHeader(Outputter&, ObjectTag tag) const;
    
    /// read header
    static void readHeader(Inputter& in, bool fat, unsigned& ix, ObjectID& id, ObjectMark& mk);

public:
    
    /// constructor
    Object() : mark_(0), flag_(0), signature_(RNG.pint32()), set_(nullptr) { }
    
    /// copy constructor
    Object(Object const& o) : mark_(o.mark_), flag_(o.flag_), signature_(o.signature_), set_(nullptr) {}
    
    /// assignment operator
    Object& operator =(const Object& o) { mark_=o.mark_; flag_=o.flag_; signature_=o.signature_; set_=nullptr; return *this; }
    
    /// destructor
    ~Object();
    
    /// a character identifying the class of this object
    virtual ObjectTag tag() const { return TAG; }
    
    /// Property associated with the Object
    virtual Property const* property() const = 0;
    
    /// write Object data to file
    virtual void    write(Outputter&) const = 0;
    
    /// read Object from file, within the Simul
    virtual void    read(Inputter&, Simul&, ObjectTag) = 0;
    
    //--------------------------

    /// returns container ObjectSet
    ObjectSet *     objset() const { return set_; }
    
    /// returns container Simul
    Simul &         simul() const;
    
    /// change container class
    void            objset(ObjectSet* s) { set_ = s; }
    
    /// true if Node is registered in a container class
    bool            linked() const { return set_ != nullptr; }

    /// concatenation of [ tag(), property()->number(), identity() ] in plain ascii
    std::string     reference() const;
    
    //--------------------------

    /// get mark
    ObjectMark      mark()          const { return mark_; }
    
    /// set mark
    void            mark(ObjectMark m)    { mark_ = m; }
    
    
    /// retrieve flag value
    ObjectFlag      flag()         const  { return flag_; }
    
    /// set flag (this value is not stored in trajectory files)
    void            flag(ObjectFlag f)    { flag_ = f; }
    
    /// set flag to match identity()
    void            flag_to_identity()    { flag_ = identity(); }

    
    /// a random number that makes objects unique
    ObjectSignature signature()     const { return signature_; }
    
    /// set signature
    void     signature(ObjectSignature s) { signature_ = s; }

    //--------------------------

    /// extends Node::next(), with a cast to preserve type
    Object *        next()          const { return static_cast<Object*>(nNext); }
    
    /// extends Node::prev(), with a cast to preserve type
    Object *        prev()          const { return static_cast<Object*>(nPrev); }
};


/// return always 'true'
bool match_all(Object const*, void const*);

/// return 'true' if ( obj->mark() == *mark )
bool match_mark(Object const* obj, void const* mark);

/// return 'true' if ( obj->property() == prop )
bool match_property(Object const* obj, void const* prop);


/// a list of pointers to Object
typedef Array<Object *> ObjectList;
//typedef std::vector<Object *> ObjectList;


/// output operator
std::ostream& operator << (std::ostream& os, ObjectList const&);


#endif
