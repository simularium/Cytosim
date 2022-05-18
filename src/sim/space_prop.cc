// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_prop.h"
#include "filepath.h"
#include "glossary.h"
#include "property_list.h"
#include "simul_prop.h"
#include "simul.h"
#include "sim.h"

#include "space.h"
#include "space_square.h"
#include "space_sphere.h"
#include "space_polygon.h"
#include "space_polygonZ.h"
#include "space_capsule.h"
#include "space_banana.h"
#include "space_torus.h"
#include "space_dice.h"
#include "space_strip.h"
#include "space_periodic.h"
#include "space_ellipse.h"
#include "space_cylinder.h"
#include "space_cylinderZ.h"
#include "space_cylinderP.h"
#include "space_ring.h"

/**
 @defgroup SpaceGroup Space and Geometry
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Space defines a confined region
 
 A Space is created by specifying shape and dimensions:
 
     set space NAME
     {
        shape = SHAPE
     }
 
     new NAME
     {
        PARAMETER = DIMENSIONS
     }
 
 PARAMETER is usually 'length' or 'radius', but also 'height' or 'width'
 DIMENSIONS is a single REAL or a comma-separated list of REAL.
 
 List of known `shape`:
 
 SHAPE         | Class                | PARAMETER
 --------------|----------------------|-------------------------------------
 `rectangle`   | SpaceSquare          | sizeX sizeY sizeZ
 `sphere`      | SpaceSphere          | radius
 `polygon`     | SpacePolygon         | file_name height
 `polygonZ`    | SpacePolygonZ        | file_name
 `capsule`     | SpaceCapsule         | length radius
 `torus`       | SpaceTorus           | radius width
 `banana`      | SpaceBanana          | length width radius (for curvature)
 `dice`        | SpaceDice            | sizeX sizeY sizeZ radius
 `strip`       | SpaceStrip           | sizeX sizeY sizeZ
 `periodic`    | SpacePeriodic        | sizeX sizeY sizeZ
 `ellipse`     | SpaceEllipse         | sizeX sizeY sizeZ
 `cylinder`    | SpaceCylinder        | length radius
 `cylinderZ`   | SpaceCylinderZ       | radius bottom top
 `cylinderP`   | SpaceCylinderP       | length radius
 `ring`        | SpaceRing            | length radius
 
 Example:
 
     set space cell
     {
         shape = sphere
     }
     new cell
     {
         radius = 5
     }
 
 */
Space * SpaceProp::newSpace() const
{
    const std::string& s = SpaceProp::shape;
    
    if ( s=="rectangle" || s=="square" )       return new SpaceSquare(this);
    if ( s=="circle" || s=="sphere" )          return new SpaceSphere(this);
    if ( s=="polygon" )                        return new SpacePolygon(this);
    if ( s=="polygonZ" )                       return new SpacePolygonZ(this);
    if ( s=="capsule" || s=="spherocylinder" ) return new SpaceCapsule(this);
    if ( s=="banana" )                         return new SpaceBanana(this);
    if ( s=="torus" )                          return new SpaceTorus(this);
    if ( s=="dice" )                           return new SpaceDice(this);
    if ( s=="strip" || s=="semi_periodic" )    return new SpaceStrip(this);
    if ( s=="periodic" )                       return new SpacePeriodic(this);
    if ( s=="ellipse" || s=="ellipsoid" )      return new SpaceEllipse(this);
#if ( DIM >= 3 )
    if ( s=="cubic" )                          return new SpaceSquare(this);
    if ( s=="cylinder" )                       return new SpaceCylinder(this);
    if ( s=="cylinderZ" )                      return new SpaceCylinderZ(this);
    if ( s=="cylinderP" )                      return new SpaceCylinderP(this);
#elif ( DIM == 2 )
    if ( s=="cylinder" )                       return new SpaceSquare(this);
    if ( s=="cylinderP" )                      return new SpaceStrip(this);
#else
    if ( s=="cylinder" )                       return new SpaceSquare(this);
    if ( s=="cylinderP" )                      return new SpacePeriodic(this);
#endif
    if ( s=="ring" )                           return new SpaceRing(this);
    
#if ( 0 )
    std::cerr << "Warning: substituting unbounded Space for unknown `"+s+"'\n";
    return new Space(this);
#endif
    throw InvalidParameter("unknown space:shape `"+s+"'");
    return nullptr;
}


Space * SpaceProp::newSpace(Glossary& opt) const
{
    Space * spc = newSpace();
    
    if ( spc )
    {
#ifdef BACKWARD_COMPATIBILITY
        std::string str = dimensions_;
        if ( str.size() || opt.set(str, "dimensions") )
        {
            std::stringstream iss(str);
            real len[8] = { 0 };
            int d = 0;
            while ( d < 8 )
            {
                real x = 0;
                iss >> x;
                if ( iss.fail() )
                    break;
                len[d++] = x;
            }
            if ( d > 0 )
            {
                spc->setLengths(len);
                return spc;
            }
        }
#endif
        // normal way to set the size:
        spc->resize(opt);
    }
    return spc;
}


//------------------------------------------------------------------------------

void SpaceProp::clear()
{
    shape         = "";
    display       = "";
    display_fresh = false;
}


void SpaceProp::read(Glossary& glos)
{    
    if ( glos.set(shape, "shape") )
    {
#ifdef BACKWARD_COMPATIBILITY
        glos.set(dimensions_, "dimensions");
    }
    else
    {
        std::string str;
        if ( glos.set(str, "geometry") )
        {
            std::stringstream iss(str);
            iss >> shape >> std::ws;
            std::getline(iss, dimensions_);
            if ( dimensions_.empty() )
                throw InvalidParameter("space:geometry should contains dimensions");
        }
#endif
    }
    
    if ( glos.set(display, "display") )
        display_fresh = true;
}

//------------------------------------------------------------------------------

void SpaceProp::complete(Simul const& sim)
{
    if ( shape.empty() )
        throw InvalidParameter("space:shape must be defined");
}

//------------------------------------------------------------------------------

void SpaceProp::write_values(std::ostream& os) const
{
    //write_value(os, "geometry",   geometry);
    write_value(os, "shape",      shape);
    write_value(os, "display",    "("+display+")");
}

