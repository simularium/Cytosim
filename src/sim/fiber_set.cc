// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_set.h"
#include "fiber_segment.h"
#include "tokenizer.h"
#include "iowrapper.h"
#include "messages.h"
#include "glossary.h"
#include "fiber_prop.h"
#include "growing_fiber_prop.h"
#include "dynamic_fiber_prop.h"
#include "classic_fiber_prop.h"
#include "treadmilling_fiber_prop.h"
#include "clapack.h"
#include "simul.h"
#include "sim.h"

//#include "vecprint.h"

//------------------------------------------------------------------------------

/**
 @defgroup FiberGroup Fiber and related
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief The Fiber has fixed length, but derived classes can change length.

 A fiber is a filament of constant length.
 Derived classes are available, where different models of how length may change
 have been implemented.
 
 List of classes accessible by specifying `fiber:activity`.
 
 `activity`    | Class               | Parameter                  |
 --------------|---------------------|-----------------------------
 `none`        | Fiber               | @ref FiberPar (default)
 `grow`        | GrowingFiber        | @ref GrowingFiberPar
 `classic`     | ClassicFiber        | @ref ClassicFiberPar
 `dynamic`     | DynamicFiber        | @ref DynamicFiberPar
 `treadmill`   | TreadmillingFiber   | @ref TreadmillingFiberPar
 
 */
Property* FiberSet::newProperty(const std::string& cat, const std::string& nom, Glossary& opt) const
{
    if ( cat == "fiber" )
    {
        std::string a;
        if ( opt.peek(a, "activity") )
        {
            if ( a == "classic" )
                return new ClassicFiberProp(nom);
            if ( a == "grow" )
                return new GrowingFiberProp(nom);
            if ( a == "dynamic" )
                return new DynamicFiberProp(nom);
            if ( a == "treadmill" )
                return new TreadmillingFiberProp(nom);
            if ( a == "none" )
                return new FiberProp(nom);


            std::cerr << "INCIDENT: substituting generic Fiber class for `"+a+"'\n";
            return new FiberProp(nom);
            //throw InvalidParameter("unknown fiber:activity `"+a+"'");
        }
        return new FiberProp(nom);
    }
    return nullptr;
}


/**
 The initialization options depend on the type of fiber: Fiber, DynamicFiber, ClassicFiber, etc.

 <hr>
 
 You may directly attach Single or Couple to the fiber, in different ways:
 
     new filament
     {
        attach1 = [NUMBER] NAME
     }
 
 `NAME` should designate the Single or Couple that will be attached to the Fiber.
 `NUMBER` will specify how many Single/Couple will be attached (by default: 1).
 Note that for a Couple, the first Hand is attached to the fiber (and not the second).
 In this case, the Single/Couple are anchored at random position distributed 
 uniformly along the Fiber.

     new filament
     {
        attach1 = [NUMBER] NAME, ABSCISSA, REFERENCE [, MODIFIER] [, POSITION]
     }
 
 If `ABSCISSA` is specified (in micrometers), the Single/Couple will be attached
 at the specified distance from the `REFERENCE = { minus_end, plus_end, center }
 (default is `minus_end`). The distance is counted towards the other end.
 Moreover, a `MODIFIER = { uniform, exponential, regular }` can be specified (default: `uniform`).
 With `uniform` Single/Couple are attached uniformly over distance `[0, DISTANCE]` from the `REFERENCE`.
 With `regular`, they are distributed regularly.
 Finally, `POSITION` can be specified. This is mostly relevant for `SINGLE` with
 activity `fixed`.
 
 One can specify multiple attachement instructions with `attach1`, `attach2`, etc.
 For example, this attaches one `simplex` at each end of the filaments:
 
     new filament
     {
        attach1 = simplex, 0.0, minus_end
        attach2 = simplex, 0.0, plus_end
     }

 @}
 */
