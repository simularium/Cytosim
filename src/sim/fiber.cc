// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <algorithm>

#include "fiber.h"
#include "field.h"
#include "messages.h"
#include "glossary.h"
#include "iowrapper.h"
#include "fiber_segment.h"
#include "fiber_prop.h"
#include "object_set.h"
#include "single.h"
#include "simul.h"
#include "meca.h"
#include "hand.h"
#include "sim.h"


#pragma mark - Step


void Fiber::step()
{
    assert_small( length1() - length() );

    //add single that act like glue
    if ( prop->glue )
    {
        setGlue(frGlue, PLUS_END, prop->confine_space_ptr);
    }
    
    
#if ( 0 )
    // Cut kinked filaments
    unsigned p = hasKink(0);
    if ( p )
    {
        LOG_ONCE("SEVER_KINKED_FIBERS\n");
        objset()->add(severPoint(p));
    }
#endif
#if ( 0 )
    // Delete kinked filaments
    if ( hasKink(0) )
    {
        LOG_ONCE("DELETE_KINKED_FIBERS\n");
        delete(this);
        return;
    }
#endif

    // perform the cuts that were registered by sever()
    if ( pendingCuts.size() )
        severNow();
    
    // delete self if shorter than 'FiberProp::min_length'
    if ( length() < prop->min_length && ! prop->persistent )
    {
        delete(this);
        return;
    }
    
    if ( needUpdate )
    {
        adjustSegmentation();
        updateFiber();
    }
}


//------------------------------------------------------------------------------

/**
 The rest of the initialization is done in FiberProp::newFiber(),
 and other newFiber() functions where the initial length is known.
 */
Fiber::Fiber(FiberProp const* p)
: handListFront(nullptr), handListBack(nullptr), frGlue(nullptr), prop(p), disp(nullptr)
{
    if ( prop )
    {
        targetSegmentation(prop->segmentation);
        if ( prop->lattice )
        {
#if FIBER_HAS_LATTICE
            //Cytosim::log << reference() <<  " new Lattice" << std::endl;
            frLattice.setUnit(prop->lattice_unit);
#else
            //throw InvalidParameter("Cytosim does not support fiber:lattice");
#endif
        }
    }
}


Fiber::~Fiber()
{
    detachHands();

    delete(frGlue);
    frGlue = nullptr;
    
    if ( disp )
    {
        /*
         Note: the destructor will not be called here, which is OK
         if LineDisp is a trivial type that does not allocate resources
         */
        free(disp);
        disp = nullptr;
    }
    
    prop = nullptr;
}


real Fiber::projectPoint(Vector const& w, real & dis) const
{
    // initialize with the minus-end:
    dis = distanceSqr(w, posP(0));
    real abs = 0, len = segmentation();
    
    // try all segments
    for ( unsigned int ii = 0; ii < nbSegments(); ++ii )
    {
        //check the segment:
        FiberSegment s(this, ii);
        real d = INFINITY;
        real a = s.projectPoint0(w, d);
        if ( len < a )
        {
            // test exact point
            real e = distanceSqr(w, posP(ii+1));
            if ( e < dis ) {
                abs = abscissaPoint(ii+1);
                dis = e;
            }
        }
        else if ( 0 <= a  &&  d < dis )
        {
            //the projection is the best found so far
            abs = abscissaPoint(ii) + a;
            dis = d;
        }
    }
    
    return abs;
}

//------------------------------------------------------------------------------
#pragma mark - Modifying

void Fiber::flipHandsPolarity()
{
    /* update abscissa of Hands to keep them in place:
     new_abs - minus = plus - old_abs
     new_abs = plus + minus - old_abs
     */
    real mid = abscissaM() + abscissaP();
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        ha->moveTo(mid-ha->abscissa());
        ha = nx;
    }
}


/**
 A portion of size `len` that includes the MINUS_END is removed.
 The Hands bound within the deleted portion are detached.
 */
void Fiber::cutM(real len)
{
    const real abs = abscissaM() + len;
    
    Chain::cutM(len);
    
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        if ( ha->abscissa() < abs )
            ha->detach();
        ha = nx;
    }
}


/**
 A portion of size `len` that includes the PLUS_END is removed.
 The Hands bound within the deleted portion are detached.
 */
void Fiber::cutP(real len)
{
    const real abs = abscissaP() - len;
    
    Chain::cutP(len);
    
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        if ( ha->abscissa() > abs )
            ha->detach();
        ha = nx;
    }
}


/**
 The Fiber is cut at point P
 - A new Fiber is created from the section [ P , PLUS_END ],
 - all FiberSite attached to this section are transferred to the new Fiber,
 - Lattice content is also transferred,
 - a pointer to the new Fiber is returned, which should be added to the Simul
 .
 @return zero, if `pti` is not an internal point
 */
