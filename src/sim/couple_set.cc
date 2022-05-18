// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "simul_prop.h"
#include "couple_set.h"
#include "couple_prop.h"
#include "fork_prop.h"
#include "crosslink_prop.h"
#include "shackle_prop.h"
#include "bridge_prop.h"
#include "duo_prop.h"
#include "glossary.h"
#include "simul.h"


/**
 @defgroup CoupleGroup Couple and related
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Couple contains two Hand, and can thus crosslink two Fibers.

 The plain Couple may crosslink two Fiber irrespective of their configuration.
 Derived classes implement specificity, angular stiffness, etc.
 
 List of classes accessible by specifying `couple:activity`.

 `activity`    | Classes                 | Parameters           | Property     |
 --------------|-------------------------|----------------------|---------------
 `diffuse`     | Couple CoupleLong       | @ref CouplePar       | CoupleProp
 `crosslink`   | Crosslink CrosslinkLong | @ref CrosslinkPar    | CrosslinkProp
 `bridge`      | Bridge                  | @ref BridgePar       | BridgeProp
 `duo`         | Duo  DuoLong            | @ref DuoPar          | DuoProp
 `slide`       | Shackle ShackleLong     | @ref ShacklePar      | ShackleProp
 `fork`        | Fork                    | @ref ForkPar         | ForkProp

 Example:

     set couple complex
     {
       hand1 = kinesin
       hand2 = kinesin
       stiffness = 100
       diffusion = 10
       activity = crosslink
       length = 0.02
     }

 */

Property* CoupleSet::newProperty(const std::string& cat, const std::string& nom, Glossary& opt) const
{
    if ( cat == "couple" )
    {
        std::string a;
        if ( opt.peek(a, "activity") )
        {
            if ( a == "fork" )
                return new ForkProp(nom);
            if ( a == "crosslink" )
                return new CrosslinkProp(nom);
            if ( a == "bridge" )
                return new BridgeProp(nom);
            if ( a == "duo" )
                return new DuoProp(nom);
            if ( a == "slide" )
                return new ShackleProp(nom);
            if ( a == "diffuse" )
                return new CoupleProp(nom);
#if ( 0 )
            throw InvalidParameter("unknown single:activity `"+a+"'");
#else
        // try to proceed anyhow:
        std::cerr << "WARNING: unknown couple:activity `" << a << "'" << std::endl;
#endif
        }
        return new CoupleProp(nom);
    }
    return nullptr;
}

//------------------------------------------------------------------------------

void CoupleSet::prepare(PropertyList const& properties)
{
    uni = uniPrepare(properties);
}