ObjectList FiberSet::newObjects(const std::string& name, Glossary& opt)
{
    FiberProp * p = simul.findProperty<FiberProp>("fiber", name);
    Fiber * fib = p->newFiber(opt);
    assert_true( fib->tag()==Fiber::TAG );
    fib->birthTime(simul.time());

    ObjectList res(2);
    res.push_back(fib);
 
    unsigned inp = 1;
    std::string spe, var = "attach1";
    
    if ( opt.has_key("attach") )
    {
        var = "attach";
        inp = 0;
    }
    
    //can add Singles or Couples to the Fiber:
    while ( opt.set(spe, var) )
    {
        size_t cnt = 1;
        Tokenizer::split_integer(cnt, spe);
        
        // search for Single and Couple:
        SingleProp * sip = static_cast<SingleProp*>(simul.properties.find("single", spe));
        CoupleProp * cop = static_cast<CoupleProp*>(simul.properties.find("couple", spe));
        
        if ( sip && cop )
            throw InvalidParameter("ambiguous fiber:attach single/couple `"+spe+"'");
        if ( !sip && !cop )
            throw InvalidParameter("could not find fiber:attach single/couple `"+spe+"'");
        
        // variables defining an abscissa:
        int mod = 7;
        real abs = 0;
        FiberEnd ref = ORIGIN;
        if ( opt.set(abs, var, 1) )
            mod = 0;
        opt.set(ref, var, 2, {{"plus_end", PLUS_END}, {"minus_end", MINUS_END}, {"center", CENTER}});
        opt.set(mod, var, 3, {{"off", 0}, {"uniform", 1}, {"exponential", 2}, {"regular", 3}});

        for ( size_t n = 0; n < cnt; ++n )
        {
            FiberSite fs(fib, fib->someAbscissa(abs, ref, mod, (real)n/std::max(1UL, cnt-1)));
            Object * cs = nullptr;
            Hand * h = nullptr;
            if ( sip )
            {
                Single * s = sip->newSingle();
                h = s->hand();
                cs = s;
            }
            else
            {
                Couple * c = cop->newCouple();
                h = c->hand1();
                cs = c;
            }
            if ( h->attachmentAllowed(fs) )
            {
                h->attach(fs);
                Vector vec;
                if ( opt.set(vec, var, 4) )
                    cs->setPosition(vec);
                else
                    cs->setPosition(fs.pos());
                res.push_back(cs);
            }
            else
            {
                delete(cs);
                throw InvalidParameter("hand cannot attach to specified fiber");
            }
        }
        var = "attach" + std::to_string(++inp);
    }

    return res;
}

/**
 The returned object is not initialized, since this is used for file input
 */
Object * FiberSet::newObject(const ObjectTag tag, unsigned num)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( tag == Fiber::TAG || tag == 'm' )
#else
    if ( tag == Fiber::TAG )
#endif
    {
        FiberProp * p = simul.findProperty<FiberProp>("fiber", num);
        Fiber * obj = p->newFiber();
        obj->birthTime(simul.time());
        return obj;
    }
    return nullptr;
}


void FiberSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Calculate the free monomer concentration. 
 Calls step() once for every Fiber.
 */

void FiberSet::step()
{
    PropertyList plist = simul.properties.find_all("fiber");
    
    // calculate the total length used for each kind of Fiber:
    for ( Property * i : plist )
        static_cast<FiberProp*>(i)->used_polymer = 0;

    for ( Fiber const* fib = first(); fib; fib = fib->next() )
        fib->prop->used_polymer += fib->length();
    
    // calculate the ratio of free polymer for each class of Fiber:
    for ( Property * i : plist )
    {
        FiberProp * p = static_cast<FiberProp*>(i);

        // update the normalized monomer concentration:
        p->free_polymer = 1.0 - p->used_polymer / p->total_polymer;
        
        if ( p->free_polymer < 0 )
        {
            Cytosim::warn << "The free monomer concentration would be negative !!!" << std::endl;
            //this should not happen
            p->free_polymer = 0;
        }
    }

    /*
     We call step() here exactly once for every Fiber.
     New Fiber may be created, for instance by Fiber::sever(), but they should
     be linked at the start of the list, and thus not considered here.
     */
    Fiber * obj = first();

    while ( obj )
    {
        Fiber * nxt = obj->next();
        obj->step();
        obj = nxt;
    }
}


/**
 Cut all Fibers along the plane defined by n.pos + a = 0.
 */
void FiberSet::planarCut(Vector const& n, const real a, state_t stateP, state_t stateM)
{
    /*
     We must ensure here that each Fiber is processed only once.
     This code works if newly created Fiber are linked at the head of the list
     */
    Fiber * obj = first();

    while ( obj )
    {
        Fiber * nxt = obj->next();
        obj->planarCut(n, a, stateP, stateM);
        obj = nxt;
    }
}