Fiber* Fiber::severPoint(unsigned int pti)
{
    if ( pti == 0  ||  pti >= lastPoint() )
        return nullptr;
    
    const real abs = abscissaPoint(pti);

    // create a new Fiber of the same class:
    Fiber* fib = prop->newFiber();
    assert_true( fib->prop == prop );
    
    // copy the vertices:
    *(static_cast<Chain*>(fib)) = *this;
    
    // the signature on both pieces should be conserved:
    fib->signature(signature());
    fib->birthTime(birthTime());

    assert_true( fib->abscissaM() == abscissaM() );
    // remove MINUS_END portion on new piece:
    fib->truncateM(pti);
    assert_true(fib->abscissaM() == abs);

    // remove PLUS_END portion on self
    truncateP(pti);
    
    // transfer Hands above point P, at same abscissa
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        if ( ha->abscissa() > abs )
            ha->relocate(fib);
        else
            ha->reinterpolate();
        ha = nx;
    }
    
    resetLattice();
    fib->resetLattice();
    
    return fib;
}


/**
The Fiber is cut at distance `abs` from its MINUS_END:
 - current Fiber is truncated to keep only the section [ MINUS_END , abs ],
 - A new Fiber is created representing the other section [ abs , PLUS_END ],
 - Hands are transfered to the new Fiber if appropriate,
 - lattice substances are also transfered,
 .
 A pointer to the new Fiber is returned (containing the PLUS_END), but this
 pointer may be zero, if `abs` was not within the valid range of abscissa.
 If a new Fiber was created, it should be added to the FiberSet.
 */
Fiber* Fiber::severM(real abs)
{
    if ( abs <= REAL_EPSILON || abs + REAL_EPSILON >= length() )
        return nullptr;
    
    //std::clog << "severM " << reference() << " at " << abscissaM()+abs << "\n";

    // create a new Fiber of the same class:
    Fiber* fib = prop->newFiber();
    assert_true( fib->prop == prop );

    // copy the vertices:
    *(static_cast<Chain*>(fib)) = *this;
    *(static_cast<Object*>(fib)) = *this;

    // the signature on both pieces should be conserved:
    assert_true(fib->signature() == signature());
    assert_true(fib->birthTime() == birthTime());

    assert_small(fib->abscissaM() - abscissaM());
    // remove MINUS_END portion on new piece
    fib->Chain::cutM(abs);
    
    assert_small(fib->abscissaM()-abs-abscissaM());
    
    // remove PLUS_END portion on self
    Chain::cutP(length()-abs);
    
    assert_small(fib->abscissaM()-abscissaP());

    // transfer all Hands above cut to new piece
    // their abscissa should not change in this transfer
    abs += abscissaM();
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        if ( ha->abscissa() >= abs )
            ha->relocate(fib);
        else
            ha->reinterpolate();
        ha = nx;
    }

    resetLattice();
    fib->resetLattice();

    return fib;
}


/**
 Perform all cuts registered in `pendingCuts`, and clear that list.
 This deletes Fibers that are shorter than FiberProp::min_length
*/
void Fiber::severNow()
{
    /**
     The std::set keeps its objects always in order of descending abscissa,
     which is essential here to avoid data loss:
     cut from high to low abscissa
     */
    for ( SeverPos const& cut : pendingCuts )
    {
        if ( cut.abs - abscissaM() <= prop->min_length )
        {
            // we check the range again, since the fiber tip may have changed:
            if ( cut.abs > abscissaM() )
            {
                cutM(cut.abs-abscissaM());
                setDynamicStateM(cut.stateM);
            }
            /*
             since we have deleted the MINUS_END section,
             the following cuts in the list, which will be of lower abscissa,
             should not be processed.
             */
            break;
        }
        else if ( abscissaP() - cut.abs <= prop->min_length )
        {
            // we check the range again, since the fiber tip may have changed:
            if ( cut.abs < abscissaP() )
            {
                cutP(abscissaP()-cut.abs);
                setDynamicStateP(cut.stateP);
            }
        }
        else
        {
            Fiber * frag = severM(cut.abs-abscissaM());
            
            // special case where the PLUS_END section is simply deleted
            if ( cut.stateM == STATE_BLACK )
            {
                delete(frag);
                continue;
            }

            //add new fragment to simulation:
            objset()->add(frag);

            if ( frag )
            {
                // check that ends spatially match:
                assert_small((frag->posEndM() - posEndP()).norm());
                
                try {
                    // old PLUS_END converves its state:
                    frag->setDynamicStateP(dynamicStateP());
                    
                    // new ends are set as wished:
                    this->setDynamicStateP(cut.stateP);
                    frag->setDynamicStateM(cut.stateM);
                }
                catch ( Exception & e )
                {
                    e << "while cutting fiber " << reference();
                    throw;
                }
            
#ifdef LOGGING
                Cytosim::log << "severed " << reference() << " at abscissa " << s->abs;
                Cytosim::log << "   creating " << frag->reference();
                Cytosim::log << "   position " << frag->posEndM() << std::endl;
#endif
                //Cytosim::log << " severed at X = " << frag->posEndM().XX << std::endl;
            }
            else
            {
                Cytosim::log << " sever abscissa " << cut.abs << " is out of range";
                Cytosim::log << " [ " << abscissaM() << "   " << abscissaP() << " ]" << std::endl;
            }
        }
    }
    pendingCuts.clear();
}


