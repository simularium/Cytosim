// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ASTER_H
#define ASTER_H

#include "object.h"
#include "organizer.h"
#include "aster_prop.h"
#include "solid.h"
#include "fiber.h"

/// A connection between a Fiber and a Solid
//@todo new Interpolation4() to replace coef1[] and coef2[]
class AsterLink
{
    friend class Aster;
    
private:
    
    /// type of link:
    /**
     0 = no link
     1 = link fiber-end with coef1, fiber-side with coef2
     2 = the interpolation corresponds exactly to point 'ref'
     */
    size_t   rank;

    /// index of first point on the Solid
    size_t   prime;
    
    /// interpolation coefficient for Fiber end
    real     coef1[4];
    
    /// interpolation coefficient for Fiber side
    real     coef2[4];
    
    /// distance between the two anchoring points
    real     len;
    
    /// index used for backward compatibility
    size_t   alt;
    
public:
    
    /// constructor
    AsterLink()
    {
        reset();
    }
    
    void reset()
    {
        rank = 0;
        prime = 0;
        len = 0;
        for ( int i = 0; i < 4; ++i )
        {
            coef1[i] = 0.0;
            coef2[i] = 0.0;
        }
        alt = 0;
    }
    
    void set(Vector const& A, Vector const& B)
    {
        len = ( A - B ).norm();
        
        coef1[1] = A.XX;
        coef2[1] = B.XX;
#if ( DIM == 1 )
        coef1[2] = 0.0;
        coef2[2] = 0.0;
        coef1[3] = 0.0;
        coef2[3] = 0.0;
        coef1[0] = 1.0 - A.XX;
        coef2[0] = 1.0 - B.XX;
#elif ( DIM == 2 )
        coef1[2] = A.YY;
        coef2[2] = B.YY;
        coef1[3] = 0.0;
        coef2[3] = 0.0;
        coef1[0] = 1.0 - A.XX - A.YY;
        coef2[0] = 1.0 - B.XX - B.YY;
#elif ( DIM == 3 )
        coef1[2] = A.YY;
        coef2[2] = B.YY;
        coef1[3] = A.ZZ;
        coef2[3] = B.ZZ;
        coef1[0] = 1.0 - A.XX - A.YY - A.ZZ;
        coef2[0] = 1.0 - B.XX - B.YY - B.ZZ;
#endif
        if ( A.norm_inf() < REAL_EPSILON )
            rank = 1;
        else
            rank = 1+DIM;
    }

    void write(Outputter& out) const
    {
        out.writeUInt16(prime);
        for ( int d = 1; d < 4; ++d )
            out.writeFloat(coef1[d]);
        for ( int d = 1; d < 4; ++d )
            out.writeFloat(coef2[d]);
    }
    
    void read(Inputter& in)
    {
        prime = in.readUInt16();
        
        for ( int d = 1; d < 4; ++d )
            coef1[d] = in.readFloat();
        coef1[0] = 1.0 - coef1[1] - coef1[2] - coef1[3];
        
        for ( int d = 1; d < 4; ++d )
            coef2[d] = in.readFloat();
        coef2[0] = 1.0 - coef2[1] - coef2[2] - coef2[3];
        
        len = (Vector3(coef1+1)-Vector3(coef2+1)).norm();
        
        if ( fabs(coef1[1]) + fabs(coef1[2]) + fabs(coef1[3]) < REAL_EPSILON )
            rank = 1;
        else
            rank = DIM;
    }
    
    void print(std::ostream& out) const
    {
        const unsigned w = 9;
        out << std::setw(w) << coef1[0];
        for ( int d = 1; d < 4; ++d )
            out << " " << std::setw(w) << coef1[d];
        out << "   " << std::setw(w) << coef2[0];
        for ( int d = 1; d < 4; ++d )
            out << " " << std::setw(w) << coef2[d];
        out << "\n";
    }
};


/// A radial configuration of Fiber(s) built around a Solid
/**
 The parameters are defined in AsterProp.
 
 Each Fiber is attached to the Solid:
 - at the end of the Fiber
 - at a secondary point that is tied to the Fiber at some distance from this end.
 .
 This anchors the Fiber to the Solid, both in position and direction.
 The stiffness of the links is defined in AsterProp::stiffness, and can be adjusted independently.
 .
 
 @ingroup OrganizerGroup
 */
class Aster : public Organizer
{
private:
    
    /// scale of local reference frame
    real       asRadius;
    
    /// store the coefficients needed to make the links between Solid and Fiber
    Array<AsterLink> asLinks;

    /// create and configure the Solid
    ObjectList makeSolid(Simul&, Glossary& opt, size_t& origin);

    /// create a Fiber for position 'inx'
    ObjectList makeFiber(Simul&, size_t inx, std::string const&, Glossary& opt);

    /// define the attachment position of fiber 'inx'
    void       placeAnchor(Vector const&, Vector const&, size_t origin);

    /// define the anchor points of Fibers
    void       placeAnchors(Glossary& opt, size_t origin, size_t nbf);
    
    /// Property
    AsterProp const* prop;
    
public:
    
    /// constructor
    Aster(AsterProp const* p) : prop(p) { asRadius = 0; }
    
    /// destructor
    virtual      ~Aster();
    
    /// construct all the dependent Objects of the Organizer
    ObjectList    build(Glossary&, Simul&);
    
    /// return the scaffolding Solid
    Solid *       solid() const { return static_cast<Solid*>(organized(0)); }
    
    /// return the center of the Solid
    Vector        position() const { return solid()->posP(0); }
    
    /// return number of fibers
    size_t        nbFibers() const { return nbOrganized() - 1; }

    /// return Fiber `n`
    Fiber *       fiber(size_t n) const { return Fiber::toFiber(organized(n+1)); }
    
    /// perform one Monte-Carlo step
    void          step();
    
    /// add interactions to a Meca
    void          setInteractions(Meca&) const;
    
    /// position of first clamp for Fiber n
    Vector        posLink1(size_t n) const;
    
    /// position of second clamp for Fiber n
    Vector        posLink2(size_t n) const;

    /// position of end on Fiber corresponding to first link
    Vector        posFiber1(size_t n) const { return fiber(n)->posEnd(prop->focus); }
    
    /// position of attachment point on Fiber corresponding to second link
    Vector        posFiber2(size_t n) const;
    
    /// retrieve link between Solid and end of Fiber number `i`, returning stiffness
    real          getLink1(size_t i, Vector&, Vector&) const;
    
    /// retrieve link between Solid and side of Fiber number `i`, returning stiffness
    real          getLink2(size_t i, Vector&, Vector&) const;
    
    /// retrieve link of type 1 if `i` is even, of type 2 if `i` is odd
    bool          getLink(size_t i, Vector&, Vector&) const;

    /// return PointDisp of Solid
    PointDisp const* disp() const { if ( solid() ) return solid()->prop->disp; return nullptr; }
    
    //--------------------------------------------------------------------------

    /// a unique character identifying the class
    static const ObjectTag TAG = 'a';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }

    /// return associated Property
    Property const* property() const { return prop; }

    /// convert pointer to Aster* if the conversion seems valid; returns 0 otherwise
    static Aster* toAster(Object * obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Aster*>(obj);
        return nullptr;
    }
    
    //--------------------------------------------------------------------------

    /// read from IO
    void          read(Inputter&, Simul&, ObjectTag);
    
    /// write to IO
    void          write(Outputter&) const;

};


#endif