/**
 Cut given Fibers along the plane defined by n.pos + a = 0.
 */
void FiberSet::planarCut(ObjectList& objs, Vector const& n, const real a, state_t stateP, state_t stateM)
{
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
            fib->planarCut(n, a, stateP, stateM);
    }
}


void FiberSet::foldPositions(Modulo const* m) const
{
    for ( Fiber * o=first(); o; o=o->next() )
        o->foldPosition(m);
}


/**
 Calculate intersection between all fibers,
 and report the corresponding abscissa in arrays 'res1' and 'res2'.
 */
void FiberSet::allIntersections(Array<FiberSite>& res1, Array<FiberSite>& res2,
                                const real max_distance) const
{
    const real sup = max_distance * max_distance;
    res1.clear();
    res2.clear();

    for ( Fiber * fib1 = first(); fib1; fib1 = fib1->next() )
    {
        for ( unsigned s1 = 0; s1 < fib1->nbSegments(); ++s1 )
        {
            FiberSegment seg1(fib1, s1);
            // check against other segments of this fiber
            for ( unsigned s2 = s1+2; s2 < fib1->nbSegments(); ++s2 )
            {
                FiberSegment seg2(fib1, s2);
                real abs1, abs2;
                if ( seg1.shortestDistance(seg2, abs1, abs2) < sup )
                {
                    if ( seg1.within(abs1) & seg2.within(abs2) )
                    {
                        res1.push_back(FiberSite(fib1, abs1+fib1->abscissaPoint(s1)));
                        res2.push_back(FiberSite(fib1, abs2+fib1->abscissaPoint(s2)));
                    }
                }
            }
            // check against other fibers:
            for ( Fiber * fib2 = fib1->next(); fib2; fib2 = fib2->next() )
            {
                for ( unsigned s2 = 0; s2 < fib2->nbSegments(); ++s2 )
                {
                    FiberSegment seg2(fib2, s2);
                    real abs1, abs2;
                    if ( seg1.shortestDistance(seg2, abs1, abs2) < sup )
                    {
                        if ( seg1.within(abs1) & seg2.within(abs2) )
                        {
                            res1.push_back(FiberSite(fib1, abs1+fib1->abscissaPoint(s1)));
                            res2.push_back(FiberSite(fib2, abs2+fib2->abscissaPoint(s2)));
                        }
                    }
                }
            }
        }
    }
}


/**
 Set a list of Locations on the fibers, chosen randomly with uniform sampling.
 The number of sites returned on a section of length `L` is  `L / spread`.
 `spread` is thus the average distance between sites.
 
 Condition: ( spread > 0 )
 */
void FiberSet::uniFiberSites(Array<FiberSite>& res, const real spread) const
{
    assert_true( spread > 0 );

    res.clear();
    Fiber * fib = first();
    real abs = spread * RNG.exponential();
    while ( fib )
    {
        real len = fib->length();
        while ( abs < len )
        {
            res.push_back(FiberSite(fib, abs+fib->abscissaM()));
            abs += spread * RNG.exponential();
        }
        abs -= len;
        fib = fib->next();
    }
}


/// a random site on the fiber, equidistributed over length
/**
 This method is unefficient if multiple sites are desired
 */
FiberSite FiberSet::randomSite() const
{
    real abs = 0;
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
        abs += fib->length();

    assert_true( abs > 0 );
    
    abs *= RNG.preal();

    for ( Fiber* fib=first(); fib; fib=fib->next() )
    {
        real len = fib->length();
        if ( abs <= len )
            return FiberSite(fib, fib->abscissaM()+abs);
        abs -= len;
    }
    
    ABORT_NOW("unexpected abscissa overrun");
    return FiberSite(first(), 0);
}


/// a random site on the fiber of class 'prop'
/**
 This method is unefficient if multiple sites are desired
 */
FiberSite FiberSet::randomSite(FiberProp * arg) const
{
    real abs = 0;
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
        if ( fib->property() == arg )
            abs += fib->length();

    if ( abs == 0 )
        throw InvalidParameter("found no fibers of requested class");

    abs *= RNG.preal();
    
    for ( Fiber* fib=first(); fib; fib=fib->next() )
        if ( fib->property() == arg )
        {
            real len = fib->length();
            if ( abs <= len )
                return FiberSite(fib, fib->abscissaM()+abs);
            abs -= len;
        }
    
    ABORT_NOW("unexpected abscissa overrun");
    return FiberSite(first(), 0);
}


