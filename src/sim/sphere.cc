// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "sim.h"
#include "smath.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "messages.h"
#include "glossary.h"
#include "mecapoint.h"
#include "sphere_prop.h"
#include "object_set.h"
#include "space_prop.h"
#include "space.h"
#include "sphere.h"
#include "wrist.h"
#include "meca.h"
#include "modulo.h"
#include "simul.h"


//------------------- construction and destruction ---------------------------

/**
 The Sphere is returned with no points
 */
Sphere::Sphere(SphereProp const* p)
: spRadius(0), spDrag(0), spDragRot(0), sRad(nullptr), prop(p)
{
}


/*
 This will create the center point
 */
Sphere::Sphere(SphereProp const* p, real rad)
: spRadius(rad), spDrag(0), spDragRot(0), sRad(nullptr), prop(p)
{
    if ( !prop )
        throw InvalidParameter("Sphere:prop should be specified");
    
    if ( rad <= 0 )
        throw InvalidParameter("sphere:radius should be > 0");
    
    // center point
    assert_true( nPoints == 0 );
    addPoint( Vector(0,0,0) );
    
    // reference points to track the orientation of the sphere
    if ( DIM >= 2 )
        addPoint( Vector(spRadius,0,0) );
    if ( DIM == 3 ) {
        addPoint( Vector(0,spRadius,0) );
        addPoint( Vector(0,0,spRadius) );
    }
}


Sphere::Sphere(const Sphere & o)
: Mecable(o), sRad(nullptr)
{
    prop     = o.prop;
    spRadius = o.spRadius;
}


Sphere & Sphere::operator =(const Sphere & o)
{
    prop     = o.prop;
    spRadius = o.spRadius;
    return *this;
}


Sphere::~Sphere()
{
    release();
    prop = nullptr;
}

//------------------------------------------------------------------------------
#pragma mark -

/*
 here 'cp' is the vector from the center to the point to be added,
 in other words, the position of the point in the local reference frame.
 */
unsigned Sphere::addSurfacePoint(Vector const& cp)
{
    return addPoint(posP(0)+cp.normalized(spRadius));
}


/**
 @ingroup NewObject
 
 Specify radius and number of surface points of a Sphere:

     new sphere NAME
     {
        radius = REAL
        point1 = INTEGER, POSITION [, SINGLE_SPEC]
     }
 
 The `INTEGER` specifies the number of points created, and `POSITION` can be a
 `VECTOR`, or the string 'surface'.  Multiple `SINGLE_SPEC` can be specified.
 
 <h3> Add Singles to a Sphere </h3>
 
 The parameter 'attach' can be used to add Single to the points of a Solid:
 
     new sphere NAME
     {
        radius   = ...
        point0   = ...
        etc.
        attach   = SINGLE_SPEC [, SINGLE_SPEC] ...
        attach0  = SINGLE_SPEC [, SINGLE_SPEC] ...
        etc.
     }
 
 Where `SINGLE_SPEC` is string containing at most 3 words: `[INTEGER] NAME [each]`,
 where the `INTEGER` specifies the number of Singles, `NAME` specifies their name,
 and the optional word `each` species that the command applies to every point.
 
 The command `attach` applies to all the points of the Solid, while `attach0`,
 `attach1`, etc. apply to the points specified by `point0`, `point1`, etc.
 With `attach`, the Singles are distributed randomly on all the points,
 and if `each` is specified, the specification is repeated for each point.
 
 For example if `grafted` is the name of a Single, one can use:
 
     new solid NAME
     {
        attach1 = 1 grafted each
        attach2 = 10 grafted
     }
 */