void CoupleSet::step()
{
    // use alternative attachment strategy:
    if ( uni )
    {
        uniCollect();
        uniAttach(simul.fibers);
    }

    /*
     ATTENTION: We ensure here that step() is called exactly once for each object.
     The Couples are stored in multiple lists, and are automatically transfered
     from one list to another one if their Hands bind or unbind.
     The code relies on the fact that a Couple will be moved to the start of the
     list to which it is transfered. By proceeding always from the node, which was
     first before any transfer could occur, we process each Couple only once.
     Moreover, we get the 'next' in the list always before calling 'step()', because
     'step()' may transfer the node to another list, changing the value of 'next()'
     */
    
    /*
    Cytosim::log("CoupleSet::step : FF %5i AF %5i FA %5i AA %5i\n",
                 ffList.size(), afList.size(), faList.size(), aaList.size());
    */
    
    Couple *const ffHead = firstFF();
    Couple *const afHead = firstAF();
    Couple *const faHead = firstFA();
    
    bool const faOdd = faList.size() & 1;
    bool const afOdd = afList.size() & 1;
    bool const ffOdd = ffList.size() & 1;

    Couple * obj, * nxt;
    
    obj = firstAA();
    // this loop is unrolled, processing objects 2 by 2:
    if ( aaList.size() & 1 )
    {
        nxt = obj->next();
        obj->stepAA();
        obj = nxt;
    }
    while ( obj )
    {
        nxt = obj->next();
        obj->stepAA();
        obj = nxt->next();
        nxt->stepAA();
    }
    
    obj = faHead;
    // this loop is unrolled, processing objects 2 by 2:
    if ( faOdd )
    {
        nxt = obj->next();
        obj->stepFA(simul);
        obj = nxt;
    }
    while ( obj )
    {
        nxt = obj->next();
        obj->stepFA(simul);
        obj = nxt->next();
        nxt->stepFA(simul);
    }

    obj = afHead;
    // this loop is unrolled, processing objects 2 by 2:
    if ( afOdd )
    {
        nxt = obj->next();
        obj->stepAF(simul);
        obj = nxt;
    }
    while ( obj )
    {
        nxt = obj->next();
        obj->stepAF(simul);
        obj = nxt->next();
        nxt->stepAF(simul);
    }
    
    //std::clog << "CoupleSet::step : FF " << ffList.size() << " head " << ffHead << std::endl;

    obj = ffHead;
    // this loop is unrolled, processing objects 2 by 2:
    if ( ffOdd )
    {
        nxt = obj->next();
        obj->stepFF(simul);
        obj = nxt;
    }
    while ( obj )
    {
        nxt = obj->next();
        obj->stepFF(simul);
        obj = nxt->next();
        nxt->stepFF(simul);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

Object * CoupleSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Couple::TAG )
    {
        CoupleProp * p = simul.findProperty<CoupleProp>("couple", num);
        return p->newCouple();
    }
    return nullptr;
}


/**
 @addtogroup CoupleGroup

 You can attach the hands of a Couple:
 
     new complex
     {
        attach1 = FIBER, REAL, REFERENCE
        attach2 = FIBER, REAL, REFERENCE
     }
 
 where:
 - FIBER designates the fiber:
     - `fiber1` of `fiber2` correspond to fibers directly
     - `first` or `last` to the oldest and youngest fiber
     - `last-1` the penultimate, etc.
     .
 - REAL is the abscissa of the attachment point.
   If the abscissa is not specified, and random position along
   along the fiber will be selected.
 - REFERENCE can be `minus_end`, `center` or `plus_end` (default = `origin`).
   This defines from which position the abscissa is measured.
 .
 
 */
ObjectList CoupleSet::newObjects(const std::string& name, Glossary& opt)
{
    CoupleProp * p = simul.findProperty<CoupleProp>("couple", name);
    Couple * obj = p->newCouple(&opt);
    
    ObjectList res;
    res.push_back(obj);
        
    // Allow user to attach hand1:
    if ( opt.has_key("attach1") )
        obj->attach1(simul.fibers.someSite("attach1", opt));

    // Allow user to attach hand2:
    if ( opt.has_key("attach2") )
        obj->attach2(simul.fibers.someSite("attach2", opt));

    /* It would be possible to create Couple with custom hand type, and the
    syntax below to attach the Hands could be better used for this */
    
    // Allow user to attach hand1:
    if ( opt.has_key("site1") )
        obj->attach1(simul.fibers.someSite("site1", opt));
    
    // Allow user to attach hand2:
    if ( opt.has_key("site2") )
        obj->attach2(simul.fibers.someSite("site2", opt));

    return res;
}

//------------------------------------------------------------------------------
#pragma mark -

void CoupleSet::relinkA1(Couple * obj)
{
    assert_true( obj->attached1() );

    if ( obj->attached2() )
    {
        faList.pop(obj);
        aaList.push_front(obj);
    }
    else
    {
        ffList.pop(obj);
        afList.push_front(obj);
    }
}


void CoupleSet::relinkD1(Couple * obj)
{
    assert_true( obj->attached1() );
    
    if ( obj->attached2() )
    {
        aaList.pop(obj);
        faList.push_front(obj);
    }
    else
    {
        afList.pop(obj);
        ffList.push_front(obj);
    }
}