/**
 Returns a Fiber location corresponding to what is specified in opt[var]:
 
       attach = FIBER, ABSCISSA, REFERENCE
 
 with
 
       FIBER = microtubule1, fiber1, fiber2, etc.
       ABSCISSA = a distance
       REFERENCE = [ plus_end, minus_end, center ]
 
 */
FiberSite FiberSet::someSite(std::string const& key, Glossary& opt) const
{
    std::string str;
    if ( opt.set(str, key) )
    {
        if ( str == "all" )
            return randomSite();
        else
        {
            Fiber* fib = Fiber::toFiber(findObject(str, title()));
            
            if ( !fib )
            {
                // without argument, a fiber name specifies uniform attachment:
                if ( opt.nb_values(key) == 1 )
                {
                    Property * p = simul.findProperty(title(), str);
                    if ( p )
                        return randomSite(static_cast<FiberProp*>(p));
                }
                throw InvalidParameter("Could not find fiber specified for attachment");
            }
            
            // variables defining an abscissa:
            int mod = 7;
            real abs = 0;
            FiberEnd ref = ORIGIN;
            if ( opt.set(abs, key, 1) )
                mod = 0;
            opt.set(ref, key, 2, {{"plus_end", PLUS_END}, {"minus_end", MINUS_END}, {"center", CENTER}});
            opt.set(mod, key, 3, {{"off", 0}, {"uniform", 1}, {"exponential", 2}});

            return FiberSite(fib, fib->someAbscissa(abs, ref, mod, 1.0));
        }
    }
    throw InvalidParameter("unrecognized site specification");
    return FiberSite();
}

/**
 Set a list of Locations near the tip of the fibers, on sections that were recently assembled.
 This relies on Fiber::freshAssembly() returning the length of polymer made in the last time_step
 The number of binding sites returned will thus be proportional to simul:time_step
 
 This is for the PLUS_END
 */
void FiberSet::newFiberSitesP(Array<FiberSite>& res, const real spread) const
{
    assert_true( spread > 0 );
    
    res.clear();
    Fiber * fib = first();
    real abs = spread * RNG.exponential();
    while ( fib )
    {
        real len = fib->freshAssemblyP();
        while ( abs < len )
        {
            res.push_back(FiberSite(fib, fib->abscissaP()-abs));
            abs += spread * RNG.exponential();
        }
        abs -= len;
        fib = fib->next();
    }
}


/**
 Set a list of Locations near the tip of the fibers, on sections that were recently assembled.
 This relies on Fiber::freshAssembly() returning the length of polymer made in the last time_step
 The number of binding sites returned will thus be proportional to simul:time_step
 
 This is for the MINUS_END
 */
void FiberSet::newFiberSitesM(Array<FiberSite>& res, const real spread) const
{
    assert_true( spread > 0 );
    
    res.clear();
    Fiber * fib = first();
    real abs = spread * RNG.exponential();
    while ( fib )
    {
        real a = fib->freshAssemblyM();
        while ( abs < a )
        {
            res.push_back(FiberSite(fib, fib->abscissaM()+abs));
            abs += spread * RNG.exponential();
        }
        abs -= a;
        fib = fib->next();
    }
}


void FiberSet::flipFiberPolarity()
{
    for ( Fiber* fib=first(); fib; fib=fib->next() )
    {
        fib->flipChainPolarity();
        fib->flipHandsPolarity();
    }
}


/**
 After reading from file, the fiber structure need to be updated,
 as well as the Hands bound to them.
 */
void FiberSet::prune(ObjectFlag f)
{
    for (Fiber* fib=first(), *n; fib; fib=n)
    {
        n = fib->next();
        if ( fib->flag() == f )
            delete(fib);
        else
        {
            fib->updateFiber();
            fib->resetLattice();
            fib->flag(0);
        }
    }
}


//------------------------------------------------------------------------------
#pragma mark -


real FiberSet::totalLength() const
{
    real res = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
        res += fib->length();
    
    return res;
}


real FiberSet::totalLength(FiberProp const* p) const
{
    real res = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
        if ( fib->prop == p )
            res += fib->length();
    
    return res;
}