ObjectList Sphere::build(Glossary & opt, Simul& sim)
{
    ObjectList res;
    std::string str;
    unsigned inp = 1, inx = 0, nbp = 1;

    if ( opt.has_key("point0") )
        throw InvalidParameter("point indices start at 1 (use `point1`, `point2`, etc.)");

    // interpret each instruction as a command to add points:
    std::string var = "point1";
    while ( opt.has_key(var) )
    {
        inx = 0;
        nbp = 1;
        if ( opt.is_positive_integer(var, 0) && opt.set(nbp, var) )
            ++inx;
        
        if ( nbp > 0 )
        {
            unsigned fip = nPoints;
            str = opt.value(var, inx);
            // add 'nbp' points:
            for ( unsigned n = 0; n < nbp; ++n )
            {
                Vector vec(0,0,0);
                if ( str == "surface" )
                    vec = Vector::randU(radius());
                else
                {
                    std::istringstream iss(str);
                    vec = Movable::readPosition(iss, nullptr);
                    if ( 8 * vec.norm() < spRadius )
                        throw InvalidParameter(var+" cannot be brought to the Sphere surface");
                }
                addSurfacePoint(vec);
            }
            
            // attach Single to this set of points:
            while ( opt.set(str, var, ++inx) )
                res.append(sim.singles.makeWrists(this, fip, nbp, str));
            
            // attach Single to this set of points:
            inx = 0;
            var = "attach" + std::to_string(inp);
            while ( opt.set(str, var, inx++) )
                res.append(sim.singles.makeWrists(this, fip, nbp, str));
        }
        
        // set next keyword:
        var = "point" + std::to_string(++inp);
    }
    
    
    // attach Singles distributed over the surface points:
    inx = 0;
    while ( opt.set(str, "attach", inx++) )
        res.append(sim.singles.makeWrists(this, nbRefPoints, nbSurfacePoints(), str));

    
    // final verification of the number of points:
    nbp = 0;
    if ( opt.set(nbp, "nb_points")  &&  nbp != nPoints )
    {
        throw InvalidParameter("could not find the number of points specified in solid:nb_points");
    }
    
    //std::cerr << *this << std::endl;
    return res;
}


//------------------------------------------------------------------------------
void Sphere::setInteractions(Meca & meca) const
{
    if ( prop->confine != CONFINE_OFF )
    {
        Space const* spc = prop->confine_space_ptr;
        
        switch ( prop->confine )
        {
            case CONFINE_INSIDE:
            {
                Vector cen(pPos);
                if ( ! spc->inside(cen) )
                    spc->setInteraction(cen, Mecapoint(this, 0), meca, prop->confine_stiffness);
            } break;
                
            case CONFINE_ALL_INSIDE:
            {
                Vector cen(pPos);
                if ( ! spc->allInside(cen, spRadius) )
                    spc->setInteraction(cen, Mecapoint(this, 0), spRadius, meca, prop->confine_stiffness);
            } break;
                
            case CONFINE_ON:
                spc->setInteraction(posP(0), Mecapoint(this, 0), meca, prop->confine_stiffness);
                
            default:
                throw InvalidParameter("Invalid sphere::confine");
        }
    }
}


void Sphere::resize(const real R)
{
    //std::clog << "Sphere::resize " << R << std::endl;
    if ( R > 0 )
    {
        spRadius = R;
        reshape();
    }
}

/**
 The mobility is that of a sphere in an infinite fluid (Stokes law):
 
 Translation:
     dposition/dtime = mu_T * force
     mu_T = 6 * PI * viscosity * radius
 
 Rotation:
     dangle/dtime = mu_R * torque
     mu_R = 8 * PI * viscosity * radius^3
 
 */
void Sphere::setDragCoefficientStokes()
{
    assert_true( spRadius > 0 );
    
    const real rad = spRadius;
    
    //hydrodynamic not corrected: infinite fluid is assumed
    spDrag    = 6 * M_PI * prop->viscosity * rad;
    spDragRot = 8 * M_PI * prop->viscosity * rad * rad * rad;

    Cytosim::log("Sphere of radius %.3f has mobility %.2e\n", spRadius, spDrag);
}


