// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_SET_H
#define FIBER_SET_H

#include "dim.h"
#include "object_set.h"
#include "fiber.h"

class FiberProp;
class CoupleProp;
class FiberSite;


/// a list of Fiber
/**
 The FiberSet stores Fiber, and derived classes.
 It constains many algorithms that specifically deal with Fibers,
 including many function to calculate average length, nematic organization, etc.
 */
class FiberSet : public ObjectSet
{
private:
    
    FiberSet();

public:
    
    /// creator
    FiberSet(Simul& s) : ObjectSet(s) {}
    
    //--------------------------
    
    /// identifies the class
    static std::string title() { return "fiber"; }
    
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

    /// first Fiber
    Fiber * first()            const { return static_cast<Fiber*>(nodes.front()); }
    
    /// last Fiber
    Fiber * last()             const { return static_cast<Fiber*>(nodes.back()); }
    
    /// first Fiber in inventory
    Fiber * firstID()          const { return static_cast<Fiber*>(inventory.first()); }

    /// next Fiber in inventory
    Fiber * nextID(Fiber const* obj) const { return static_cast<Fiber*>(inventory.next(obj)); }

    /// return pointer to the Object of given ID, or zero if not found
    Fiber * findID(ObjectID n) const { return static_cast<Fiber*>(inventory.get(n)); }

    /// Cut all segments intersecting the plane defined by <em> n.pos + a = 0 </em>
    void planarCut(Vector const& n, real a, state_t stateP, state_t stateM);

    /// Cut fibers in the list
    void planarCut(ObjectList&, Vector const& n, real a, state_t stateP, state_t stateM);
    
    /// Monte-Carlo step for every Fiber
    void step();
    
    /// bring all objects to centered image using periodic boundary conditions
    void foldPositions(Modulo const*) const;
    
    /// find intersections between fibers in entire network, within given threshold
    void allIntersections(Array<FiberSite>&, Array<FiberSite>&, real max_distance) const;

    /// set random sites along the fibers, separated on average by `spread`
    void uniFiberSites(Array<FiberSite>&, real spread) const;
    
    /// a random site on the fibers, uniformly distributed over all Fibers
    FiberSite randomSite() const;
    
    /// a random site on the fibers of specified class, uniformly distributed
    FiberSite randomSite(FiberProp *) const;

    /// a site on a fiber, as specified by Glossary[key]
    FiberSite someSite(std::string const& key, Glossary&) const;

    /// set random sites on newly polymerized Fiber sites at the PLUS_END
    void newFiberSitesP(Array<FiberSite>&, real spread) const;
    
    /// set random sites on newly polymerized Fiber sites at the MINUS_END
    void newFiberSitesM(Array<FiberSite>&, real spread) const;
    
    /// reverse the polarity of all fibers
    void flipFiberPolarity();
    
    /// delete marked object after import
    void prune(ObjectFlag);

    //--------------------------------------------------------------------------
    
    /// total length of Fiber 
    real         totalLength() const;

    /// total length of Fiber for Fibers with given FiberProp
    real         totalLength(FiberProp const *) const;
    
    /// calculate: number of fibers, mean, standard-deviation, min and max of fiber length
    static void  infoLength(ObjectList const&, unsigned& cnt, real& avg, real& dev, real& mn, real& mx);
    
    /// calculate: number of fibers, mean, standard-deviation, min and max of fiber length
    static void  infoBirthtime(ObjectList const&, unsigned& cnt, real& avg, real& dev, real& mn, real& mx);

    /// calculate: number of fibers, number of joints and number of kinks
    static void  infoSegments(ObjectList const&, unsigned& cnt, unsigned& joints, real&, real&);
    
    /// calculate: number of fibers, number of joints and number of kinks
    static unsigned nbKinks(ObjectList const&);

    /// calculate center of gravity G, average of MINUS_END and PLUS_END
    static real  infoPosition(ObjectList const& objs, Vector& M, Vector& G, Vector& P);

    /// calculate the nematic directors, return the nematic scalar order parameter
    static real  infoNematic(ObjectList const&, real vec[9]);

    /// calculate center of gravity G, and principal components axes
    static int   infoComponents(ObjectList const&, real& sum, real avg[3], real mom[9], real vec[9]);

    /// Count Fibers intersecting the plane defined by <em> n.pos + a = 0 </em>
    void         infoPlane(int& np, int& na, Vector const& n, real a) const;
    
    /// Calculate characteristics of bendingEnergy()
    static void  infoBendingEnergy(ObjectList const&, unsigned& cnt, real& avg, real& dev);
    
    /// sum Lagrange multipliers for segments that intersect the plane <em> n.pos + a = 0 </em>
    void  infoTension(unsigned&, real& hten, Vector const& n, real a) const;
    
    /// sum Lagrange multipliers for all fibers
    void  infoTension(unsigned&, real& hten) const;

    /// Calculate spindle indices
    void  infoSpindle(real& ixa, real& ixs, Vector const& n, real a, real m, real da) const;

    /// Calculate averaged distance from origin - for all vertices
    void  infoRadius(unsigned&, real& rad) const;
    
    /// Calculate averaged distance from origin - for fiber ends
    void  infoRadius(unsigned&, real& rad, FiberEnd) const;

};


#endif