void FiberSet::infoLength(ObjectList const& objs,
                          unsigned& cnt, real& avg, real& dev, real& mn, real& mx)
{
    cnt = 0;
    avg = 0;
    dev = 0;
    mn = INFINITY;
    mx = 0;

    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
        {
            ++cnt;
            real x = fib->length();
            avg += x;
            dev += x * x;
            if ( x < mn ) mn = x;
            if ( x > mx ) mx = x;
        }
    }
    
    if ( cnt )
    {
        avg /= cnt;
        real v = dev/cnt - avg*avg;
        // the variance can be numerically negative, which is mathematically impossible
        if ( v > 0 )
            dev = sqrt(v);
        else
            dev = 0;
    }
}


void FiberSet::infoBirthtime(ObjectList const& objs, unsigned& cnt,
                             real& avg, real& dev, real& mn, real& mx)
{
    cnt = 0;
    avg = 0;
    dev = 0;
    mn  = INFINITY;
    mx  = 0;
    
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
        {
            ++cnt;
            real x = fib->birthTime();
            avg += x;
            dev += x * x;
            if ( x < mn ) mn = x;
            if ( x > mx ) mx = x;
        }
    }
    
    if ( cnt )
    {
        avg /= cnt;
        real v = dev/cnt - avg*avg;
        // the variance can be numerically negative, which is mathematically impossible
        if ( v > 0 )
            dev = sqrt(v);
        else
            dev = 0;
    }
}


void FiberSet::infoSegments(ObjectList const& objs,
                            unsigned& cnt, unsigned& joints, real& mn, real& mx)
{
    cnt = 0;
    joints = 0;
    mn = INFINITY;
    mx = 0;
    
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
        {
            ++cnt;
            real n, x;
            joints += fib->nbPoints() - 2;
            fib->segmentationMinMax(n, x);
            if ( n < mn )
                mn = n;
            if ( x > mx )
                mx = x;
        }
    }
}


unsigned FiberSet::nbKinks(ObjectList const& objs)
{
    unsigned cnt = 0;
    
    for ( Object * i : objs )
        cnt += Fiber::toFiber(i)->nbKinks();

    return cnt;
}


/**
 Each Fiber segment is weigthed by its length.
 
 @return G = average center of gravity
 @return D = average direction
 
 The average direction is the average of the filament's tangents at each segment.
 */
real FiberSet::infoPosition(ObjectList const& objs, Vector& M, Vector& G, Vector& P)
{
    real S = 0;
    G.reset();
    P.reset();
    M.reset();
    
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
        {
            const real w = fib->length();
            S += w;
            M += w * fib->posEndM();
            P += w * fib->posEndP();
 
            Vector G1 = 0.5 * ( fib->posEndM() + fib->posEndP() );
            for ( unsigned n = 1; n < fib->nbSegments(); ++n )
                G1 += fib->posP(n);
            G += G1 * ( w / fib->nbSegments() );
        }
    }
    
    if ( S > 0 )
    {
        G /= S;
        M /= S;
        P /= S;
    }
    return S;
}

/**
 Each Fiber segment is weigthed by its length.
 The Nematic direction is an eigenvector of the second rank tensor order parameter.
 
@return `res`, a 9-elements matrix containing the first two principal component vectors
 
 if DIM == 2:
 Component 1 is { vec[0], vec[1] }
 if DIM == 3:
 Component 1 is { vec[0], vec[1], vec[2] }
 Component 2 is { vec[3], vec[4], vec[5] }
 */