/**
 Expect higher friction due to flow around the sphere in a narrow tube.
 This is only valid if (r -a)/a << 1, where r = radius of the tube, and
 a = radius of the sphere.
 
 The formula are taken from:
 <em>The Motion of a Closely-Fitting Sphere in a Fluid-Filled Tube</em>\n
 <b>P. Bungay and H. Brenner, Int. J. Multiphase Flow</b>\n
 Vol 1, pp. 25-56, 1973 (see 3.6, 4.68a and 5.11)
 @latex
     \gamma = \frac{9 \pi^2 \sqrt{2} }{ 4\epsilon^{5/2}} \,\eta \, r
     \gamma^{rot} = 2 \pi^2 \sqrt{\frac{2}{\epsilon}} \,\eta\, r^3
 @end
 */
void Sphere::setDragCoefficientPiston()
{
    assert_true( radius() > 0 );
    assert_true( prop->confine_space_ptr );
    
    const real rad = radius();
    real thickness = prop->confine_space_ptr->thickness();
    real eps = ( thickness - rad ) / rad;
    
    if ( eps <= 0 || eps > 1 )
        throw InvalidParameter("Error: piston formula yields invalid value");

    spDrag    = 9*M_PI*M_PI * prop->viscosity * rad * M_SQRT2 / ( 4 * pow(eps,2.5) );
    spDragRot = 2*M_PI*M_PI * prop->viscosity * rad * rad * rad * sqrt(2.0/eps);
        
    //report the reduced mobility of the sphere:
    Cytosim::out << "Sphere of radius "<< radius() <<" has drag coefficient "<<spDrag<<", due to piston effect" << std::endl;
}


void Sphere::setDragCoefficient()
{
    setDragCoefficientStokes();

    if ( prop->piston_effect )
    {
        if ( prop->confine_space_ptr )
            setDragCoefficientPiston();
        else
            Cytosim::warn << "Piston effect ignored because space is undefined" << std::endl;
    }
}


#pragma mark -

size_t Sphere::allocateMecable(size_t nbp)
{
    size_t ms = Mecable::allocateMecable(nbp);
    if ( ms )
    {
        free_real(sRad);
        sRad = new_real(DIM*ms);
    }
    return ms;
}


void Sphere::release()
{
    free_real(sRad);
    sRad = nullptr;
}


void Sphere::prepareMecable()
{
    setDragCoefficient();
    assert_true( spDrag > 0 );
    assert_true( spDragRot > 0 );
    
    makeProjection();
}

//------------------------------------------------------------------------------

real Sphere::addBrownianForces(real const* rnd, real sc, real* res) const
{
    real bS, bT = sqrt( 2 * sc * spDrag );
    if ( prop->point_mobility > 0 )
        bS = sqrt( 2 * sc / prop->point_mobility );
    else
        bS = 0;

    Vector F(0, 0, 0);
    Torque T(nullTorque);

    real cx = pPos[0];
    real cy = pPos[1];
    real cz = pPos[2];

    /*
     Add random forces to the surface points, and calculate the resulting force
     and momentum in F and T. They will be subtracted from the reference points.
     */
    for ( unsigned p = nbRefPoints; p < nPoints; ++p )
    {
        real * rhs = res + DIM * p;
        real * pos = pPos + DIM * p;
        Vector fp = bS * Vector(rnd+DIM*p);
        
        F += fp;
        
        rhs[0] += fp.XX;
        
#if   ( DIM == 2 )
        rhs[1] += fp.YY;
        T += cross(Vector(pos[0]-cx, pos[1]-cy), fp);
#elif ( DIM == 3 )
        rhs[1] += fp.YY;
        rhs[2] += fp.ZZ;
        T += cross(Vector(pos[0]-cx, pos[1]-cy, pos[2]-cz), fp);
#endif
    }

    /*
     The Torque is distributed to the surface points.
     In 2D, there is one point, and the coefficient is therefore 1.
     in 3D, there are 3 points, but always one is parallel to the axis of the torque,
     and the decomposition over these 3 points gives a factor 2.
     */
    T /= - ( DIM - 1 ) * spRadius * spRadius;
    Vector R = cross(Vector(cx,cy,cz), T);

    for ( unsigned p = 1; p < nbRefPoints; ++p )
    {
        real * rhs = res + DIM * p;
        real const* pos = pPos + DIM * p;
        Vector fp = bT * Vector(rnd+p);
#if   ( DIM == 2 )
        rhs[0] += R.XX - T * pos[1] + fp.XX;
        rhs[1] += R.YY + T * pos[0] + fp.YY;
        F += fp + cross(T, Vector(pos[0]-cx, pos[1]-cy));
#elif ( DIM == 3 )
        rhs[0] += R.XX + T.YY * pos[2] - T.ZZ * pos[1] + fp.XX;
        rhs[1] += R.YY + T.ZZ * pos[0] - T.XX * pos[2] + fp.YY;
        rhs[2] += R.ZZ + T.XX * pos[1] - T.YY * pos[0] + fp.ZZ;
        F += fp + cross(T, Vector(pos[0]-cx, pos[1]-cy, pos[2]-cz));
#endif
    }
    
    // center of the sphere:
#if   ( DIM == 2 )
    res[0] -= F.XX + bT * rnd[0];
    res[1] -= F.YY + bT * rnd[1];
#elif ( DIM == 3 )
    res[0] -= F.XX + bT * rnd[0];
    res[1] -= F.YY + bT * rnd[1];
    res[2] -= F.ZZ + bT * rnd[2];
#endif

    return std::max(bT/spDrag, bS*prop->point_mobility);
}


