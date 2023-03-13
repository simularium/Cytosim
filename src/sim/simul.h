// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SIMUL_H
#define SIMUL_H

#include "assert_macro.h"
#include <iostream>
#include <string>
#include <stack>
#include <map>

#include "single_set.h"
#include "couple_set.h"
#include "fiber_set.h"
#include "fiber_grid.h"
#include "bead_set.h"
#include "solid_set.h"
#include "sphere_set.h"
#include "organizer_set.h"
#include "field_set.h"
#include "space_set.h"
#include "event_set.h"
#include "point_grid.h"
#include "property_list.h"
#include "field_values.h"
#include "meca.h"

class Meca1D;
class SimulProp;

/// default name for output trajectory file
const char TRAJECTORY[] = "objects.cmo";


/// Simulator class containing all Objects
class Simul
{
public:
    
    /// Meca used to set and integrate the equations of motion of Mecables
    mutable Meca      sMeca;
    
    /// grid used for attachment of Hand to Fiber
    mutable FiberGrid fiberGrid;
    
    /// grid used for steric interaction between Fiber/Solid/Bead/Sphere
    mutable PointGrid pointGrid;
    
    /// Meca used to solve the system with option 'solve=horizontal'
    Meca1D *          pMeca1D;

private:
    
    /// signals that Simul is ready to perform a Monte-Carlo step
    bool            sReady;

    /// preconditionning method used in last solve
    unsigned int    precondMethod;
    
    /// counter that is used to automatically set the preconditionning option
    unsigned int    precondCounter;
    
    /// stores cpu time to automatically set the preconditionning option
    double          precondCPU[4];
    
    /// a copy of the properties as they were stored to file
    mutable std::string properties_saved;

public:

    /// Global cytosim parameters
    SimulProp *     prop;
    
    /// list of all Object Property (does not include the SimulProp)
    PropertyList    properties;
    
    
    /// list of Space in the Simulation
    SpaceSet        spaces;
    
    /// list of Field in the Simulation
    FieldSet        fields;
    
    /// list of Fiber in the Simulation
    FiberSet        fibers;
    
    /// list of Sphere in the Simulation
    SphereSet       spheres;
    
    /// list of Bead in the Simulation
    BeadSet         beads;
    
    /// list of Solid in the Simulation
    SolidSet        solids;
    
    /// list of Single in the Simulation
    SingleSet       singles;
    
    /// list of Couple in the Simulation
    CoupleSet       couples;
    
    /// list of Organizer in the Simulation
    OrganizerSet    organizers;

    /// list of Events in the Simulation
    EventSet        events;
    
    //--------------------------------------------------------------------------
    
    /// constructor
    Simul();
    
    /// destructor
    virtual ~Simul();
        
    //----------------------------- POPULATING ---------------------------------
    
    /// add Object to Simulation
    void            add(Object *);

    /// add all Objects from given list
    void            add(ObjectList const&);

    /// remove Object from Simulation
    void            remove(Object *);

    /// remove all Objects from given list
    void            remove(ObjectList const&);
    
    /// remove and delete object
    void            erase(Object *);
    
    /// remove and delete all objects from given list
    void            erase(ObjectList const&);

    /// mark objects from given list
    static void     mark(ObjectList const&, ObjectMark);

    /// reset simulation world (clear all sub-lists and variables)
    void            erase();

    /// total number of objects in the Simulation
    size_t          nbObjects() const;

    //----------------------------- SIMULATING ---------------------------------
   
    /// perform basic initialization; register callbacks
    void            initialize(Glossary&);
    
    /// ready the engine for a subsequent call to `step()` and `solve()`
    void            prepare();
    
    /// perform one Monte-Carlo step, corresponding to `time_step`
    void            step();
    
    /// time in the simulated world (shortcut to `prop->time`)
    real            time()   const;
    
    /// shortcut to prop->time_step;
    real            time_step() const;

    /// this is called after a sequence of `step()` have been done
    void            relax();
    
    /// this is called after a sequence of `step()` have been done
    void            unrelax() { sReady = true; }

    /// true if engine is ready to go (between `prepare()` and `relax()`)
    bool            ready() const { return sReady; }

    
    /// call setInteractions(Meca) for all objects (this is called before `solve()`
    void            setAllInteractions(Meca&) const;

