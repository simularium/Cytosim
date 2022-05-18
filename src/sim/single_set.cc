// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "single_set.h"
#include "single_prop.h"
#include "glossary.h"
#include "iowrapper.h"

#include "simul.h"
#include "wrist.h"
#include "wrist_long.h"

//------------------------------------------------------------------------------
/**
 @copydetails SingleGroup
 */
Property* SingleSet::newProperty(const std::string& cat, const std::string& nom, Glossary& opt) const
{
    if ( cat == "single" )
        return new SingleProp(nom);
    else
        return nullptr;
}

//------------------------------------------------------------------------------

void SingleSet::prepare(PropertyList const& properties)
{
    uni = uniPrepare(properties);
}


void SingleSet::step()
{
    // use alternate attachment strategy:
    if ( uni )
        uniAttach(simul.fibers);

    /*
     ATTENTION: we have multiple lists, and Objects are automatically transfered
     from one list to another if their Hands bind or unbind. We ensure here that
     step() is called exactly once for each object. THe code relies on the fact
     that a transfered node would be linked at the start of the new list.
     We start always at the node, which was first before any transfer could occur.
     */
    
    //Cytosim::log("SingleSet::step entry : F %5i A %5i\n", fList.size(), aList.size());
    
    
    Single *const fHead = firstF();
    Single * obj, * nxt;
    
    obj = firstA();
    while ( obj )
    {
        nxt = obj->next();
        obj->stepA();
        if ( ! nxt ) break;
        obj = nxt->next();
        nxt->stepA();
    }
    
    obj = fHead;
    while ( obj )
    {
        nxt = obj->next();
        obj->stepF(simul);
        if ( ! nxt ) break;
        obj = nxt->next();
        nxt->stepF(simul);
    }
}


//------------------------------------------------------------------------------
#pragma mark -


Object * SingleSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Single::TAG )
    {
        SingleProp * p = simul.findProperty<SingleProp>("single", num);
        return p->newSingle();
    }
    else if ( tag == Wrist::TAG )
    {
        SingleProp * p = simul.findProperty<SingleProp>("single", num);
        return p->newWrist(nullptr, 0);
    }
    return nullptr;
}

/**
 @addtogroup SingleGroup
 
 A newly created Single can be anchored to a Mecable:
 
     new NAME {
       base = OBJECT, POINT
     }
 
 
 where:
 - OBJECT is the concatenation of the class name with the serial number of the object:
     - 'bead1' for the first bead
     - 'bead2' for the second...
     - 'bead0' designates the last bead made,
     - 'bead-1' is the penultimate one, etc.
     .
 - POINT designates a point on this object:
     - point1 = first point
     - point2 = second point...
     .
 .

 You can attach a Single to a fiber:
 
     new simplex
     {
        attach = FIBER, REAL, REFERENCE
     }
 
 where:
 - FIBER designates the fiber:
     - `fiber1` of `fiber2` correspond to fibers directly
     - `first` or `last` to the oldest and youngest fiber
     - `fiber-1` the penultimate, etc.
     .
 - REAL is the abscissa of the attachment point.
   If the abscissa is not specified, and random position along
   along the fiber will be selected.
 - REFERENCE can be `minus_end`, `center` or `plus_end` (default = `origin`).
   This defines from which position the abscissa is measured.
 .
 */
ObjectList SingleSet::newObjects(const std::string& name, Glossary& opt)
{
    SingleProp * p = simul.findProperty<SingleProp>("single", name);
    
    Single * obj = nullptr;
    std::string str;
    if ( opt.set(str, "base") )
    {
        Mecable * mec = simul.findMecable(str);
        if ( !mec )
            throw InvalidParameter("Could not find Mecable specified in single:base");
        // get index of point in second argument
        unsigned ip = 0;
        if ( opt.set(str, "base", 1) )
            ip = Mecable::point_index(str, mec->nbPoints());
         
        obj = p->newWrist(mec, ip);
    }
    else
        obj = p->newSingle();

    // Allow user to attach Hand to an existing fiber
    if ( opt.has_key("attach") )
        obj->attach(simul.fibers.someSite("attach", opt));
    
    // Allow user to attach Hand to an existing fiber
    if ( opt.has_key("site") )
        obj->attach(simul.fibers.someSite("site", opt));

    ObjectList res;
    res.push_back(obj);
    return res;
}