void CoupleSet::relinkA2(Couple * obj)
{
    assert_true( obj->attached2() );

    if ( obj->attached1() )
    {
        afList.pop(obj);
        aaList.push_front(obj);
    }
    else
    {
        ffList.pop(obj);
        faList.push_front(obj);
    }
}


void CoupleSet::relinkD2(Couple * obj)
{
    assert_true( obj->attached2() );

    if ( obj->attached1() )
    {
        aaList.pop(obj);
        afList.push_front(obj);
    }
    else
    {
        faList.pop(obj);
        ffList.push_front(obj);
    }
}


void CoupleSet::link(Object * obj)
{
    assert_true( !obj->objset() );
    assert_true( obj->tag() == Couple::TAG );
    
    obj->objset(this);
    Couple * c = static_cast<Couple*>(obj);
    sublist(c->attached1(), c->attached2()).push_front(obj);
}


/**
 This will also detach the Hands
 */
void CoupleSet::unlink(Object * obj)
{
    assert_true( obj->objset() == this );

    Couple * c = static_cast<Couple*>(obj);

    if ( c->hand1()->attached() )
        c->hand1()->detach();
    
    if ( c->hand2()->attached() )
        c->hand2()->detach();
    
    obj->objset(nullptr);
    sublist(c->attached1(), c->attached2()).pop(obj);
}


void CoupleSet::relink(Object * obj, const bool s1, const bool s2)
{
    assert_true( obj->objset() == this );
    sublist(s1, s2).pop(obj);
    Couple * c = static_cast<Couple*>(obj);
    sublist(c->attached1(), c->attached2()).push_front(obj);
}


//------------------------------------------------------------------------------
#pragma mark -

void CoupleSet::foldPositions(Modulo const* m) const
{
    Couple * cx;
    for ( cx=firstAA(); cx; cx=cx->next() )  cx->foldPosition(m);
    for ( cx=firstFA(); cx; cx=cx->next() )  cx->foldPosition(m);
    for ( cx=firstAF(); cx; cx=cx->next() )  cx->foldPosition(m);
    for ( cx=firstFF(); cx; cx=cx->next() )  cx->foldPosition(m);
}


void CoupleSet::shuffle()
{
    ffList.shuffle();
    afList.shuffle();
    faList.shuffle();
    aaList.shuffle();
}


void CoupleSet::erase()
{
    relax();
    ObjectSet::erase(aaList);
    ObjectSet::erase(faList);
    ObjectSet::erase(afList);
    ObjectSet::erase(ffList);
    inventory.clear();
}


void CoupleSet::freeze(ObjectFlag f)
{
    relax();
    ObjectSet::flag(aaList, f);
    ObjectSet::flag(faList, f);
    ObjectSet::flag(afList, f);
    ObjectSet::flag(ffList, f);
}


void CoupleSet::deleteAA(Couple * c)
{
    c->hand1()->detachHand();
    c->hand2()->detachHand();
    inventory.unassign(c);
    c->objset(nullptr);
    aaList.pop(c);
    delete(c);
}


void CoupleSet::deleteFA(Couple * c)
{
    c->hand2()->detachHand();
    inventory.unassign(c);
    c->objset(nullptr);
    faList.pop(c);
    delete(c);
}


void CoupleSet::deleteAF(Couple * c)
{
    c->hand1()->detachHand();
    inventory.unassign(c);
    c->objset(nullptr);
    afList.pop(c);
    delete(c);
}