/**
 Here we start from the i-th Vector and make the other ones orthogonal.
 There must be a better way to do this...
 */
void Sphere::orthogonalize(unsigned i)
{
#if ( DIM == 3 )
    const unsigned ix = 1 + i;
    const unsigned iy = 1 + (i+1)%3;
    const unsigned iz = 1 + (i+2)%3;
    
    Vector cen(pPos);
    assert_true( nPoints >= nbRefPoints );
    
    // reduce to the center of mass an normalize
    Vector tmpX = posP(ix) - cen;
    Vector tmpY = posP(iy) - cen;
    Vector tmpZ = normalize( posP(iz) - cen );
    
    // make tmpY orthogonal to tmpZ, and normalized
    tmpY = normalize(tmpY - dot(tmpZ, tmpY) * tmpZ);
    
    // make tmpX orthogonal to tmpZ and tmpY
    tmpX = normalize(tmpX - dot(tmpZ, tmpX) * tmpZ - dot(tmpY, tmpX) * tmpY);
    
    // store corrected vectors back into the array
    ( cen + spRadius * tmpX ).store(pPos+DIM*ix);
    ( cen + spRadius * tmpY ).store(pPos+DIM*iy);
    ( cen + spRadius * tmpZ ).store(pPos+DIM*iz);
#endif
}


/**
 we get rid of finite-step errors but conserve the shape
 by projecting back onto the sphere,
 without changing the position of point zero (the center)
*/
void Sphere::reshape()
{
    assert_true( nPoints > 0 );
    assert_true( spRadius > 0 );
    Vector axis;
    Vector cen(pPos);
    
    for ( unsigned j = 1; j < nPoints; ++j )
    {
        axis = ( posP(j) - cen ).normalized(spRadius);
        setPoint(j, cen + axis);
    }
    
#if ( DIM == 3 )
    orthogonalize(RNG.pint32(3));
#endif
}


//------------------------------------------------------------------------------
//------------------- methods for the projection -------------------------------
#pragma mark -

#if ( DIM == 1 )

//this is unsafe, don't use the sphere in 1D!
void Sphere::makeProjection() { ABORT_NOW("Sphere is not implemented in 1D"); }
void Sphere::projectForces(const real* X, real* Y) const {}

#else

/**
 prepare variables for the projection 
 */
void Sphere::makeProjection()
{
    assert_true( nPoints >= nbRefPoints );
    
    // calculate radial vectors from center:
    real curv = 1.0 / spRadius;
    for ( unsigned p = nbRefPoints; p < nPoints; ++p )
    {
        real * ppp = sRad + DIM * p;
        real * pos = pPos + DIM * p;
        for ( int d = 0; d < DIM; ++d )
            ppp[d] = curv * ( pos[d] - pPos[d] );
    }
}