/**
 returns index of first point for which ( cos(angle) < max_cosine ),
 or zero
 */
unsigned Fiber::hasKink(const real max_cosine) const
{
    unsigned end = nPoints - 2;
    for ( unsigned p = 0; p < end; ++p )
    {
        if ( dot(diffPoints(p), diffPoints(p+1)) < max_cosine )
            return p+1;
    }
    return 0;
}


void Fiber::planarCut(Vector const& n, const real a, state_t stateP, state_t stateM)
{
    Array<real> cuts;
    
    /*
     The cuts should be processed in order of decreasing abscissa,
     hence we check intersections from PLUS_END to MINUS_END
    */
    for ( int s = lastSegment(); s >=0 ; --s )
    {
        real abs = planarIntersect(s, n, a);
        if ( 0 <= abs  &&  abs < 1 )
            cuts.push_back(abscissaPoint(s+abs));
    }
    
    for ( real abs : cuts )
    {
        Fiber * fib = severM(abs-abscissaM());
        if ( fib )
        {
            // old PLUS_END converves its state:
            fib->setDynamicStateP(dynamicStateP());
            // dynamic of new ends are set as usual:
            setDynamicStateP(stateP);
            fib->setDynamicStateM(stateM);
            //assert_true(!fib->linked());
            objset()->add(fib);
        }
    }
}


/**
 The given `fib` is added past the PLUS_END of `*this`,
 Hands bound to `fib` are transfered to *this.
 The dynamic state of the PLUS_END is also transferred.
 `fib` is enventually deleted
*/
void Fiber::join(Fiber * fib)
{
    assert_true( fib );
    // the two fibers should be of the same class:
    assert_true( prop == fib->prop );
    
    // shift in abscissa must be calculated before joining
    real shift = abscissaP() - fib->abscissaM();

    // join backbones
    Chain::join(fib);
    
    //transfer dynamic state of PLUS_END:
    setDynamicStateP(fib->dynamicStateP());

    // transfer all Hands
    Hand * ha = fib->handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        ha->relocate(this, ha->abscissa()+shift);
        ha = nx;
    }
    delete(fib);

    resetLattice();
}


//------------------------------------------------------------------------------
#pragma mark - Mobility

/**
 From "Random Walks in Biology" by HC. Berg, Princeton University Press,
 drag coefficients for an ellipsoid are,

     drag_transverse = 2*drag_parallel = 4*PI*length*visc / log(length/radius)

 We should average the mobility coefficients:  speed = mu * f
     mu_X = mu_parallel   = 2 * mu
     mu_Y = mu_transverse = mu
     mu_Z = mu_transverse = mu
 Hence:
     mu_averaged = ( mu + mu + 2*mu ) / 3 = 4/3 * mu.
 drag_averaged = 3*PI*length*viscosity / log(length/radius)

APPROXIMATE FORMULA FOR ELLIPSOIDAL PARTICLE
 > Clift R, Grace JR, Weber ME. Bubbles, drops, and particles
 > Courier Corporation; 2005.

     aspect = length / diameter;
     drag = 3.0 * M_PI * viscosity * diameter * ( 3 + 2 * length/diameter ) / 5.0;

 */
real Fiber::dragCoefficientEllipsoid(const real len, FiberProp const* prop)
{
    // Stokes' law for a sphere having diameter of filament:
    assert_true( prop->drag_radius > 0 );
    real drag = 6 * prop->drag_radius;

    assert_true( len > REAL_EPSILON );
    assert_true( prop->drag_length > REAL_EPSILON );
    // hydrodynamic cut-off on length:
    const real lenc = std::max(std::min(len, prop->drag_length), prop->drag_radius);

    // drag of 3D ellipsoid
    const real drag_ellipsoid = 3 * len / log( lenc / prop->drag_radius );
    
    // length below which ellipsoid formula is not valid:
    const real min_len = exp( 1 + log(prop->drag_radius) );
    
    if ( len > min_len )
    {
        // use largest drag coefficient
        drag = std::max(drag, drag_ellipsoid);
    }

    assert_true( prop->viscosity > 0 );
    return M_PI * prop->viscosity * drag;
}