void CoupleSet::prune(ObjectFlag f)
{
    /* After reading from file, the Hands should not
     update any Fiber, Single or Couple as they will be deleted */
    for (Couple* c=firstAF(), *n; c; c=n)
    {
        n = c->next();
        if ( c->flag() == f )
            deleteAF(c);
    }
    for (Couple* c=firstFA(), *n; c; c=n)
    {
        n = c->next();
        if ( c->flag() == f )
            deleteFA(c);
    }
    for (Couple* c=firstAA(), *n; c; c=n)
    {
        n = c->next();
        if ( c->flag() == f )
            deleteAA(c);
    }

    //ObjectSet::prune(aaList, f, 0);
    //ObjectSet::prune(faList, f, 0);
    //ObjectSet::prune(afList, f, 0);
    ObjectSet::prune(ffList, f, 0);
}


void CoupleSet::thaw()
{
    ObjectSet::flag(aaList, 0);
    ObjectSet::flag(faList, 0);
    ObjectSet::flag(afList, 0);
    ObjectSet::flag(ffList, 0);
}


void CoupleSet::writeAA(Outputter& out) const
{
    out.put_line("\n#section couple AA", out.binary());
    writeNodes(out, aaList);
}

void CoupleSet::writeAF(Outputter& out) const
{
    out.put_line("\n#section couple AF", out.binary());
    writeNodes(out, afList);
}

void CoupleSet::writeFA(Outputter& out) const
{
    out.put_line("\n#section couple FA", out.binary());
    writeNodes(out, faList);
}

void CoupleSet::writeFF(Outputter& out) const
{
    out.put_line("\n#section couple FF", out.binary());
    writeNodes(out, ffList);
}

void CoupleSet::write(Outputter& out) const
{
    if ( sizeAA() > 0 )
        writeAA(out);
    if ( sizeAF() > 0 )
        writeAF(out);
    if ( sizeFA() > 0 )
        writeFA(out);
    if ( sizeFF() > 0 && !simul.prop->skip_free_couple )
        writeFF(out);
}


void CoupleSet::report(std::ostream& os) const
{
    if ( size() > 0 )
    {
        os << '\n' << title();
        PropertyList plist = simul.properties.find_all(title());
        for ( Property * i : plist )
        {
            CoupleProp * p = static_cast<CoupleProp*>(i);
            unsigned cnt = count(match_property, p);
            os << '\n' << std::setw(10) << cnt << " " << p->name();
            os << " ( " << p->hand1 << " | " << p->hand2 << " )";
        }
        if ( plist.size() > 1 )
            os << '\n' << std::setw(10) << size() << " total";
    }
}


ObjectList CoupleSet::collect() const
{
    ObjectList res = ObjectSet::collect(ffList);
    res.append( ObjectSet::collect(afList) );
    res.append( ObjectSet::collect(faList) );
    res.append( ObjectSet::collect(aaList) );
    return res;
}


ObjectList CoupleSet::collect(bool (*func)(Object const*, void const*), void const* arg) const
{
    ObjectList res = ObjectSet::collect(ffList, func, arg);
    res.append( ObjectSet::collect(afList, func, arg) );
    res.append( ObjectSet::collect(faList, func, arg) );
    res.append( ObjectSet::collect(aaList, func, arg) );
    return res;
}


unsigned CoupleSet::count(bool (*func)(Object const*, void const*), void const* arg) const
{
    unsigned ff = ObjectSet::count(ffList, func, arg);
    unsigned af = ObjectSet::count(afList, func, arg);
    unsigned fa = ObjectSet::count(faList, func, arg);
    unsigned aa = ObjectSet::count(aaList, func, arg);
    return ff + af + fa + aa;
}