//------------------------------------------------------------------------------
#pragma mark -


void SingleSet::relinkA(Single * obj)
{
    fList.pop(obj);
    aList.push_front(obj);
}


void SingleSet::relinkD(Single * obj)
{
    aList.pop(obj);
    fList.push_front(obj);
}


void SingleSet::link(Object * obj)
{
    assert_true( !obj->objset() );
    assert_true( obj->tag()==Single::TAG || obj->tag()==Wrist::TAG );

    obj->objset(this);

    if ( static_cast<Single*>(obj)->attached() )
        aList.push_front(obj);
    else
        fList.push_front(obj);
}

/**
 This will also detach the Hand
 */
void SingleSet::unlink(Object * obj)
{
    assert_true( obj->objset() == this );
    
    Single * s = static_cast<Single*>(obj);
  
    if ( s->attached() )
        s->detach();
    
    obj->objset(nullptr);

    if ( s->attached() )
        aList.pop(obj);
    else
        fList.pop(obj);
}


void SingleSet::foldPositions(Modulo const* m) const
{
    Single * obj;
    for ( obj=firstF(); obj; obj=obj->next() ) obj->foldPosition(m);
    for ( obj=firstA(); obj; obj=obj->next() ) obj->foldPosition(m);
}


void SingleSet::shuffle()
{
    aList.shuffle();
    fList.shuffle();
}


void SingleSet::erase()
{
    relax();
    ObjectSet::erase(fList);
    ObjectSet::erase(aList);
    inventory.clear();
}


void SingleSet::freeze(ObjectFlag f)
{
    relax();
    ObjectSet::flag(aList, f);
    ObjectSet::flag(fList, f);
}


void SingleSet::deleteA(Single * s)
{
    s->hand()->detachHand();
    inventory.unassign(s);
    s->objset(nullptr);
    aList.pop(s);
    delete(s);
}


void SingleSet::prune(ObjectFlag f)
{
    /* After reading from file, the Hands should not
     update any Fiber, Single or Couple as they will be deleted */
    for (Single* s=firstA(), *n; s; s=n)
    {
        n = s->next();
        if ( s->flag() == f )
            deleteA(s);
    }

    //ObjectSet::prune(aList, f, 0);
    ObjectSet::prune(fList, f, 0);
}


void SingleSet::thaw()
{
    ObjectSet::flag(aList, 0);
    ObjectSet::flag(fList, 0);
}


void SingleSet::write(Outputter& out) const
{
    if ( sizeA() > 0 )
    {
        out.put_line("\n#section single A", out.binary());
        writeNodes(out, aList);
    }
    if ( sizeF() > 0 )
    {
        out.put_line("\n#section single F", out.binary());
        writeNodes(out, fList);
    }
}


int SingleSet::bad() const
{
    int code = 0;
    Single * ghi;
    code = fList.bad();
    if ( code ) return code;
    for ( ghi = firstF(); ghi ; ghi=ghi->next() )
    {
        if ( ghi->attached() ) return 100;
    }
    
    code = aList.bad();
    if ( !code )
    {
        for ( ghi = firstA();  ghi ; ghi=ghi->next() )
            if ( !ghi->attached() )
                return 101;
    }
    
    return code;
}

//------------------------------------------------------------------------------
#pragma mark -