/** 
 dragCoefficientCylinder() calculates the mobility for the entire fiber,
 considering that the cylinder is straight and moving in an infinite fluid.
 fiber:drag_length is a hydrodynamic cutoff that makes the
 drag coefficient proportional to length beyond the cutoff.
 
 The drag is determined by the viscosity and the length and diameter of the
 filament. The aspect ratio is defined by:
 
     shape = length / diameter;

 The formula for a cylinder were calculated numerically in:
 > Tirado and de la Torre. J. Chem. Phys 71(6) 1979
 > http://doi.org/10.1063/1.438613
 > Page 2584, Table 1, last column, last line for infinite aspect ratio

 The translational drag coefficient is averaged over all possible configurations:
 
       drag_cylinder = 3 * PI * viscosity * length / ( log(shape) + 0.312 );
 
 If the length is shorter than the diameter, the formula above fails and may
 even give negative result. Hence we also calculate the drag of a sphere with
 the same radius as the cylinder:

       drag_sphere = 6 * PI * viscosity * radius

 We use the maximum value between 'drag_sphere' and 'drag_cylinder'.
 */
/*
 Ct =  0.312 + 0.565/shape - 0.100/(shape*shape);

 The rotational diffusion coefficient is given by:
 > Tirado and de la Torre. J. Chem. Phys 73(4) 1980
 
     Cr = -0.662 + 0.917/aspect - 0.050/(shape*shape);
     drag_rotation = 1/3*M_PI*viscosity*length^3 / ( log(shape) + Cr )

 */
real Fiber::dragCoefficientCylinder(const real len, FiberProp const* prop)
{
    // Stokes' law for a sphere having diameter of filament:
    assert_true( prop->drag_radius > 0 );
    real drag = 6 * prop->drag_radius;

    assert_true( len > REAL_EPSILON );
    assert_true( prop->drag_length > REAL_EPSILON );
    // hydrodynamic cut-off on length:
    const real lenc = std::max(std::min(len, prop->drag_length), prop->drag_radius);
    
    /// drag of a cylinder:
    const real drag_cylinder = 3 * len / ( log(0.5*lenc/prop->drag_radius) + 0.32 );

    // length below which the formula is not valid anymore ( ~ 3.94 * radius )
    // this corresponds to the minimun value of the formula above
    const real min_len = 2 * prop->drag_radius * exp(1.0-0.32);
    
    if ( len > min_len )
    {
        // use largest drag coefficient
        drag = std::max(drag, drag_cylinder);
    }
    
    assert_true( prop->viscosity > 0 );
    return M_PI * prop->viscosity * drag;
}


/**
 dragCoefficientSurface() uses a formula calculated by F. Gittes in:
 > Hunt et al. Biophysical Journal (1994) v 67 pp 766-781  
 > http://dx.doi.org/10.1016/S0006-3495(94)80537-5
 
 It applies to a cylinder moving parallel to its axis and near an immobile surface:

       drag_per_unit_length = 2 &pi &eta / acosh(h/r)
 
 With:
 - r = cylinder radius,
 - h = distance between cylinder bottom and surface,
 - &eta = viscosity of the fluid.
 
 If the cylinder is exactly touching the surface, `h=0` and the drag coefficient is infinite.
 
 The drag coefficient for motion perpendicular to the cylinder axis would be twice higher,
 but for gliding assays, the parallel drag coefficient is the appropriate choice.  
 
 Note that this is usually equivalent to the approximate formula:

       drag_per_unit_length = 2 &pi &eta / log(2*h/r)

 because

       acosh(x) = ln[ x + sqrt(x^2-1)) ] ~ ln[2x] if x >> 1

 Hunt et al. also credit this reference for the formula:
 > The slow motion of a cylinder next to a plane wall.
 > Jeffrey, D.J. & Onishi, Y. (1981) Quant. J. Mech. Appl. Math. 34, 129-137.
*/
real Fiber::dragCoefficientSurface(const real len, FiberProp const* prop)
{
    if ( prop->drag_gap <= 0 )
        throw InvalidParameter("fiber:drag_model[1] (height above surface) must set and > 0!");
    
    // use the higher drag: perpendicular to the cylinder (factor 2)
    real drag = 2 * len / acosh( 1 + prop->drag_gap/prop->drag_radius );

    return M_PI * prop->viscosity * drag;
}


/**
 Calculate drag coefficient by calling one of the other function:

        dragCoefficientSurface();
        dragCoefficientEllipsoid();
        dragCoefficientCylinder();

 */
void Fiber::setDragCoefficient()
{
    const real len = length();
    assert_true( len > 0 );
    
    real drag = 0;
    
    if ( prop->drag_model )
    {
        drag = dragCoefficientSurface(len, prop);
#if ( 0 )
        real d = dragCoefficientCylinder(len, prop);
        Cytosim::log << "Drag coefficient of Fiber near a planar surface amplified by " << drag/d << std::endl;
#endif
    }
    else
        drag = dragCoefficientCylinder(len, prop);

    assert_true( drag > 0 );
    // distribute drag equally to all points, to set point's mobility
    rfPointMobility = nPoints / drag;
    
#if ( 0 )
    std::ostream& os = std::cerr; //Cytosim::log;
    os << "Fiber " << std::setw(16) << prop->name() << " length " << std::setw(7) << length();
    os << " drag " << std::setw(9) << drag << " point_mobility " << rfPointMobility << std::endl;
#endif
}