    /// display Meca's links
    void            drawLinks() const;
    
    /// bring all objects to centered image using periodic boundary conditions
    void            foldPositions() const;

    /// simulate the mechanics of the system and move Mecables accordingly, corresponding to `time_step`
    void            solve();
    
    /// like 'solve' but automatically select the fastest preconditionning method
    void            solve_auto();

    /// do nothing
    void            solve_not() {};
    
    /// calculate the motion of objects, but only in the X-direction
    void            solveX();
    
    /// calculate Forces and Lagrange multipliers on the Mecables, but do not move them
    void            computeForces() const;
    
    /// dump matrix and vector from Meca in a format that can be read in MATLAB
    void            dump(const char dirname[]) const;
    
    /// dump system matrix and vector in sparse text format
    void            saveSystem(const char dirname[]) const;

private:
    
    /// give an estimate of the cell size of the FiberGrid
    real            estimateFiberGridStep() const;
    
    /// set FiberGrid and StericGrid over the given space
    void            setFiberGrid(Space const*) const;
    
    /// give an estimate for the cell size of the PointGrid used for steric interactions
    real            estimateStericRange() const;
    
    /// initialize the grid for steric interaction (pointGrid)
    void            setStericGrid(Space const*) const;
    
    /// add steric interactions between spheres, solids and fibers to Meca
    void            setStericInteractions(Meca&) const;
    
    //----------------------------- PARSING ------------------------------------
    
    /// return the ObjectSet corresponding to this Tag in the simulation (used for IO)
    ObjectSet*      findSetT(const ObjectTag);

public:
    
    /// Parse a text containing cytosim commands
    void            evaluate(std::string const&);

    /// return the ObjectSet corresponding to a class
    ObjectSet*      findSet(const std::string& cat);
    
    /// convert Object to Mecable* if the conversion seems valid; returns 0 otherwise
    static Mecable* toMecable(Object *);

    /// find a Mecable from a string specifying name and inventory number (e.g. 'fiber1')
    Mecable*        findMecable(const std::string& spec) const;
    
    /// return first Space with given name
    Space const*    findSpace(std::string const& name) const;
    
    //---------------------------- PROPERTIES ----------------------------------

    /// change the name of the simulation
    void            rename(std::string const&);
    
    /// read an Object reference and return the corresponding Object (`tag` is set)
    Object*         readReference(Inputter&, ObjectTag& tag);

    /// check if `name` corresponds to a property class, but excluding 'simul'
    bool            isCategory(const std::string& name) const;
    
    /// return existing property of given class and name, or return zero
    Property*       findProperty(const std::string&, const std::string&) const;
    
    /// return existing property of given name, or return zero
    Property*       findProperty(const std::string&) const;

    /// return all existing properties of requested class
    PropertyList    findAllProperties(const std::string&) const;
    
    /// return Property in the requested type, or throw an exception
    template < typename T >
    T* findProperty(std::string const& cat, unsigned ix) const
    {
        Property * p = properties.find(cat, ix);
        if ( !p )
            throw InvalidIO("could not find `"+cat+"' class with id "+std::to_string(ix));
        return static_cast<T*>(p);
    }
    
    /// return Property in the requested type, or throw an exception
    template < typename T >
    T* findProperty(std::string const& cat, std::string const& nom) const
    {
        Property * p = properties.find(cat, nom);
        if ( !p )
            throw InvalidIO("could not find "+cat+" class `"+nom+"'");
        return static_cast<T*>(p);
    }

    /// create a new property
    Property* newProperty(const std::string&, const std::string&, Glossary&);
    
    /// export all Properties to speficied file
    void      writeProperties(std::ostream&, bool prune) const;
    
    /// export all Properties to a new file with specified name
    void      writeProperties(char const* filename, bool prune) const;
    
    /// load the properties contained in the standard output property file
    void      loadProperties();

    //---------------------------- LOAD OBJECTS --------------------------------
    
    /// current file format
    const static int currentFormatID = 52;
    
    /// class for reading trajectory file
    class     InputLock;
    