void SingleSet::report(std::ostream& os) const
{
    if ( size() > 0 )
    {
        os << '\n' << title();
        PropertyList plist = simul.properties.find_all(title());
        for ( Property * i : plist )
        {
            SingleProp * p = static_cast<SingleProp*>(i);
            unsigned cnt = count(match_property, p);
            os << '\n' << std::setw(10) << cnt << ' ' << p->name();
            os << " ( " << p->hand << " )";
        }
        if ( plist.size() > 1 )
            os << '\n' << std::setw(10) << size() << " total";
    }
}


ObjectList SingleSet::collect() const
{
    ObjectList res = ObjectSet::collect(fList);
    res.append( ObjectSet::collect(aList) );
    return res;
}


ObjectList SingleSet::collect(bool (*func)(Object const*, void const*), void const* arg) const
{
    ObjectList res = ObjectSet::collect(fList, func, arg);
    res.append( ObjectSet::collect(aList, func, arg) );
    return res;
}


unsigned SingleSet::count(bool (*func)(Object const*, void const*), void const* arg) const
{
    unsigned f = ObjectSet::count(fList, func, arg);
    unsigned a = ObjectSet::count(aList, func, arg);
    return f + a;
}

//------------------------------------------------------------------------------
#pragma mark - Wrists

/**
 This will create Wrists with `obj` as Base, following the specifications given in `arg`.
 These Wrists will be anchored on points `fip` to `fip+nbp-1` of `obj`.
 
 The syntax understood for `arg` is as follows:

     [INTEGER] NAME_OF_SINGLE [each]

 The first optional integer specifies the number of Singles to be attached.
 Then follows the name of the Single to be created.
 If 'each' is specified, this number is multiplied by the number of point `nbp`,
 and every point receives the same number of Singles.
 
 This is used to decorate Solid and Sphere
 */
ObjectList SingleSet::makeWrists(Mecable const* obj, unsigned fip, unsigned nbp, std::string& arg)
{
    ObjectList res;
    unsigned num = 1;

    std::istringstream iss(arg);
    iss >> num;
    
    if ( iss.fail() )
    {
        num = 1;
        iss.clear();
    }
    
    if ( num == 0 || nbp == 0 )
        return res;
    
    std::string str, mod;
    iss >> str >> mod;
    
    SingleProp * sip = simul.findProperty<SingleProp>("single", str);

    if ( mod == "each" )
    {
        for ( unsigned u = 0; u < num; ++u )
        {
            for ( unsigned i = 0; i < nbp; ++i )
                res.push_back(sip->newWrist(obj, fip+i));
        }
    }
    else
    {
        for ( unsigned u = 0; u < num; ++u )
        {
            res.push_back(sip->newWrist(obj, fip+RNG.pint32(nbp)));
        }
    }
    
    return res;
}


SingleList SingleSet::collectWrists(Object const* arg) const
{
    SingleList res;
    
    for ( Single * s=firstF(); s; s=s->next() )
        if ( s->base() == arg )
            res.push_back(s);
    
    for ( Single * s=firstA(); s; s=s->next() )
        if ( s->base() == arg )
            res.push_back(s);
    
    return res;
}


void SingleSet::removeWrists(Object const* arg)
{
    Single *nxt, *obj;
    
    obj = firstF();
    while ( obj )
    {
        nxt = obj->next();
        if ( obj->base() == arg )
            delete(obj);
        obj = nxt;
    }

    obj = firstA();
    while ( obj )
    {
        nxt = obj->next();
        if ( obj->base() == arg )
            delete(obj);
        obj = nxt;
    }
}


//------------------------------------------------------------------------------
#pragma mark - Fast Diffusion


/**
 Distribute Singles on the sites specified in `loc`.
 */