void Fiber::prepareMecable()
{
    setDragCoefficient();
    storeDirections();
    makeProjection();

    assert_true( rfPointMobility >= 0 );

    // the scaling of the bending elasticity depends on the length of the segments
    rfRigidity = prop->rigidity / segmentationCube();
#if NEW_FIBER_LOOP
    rfRigidityLoop = prop->loop;
#endif
#if ( 0 )
    real energy = bendingEnergy();
    real euler = M_PI * M_PI * prop->rigidity / ( length() * length() );
    Cytosim::log << "Euler buckling = " << euler << "    ";
    Cytosim::log << "Bending energy = " << energy << std::endl;
#endif
}


//------------------------------------------------------------------------------

void Fiber::setInteractions(Meca & meca) const
{
#if OLD_SQUEEZE_FORCE
    if ( prop->squeeze == 1 )
    {
        // squeezing force in the YZ-plane:
        const real f = prop->squeeze_force;
        const real r = prop->squeeze_range;
        for ( unsigned pp = 0; pp < nPoints; ++pp )
        {
#if ( DIM == 2 )
            unsigned ii = DIM * ( matIndex() + pp ) + 1;
            real p = posP(pp).YY;
            if ( fabs(p) > r )
                meca.base(ii) -= std::copysign(f, p);
            else
                meca.mC(ii, ii) -= f / r;
#elif ( DIM == 3 )
            unsigned jj = DIM * ( matIndex() + pp );
            Vector p = posP(pp);
            if ( p.norm() < r )
            {
                meca.mC(jj+1, jj+1) -= f / r;
                meca.mC(jj+2, jj+2) -= f / r;
            }
            else
            {
                Vector n = p.normalized(f);
                meca.base(jj+1) -= n.YY;
                meca.base(jj+2) -= n.ZZ;
            }
#endif
        }
    }
#endif
    
    
#if NEW_COLINEAR_FORCE
    /*
     Add a length-dependent force acting parallel to the filament.
     A force proportional to the length of the segments is distributed
     to the vertices.
     */
    if ( prop->colinear_force )
    {
        real s = 0.5 * prop->colinear_force * segmentation();
        for ( unsigned i = 0; i < nbSegments(); ++i )
        {
            Vector f = s * dirSegment(i);
            meca.addForce(Mecapoint(this, i  ), f);
            meca.addForce(Mecapoint(this, i+1), f);
        }
    }
#endif
    
    if ( prop->confine != CONFINE_OFF )
    {
        Space const* spc = prop->confine_space_ptr;
        switch ( prop->confine )
        {
            case CONFINE_INSIDE:
                for ( unsigned i = 0; i < nPoints; ++i )
                {
                    Vector pos = posP(i);
                    if ( spc->outside(pos) )
                        spc->setInteraction(pos, Mecapoint(this, i), meca, prop->confine_stiffness);
                }
                break;
                
            case CONFINE_OUTSIDE:
                for ( unsigned i = 0; i < nPoints; ++i )
                {
                    Vector pos = posP(i);
                    if ( spc->inside(pos) )
                        spc->setInteraction(pos, Mecapoint(this, i), meca, prop->confine_stiffness);
                }
                break;
                
            case CONFINE_ON:
                for ( unsigned i = 0; i < nPoints; ++i )
                    spc->setInteraction(posP(i), Mecapoint(this, i), meca, prop->confine_stiffness);
                break;
                
            case CONFINE_MINUS_END:
                spc->setInteraction(posP(0), Mecapoint(this, 0), meca, prop->confine_stiffness);
                break;

            case CONFINE_PLUS_END:
            {
                unsigned L = lastPoint();
                spc->setInteraction(posP(L), Mecapoint(this, L), meca, prop->confine_stiffness);
            } break;
                
            case CONFINE_BOTH_ENDS:
            {
                spc->setInteraction(posP(0), Mecapoint(this, 0), meca, prop->confine_stiffness);
                const unsigned L = lastPoint();
                spc->setInteraction(posP(L), Mecapoint(this, L), meca, prop->confine_stiffness);
            } break;
                
            case CONFINE_PLUS_OUT:
            {
                unsigned L = lastPoint();
                Vector pos = posP(L);
                if ( spc->inside(pos) )
                    spc->setInteraction(pos, Mecapoint(this, L), meca, prop->confine_stiffness);
            } break;
            default:
                throw InvalidParameter("Invalid fiber::confine");
        }
    }
}


//------------------------------------------------------------------------------
#pragma mark - list of bound Hands