int CoupleSet::bad() const
{
    int code = 0;
    Couple * obj;
    code = ffList.bad();
    if ( code ) return 100+code;
    for ( obj=firstFF(); obj ; obj = obj->next() )
    {
        if ( obj->attached1() || obj->attached2() )
            return 100;
    }
    
    code = afList.bad();
    if ( code ) return 200+code;
    for ( obj=firstAF(); obj ; obj = obj->next() )
    {
        if ( !obj->attached1() || obj->attached2() )
            return 200;
    }
    
    code = faList.bad();
    if ( code ) return 300+code;
    for ( obj=firstFA(); obj ; obj = obj->next() )
    {
        if ( obj->attached1() || !obj->attached2() )
            return 300;
    }
    
    code = aaList.bad();
    if ( code ) return 400+code;
    for ( obj=firstAA(); obj ; obj = obj->next() )
    {
        if ( !obj->attached1() || !obj->attached2() )
            return 400;
    }
    return code;
}


//------------------------------------------------------------------------------
#pragma mark - Fast Diffusion


/**
Distribute Hand1 of Couples on the sites specified in `loc`.
 */
void CoupleSet::uniAttach1(Array<FiberSite>& loc, CoupleReserveList& can)
{
    for ( FiberSite & i : loc )
    {
        if ( can.empty() )
            return;
        Couple * c = can.back();
        if ( c->hand1()->attachmentAllowed(i) )
        {
            can.pop_back();
            c->attach1(i);
            link(c);
        }
    }
}


/**
 Distribute Hand2 of Couples on the sites specified in `loc`.
 */
void CoupleSet::uniAttach2(Array<FiberSite>& loc, CoupleReserveList& can)
{
    for ( FiberSite & i : loc )
    {
        if ( can.empty() )
            return;
        Couple * c = can.back();
        if ( c->hand2()->attachmentAllowed(i) )
        {
            can.pop_back();
            c->attach2(i);
            link(c);
        }
    }
}


/**
 Distribute Couples on crossing points specified in `loc`.
 `loc` contains positions on the fibers corresponding to crossing points
 as returned by FiberSet::allIntersections()
 */
void CoupleSet::uniAttach12(Array<FiberSite>& loc1, Array<FiberSite>& loc2,
                            CoupleReserveList& can, size_t nb)
{
    size_t nbc = loc1.size();
    assert_true(nbc == loc2.size());
    const size_t sup = std::min(nb, can.size());

    for ( size_t n = 0; n < sup; ++n )
    {
        Couple * c = can.back();
        can.pop_back();
        size_t p = RNG.pint32(nbc);
        c->attach1(loc1[p]);
        c->attach2(loc2[p]);
        link(c);
    }
}


/**
 Implements a Monte-Carlo approach for attachments of free Couple, assumming that
 diffusion is sufficiently fast to maintain a uniform spatial distribution,
 and that the distribution of fibers is more-or-less uniform such that the
 attachments are distributed randomly along the fibers.
 
 Diffusing (free) Couple are removed from the standard list, and thus the
 random walk that is used for simulating diffusion will be skipped,
 as well as the detection of neighboring fibers done for attachments.
 The attachment of already attached Couple is unchanged.
 
 Algorithm:
 - Remove diffusing Single from the simulation, transfering them to a 'reserve'.
 - Estimate the distance between binding sites occuring in one time-step, from:
    - the total length of fibers,
    - the volume of the Space,
    - the binding parameters of the relevant Hand.
    .
 - Attach Singles from the reserve, at random positions along the Fibers
 .
 
 Note: there is a similar feature for Single
 */