    /// read objects from file, and add them to the simulation state
    int       readObjects(Inputter&, ObjectSet* subset);

    /// load objects from a file, adding them to the simulation state
    int       loadObjects(Inputter&, ObjectSet* subset = nullptr);

    /// load sim-world from the named file
    int       loadObjects(char const* filename);
    
    /// import objects from file, and delete objects that were not referenced in the file
    int       reloadObjects(Inputter&, ObjectSet* subset = nullptr);

    /// write sim-world to specified file
    void      writeObjects(Outputter&) const;
    
    /// write sim-world in binary or text mode, appending to existing file or creating new file
    void      writeObjects(std::string const& filename, bool append, bool binary) const;
    
    //----------------------------- REPORTING ----------------------------------

    /// call `Simul::report0`, adding lines before and after with 'start' and 'end' tags.
    void      report(std::ostream&, std::string, Glossary&) const;
    
    /// call one of the report function
    void      report0(std::ostream&, std::string const&, Glossary&) const;

    /// print time
    void      reportTime(std::ostream&) const;
    
    /// give a short inventory of the simulation state, obtained from ObjectSet::report()
    void      reportInventory(std::ostream&) const;
    
    /// give a summary of the System
    void      reportSystem(std::ostream&) const;

    /// print the length and the points of each fiber

    /// print the length and the points of each fiber
    void      reportFibers(std::ostream&, Property const*) const;
    
    /// print the length and the points of each fiber
    void      reportFibers(std::ostream&, std::string const&) const;

    /// print the length and the points of each fiber
    void      reportFibers(std::ostream&) const;

    /// print the coordinates of the vertices of each fiber
    void      reportFiberPoints(std::ostream&) const;
    
    /// print the coordinates of the vertices of each fiber
    void      reportFiberDisplacement(std::ostream&) const;
    
    /// print the mean and standard deviation of vertices of all fibers
    void      reportFiberMoments(std::ostream&) const;

    /// print the positions and the states of the two ends of each fiber
    void      reportFiberEnds(std::ostream&) const;
    
    /// print average age and standard deviation for each class of fiber
    void      reportFiberAge(std::ostream&) const;

    /// print average length and standard deviation for each class of fiber
    void      reportFiberLengths(std::ostream&) const;
    
    /// print length distribution for each class of fiber
    void      reportFiberLengthDistribution(std::ostream&, Glossary&) const;

    /// print number of kinks in each class of Fiber
    void      reportFiberSegments(std::ostream&) const;
    
    /// print number of fibers according to dynamic state of end
    void      reportFiberDynamic(std::ostream&, FiberEnd) const;
    
    /// print number of fibers according to their dynamic states
    void      reportFiberDynamic(std::ostream&) const;
    
    /// print coordinates of speckles that follow a frozen random sampling
    void      reportFiberSpeckles(std::ostream&, Glossary&) const;
    
    /// print coordinates of points randomly and freshly distributed
    void      reportFiberSamples(std::ostream&, Glossary&) const;

    /// print dynamic states of Fiber
    void      reportFiberStates(std::ostream&) const;
    
    /// print the coordinates and forces on the vertices of each fiber
    void      reportFiberForces(std::ostream&) const;

    /// print Fiber tensions along certain planes defined in `opt`
    void      reportFiberTension(std::ostream&, Glossary&) const;
    
    /// print sum of all bending energy
    void      reportFiberBendingEnergy(std::ostream&) const;

    /// print bending energy along segments
    void      reportFiberSegmentEnergy(std::ostream&) const;  

    /// print component of forces experienced by Fibers due to confinement
    void      reportFiberConfineForce(std::ostream& out) const;

    /// print radial component of forces experienced by Fibers due to confinement
    real      reportFiberConfinement(std::ostream& out) const;

    /// print position of hands bound to fibers
    void      reportFiberHands(std::ostream&) const;
    
    /// print position of bound hands that are associated with stiffness
    void      reportFiberLinks(std::ostream&) const;

    /// print summary of Fiber's lattice quantities
    void      reportFiberLattice(std::ostream&, bool density) const;
    
    /// print network surface area
    void      reportNetworkSize(std::ostream&) const;

    /// print positions of interection between two fibers
    void      reportFiberIntersections(std::ostream&, Glossary&) const;