/**
Link the hand at the front of the list.

Keeping track of the bound Hands is needed to run Cytosim if the filaments are
dynamic, so that a Fiber that has changed can update all the Hands bound to it.
The list is also used in reports, or to quickly count Hands bound to the fiber.
*/
void Fiber::addHand(Hand * n) const
{
    n->prev(nullptr);
    n->next(handListFront);
    if ( handListFront )
        handListFront->prev(n);
    else
        handListBack = n;
    handListFront = n;
}


void Fiber::removeHand(Hand * n) const
{
    Hand * x = n->next();
    if ( n->prev() )
        n->prev()->next(x);
    else {
        assert_true( handListFront == n );
        handListFront = x;
    }
    
    if ( x )
        x->prev(n->prev());
    else {
        assert_true( handListBack == n );
        handListBack = n->prev();
    }
}


void Fiber::updateHands() const
{
    for ( Hand * ha = handListFront; ha; ha = ha->next() )
        ha->reinterpolate();
}


void Fiber::detachHands() const
{
    // we must iterate one step ahead, because detach() will unlink
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        ha->detach();
        ha = nx;
    }
}

/**
Sort in ascending order
*/
int compareAbscissa(const void* a, const void* b)
{
    real ax = (*static_cast<Hand *const*>(a))->abscissa();
    real bx = (*static_cast<Hand *const*>(b))->abscissa();
    return ( ax > bx ) - ( bx > ax );
}

/**
 This sorts the Hands in order of increasing abscissa
 Sorting is done by copying to temporary array space, using std::qsort
 */
void Fiber::sortHands() const
{
    size_t cnt = nbHands();
    if ( cnt > 1 )
    {
        Hand ** tmp = new Hand*[cnt];
        
        size_t i = 0;
        Hand * n = handListFront;
        
        while ( n )
        {
            tmp[i++] = n;
            n = n->next();
        }
        
        qsort(tmp, cnt, sizeof(Hand*), compareAbscissa);
        
        n = tmp[0];
        handListFront = n;
        n->prev(nullptr);
        for ( i = 1; i < cnt; ++i )
        {
            n->next(tmp[i]);
            tmp[i]->prev(n);
            n = tmp[i];
        }
        n->next(nullptr);
        handListBack = n;
        
        delete[] tmp;
    }
}


unsigned Fiber::nbHands() const
{
    unsigned res = 0;
    
    for ( Hand const* ha = handListFront; ha; ha = ha->next() )
        ++res;
    
    return res;
}


int Fiber::nbHands(int (*count)(Hand const*)) const
{
    int res = 0;
    
    for ( Hand const* ha = handListFront; ha; ha = ha->next() )
        res += count(ha);
    
    //printf("nbHands(%p) = %u\n", count, res);
    return res;
}


unsigned Fiber::nbHandsInRange(real a, real b, const FiberEnd ref) const
{
    unsigned res = 0;
    if ( b > a )
    {
        // Convert to absolute abscissa:
        a = abscissaFrom(a, ref);
        b = abscissaFrom(b, ref);
        if ( b < a )
            std::swap(a, b);
        
        for ( Hand const* ha = handListFront; ha; ha = ha->next() )
            res += ( a <= ha->abscissa()  &&  ha->abscissa() <= b );
    }
    //printf("nbHandsInRange(%8.2f, %8.2f) = %u\n", a, b, res);
    return res;
}


unsigned Fiber::nbHandsNearEnd(const real len, const FiberEnd ref) const
{
    unsigned res = 0;
    real i = -INFINITY, s = INFINITY;
    
    if ( len > 0 )
    {
        if ( ref == PLUS_END )
            i = abscissaP() - len;
        else if ( ref == MINUS_END )
            s = abscissaM() + len;
        else
            throw("invalid argument value to nbHadsNearEnd()");
        
        for ( Hand const* ha = handListFront; ha; ha = ha->next() )
            res += ( i <= ha->abscissa() && ha->abscissa() <= s );
    }
    //printf("nbHandsNearEnd(%8.2f) = %u\n", len, res);
    return res;
}

//------------------------------------------------------------------------------
#pragma mark - Dynamic ends

state_t Fiber::dynamicState(FiberEnd end) const
{
    if ( end == PLUS_END )
        return dynamicStateP();
    if ( end == MINUS_END )
        return dynamicStateM();
    ABORT_NOW("invalid argument value");
    return 0;
}


void Fiber::setDynamicState(const FiberEnd end, const state_t s)
{
    if ( end == PLUS_END )
        setDynamicStateP(s);
    else if ( end == MINUS_END )
        setDynamicStateM(s);
}


real Fiber::freshAssembly(const FiberEnd end) const
{
    if ( end == PLUS_END )
        return freshAssemblyP();
    if ( end == MINUS_END )
        return freshAssemblyM();
    ABORT_NOW("invalid argument value");
    return 0;
}