real FiberSet::infoNematic(ObjectList const& objs,
                           real res[9])
{
    real S = 0;
    real M[9] = { 0 };
    
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
        {
            const real w = square(fib->segmentation());
            for ( unsigned n = 0; n < fib->nbSegments(); ++n )
            {
                Vector p = fib->dirSegment(n);
                
                M[0] += w * ( p.XX * p.XX );
#if ( DIM > 1 )
                M[1] += w * ( p.YY * p.XX );
                M[4] += w * ( p.YY * p.YY );
#endif
#if ( DIM > 2 )
                M[2] += w * ( p.ZZ * p.XX );
                M[5] += w * ( p.ZZ * p.YY );
                M[8] += w * ( p.ZZ * p.ZZ );
#endif
            }
            S += fib->nbSegments();
        }
    }
    
    if ( S == 0 )
        return 0;
    // rescale matrix, to ensure eigenvalue = 1 in perfect order
    real beta = ( DIM >= 3 ) ? 0.5 : 1.0;
    S = beta * DIM / S;
    for ( unsigned d = 0; d < 9; ++d )
        M[d] = M[d] * S;
    // subtract trace:
    M[0] -= beta;
    if ( DIM > 1 ) M[4] -= beta;
    if ( DIM > 2 ) M[8] -= beta;

    int nv;
    real vec[9] = { 0 };
    real val[3] = { 0 };
    real work[32];
    int iwork[16];
    int ifail[4];
    
    // calculate two largest eigenvalues in 3D, one in 2D:
    int info = 0;
    lapack::xsyevx('V','I','L', DIM, M, 3, 0, 0, 2, DIM, REAL_EPSILON,
                   &nv, val, vec, 3, work, 32, iwork, ifail, &info);

#if ( DIM > 2 )
    //std::clog << "Eigen value1 " << val[0] << "  vector  " << Vector(vec) << std::endl;
    //std::clog << "Eigen value2 " << val[1] << "  vector  " << Vector(vec+3) << std::endl;
    real u = std::copysign(1, vec[3]);
    real v = std::copysign(1, vec[0]);
    // order the 2 vectors in decreasing eigenvalues (reverse order from LAPACK).
    res[0] = u * vec[3];
    res[1] = u * vec[4];
    res[2] = u * vec[5];
    res[3] = v * vec[0];
    res[4] = v * vec[1];
    res[5] = v * vec[2];
    // calculate third vector as vector product of first two:
    res[6] = res[1]*res[5] - res[2]*res[4];
    res[7] = res[2]*res[3] - res[0]*res[5];
    res[8] = res[0]*res[4] - res[1]*res[3];
#else
    //std::clog << "Eigen value1 " << val[0] << "  vector  " << Vector(vec) << std::endl;
    real u = std::copysign(1, vec[0]);
    res[0] =  u * vec[0];
    res[1] =  u * vec[1];
    res[2] =  0;
    // second vector is orthogonal:
    res[3] = -u * vec[1];
    res[4] =  u * vec[0];
    res[5] =  0;
    // third vector set in Z-direction
    res[6] =  0;
    res[7] =  0;
    res[8] =  1;
#endif
    
    return val[nv-1];
}


/**
 Return the principal component directions of the cloud of vertices.
 Each fiber is weighted by its length.
 
 @return G = average center of gravity
 @return `mom`, a 9-elements matrix containing the moments in its lower part:
 - mom[0] = sum( ( X - mean(X) ) * ( X - mean(X) ) ) / S
 - mom[1] = sum( ( X - mean(X) ) * ( Y - mean(Y) ) ) / S
 - mom[2] = sum( ( X - mean(X) ) * ( Z - mean(Z) ) ) / S
 - mom[4] = sum( ( Y - mean(Y) ) * ( Y - mean(Y) ) ) / S
 - mom[5] = sum( ( Y - mean(Y) ) * ( Z - mean(Z) ) ) / S
 - mom[8] = sum( ( Z - mean(Z) ) * ( Z - mean(Z) ) ) / S
 .
 
 @return `res`, a 9-elements matrix containing the first two principal component vectors
 
 if DIM == 2:
   Component 1 is { vec[0], vec[1] }
 if DIM == 3:
   Component 1 is { vec[0], vec[1], vec[2] }
   Component 2 is { vec[3], vec[4], vec[5] }
 
 */
int FiberSet::infoComponents(ObjectList const& objs,
                             real& sum, real avg[3], real mom[9], real res[9])
{
    sum = 0;
    avg[0] = 0.0;
    avg[1] = 0.0;
    avg[2] = 0.0;
    real M[9] = { 0 };
    
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
       if ( fib )
        {
            const real w = fib->length() / fib->nbPoints();
            for ( unsigned n = 0; n < fib->nbPoints(); ++n )
            {
                Vector p = fib->posP(n);
                avg[0] += w * p.XX;
                M[0]   += w * p.XX * p.XX;
#if ( DIM > 1 )
                avg[1] += w * p.YY;
                M[1]   += w * p.YY * p.XX;
                M[4]   += w * p.YY * p.YY;
#endif
#if ( DIM > 2 )
                avg[2] += w * p.ZZ;
                M[2]   += w * p.ZZ * p.XX;
                M[5]   += w * p.ZZ * p.YY;
                M[8]   += w * p.ZZ * p.ZZ;
#endif
            }
            sum += w * fib->nbPoints();
        }
    }
    
    if ( sum == 0 )
        return 0;
    
    /**
     Remove the mean:
       (x-a)*(x-a) = x*x - 2x*a + a*a
       (x-a)*(y-b) = x*y - x*b - y*a + a*b
     */
    
    avg[0] /= sum;
    M[0] = M[0]/sum - avg[0] * avg[0];