// The function should set Y <- sc * mobility * X.
void Sphere::projectForces(const real* X, real* Y) const
{
    // total force:
    Vector F(0,0,0);
    
    // total torque:
#if   ( DIM == 2 )
    real T = 0;
#elif ( DIM >= 3 )
    Vector T(0,0,0);
#endif
    
    for ( unsigned p = 0; p < nPoints; ++p )
    {
        real * pos = pPos + DIM * p;
        real const* xxx = X + DIM * p;
        
        F.XX += xxx[0];
        F.YY += xxx[1];
#if   ( DIM == 2 )
        T    += pos[0] * xxx[1] - pos[1] * xxx[0];
#elif ( DIM >= 3 )
        F.ZZ += xxx[2];
        T.XX += pos[1] * xxx[2] - pos[2] * xxx[1];
        T.YY += pos[2] * xxx[0] - pos[0] * xxx[2];
        T.ZZ += pos[0] * xxx[1] - pos[1] * xxx[0];
#endif
    }
    
    Vector cen(pPos);

    T -= cross(cen, F);       // reduce the torque to the center of mass
    T *= 1.0/spDragRot;       // multiply by the mobility
    F  = F*(1.0/spDrag) + cross(cen, T);
    
    //scale by point mobility:
    real mob = prop->point_mobility;

    for ( unsigned p = 0; p < nbRefPoints; ++p )
    {
        real * yyy = Y + DIM * p;
        real * pos = pPos + DIM * p;

        for ( int d = 0; d < DIM; ++d )
#if   ( DIM == 2 )
        yyy[0] = F.XX - T * pos[1];
        yyy[1] = F.YY + T * pos[0];
#elif ( DIM >= 3 )
        yyy[0] = F.XX + T.YY * pos[2] - T.ZZ * pos[1];
        yyy[1] = F.YY + T.ZZ * pos[0] - T.XX * pos[2];
        yyy[2] = F.ZZ + T.XX * pos[1] - T.YY * pos[0];
#endif
    }
    
    for ( unsigned p = nbRefPoints; p < nPoints; ++p )
    {
        real * yyy = Y + DIM * p;
        real * pos = pPos + DIM * p;
        real * rad = sRad + DIM * p;
        real const* xxx = X + DIM * p;
#if   ( DIM == 2 )
        real a = rad[0] * xxx[0] + rad[1] * xxx[1];
        yyy[0] = F.XX - T * pos[1] + mob * ( xxx[0] - a * rad[0] );
        yyy[1] = F.YY + T * pos[0] + mob * ( xxx[1] - a * rad[1] );
#elif ( DIM >= 3 )
        real a = rad[0] * xxx[0] + rad[1] * xxx[1] + rad[2] * xxx[2];
        yyy[0] = F.XX + T.YY * pos[2] - T.ZZ * pos[1] + mob * ( xxx[0] - a * rad[0] );
        yyy[1] = F.YY + T.ZZ * pos[0] - T.XX * pos[2] + mob * ( xxx[1] - a * rad[1] );
        yyy[2] = F.ZZ + T.XX * pos[1] - T.YY * pos[0] + mob * ( xxx[2] - a * rad[2] );
#endif
    }
}

#endif


//------------------------------------------------------------------------------
#pragma mark -

void Sphere::write(Outputter& out) const
{
    out.writeFloat(radius());
    Mecable::write(out);
}


void Sphere::read(Inputter& in, Simul& sim, ObjectTag tag)
{
    try
    {
        real rad;
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 36 )
            rad = radius();
        else
#endif
        rad = in.readFloat();
        Mecable::read(in, sim, tag);
        resize(rad);
    }
    catch( Exception & e )
    {
        clearPoints();
        throw;
    }
}


void Sphere::print(std::ostream& os) const

{
    os << "new sphere " << reference() << '\n';
    os << "{\n";
    os << " nb_points = " << nbPoints() << '\n';
    for ( unsigned i = 0; i < nbPoints() ; ++i )
        os << " point" << i+1 << " = " << posP(i) << '\n';
    os << "}" << '\n';
}


std::ostream& operator << (std::ostream& os, Sphere const& obj)
{
    obj.print(os);
    return os;
}