void CoupleSet::uniAttach(FiberSet const& fibers)
{
    // preallocate array:
    Array<FiberSite> loc(1024);
    
#if ( 0 )
    
    // this performs a basic verification of fibers.uniFiberSites()
    real dis = 1;
    unsigned cnt = 1<<10;
    real avg = 0;
    real var = 0;
    for ( unsigned i = 0; i < cnt; ++i )
    {
        fibers.uniFiberSites(loc, dis);
        real s = loc.size();
        avg += s;
        var += s*s;
    }
    avg /= (real)cnt;
    var = var/(real)cnt - avg * avg;
    printf("UNI-FIBER-SITES(1)  avg = %9.2f   var = %9.2f\n", avg, var);
    
#endif
    
    // uniform attachment for reserved couples:
    for ( CoupleReserveList & reserve : uniLists )
    {
        if ( reserve.empty() )
            continue;
        
        CoupleProp const * p = reserve.back()->prop;
        
        const real alpha = 2 * p->spaceVolume() / reserve.size();

        if ( p->fast_diffusion == 2 )
        {
            real dis = alpha / p->hand1_prop->bindingSectionRate();
            fibers.newFiberSitesP(loc, dis);
        }
        else
        {
            real dis = alpha / p->hand1_prop->bindingSectionProb();
            fibers.uniFiberSites(loc, dis);
        }
        
        uniAttach1(loc, reserve);
        
        if ( reserve.empty() || p->trans_activated )
            continue;
        
        // if ( couple:trans_activated == true ), Hand2 cannot bind
        if ( p->fast_diffusion == 2 )
        {
            real dis = alpha / p->hand2_prop->bindingSectionRate();
            fibers.newFiberSitesP(loc, dis);
        }
        else
        {
            real dis = alpha / p->hand2_prop->bindingSectionProb();
            fibers.uniFiberSites(loc, dis);
        }
        
        uniAttach2(loc, reserve);
    }
}


/**
 
 Return true if at least one couple:fast_diffusion is true,
 and in this case allocate uniLists.
 
 The Volume of the Space is assumed to remain constant until the next uniPrepare() 
 */
bool CoupleSet::uniPrepare(PropertyList const& properties)
{
    bool res = false;
    unsigned last = 0;
    
    for ( Property const* i : properties.find_all("couple") )
    {
        CoupleProp const * p = static_cast<CoupleProp const*>(i);
        res |= p->fast_diffusion;
        last = std::max(last, p->number());
    }
    
    if ( res )
        uniLists.resize(last+1);
    
    return res;
}


/**
 Transfer free complex that fast-diffuse to the reserve lists
*/
void CoupleSet::uniCollect()
{
    Couple * obj = firstFF(), * nxt;
    while ( obj )
    {
        nxt = obj->next();
        CoupleProp const* p = static_cast<CoupleProp const*>(obj->property());
        if ( p->fast_diffusion )
        {
            unlink(obj);
            assert_true((size_t)p->number() < uniLists.size());
            uniLists[p->number()].push_back(obj);
        }
        obj = nxt;
    }
}


/**
 empty uniLists, reversing all Couples in the normal lists.
 This is useful if ( couple:fast_diffusion == true )
 */
void CoupleSet::uniRelax()
{
    for ( CoupleReserveList & reserve : uniLists )
    {
        for ( Couple * c : reserve )
        {
            assert_true(!c->attached1() && !c->attached2());
            c->randomizePosition();
            link(c);
        }
        reserve.clear();
    }
}


//------------------------------------------------------------------------------
# pragma mark - Equilibrate


