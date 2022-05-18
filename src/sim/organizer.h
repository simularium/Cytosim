// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ORGANIZER_H
#define ORGANIZER_H

#include "assert_macro.h"
#include "glossary.h"
#include "object.h"
#include "buddy.h"

class Meca;
class Simul;
class Mecable;
class PointDisp;
class Display;

/// an assemblage of Mecable
/** 
Organizer contains an Array of pointers of type Mecable*.
These Mecables are organized by Organizer::setInteraction()
which is implemented in the derived classes, eg. Bundle, Aster & Nucleus.
*/
class Organizer: public Object, public Buddy
{

private:

    typedef std::vector<Mecable*> MecableList;
    
    /// list of Objects that are `organized`
    MecableList   mObjects;
    
public:

    /// default constructor
    Organizer() { }
    
    /// destructor
    virtual ~Organizer();
    
    /// create all the Objects of the Organizer, and return in list
    virtual ObjectList    build(Glossary&, Simul&) = 0;

    //--------------------------------------------------------------------------

    /// number of objects currently organized
    size_t                nbOrganized() const  { return mObjects.size(); }
    
    /// set number of objects
    void                  nbOrganized(size_t n) { mObjects.resize(n, nullptr); }
    
    /// return Mecable at index `n`
    Mecable *             organized(size_t n) const { assert_true(n<mObjects.size()); return mObjects[n]; }
    
    /// add Mecable at end of list
    void                  grasp(Mecable *);

    /// add Mecable at index `n`
    void                  grasp(Mecable *, size_t);

    /// handles the disapearance of one of the organized object
    void                  goodbye(Buddy *);
    
    /// add objects to Simul if they are not already linked
    virtual void          addOrganized(Simul&);
    
    /// delete all objects
    virtual void          eraseOrganized();
    
    /// move all associated objects
    void                  moveOrganized(Isometry const&);

    //--------------------------------------------------------------------------

    /// Organizer cannot be moved
    int                   mobile() const { return 0; }
    
    /// return the center of gravity
    virtual Vector        position() const;

    /// return the average of all vertices
    virtual Vector        positionP(unsigned) const;
/*
    /// move all associated objects
    void                  translate(Vector const& T);
    
    /// rotate all associated objects
    void                  rotate(Rotation const& T);
*/
    /// monte-carlo simulation step
    virtual void          step() {}
    
    /// add interactions to a Meca
    virtual void          setInteractions(Meca &) const {}
    
    /// sum the drag coefficient of all objects
    real                  dragCoefficient() const;
    
    
    /// retrieve end positions of link number `inx`, or returns zero if this link does not exist
    virtual bool          getLink(size_t inx, Vector&, Vector&) const { return false; }
    
    /// display parameters 
    virtual PointDisp const* disp() const { return nullptr; }
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    Organizer *   next()  const  { return static_cast<Organizer*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Organizer *   prev()  const  { return static_cast<Organizer*>(nPrev); }
    
    //--------------------------------------------------------------------------

    /// read
    void          read(Inputter&, Simul&, ObjectTag);
    
    /// write
    void          write(Outputter&) const;
};


#endif