#if ( DIM > 1 )
    avg[1] /= sum;
    M[1] = M[1]/sum - avg[1] * avg[0];
    M[4] = M[4]/sum - avg[1] * avg[1];
#endif
#if ( DIM > 2 )
    avg[2] /= sum;
    M[2] = M[2]/sum - avg[2] * avg[0];
    M[5] = M[5]/sum - avg[2] * avg[1];
    M[8] = M[8]/sum - avg[2] * avg[2];
#endif
    
    // copy moments:
    for ( int i = 0; i < 9; ++i )
        mom[i] = M[i];
    
    int nv;
    real vec[9] = { 0 };
    real val[3] = { 0 };
    real work[32];
    int iwork[16];
    int ifail[4];
    
    // calculate two largest eigenvalues in 3D, one in 2D:
    int info = 0;
    lapack::xsyevx('V','I','L', DIM, M, 3, 0, 0, 2, DIM, REAL_EPSILON,
                   &nv, val, vec, 3, work, 32, iwork, ifail, &info);
    
#if ( DIM > 2 )
    real u = std::copysign(1, vec[3]);
    real v = std::copysign(1, vec[0]);
    // order the 2 vectors in decreasing eigenvalues.
    res[0] = u * vec[3];
    res[1] = u * vec[4];
    res[2] = u * vec[5];
    res[3] = v * vec[0];
    res[4] = v * vec[1];
    res[5] = v * vec[2];
    // calculate third vector as vector product for first two:
    res[6] = res[1]*res[5] - res[2]*res[4];
    res[7] = res[2]*res[3] - res[0]*res[5];
    res[8] = res[0]*res[4] - res[1]*res[3];
#else
    real u = std::copysign(1, vec[0]);
    res[0] =  u * vec[0];
    res[1] =  u * vec[1];
    res[2] =  0;
    // second vector is orthogonal:
    res[3] = -u * vec[1];
    res[4] =  u * vec[0];
    res[5] =  0;
    res[6] =  0;
    res[7] =  0;
    res[8] =  1;
#endif

    //VecPrint::print(std::clog, 3, 3, vec);
    
    return ( info == 0  &&  nv == DIM-1 );
}


/**
 Counts the number of fiber intersecting the plane defined by <em> n.pos + a = 0 </em>
 in two categories, depending on the direction with which they cross the plane:
 - `np` = number of parallel segments ( the scalar product dir.n is strictly positive )
 - `na` = number of anti-parallel segments ( dir.n < 0 )
 .
 */
void FiberSet::infoPlane(int& np, int& na, Vector const& n, real a) const
{
    np = 0;
    na = 0;
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        for ( unsigned s = 0; s < fib->nbSegments(); ++s )
        {
            real abs = fib->planarIntersect(s, n, a);
            if ( 0 <= abs  &&  abs < 1 )
            {
                real sec = dot(n, fib->dirSegment(s));
                if ( sec > 0 )
                    ++np;
                else if ( sec < 0 )
                    ++na;
            }
        }
    }
}


/**
 Calculate two indices characterizing the organization of the fibers along the axis `n`.
 - `ixa` = average { ( o - i ) }
 - `ixp` = average { ( r - l ) }
 .
 where:
 - `o` = number of fiber pointing outward (away from the mid-plane),
 - `i` = number of fiber pointing inward (toward the mid-plane),
 - `r` = number of fiber pointing right (ie. in the direction of `n`),
 - `l` = number of fiber pointing left.
 .
 
 The indices are averaged over planar sections taken every `dm` units of space,
 and the values for each planar section are weighted by the number of fibers.
 The central symmetry plane is defined by `n.x+a=0`, and the edges correspond to `n.x+a=+/-m`.
 
 The results characterize broadly the type of fiber organization:
 - `ixa =  1, ixp = 0`:   aster,
 - `ixa = -1, ixp = 0`:   anti-aster,
 - `ixa =  0, ixp = 1`:   parallel overlap,
 - `ixa =  0, ixp = 0`:   anti-parallel overlap (50/50).
 .
 */