void CoupleSet::equilibrateSym(FiberSet const& fibers, CoupleReserveList& reserve, CoupleProp const* cop)
{
    if ( cop->hand1_prop != cop->hand2_prop )
        throw InvalidParameter("Cannot equilibrate heterogeneous Couple");
    
    if ( cop->trans_activated )
        throw InvalidParameter("Cannot equilibrate trans_activated Couple");

    const real space_volume = cop->spaceVolume();
    const real total_length = fibers.totalLength();

    const real binding_rate = cop->hand1_prop->binding_rate;
    const real binding_range = cop->hand1_prop->binding_range;
    const real unbinding_rate = cop->hand1_prop->unbinding_rate;
    
    // get all crosspoints:
    Array<FiberSite> loc1(1024), loc2(1024);
    fibers.allIntersections(loc1, loc2, binding_range);
    const size_t nb_crossings = loc1.size();
    //const real nb_crossings = square(total_length) / ( M_PI * space_volume );

    const real ratio_fibs = 2 * total_length * binding_range / space_volume;
    const real ratio_cros = 4 * M_PI * nb_crossings * square(binding_range) / space_volume;
    
    real bind = binding_rate / unbinding_rate;
    real BsG = bind / 2;
    real AsF = ( ratio_fibs - ratio_cros ) * bind;
    real GsF = ratio_cros * bind;
    
    real popF = reserve.size() / ( 1 + AsF + GsF + BsG * GsF );
    real popA = AsF * popF;
    real popG = GsF * popF;
    real popB = BsG * popG;
    
#if ( 0 )
    printf("Couple::equilibrate %s (sym):\n", cop->name_str());
    printf("     total %lu\n", reserve.size());
    const real nb_fiber = fibers.size();
    const real fiber_length = total_length / nb_fiber;
    const real nbc = nb_fiber * ( nb_fiber - 1 ) * square(fiber_length) / ( M_PI * space_volume );
    //const real nbc = square(total_length) / ( M_PI * space_volume );
    printf("     nb_crossings predicted  %9.2f   true %9i\n", nbc, nb_crossings);
    printf("     F %9.2f A %9.2f G %9.2f B %9.2f\n", popF, popA, popG, popB);
#endif
    
    // create doubly-attached Couples at the crossing positions:
    uniAttach12(loc1, loc2, reserve, RNG.poisson(popB));
    
    real dis = 2 * total_length / ( popA + popG );

    if ( !reserve.empty() )
    {
        fibers.uniFiberSites(loc1, dis);
        uniAttach1(loc1, reserve);
    }
    
    if ( !reserve.empty() )
    {
        fibers.uniFiberSites(loc2, dis);
        uniAttach2(loc2, reserve);
    }
}


/**
 This attempts to create a configuration of Couple that is close to the equilibrium
 that would be reached, after a sufficient time is given for binding and unbinding.
 This assumes that the configuration of filaments does not change, and also that
 it is random, in particular without bundles. The motion of the motor is also ignored.
 */
void CoupleSet::equilibrate(FiberSet const& fibers, CoupleReserveList& reserve, CoupleProp const* cop)
{
    if ( cop->trans_activated )
        throw InvalidParameter("Cannot equilibrate trans_activated Couple");
    
    const real space_volume = cop->spaceVolume();
    const real total_length = fibers.totalLength();

    const real binding_rate1 = cop->hand1_prop->binding_rate;
    const real binding_range1 = cop->hand1_prop->binding_range;
    const real unbinding_rate1 = cop->hand1_prop->unbinding_rate;

    const real binding_rate2 = cop->hand2_prop->binding_rate;
    const real binding_range2 = cop->hand2_prop->binding_range;
    const real unbinding_rate2 = cop->hand2_prop->unbinding_rate;

    
    // get all crosspoints:
    Array<FiberSite> loc1(1024), loc2(1024);
    fibers.allIntersections(loc1, loc2, std::max(binding_range1, binding_range2));
    const size_t nb_crossings = loc1.size();
    
    const real ratio_fibs1 = 2 * total_length * binding_range1 / space_volume;
    const real ratio_fibs2 = 2 * total_length * binding_range2 / space_volume;
    const real ratio_cros1 = 4 * M_PI * nb_crossings * square(binding_range1) / space_volume;
    const real ratio_cros2 = 4 * M_PI * nb_crossings * square(binding_range2) / space_volume;
    
    real BsG1 = binding_rate1 / unbinding_rate1;
    real BsG2 = binding_rate2 / unbinding_rate2;
    real A1sF = ( ratio_fibs1 - ratio_cros1 ) * BsG1 / 2;
    real A2sF = ( ratio_fibs2 - ratio_cros2 ) * BsG2 / 2;
    real G1sF = ratio_cros1 * BsG1 / 2;
    real G2sF = ratio_cros2 * BsG2 / 2;
    
    // the two should be equal
    real BsF = 0.5 * ( BsG1 * G1sF + BsG2 * G2sF );

    real popF = reserve.size() / ( 1.0 + A1sF + A2sF + G1sF + G2sF + BsF );
    real popA1 = A1sF * popF;
    real popA2 = A2sF * popF;
    real popG1 = G1sF * popF;
    real popG2 = G2sF * popF;
    real popB = BsF * popF;

#if ( 0 ) && ( DIM == 2 )
    printf("Couple::equilibrate %s:\n", cop->name_str());
    printf("     total %lu\n", reserve.size());
    const real nb_fiber = fibers.size();
    const real fiber_length = total_length / nb_fiber;
    const real nbc = nb_fiber * ( nb_fiber - 1 ) * square(fiber_length) / ( M_PI * space_volume );
    //const real nbc = square(total_length) / ( M_PI * space_volume );
    printf("     nb_crossings predicted  %9.2f   true %9i\n", nbc, nb_crossings);
    printf("     F %9.2f A %9.2f G %9.2f B %9.2f\n", popF, popA1+popA2, popG1+popG2, popB);
#endif

    // create doubly-attached Couples at the crossing positions:
    uniAttach12(loc1, loc2, reserve, RNG.poisson(popB));
    
    if ( !reserve.empty() )
    {
        const real dis = total_length / ( popA1 + popG1 );
        fibers.uniFiberSites(loc1, dis);
        uniAttach1(loc1, reserve);
    }
    
    if ( !reserve.empty() )
    {
        const real dis = total_length / ( popA2 + popG2 );
        fibers.uniFiberSites(loc2, dis);
        uniAttach2(loc2, reserve);
    }
}