void SingleSet::uniAttach(Array<FiberSite>& loc, SingleReserveList& can)
{
    for ( FiberSite & i : loc )
    {
        if ( can.empty() )
            return;
        Single * s = can.back();
        Hand const* h = s->hand();
        
        if ( h->attachmentAllowed(i) )
        {
            Vector pos = i.pos();
            
            if ( s->prop->confine == CONFINE_ON )
            {
                // Only attach if position is near the edge of the Space:
                Vector prj = s->confineSpace()->project(pos);
                if ( distanceSqr(pos, prj) >= square(h->prop->binding_range) )
                    continue;
                // Single will be placed on the edge of the Space:
                pos = prj;
            }
            else
            {
#if ( DIM > 1 )
                /*
                 Place the Single in the line perpendicular to the attachment point,
                 at a random distance within the range of attachment of the Hand.
                 This simulates a uniform spatial distribution of Single.
                 */
                pos += i.dirFiber().randOrthoB(h->prop->binding_range);
#endif
            }

            can.pop_back();
            s->setPosition(pos);
            s->attach(i);
            link(s);
        }
    }
}


/**
 Implements a Monte-Carlo approach for attachments of free Single, assumming that
 diffusion is sufficiently fast to maintain a uniform spatial distribution,
 and that the distribution of fibers is more-or-less uniform such that the
 attachments are distributed randomly along the fibers.
 
 Diffusing (free) Single are removed from the standard list, and thus the
 random walk that is used for simulating diffusion will be skipped,
 as well as the detection of neighboring fibers done for attachments.
 
 Algorithm:
 - Remove diffusing Single from the simulation, transfering them to a 'reserve'.
 - Estimate the distance between binding sites occuring in one time-step, from:
    - the total length of fibers,
    - the volume of the Space,
    - the binding parameters of the relevant Hand.
    .
 - Attach Singles from the reserve, at random positions along the Fibers
 .
 
 Note: there is a similar feature for Couple
 */
void SingleSet::uniAttach(FiberSet const& fibers)
{
    // transfer free Single that fast-diffuse to the reserve
    Single * obj = firstF(), * nxt;
    while ( obj )
    {
        nxt = obj->next();
        SingleProp const* p = obj->prop;
        if ( p->fast_diffusion )
        {
            unlink(obj);
            assert_true((size_t)p->number() < uniLists.size());
            uniLists[p->number()].push_back(obj);
        }
        obj = nxt;
    }
    
    Array<FiberSite> loc(1024);
    
    // uniform attachment for the reserves:
    for ( SingleReserveList & reserve : uniLists )
    {
        if ( !reserve.empty() )
        {
            SingleProp const* p = reserve.back()->prop;
            
            const real vol = p->spaceVolume();
            const size_t cnt = reserve.size();
            
            if ( p->fast_diffusion == 2 )
            {
                real dis = vol / ( cnt * p->hand_prop->bindingSectionRate() );
                fibers.newFiberSitesP(loc, dis);
            }
            else
            {
                real dis = vol / ( cnt * p->hand_prop->bindingSectionProb() );
                fibers.uniFiberSites(loc, dis);
            }
            
            uniAttach(loc, reserve);
        }
    }
}


/**
 
 Return true if at least one single:fast_diffusion is true,
 and in this case allocate uniLists.
 
 The Volume of the Space is assumed to remain constant until the next uniPrepare()
 */
bool SingleSet::uniPrepare(PropertyList const& properties)
{
    bool res = false;
    unsigned last = 0;

    for ( Property const* i : properties.find_all("single") )
    {
        SingleProp const* p = static_cast<SingleProp const*>(i);
        res |= p->fast_diffusion;
        last = std::max(last, p->number());
    }
    
    if ( res )
        uniLists.resize(last+1);
    
    return res;
}


/**
 empty uniLists, reversing all Singles in the normal lists.
 This is useful if ( single:fast_diffusion == true )
 */
void SingleSet::uniRelax()
{
    for ( SingleReserveList & reserve : uniLists )
    {
        for ( Single * s : reserve )
        {
            assert_true(!s->attached());
            s->randomizePosition();
            link(s);
        }
        reserve.clear();
    }
}