void FiberSet::infoSpindle(real& ixa, real& ixp, Vector const& n, real a, real m, real dm) const
{
    ixa = 0;
    ixp = 0;
    int no, ni, nio;
    int sum = 0;
    for ( real p = dm/2 ; p < m ; p += dm )
    {
        // left side
        infoPlane(ni, no, n, a+p);
        nio = ni + no;
        if ( nio )
        {
            ixa += ( no - ni );
            ixp += ( ni - no );
            sum += nio;
        }
    
        // right side
        infoPlane(no, ni, n, a-p);
        nio = ni + no;
        if ( nio )
        {
            ixa += ( no - ni );
            ixp += ( no - ni );
            sum += nio;
        }
    }
    if ( sum )
    {
        ixa /= sum;
        ixp /= sum;
    }
}


/**
 Sum elastic bending energy of all the fibers `fib` for which func(fib, arg) == true
 */
void FiberSet::infoBendingEnergy(ObjectList const& objs,
                                 unsigned& cnt, real& avg, real& dev)
{
    cnt = 0;
    avg = 0;
    dev = 0;
    
    for ( Object * i : objs )
    {
        Fiber * fib = Fiber::toFiber(i);
        if ( fib )
        {
            ++cnt;
            real x = fib->bendingEnergy();
            avg += x;
            dev += x * x;
        }
    }
    
    if ( cnt )
    {
        avg /= cnt;
        real v = dev/cnt - avg*avg;
        // the variance can be numerically negative, which is mathematically impossible
        if ( v > 0 )
            dev = sqrt(v);
        else
            dev = 0;
    }
}

/**
 Sum tensions of all fiber segments intersecting the plane defined by
      <em> n.pos + a = 0 </em>
 (the vector `n` defines the direction orthogonal to the plane)
 
 The intersecting segments are determined by testing all Fibers.
 The tension dipole along a segment is obtained from the Lagrange multiplier 
 associated with the length of this segment. It is positive if the segment is stretched.
 The magnitude of the dipole is multiplied by the cosine of the angle measured between 
 the segment and the plane normal, yielding axial components that can be summed.
 
 @return cnt = number of segments intersecting the plane
 @return ten = sum of tension in these segments
 */
void FiberSet::infoTension(unsigned& cnt, real& ten, Vector const& n, real a) const
{
    cnt = 0;
    ten = 0;
    
    Vector dir = normalize(n);
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        for ( unsigned s = 0; s < fib->nbSegments(); ++s )
        {
            real abs = fib->planarIntersect(s, n, a);
            if ( 0 <= abs  &&  abs < 1 )
            {
                ten += fabs(dot(dir, fib->dirSegment(s))) * fib->tension(s);
                ++cnt;
            }
        }
    }
}


/**
 Sum tension of all the segments
 
 @return cnt = total number of segments
 @return ten = sum of tension
 */
void FiberSet::infoTension(unsigned& cnt, real& ten) const
{
    cnt = 0;
    ten = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        for ( unsigned s = 0; s < fib->nbSegments(); ++s )
        {
            ten += fib->tension(s);
            ++cnt;
        }
    }
}


void FiberSet::infoRadius(unsigned& cnt, real& rad) const
{
    real r = 0;
    cnt = 0;
    
    for ( Fiber const* f=first(); f; f=f->next() )
    {
        for ( unsigned p = 0; p < f->nbPoints() ; ++p )
        {
            r += f->posP(p).norm();
            ++cnt;
        }
    }
    if ( cnt )
        rad = r / cnt;
}


void FiberSet::infoRadius(unsigned& cnt, real& rad, FiberEnd end) const
{
    real r = 0;
    cnt = 0;
    
    for ( Fiber const* f=first(); f; f=f->next() )
    {
        r += f->posEnd(end).norm();
        ++cnt;
    }
    if ( cnt )
        rad = r / cnt;
}