/**
Distributes Couples for which `trans_activated==true` on the filaments
*/
void CoupleSet::equilibrate(FiberSet const& fibers, PropertyList const& properties)
{
    for ( Property * i : properties.find_all("couple") )
    {
        CoupleProp * cop = static_cast<CoupleProp *>(i);
        cop->complete(simul);
        
        if ( !cop->trans_activated )
        {
            CoupleReserveList list;
            
            // collect all Couple of this kind:
            Couple * c = firstFF(), * nxt;
            while ( c )
            {
                nxt = c->next();
                if ( c->property() == cop )
                {
                    unlink(c);
                    list.push_back(c);
                }
                c = nxt;
            }
            if ( list.size() > 0 )
            {
                equilibrate(fibers, list, cop);
                
                // release all collected Couple
                for ( Couple * cx : list )
                    link(cx);
                list.clear();
            }
        }
    }
    printf("Couple::equilibrate    FF %lu FA %lu AF %lu AA %lu\n", sizeFF(), sizeFA(), sizeAF(), sizeAA());
}


/**
 This takes all the Free Couple and attach them at the intersection points of the network of filaments
 */
void CoupleSet::bindToIntersections(FiberSet const& fibers, PropertyList const& properties)
{
    // calculate maximum range of Hands
    real range = 0;
    for ( Property * i : properties.find_all("couple") )
    {
        CoupleProp const* cop = static_cast<CoupleProp const*>(i);
        range = std::max(range, cop->hand1_prop->binding_range);
        range = std::max(range, cop->hand2_prop->binding_range);
    }
    
    if ( range <= 0 )
        throw InvalidParameter("cannot connect network!");
    
    // get all crosspoints within this range:
    Array<FiberSite> loc1(1024), loc2(1024);
    fibers.allIntersections(loc1, loc2, range);
    const size_t nbc = loc1.size();
    assert_true(nbc == loc2.size());
    
    //std::clog << nbc << " intersections at range " << range << "\n";

    if ( nbc > 0 )
    {
        Couple * c = firstFF(), * nxt;
        while ( c )
        {
            nxt = c->next();
            size_t p = RNG.pint32(nbc);
            c->attach1(loc1[p]);
            c->attach2(loc2[p]);
            c = nxt;
        }
    }
}