    /// print Organizer positions
    void      reportOrganizer(std::ostream&) const;

    /// print Aster positions
    void      reportAster(std::ostream&) const;
    
    /// print Bead positions 
    void      reportBeadSingles(std::ostream&) const;

    /// print Bead positions
    void      reportBeadPosition(std::ostream&) const;

    /// print Solid positions 
    void      reportSolidPosition(std::ostream&) const;

    /// print Solid's anchored Hands
    void      reportSolidHands(std::ostream&) const;

    /// print state of Couples 
    void      reportCouple(std::ostream&) const;
    
    /// print state of Couples
    void      reportCoupleAnatomy(std::ostream&) const;
    
    /// print position of Couples 
    void      reportCoupleState(std::ostream&) const;
    
    /// print position of Couples of a certain kind
    void      reportCoupleState(std::ostream&, std::string const&) const;
    
    /// print position of active Couples of a certain kind
    void      reportCoupleActive(std::ostream&, std::string const&) const;
    
    /// print position and forces of doubly-attached Couples
    void      reportCoupleLink(std::ostream&, std::string const&) const;
    
    /// print configurations of doubly-attached Couples
    void      reportCoupleConfiguration(std::ostream&, std::string const&, Glossary&) const;

    /// print position and forces of Couples of a certain kind
    void      reportCoupleForce(std::ostream&, Glossary&) const;
    
    /// print state of Singles
    void      reportSingle(std::ostream&) const;
    
    /// print position of Singles of a certain kind
    void      reportSingleState(std::ostream&, std::string const&) const;

    /// print position of Singles
    void      reportSinglePosition(std::ostream&, std::string const&) const;
   
    /// print position of Singles
    void      reportAttachedSingle(std::ostream&, std::string const&) const;

    /// print state of Couples 
    void      reportSpherePosition(std::ostream&) const;

    /// print something about Spaces
    void      reportSpace(std::ostream&) const;
  
    /// print force on Spaces
    void      reportSpaceForce(std::ostream&) const;

    /// print something about Fields
    void      reportField(std::ostream&) const;
    
    //------------------------------ CUSTOM ------------------------------------
    
    /// flag fibers according to connectivity defined by Couple of given type
    void      flagClustersCouples(Property const*) const;
    
    /// flag fibers according to connectivity defined by Couple
    void      flagClustersCouples() const;
    
    /// flag fibers according to connectivity defined by Solids
    void      flagClustersSolids() const;

    /// order clusters in decreasing number of fibers
    int       orderClusters(std::ostream&, size_t threshold, int details) const;
    
    /// print size of clusters defined by connections with Couples
    void      reportClusters(std::ostream&, Glossary&) const;
    
    /// analyse the network connectivity to identify isolated sub-networks
    void      flagClusters(bool order) const;

    /// flag the fibers that appear to constitute a ring
    size_t    flagRing() const;
    
    /// extract radius and length of ring
    void      analyzeRing(ObjectFlag, real& length, real& radius) const;
    
    /// estimates if Fibers form a connected ring around the Z-axis
    void      reportRing(std::ostream&) const;

    /// custom report for Platelet project
    void      reportPlatelet(std::ostream&) const;
    
    /// print Aster & Spindle indices
    void      reportIndices(std::ostream&) const;

    /// print number of Fibers pointing left and right that intersect plane YZ at different X positions
    void      reportProfile(std::ostream&) const;

    /// a special print for Romain Gibeaux
    void      reportAshbya(std::ostream&) const;
    
    /// print something
    void      reportCustom(std::ostream&) const;

    //-------------------------------------------------------------------------------
    
    /// custom function
    void      custom0(Glossary&);
    /// custom function
    void      custom1(Glossary&);
    /// custom function
    void      custom2(Glossary&);
    /// custom function
    void      custom3(Glossary&);
    /// custom function
    void      custom4(Glossary&);
    /// custom function
    void      custom5(Glossary&);
    /// custom function
    void      custom6(Glossary&);
    /// custom function
    void      custom7(Glossary&);
    /// custom function
    void      custom8(Glossary&);
    /// custom function
    void      custom9(Glossary&);
};

#endif