/**
 Assuming that the length has changed, or that the abscissa of the ends have changed,
 this updates the segmentation of the fiber if needed, the position of the Hands,
 and the boundaries of the Lattice if present.
 */
void Fiber::updateFiber()
{
    needUpdate = false;
#if ( 0 )
    Cytosim::log << reference() << " update [ "  << std::setw(9) << std::left << abscissaM();
    Cytosim::log << " "  << std::setw(9) << std::left << abscissaP() << " ]" << std::endl;
#endif
    
    /*
     Update all bound Hands.
     Some Hands may be updated more than once in a time-step,
     but that is only a small performance penalty.

     We must iterate one step ahead, because `checkFiberRange()` may lead to detachment.
     The loop could be unrolled, or parallelized
     */
    Hand * ha = handListFront;
    while ( ha )
    {
        Hand * nx = ha->next();
        assert_true(ha->fiber()==this);
        ha->reinterpolate();
        ha->checkFiberRange();
        ha = nx;
    }
    
#if FIBER_HAS_LATTICE
    // this will allocate the Lattice's site to cover the range of Abscissa:
    if ( frLattice.ready() )
    {
        frLattice.setRange(abscissaM(), abscissaP());
    }
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Lattice


void Fiber::printLattice(std::ostream& os, FiberLattice const& lat) const
{
    using std::setw;
    const auto inf = lat.indexM();
    const auto sup = lat.indexP();
    os << "Lattice for " << reference() << ":\n";
    os << "    inf  " << inf << "  " << abscissaM() << "\n";
    os << "    sup  " << sup << "  " << abscissaP() << "\n";
    for ( auto h = inf; h < sup; ++h )
        os << setw(8) << h << "  " << setw(10) << lat.abscissa(h) << setw(10) << lat.data(h) << "\n";
    os << "\n";
}


void Fiber::infoLattice(real& len, unsigned& cnt, real& sm, real& mn, real& mx) const
{
#if FIBER_HAS_LATTICE
    FiberLattice const& lat = frLattice;
    if ( lat.ready() )
    {
        len += length();
        const auto sup = lat.indexP();
        for ( auto i = lat.indexM(); i <= sup; ++i )
        {
            ++cnt;
            sm += lat.data(i);
            real x = lat.data(i);
            mn = std::min(mn, x);
            mx = std::max(mx, x);
        }
    }
#endif
}


FiberLattice const* Fiber::drawableLattice() const
{
#if FIBER_HAS_LATTICE
    return &frLattice;
#else
    return nullptr;
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Glue

/**
 fiber:glue=1 creates a Single if the tip of the Fiber goes outside the Space.
 The Single's hand is managed to always be attached at the tip of the Fiber.
 The Single detaches if the Fiber tip is pulled inside.
 This generates mostly a pushing force from the cortex
 */
void Fiber::setGlue1(Single* glue, const FiberEnd end, Space const* spc)
{
    assert_true(spc);
    if ( spc->inside(posEnd(end)) )
    {
        //detach immediately if the tip is inside the Space
        if ( glue->attached() )
            glue->detach();
    }
    else
    {
        if ( glue->attached() )
        {
            //always keep tracking the tip:
            glue->moveToEnd(end);
        }
        else {
            //reposition the Single base:
            glue->setPosition(spc->project(posEnd(end)));
            //attach to the MT-tip:
            glue->attachEnd(this, end);
        }
    }
}


/**
 fiber:glue=2
 The Single's hand is managed to always be attached at the tip of the Fiber.
 The Single's hand detaches only spontaneously.
 This creates both pulling and pushing force from the cortex
 */
void Fiber::setGlue2(Single* glue, const FiberEnd end, Space const* spc)
{
    assert_true(spc);
    if ( glue->attached() )
    {
        //keep tracking the tip of the fiber while attached
        glue->moveToEnd(end);
    }
    else
    {
        // Attach a new grafted if MT-tip is outside and growing:
        if ( isGrowing(end) && spc->outside(posEnd(end)) )
        {
            //reposition the Single base:
            glue->setPosition(spc->project(posEnd(end)));
            //attach to the MT-tip:
            glue->attachEnd(this, end);
        }
    }
}


/**
 fiber:glue=3 creates a Single at the position where the Fiber crosses the Space's edge.
 This makes an anchor point exactly at the cortex.
 The Single's Hand behaves and detaches normally.
 */
void Fiber::setGlue3(Single* glue, Space const* spc)
{    
    assert_true(spc);
    /*
     If the glue is not already attached, we first check if the fiber intersects
     the edge of the Space:
     */
    if ( ! glue->attached() )
    {
        bool in = spc->inside(posEndM());
        
        if ( in == spc->inside(posEndP()) )
            return;
        
        // find a vertex that is on the other side of the Space edge:
        for ( unsigned i = 1; i < nPoints; ++i )
        {
            if ( spc->inside(posP(i)) != in )
            {
                // the abscissa is interpolated using the distances of P1 and P2 to the edge
                real d1 = spc->distanceToEdge(posP(i-1));
                real d2 = spc->distanceToEdge(posP(i));
                if ( d1 + d2 > REAL_EPSILON )
                {
                    /* we find the abscissa corresponding to the intersection,
                     assuming that the edge is locally straight */
                    FiberSite fs(this, abscissaPoint(i-1+d1/(d2+d1)));
                    glue->setPosition(fs.pos());
                    glue->attach(fs);
                    break;
                }
            }
        }
    }
}


/**
 Search for a glue in the list of bound HandSingle
 this is useful when a simulation is restarted from file
 */
void Fiber::makeGlue(Single*& glue)
{
    SingleSet& set = simul().singles;

    for ( Single * gh = set.firstA(); gh; gh=gh->next() )
    {
        if ( gh->hand()->fiber() == this  &&  gh->mark() == identity() )
        {
            glue = gh;
            //Cytosim::log << "found Fiber:glue for " << reference() << std::endl;
            return;
        }
    }
    
    // create the Single if needed
    if ( !glue )
    {
        glue = prop->glue_prop->newSingle();
        glue->mark(identity());
        set.add(glue);
    }
}


/**
 setGlue() creates Single when MT interact with the edge of the Space
*/
void Fiber::setGlue(Single*& glue, const FiberEnd end, Space const* space)
{
    assert_true(space);
    
    if ( !glue )
        makeGlue(glue);
    
    switch( prop->glue )
    {
        case 1:  setGlue1(glue, end, space);  break;
        case 2:  setGlue2(glue, end, space);  break;
        case 3:  setGlue3(glue, space);       break;
        default: throw InvalidParameter("invalid value of fiber:glue");
    }
    
#if ( 1 )
    // we keep the Single linked in the simulation only if it is attached:
    if ( glue->attached() )
    {
        if ( !glue->linked() )
            simul().singles.link(glue);
    }
    else
    {
        if ( glue->linked() )
            simul().singles.unlink(glue);
    }
#endif
}


//------------------------------------------------------------------------------
#pragma mark - I/O

void Fiber::write(Outputter& out) const
{
    Chain::write(out);

#if FIBER_HAS_LATTICE
    if ( frLattice.ready() )
    {
        writeHeader(out, TAG_LATTICE);
        // frLattice.write(out);
        // only write information corresponding to actual Fiber abscissa range:
        frLattice.write(out, frLattice.indexM(), frLattice.indexP()+1);
    }
#endif
}


void Fiber::read(Inputter& in, Simul& sim, ObjectTag tag)
{
    //std::clog << this << " Fiber::read(" << tag << ")\n";
#ifdef BACKWARD_COMPATIBILITY
        
    if ( in.formatID() == 33 )
        mark(in.readUInt32());
    
    if ( tag == 'm' )
    {
        if ( in.formatID()==31 )
        {
            unsigned p = in.readUInt16();
            prop = sim.findProperty<FiberProp>("fiber", p);
        }
        //tag = TAG;
    }
    
    if ( in.formatID() < 31 )
    {
        setDynamicStateM(in.readUInt8());
        setDynamicStateP(in.readUInt8());
    }
    
#endif
    
    if ( tag == TAG )
    {
        Chain::read(in, sim, tag);
        
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() > 47 && in.formatID() < 50 ) // 4.7.2018 added birthTime
            fnBirthTime = in.readFloat();
#endif

        if ( length() + 128*FLT_EPSILON < prop->min_length )
        {
            Cytosim::log << "Warning: fiber length < fiber:min_length";
            Cytosim::log << " ( " << length() << " < " << prop->min_length << " )" << std::endl;
        }

        frGlue = nullptr;
    }
    else if ( tag == TAG_LATTICE )
    {
        try {
#if FIBER_HAS_LATTICE
            frLattice.read(in);
#else
            FiberLattice dummy;
            dummy.read(in);
            // store unit, to get digits at the right abscissa
            const_cast<FiberProp*>(prop)->lattice_unit = dummy.unit();
#endif
        }
        catch( Exception & e ) {
            e << "reading Lattice for " << reference();
            throw;
        }
    }
#ifdef BACKWARD_COMPATIBILITY
    else if ( tag == TAG_DYNAMIC )
    {
        static bool virgin = true;
        // that is for Fiber class we do not know...
        if ( virgin )
        {
            virgin = false;
            std::cerr << "INCIDENT: skipping dynamic states for unknown fiber class\n";
        }
        in.readUInt32();
        in.readUInt32();
    }
    else if ( tag == 'm' )
    {
        Chain::read(in, sim, tag);
    }
#endif
    else
    {
        Cytosim::log << "unknown Fiber TAG `" << (char)tag << "'" << std::endl;
    }
}


