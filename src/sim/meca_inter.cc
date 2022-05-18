// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "space.h"
#include "simul.h"
#include "modulo.h"
#include "mecapoint.h"
#include "interpolation.h"
#include "matrix11.h"
#include "matrix22.h"
#include "matrix33.h"
#include "matrix44.h"

//#include "vecprint.h"

extern Modulo const* modulo;

/// set TRUE to update matrix mC using block directives
/** This is significantly faster */
#define USE_MATRIX_BLOCK ( DIM > 1 )

//------------------------------------------------------------------------------
#pragma mark - Accessory functions

#if DRAW_MECA_LINKS

#  include "gle.h"
#  include "gle_color_list.h"

/// this performs the modulo on `c`
void drawLink(Vector const& a, Vector const& ab, Vector c)
{
    if ( modulo ) modulo->fold(c, a);
    gle::drawLink(a, ab, c);
}

#endif


/// true if any two values are equal
inline bool any_equal(const index_t a, const index_t b,
                      const index_t c)
{
    //if ( a == b ) return true;
    return ( a == c ) || ( b == c );
}


/// true if any two values are equal
inline bool any_equal(const index_t a, const index_t b,
                      const index_t c, const index_t d)
{
    //if ( a == b ) return true;
    //if ( c == d ) return true;
    return ( a == c ) || ( a == d ) || ( b == c ) || ( b == d );
}


//------------------------------------------------------------------------------
#pragma mark - Explicit (constant) Forces
//------------------------------------------------------------------------------


/**
 Add constant force to `pte`
 */
void Meca::addForce(const Mecapoint & pte, Vector const& force)
{
    const index_t inx = DIM * pte.matIndex();
    force.add_to(vBAS+inx);
}


/**
Add constant force to `pti`
 */
void Meca::addForce(const Interpolation & pti, Vector const& force)
{
    const index_t ii0 = DIM * pti.matIndex1();
    const index_t ii1 = DIM * pti.matIndex2();
    
    force.add_to(pti.coef2(), vBAS+ii0);
    force.add_to(pti.coef1(), vBAS+ii1);
}


//------------------------------------------------------------------------------
#pragma mark - Explicit (constant) Torque
//------------------------------------------------------------------------------

/**
 Add constant torque in `pti`:
 
     force = cross(torque, position)
 
 This is explicit and all contributions go in the force vector vBAS[]
*/
void Meca::addTorque(const Interpolation & pti, const Torque & torque)
{
    const index_t ii0 = DIM * pti.matIndex1();
    const index_t ii1 = DIM * pti.matIndex2();
    
    Vector d = pti.diff();
    Vector f = cross(torque/d.normSqr(), d);
    
    f.sub_to(vBAS+ii0);
    f.add_to(vBAS+ii1);
}


/**
 Add an explicit torque to constrain a segment in a given direction `dir`,
 with a given weight:
 
     torque = weigth * cross(normalize(segment), dir)
     force = cross(torque, position)
 
 This code assumes norm(dir) == 1
 This is explicit and all contributions go in the force vector vBAS[]
*/
void Meca::addTorqueClamp(const Interpolation & pti,
                          Vector const& dir,
                          const real weight)
{
    assert_true( weight >= 0 );
    const index_t ii0 = DIM * pti.matIndex1();
    const index_t ii1 = DIM * pti.matIndex2();
    
    Vector d = pti.diff();
    real n = d.normSqr();

    Torque Tq = cross(d, dir);

#if ( DIM >= 3 )
    const real Tn = Tq.norm();
#else
    const real Tn = fabs(Tq);
#endif
    
    // calculate current angle between segments in [0, pi]:
    const real angle = atan2(Tn, dot(d, dir));
    
    /**
     Scale torque to make it proportional to angle:
     we multiply the vector Tq by angle / sin(angle),
     knowing that Tn = Tq.norm() = sin(angle) * sqrt(n)
     
     To have a Torque proportional to sin(angle), use:
     real nn = weight / ( n * sqrt(n) );
     */
    real nn = weight * angle / ( n * Tn );
    
    Vector f = cross(Tq * nn, d);
    
    f.sub_to(vBAS+ii0);
    f.add_to(vBAS+ii1);
}


/**
 Add an explicit torque to bring two segments parallel to each other,
 with a given weight:
 
     torque = weigth * cross(dirA, dirB)
     forceA =  cross(torque, dirA)
     forceB = -cross(torque, dirB)
 
 This is explicit and all contributions go in the force vector vBAS[]
 */
void Meca::addTorque(const Interpolation & pta,
                     const Interpolation & ptb,
                     const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;
    
    Vector da = pta.diff();
    Vector db = ptb.diff();

    real na = da.normSqr();
    real nb = db.normSqr();

    Torque Tq = cross(da, db);
    
#if ( DIM >= 3 )
    const real Tn = Tq.norm();
#else
    const real Tn = fabs(Tq);
#endif
    
    // calculate current angle between segments in [0, pi]:
    const real angle = atan2(Tn, dot(da, db));
    
    /**
     Scale torque to make it proportional to angle:
     we multiply the vector Tq by angle / sin(angle),
     knowing that Tn = Tq.norm() = sin(angle) * sqrt( na * nb )
     
     To have a Torque proportional to sin(angle), use:
     real nn = sqrt( na * nb ); or nn = Tn / angle
     na = weight / ( na * nn );
     nb = weight / ( nb * nn );
     */
    na = weight * angle / ( na * Tn );
    nb = weight * angle / ( nb * Tn );
    
    Vector fa = cross(Tq * na, da);
    Vector fb = cross(db, Tq * nb);

    fa.sub_to(vBAS+ii0);
    fa.add_to(vBAS+ii1);
    fb.sub_to(vBAS+ii2);
    fb.add_to(vBAS+ii3);
}


/**
 Add an explicit torque to induce two segments to make an angle
 defined by (cosinus, sinus) relative to each other:
 
     torque = weigth * cross( dirA , dirB.rotated(angle) )
     forceA =  cross(torque, dirA)
     forceB = -cross(torque, dirB)
 
 The direction of `ptb` is rotated around `axis` defined as cross( dirA, dirB ).
 The calculation is explicit and all contributions go in the force vector vBAS[]
 It is assumed that `cosinus^2 + sinus^2 = 1`
 Note that if ( sinus == 0 ), you can use addTorque(pta, ptb, weight)
 */
void Meca::addTorque(const Interpolation & pta,
                     const Interpolation & ptb,
                     const real cosinus, const real sinus,
                     const real weight)
{
    assert_true( weight >= 0 );
    assert_small( cosinus*cosinus + sinus*sinus - 1.0 );

    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;
    
    Vector da = pta.diff();
    Vector db = ptb.diff();

    real na = da.normSqr();
    real nb = db.normSqr();
    
#if ( DIM >= 3 )
    
    /*
     in 3D the axis of torque is perpendicular to both `da` and `db`,
     and the angle is only defined between 0 and PI,
     */
    Vector axis = cross(db, da).normalized(std::copysign(1.0, sinus));
    
    // rotate vector `db` around `arm` by angle specified as (cosinus, sinus):
    Vector rot = cosinus * db + sinus * cross(axis, db);
    
#elif ( DIM == 2 )

    // this correspond to the Z-direction, up or down:
    real dir = std::copysign(1, cross(da, db));

    // rotate vector `db` by angle defined by (cosinus, sinus) around Z
    Vector rot( db.XX*cosinus + db.YY*sinus*dir, db.YY*cosinus - db.XX*sinus*dir );
    
#else
    
    // this is meaningless but makes compilation possible
    Vector rot(0.0);
    
    throw InvalidParameter("Meca::addTorque is meaningless in 1D");

#endif

    // calculate torque by vector-product:
    Torque Tq = cross(da, rot);
    
#if ( DIM >= 3 )
    const real Tn = Tq.norm();
#else
    const real Tn = fabs(Tq);
#endif
    
    // calculate current angle between segments in [0, pi]:
    const real angle = atan2(Tn, dot(da, rot));
    
    /**
     Scale torque to make it proportional to angle:
     we multiply the vector Tq by angle / sin(angle),
     but knowing that Tn = Tq.norm() = sin(angle) * sqrt( na * nb )
     
     To have a Torque proportional to sin(angle), use:
     real nn = sqrt( na * nb ); or nn = Tn / angle;
     na = weight / ( na * nn );
     nb = weight / ( nb * nn );
     */
    na = weight * angle / ( na * Tn );
    nb = weight * angle / ( nb * Tn );
 
    // forces are divided appropriately to reach the desired torque:
    Vector fa = cross(Tq * na, da);
    Vector fb = cross(db, Tq * nb);
    
    // explicit contributions in vBAS
    fa.sub_to(vBAS+ii0);
    fa.add_to(vBAS+ii1);
    fb.sub_to(vBAS+ii2);
    fb.add_to(vBAS+ii3);
}


#if ( DIM == 2 )
/**
 Add torque between segments AB and CD containing `pt1` and `pt2`.
 Implicit version with linearized force 2D
 Angle is between AB and CD. Force is along normal N_A and N_C pointing to the other filament
 L_AB and L_CD is the length of the segments AB and CD
 force_A = torque_weight * ( Delta angle ) * N_A/L_AB =-force_B
 force_C = torque_weight * ( Delta angle ) * N_C/L_CD =-force_D
 Delta_angle is the difference between actual angle and resting angle between AB and CD
 
 Antonio Politi, 2013
 */
void Meca::addTorquePoliti(const Interpolation & pt1,
                           const Interpolation & pt2,
                           const real cosinus, const real sinus,
                           const real weight)
{
    assert_true( weight >= 0 );
    assert_small( cosinus*cosinus + sinus*sinus - 1.0 );

    if ( pt1.overlapping(pt2) )
        return;
    
    //index in the matrix mC:
    const index_t index[] = { DIM*pt1.matIndex1(), DIM*pt1.matIndex1()+1,
                              DIM*pt1.matIndex2(), DIM*pt1.matIndex2()+1,
                              DIM*pt2.matIndex1(), DIM*pt2.matIndex1()+1,
                              DIM*pt2.matIndex2(), DIM*pt2.matIndex2()+1 };
    
    //Vectors and points of torque
    Vector ab = pt1.diff();
    Vector cd = pt2.diff();
    Vector a = pt1.pos1();
    Vector b = pt1.pos2();
    Vector c = pt2.pos1();
    Vector d = pt2.pos2();
    const real coord[]={a.XX, a.YY, b.XX, b.YY, c.XX, c.YY, d.XX, d.YY};
    //Helping vector this vector is at torque_angle from cd.
    //Therefore in resting state angle difference between ab and ce is zero. This vector is used to compute the strength of torque
    Vector ce;
    ce.XX =  cd.XX*cosinus + cd.YY*sinus;
    ce.YY = -cd.XX*sinus   + cd.YY*cosinus;
    //normalize
    const real abn = ab.norm();
    const real abnS= ab.normSqr();
    const real cdn = cd.norm();
    const real cdnS= cd.normSqr();
    if (abn < REAL_EPSILON || cdn < REAL_EPSILON ) return;
    
    //normalize the vectors
    ab /= abn; cd /= cdn; ce /= cdn;
    
    //Coordinates of normal vectors yielding the direction of the force
    //fa = torque_weight*dangle*(h[0], h[1]) = torque_weight*dangle*na/la
    const real h[]={ ab.YY/abn, -ab.XX/abn, -ab.YY/abn, ab.XX/abn, -cd.YY/cdn, cd.XX/cdn, cd.YY/cdn, -cd.XX/cdn };
    
    //dangle = angle - torque_angle
    //real dangle = atan2( cross(ab, ce), dot(ab, ce) );
    real dangle = atan2( ab.XX*ce.YY - ab.YY*ce.XX, dot(ab, ce) );
    //Computation of the jacobian for the linearization
    //M = d_x f = M1 + M2
    //M1 = w1/l normal d_x dangle
    //M2 = w2 * dangle  d_x normal/l
    real w1 = weight;
    real w2 = weight*dangle;
    
    
    //Matrix M1 with k*hxh (outer product) this yieald a matrix stored with its lower triangular part in m. The -w1 is because ab = b-a
    real m[36] = { 0 };
    //blas::xspr('U', 8, -w1, h, 1, m);
    blas::xspr('L', 8, -w1, h, 1, m);
    
    
    
    //Matrix M2
    real Da = w2*( -2*ab.XX*ab.YY )/abnS;
    real da = w2*( ab.XX*ab.XX-ab.YY*ab.YY )/abnS;
    real Dc = w2*( -2*cd.XX*cd.YY )/cdnS;
    real dc = w2*(  cd.XX*cd.XX-cd.YY*cd.YY )/cdnS;
    real entrya[] = {-Da, -da, Da,  da}; //={d(na_x/la)/dxa, d(na_x/la)/dya, d(na_x/l)/dxb, ...}
    real entryc[] = { Dc,  dc,  -Dc,  -dc};//={ d(nc_x/lc)/dxc, d(nc_x/lc)/dyc, d(nc_x/l)/dxd, ...}
    int shifta = 0;
    int shiftc= 26;
    int mm;
    
    //Add second part of matrix.
    //The pos(-1, jj) accounts for the different signs of the matrix
    for ( int jj=0; jj <  4; ++jj) {
        for ( int ii=jj ; ii < 4; ++ii ) {
            m[ii + shifta] += pow(-1,jj)*entrya[ii-jj];
            m[ii + shiftc] += pow(-1,jj)*entryc[ii-jj];
        }
        shifta += 7 - jj;
        shiftc += 3 - jj;
    }
    
    
    //very Cumbersome!!!
    //Entries for Matrix mC  and vector vBAS
    //vBAS = fa - M*P0
    for ( int ii = 0; ii < 8; ++ii )
    {
        vBAS[index[ii]] += w2*h[ii];
        for (int jj = 0; jj < 8; ++jj) {
            if (jj < ii)
                mm = int(jj*(7.5-0.5*jj)+ii);
            else {
                mm = int(ii*(7.5-0.5*ii)+jj);
                mC( index[ii], index[jj] ) += m[mm];
            }
            vBAS[index[ii]] -= m[mm]*coord[jj];
        }
    }
}
#endif


//------------------------------------------------------------------------------
#pragma mark - Links between Mecables
//------------------------------------------------------------------------------


/**
 Link `pta` (A) and `ptb` (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 In practice, Meca::addLink() will update the matrix mB,
 adding `weight` at the indices corresponding to `A` and `B`.
 
 Note: with modulo, the position of the fibers may be shifted in space,
 and a correction is necessary to make the force calculation correct:
 
     force_A = weight * ( B - A - offset )
     force_B = weight * ( A - B + offset )

 Here 'offset' is a multiple of the space periodicity, corresponding to B-A:
 offset = modulo->offset( A - B )

 In practice, Meca::addLink() will update the vector vBAS[]:
 
     vBAS[A] += weight * offset;
     vBAS[B] -= weight * offset;
 
 In principle, what goes to vBAS[] with modulo can be derived
 simply by multiplying the matrix block by 'offset'.
 */

void Meca::addLink(const Mecapoint & pta,
                   const Mecapoint & ptb,
                   const real weight)
{
    assert_true( weight >= 0 );
    
    const index_t ii0 = pta.matIndex();
    const index_t ii1 = ptb.matIndex();

    if ( ii0 == ii1 )
        return;

    mB(ii0, ii0) -= weight;
    mB(ii1, ii1) -= weight;
    mB(ii0, ii1) += weight;

    if ( modulo )
    {
        const real ww[] = { weight, -weight };
#if ( 1 )
        Vector off = modulo->offset(pta.pos() - ptb.pos());
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
        }
#else
        const index_t inx[] = { DIM*ii0, DIM*ii1 };
        Vector off = modulo->offset(position2(inx, ww));
        if ( off.is_not_zero() )
        {
            off.add_to(vBAS+DIM*ii0);
            off.sub_to(vBAS+DIM*ii1);
        }
#endif
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), ptb.pos());
    }
#endif
}


/**
 Link `pti` (A) and `pte` (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 */

void Meca::addLink(const Interpolation & pti,
                   const Mecapoint & pte,
                   const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pti.matIndex1();
    const index_t ii1 = pti.matIndex2();
    const index_t ii2 = pte.matIndex();
    
    if ( any_equal(ii0, ii1, ii2) )
        return;

    //coefficients on the points:
    const real cc[] = { pti.coef2(),   pti.coef1(),    -1.0 };
    const real ww[] = { weight*cc[0], weight*cc[1], -weight };
    
    mB(ii0, ii0) -= ww[0] * cc[0];
    mB(ii1, ii0) -= ww[1] * cc[0];
    mB(ii2, ii0) += ww[0];         // since cc[2] == -1
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) += ww[1];         // since cc[2] == -1
    mB(ii2, ii2) += ww[2];         // since cc[2] == -1
 
    if ( modulo )
    {
#if ( 1 )
        Vector off = modulo->offset(pti.pos() - pte.pos());
#else
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2 };
        Vector off = modulo->offset(position3(inx, cc));
#endif
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pti.mecable()->signature()).load();
        gle::drawLink(pti.pos(), pte.pos());
    }
#endif
}


/**
 Link `pte` (A) and `pti` (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )

 */

void Meca::addLink(const Mecapoint & pte,
                   const Interpolation & pti,
                   const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pte.matIndex();
    const index_t ii1 = pti.matIndex1();
    const index_t ii2 = pti.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2) )
        return;
    
    //coefficients on the points:
    const real cc[] = {    1.0, -pti.coef2(), -pti.coef1() };
    const real ww[] = { weight, weight*cc[1], weight*cc[2] };
    
    mB(ii0, ii0) -= ww[0];  // since cc[0]==1
    mB(ii1, ii0) -= ww[1];  // since cc[0]==1
    mB(ii2, ii0) -= ww[2];  // since cc[0]==1
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
  
    if ( modulo )
    {
#if ( 1 )
        Vector off = modulo->offset(pte.pos() - pti.pos());
#else
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2 };
        Vector off = modulo->offset(position3(inx, cc));
#endif
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pti.mecable()->signature()).load();
        gle::drawLink(pti.pos(), pte.pos());
    }
#endif
}


/**
 Link `pta` (A) and `ptb` (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )

 */

void Meca::addLink(const Interpolation & pta,
                   const Interpolation & ptb,
                   const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pta.matIndex1();
    const index_t ii1 = pta.matIndex2();
    const index_t ii2 = ptb.matIndex1();
    const index_t ii3 = ptb.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;
    
    //interpolation coefficients:
    const real cc[] = {  pta.coef2(),  pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real ww[] = { weight*cc[0], weight*cc[1], weight*cc[2], weight*cc[3] };
    
    mB(ii0, ii0) -= ww[0] * cc[0];
    mB(ii1, ii0) -= ww[1] * cc[0];
    mB(ii2, ii0) -= ww[2] * cc[0];
    mB(ii3, ii0) -= ww[3] * cc[0];
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    mB(ii3, ii1) -= ww[3] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    mB(ii3, ii2) -= ww[3] * cc[2];
    
    mB(ii3, ii3) -= ww[3] * cc[3];
    
    if ( modulo )
    {
#if ( 1 )
        Vector off = modulo->offset(pta.pos() - ptb.pos());
#else
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2, DIM*ii3 };
        Vector off = modulo->offset(position4(inx, cc));
#endif
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
            off.add_to(ww[3], vBAS+DIM*ii3);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), ptb.pos());
    }
#endif
    
}

//------------------------------------------------------------------------------
#pragma mark - Links between Mecable (higher order interpolation)
//------------------------------------------------------------------------------

/**
 Link `pti` (A) and vertex (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is an Interpolation.
 Point B in the vertex of a Mecable, at index 'pts'.
 Diagonal and lower elements of mB are set.
 */
void Meca::addLink1(const Interpolation & pti,
                    const index_t pts,
                    const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pts;
    const index_t ii1 = pti.matIndex1();
    const index_t ii2 = pti.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2) )
        return;
    
    const real cc[] = {    1.0, -pti.coef2(), -pti.coef1() };
    const real ww[] = { weight, weight*cc[1], weight*cc[2] };
    
    mB(ii0, ii0) -= ww[0]; // since cc[0] == 1.0
    mB(ii1, ii0) -= ww[1]; // since cc[0] == 1.0
    mB(ii2, ii0) -= ww[2]; // since cc[0] == 1.0
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    
    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2 };
        Vector off = modulo->offset(position3(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
        }
    }
}


/**
 Link `pte` (A) and interpolated point (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is a Mecapoint (pte).
 Point B in interpolated over 2 vertices, specified by index in 'pts[]'
 using the coefficients given in `coef[]`.
 Diagonal and lower elements of mB are set.
 */
void Meca::addLink2(const Mecapoint & pte,
                    const index_t pts[2], const real coef[2],
                    const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pte.matIndex();
    const index_t ii1 = pts[0];
    const index_t ii2 = pts[1];
    
    if ( any_equal(ii0, ii1, ii2) )
        return;

    const real cc[] = {    1.0,      -coef[0],     -coef[1] };
    const real ww[] = { weight,  weight*cc[1], weight*cc[2] };
    
    assert_small(coef[0]+coef[1]-1.0);
    
    mB(ii0, ii0) -= ww[0]; // since cc[0] == 1.0
    mB(ii1, ii0) -= ww[1]; // since cc[0] == 1.0
    mB(ii2, ii0) -= ww[2]; // since cc[0] == 1.0
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    
    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2 };
        Vector off = modulo->offset(position3(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
        }
    }
}

/**
 Link `pti` (A) and interpolated point (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is an Interpolation.
 Point B in interpolated over 2 vertices, specified by index in 'pts[]'
 using the coefficients given in `coef[]`.
 Diagonal and lower elements of mB are set.
 */
void Meca::addLink2(const Interpolation & pti,
                    const index_t pts[2], const real coef[2],
                    const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pti.matIndex1();
    const index_t ii1 = pti.matIndex2();
    const index_t ii2 = pts[0];
    const index_t ii3 = pts[1];
    
    const real cc[] = { -pti.coef2(), -pti.coef1(),      coef[0],      coef[1] };
    const real ww[] = { weight*cc[0], weight*cc[1], weight*cc[2], weight*cc[3] };

    assert_small(coef[0]+coef[1]-1.0);
    
    mB(ii0, ii0) -= ww[0] * cc[0];
    mB(ii1, ii0) -= ww[1] * cc[0];
    mB(ii2, ii0) -= ww[2] * cc[0];
    mB(ii3, ii0) -= ww[3] * cc[0];

    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    mB(ii3, ii1) -= ww[3] * cc[1];

    mB(ii2, ii2) -= ww[2] * cc[2];
    mB(ii3, ii2) -= ww[3] * cc[2];

    mB(ii3, ii3) -= ww[3] * cc[3];
    
    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2, DIM*ii3 };
        Vector off = modulo->offset(position4(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
            off.add_to(ww[3], vBAS+DIM*ii3);
        }
    }
}


/**
 Link `pte` (A) and interpolated point (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is a Mecapoint.
 Point B in interpolated over 3 vertices, specified by index in 'pts[]'
 using the coefficients given in `coef[]`.
 Diagonal and lower elements of mB are set.
 */
void Meca::addLink3(const Mecapoint & pte,
                    const index_t pts[3], const real coef[3],
                    const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pte.matIndex();
    const index_t ii1 = pts[0];
    const index_t ii2 = pts[1];
    const index_t ii3 = pts[2];

    const real cc[] = {    1.0,     -coef[0],     -coef[1],     -coef[2] };
    const real ww[] = { weight, weight*cc[1], weight*cc[2], weight*cc[3] };

    assert_small(coef[0]+coef[1]+coef[2]-1.0);
    
    mB(ii0, ii0) -= ww[0];
    mB(ii1, ii0) -= ww[1];
    mB(ii2, ii0) -= ww[2];
    mB(ii3, ii0) -= ww[3];
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    mB(ii3, ii1) -= ww[3] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    mB(ii3, ii2) -= ww[3] * cc[2];
    
    mB(ii3, ii3) -= ww[3] * cc[3];
    
    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2, DIM*ii3 };
        Vector off = modulo->offset(position4(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
            off.add_to(ww[3], vBAS+DIM*ii3);
        }
    }
}


/**
 Link `pti` (A) and interpolated point (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is an Interpolation.
 Point B in interpolated over 4 vertices, specified by index in 'pts[]'
 using the coefficients given in `coef[]`.
 Diagonal and lower elements of mB are set.
*/
void Meca::addLink3(const Interpolation & pti,
                    const index_t pts[3], const real coef[3],
                    const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pti.matIndex1();
    const index_t ii1 = pti.matIndex2();
    const index_t ii2 = pts[0];
    const index_t ii3 = pts[1];
    const index_t ii4 = pts[2];

    const real cc[] = { -pti.coef2(), -pti.coef1(),      coef[0],      coef[1],      coef[2] };
    const real ww[] = { weight*cc[0], weight*cc[1], weight*cc[2], weight*cc[3], weight*cc[4] };
    
    assert_small(coef[0]+coef[1]+coef[2]-1.0);
    
    mB(ii0, ii0) -= ww[0] * cc[0];
    mB(ii1, ii0) -= ww[1] * cc[0];
    mB(ii2, ii0) -= ww[2] * cc[0];
    mB(ii3, ii0) -= ww[3] * cc[0];
    mB(ii4, ii0) -= ww[4] * cc[0];
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    mB(ii3, ii1) -= ww[3] * cc[1];
    mB(ii4, ii1) -= ww[4] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    mB(ii3, ii2) -= ww[3] * cc[2];
    mB(ii4, ii2) -= ww[4] * cc[2];
    
    mB(ii3, ii3) -= ww[3] * cc[3];
    mB(ii4, ii3) -= ww[4] * cc[3];
    
    mB(ii4, ii4) -= ww[4] * cc[4];
    
    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2, DIM*ii3, DIM*ii4 };
        Vector off = modulo->offset(position5(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
            off.add_to(ww[3], vBAS+DIM*ii3);
            off.add_to(ww[4], vBAS+DIM*ii4);
        }
    }
}

/**
 Link `pte` (A) and interpolated point (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is a Mecapoint.
 Point B in interpolated over 4 vertices, specified by index in 'pts[]'
 using the coefficients given in `coef[]`.
 Diagonal and lower elements of mB are set.
 */
void Meca::addLink4(const Mecapoint & pte,
                    const index_t pts[4], const real coef[4],
                    const real weight)
{
    assert_true( weight >= 0 );

    //index in the matrix mB:
    const index_t ii0 = pte.matIndex();
    const index_t ii1 = pts[0];
    const index_t ii2 = pts[1];
    const index_t ii3 = pts[2];
    const index_t ii4 = pts[3];

    const real cc[] = {    1.0,     -coef[0],     -coef[1],     -coef[2],     -coef[3] };
    const real ww[] = { weight, weight*cc[1], weight*cc[2], weight*cc[3], weight*cc[4] };
    
    assert_small(coef[0]+coef[1]+coef[2]+coef[3]-1.0);
    
    mB(ii0, ii0) -= ww[0];
    mB(ii1, ii0) -= ww[1];
    mB(ii2, ii0) -= ww[2];
    mB(ii3, ii0) -= ww[3];
    mB(ii4, ii0) -= ww[4];
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    mB(ii3, ii1) -= ww[3] * cc[1];
    mB(ii4, ii1) -= ww[4] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    mB(ii3, ii2) -= ww[3] * cc[2];
    mB(ii4, ii2) -= ww[4] * cc[2];
    
    mB(ii3, ii3) -= ww[3] * cc[3];
    mB(ii4, ii3) -= ww[4] * cc[3];
    
    mB(ii4, ii4) -= ww[4] * cc[4];
    
    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2, DIM*ii3, DIM*ii4 };
        Vector off = modulo->offset(position5(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
            off.add_to(ww[3], vBAS+DIM*ii3);
            off.add_to(ww[4], vBAS+DIM*ii4);
        }
    }
}


/**
 Link `pti` (A) and interpolated point (B)
 The force is linear with a zero resting length:
 
     force_A = weight * ( B - A )
     force_B = weight * ( A - B )
 
 Point A is an Interpolation.
 Point B in interpolated over 4 vertices, specified by index in 'pts[]'
 using the coefficients given in `coef[]`.
 Diagonal and lower elements of mB are set.
*/
void Meca::addLink4(const Interpolation & pti,
                    const index_t pts[4], const real coef[4],
                    const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    const index_t ii0 = pti.matIndex1();
    const index_t ii1 = pti.matIndex2();
    const index_t ii2 = pts[0];
    const index_t ii3 = pts[1];
    const index_t ii4 = pts[2];
    const index_t ii5 = pts[3];

    const real cc[] = { -pti.coef2(), -pti.coef1(),      coef[0],      coef[1],      coef[2],      coef[3] };
    const real ww[] = { weight*cc[0], weight*cc[1], weight*cc[2], weight*cc[3], weight*cc[4], weight*cc[5] };
    
    assert_small(coef[0]+coef[1]+coef[2]+coef[3]-1.0);
    
    mB(ii0, ii0) -= ww[0] * cc[0];
    mB(ii1, ii0) -= ww[1] * cc[0];
    mB(ii2, ii0) -= ww[2] * cc[0];
    mB(ii3, ii0) -= ww[3] * cc[0];
    mB(ii4, ii0) -= ww[4] * cc[0];
    mB(ii5, ii0) -= ww[5] * cc[0];
    
    mB(ii1, ii1) -= ww[1] * cc[1];
    mB(ii2, ii1) -= ww[2] * cc[1];
    mB(ii3, ii1) -= ww[3] * cc[1];
    mB(ii4, ii1) -= ww[4] * cc[1];
    mB(ii5, ii1) -= ww[5] * cc[1];
    
    mB(ii2, ii2) -= ww[2] * cc[2];
    mB(ii3, ii2) -= ww[3] * cc[2];
    mB(ii4, ii2) -= ww[4] * cc[2];
    mB(ii5, ii2) -= ww[5] * cc[2];
    
    mB(ii3, ii3) -= ww[3] * cc[3];
    mB(ii4, ii3) -= ww[4] * cc[3];
    mB(ii5, ii3) -= ww[5] * cc[3];
    
    mB(ii4, ii4) -= ww[4] * cc[4];
    mB(ii5, ii4) -= ww[5] * cc[4];

    mB(ii5, ii5) -= ww[5] * cc[5];

    if ( modulo )
    {
        const index_t inx[] = { DIM*ii0, DIM*ii1, DIM*ii2, DIM*ii3, DIM*ii4, DIM*ii5 };
        Vector off = modulo->offset(position6(inx, cc));
        if ( off.is_not_zero() )
        {
            off.add_to(ww[0], vBAS+DIM*ii0);
            off.add_to(ww[1], vBAS+DIM*ii1);
            off.add_to(ww[2], vBAS+DIM*ii2);
            off.add_to(ww[3], vBAS+DIM*ii3);
            off.add_to(ww[4], vBAS+DIM*ii4);
            off.add_to(ww[5], vBAS+DIM*ii5);
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark - Links with resting length
//------------------------------------------------------------------------------

void Meca::add_block(index_t i, index_t j, MatrixBlock const& T)
{
#if ( DIM == 1 )
    mC(i,j) += T.value();
#else
    if ( i == j )
    {
        for ( int x = 0; x < DIM; ++x )
        for ( int y = x; y < DIM; ++y )
            mC(i+y, j+x) += T(y,x);
    }
    else
    {
        for ( int x = 0; x < DIM; ++x )
        for ( int y = 0; y < DIM; ++y )
            mC(i+y, j+x) += T(y,x);
    }
#endif
}

void Meca::add_block(index_t i, index_t j, real alpha, MatrixBlock const& T)
{
#if ( DIM == 1 )
    mC(i,j) += alpha * T.value();
#else
    if ( i == j )
    {
        for ( int x = 0; x < DIM; ++x )
        for ( int y = x; y < DIM; ++y )
            mC(i+y, j+x) += alpha * T(y,x);
    }
    else
    {
        for ( int x = 0; x < DIM; ++x )
        for ( int y = 0; y < DIM; ++y )
            mC(i+y, j+x) += alpha * T(y,x);
    }
#endif
}

void Meca::sub_block(index_t i, index_t j, MatrixBlock const& T)
{
#if ( DIM == 1 )
    mC(i,j) -= T.value();
#else
    if ( i == j )
    {
        for ( int x = 0; x < DIM; ++x )
        for ( int y = x; y < DIM; ++y )
            mC(i+y, j+x) -= T(y,x);
    }
    else
    {
        for ( int x = 0; x < DIM; ++x )
        for ( int y = 0; y < DIM; ++y )
            mC(i+y, j+x) -= T(y,x);
    }
#endif
}

/**
 Link `pta` (A) and `ptb` (B),
 The force is affine with non-zero resting length: 
 
     force_A = weight * ( B - A ) * ( length / |AB| - 1 )
     force_B = weight * ( A - B ) * ( length / |AB| - 1 )
 
 */

void Meca::addLongLink(const Mecapoint & pta,
                       const Mecapoint & ptb,
                       const real len,
                       const real weight)
{
    assert_true( weight >= 0 );
    assert_true( len >= 0 );

    const index_t ia = DIM * pta.matIndex();  // coef is +weight
    const index_t ib = DIM * ptb.matIndex();  // coef is -weight

    if ( ia == ib )
        return;
    
    Vector off, axi = ptb.pos() - pta.pos();

    if ( modulo )
        modulo->foldOffset(axi, off);
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), axi, len);
    }
#endif

    const real abn = axi.norm();
    if ( abn < REAL_EPSILON )
        return;
    
    const real wla = weight * len / abn;

    axi.add_to(-wla, vBAS+ia);
    axi.add_to( wla, vBAS+ib);
    
    axi /= abn;

    /* To stabilize the matrix with compression, we remove the negative eigenvalues
        This is done by using len = 1 in the formula if len > 1.0. */
    const bool cooked = ( len > abn );
    
    MatrixBlock T;

    if ( cooked )
        T = MatrixBlock::outerProduct(axi, weight);
    else
        T = MatrixBlock::offsetOuterProduct(weight-wla, axi, wla);
    
#if USE_MATRIX_BLOCK
    mC.diag_block(ia).sub_half(T);
    mC.diag_block(ib).sub_half(T);
    mC.block(ia, ib).add_full(T);
#else
    sub_block(ia, ia, T);
    sub_block(ib, ib, T);
    add_block(ia, ib, T);
#endif
    
    if ( modulo && off.is_not_zero() )
    {
        if ( cooked )
            off = ( weight * dot(off, axi) ) * axi;
        else
            off = ( wla * dot(off, axi) ) * axi + ( weight - wla ) * off;
        off.sub_to(vBAS+ia);
        off.add_to(vBAS+ib);
    }
}


/**
 Link `pte` (A) and `pti` (B),
 The force is affine with non-zero resting length: 
 
     force_A = weight * ( B - A ) * ( len / |AB| - 1 )
     force_B = weight * ( A - B ) * ( len / |AB| - 1 )
 
 */

void Meca::addLongLink(const Mecapoint & pte,
                       const Interpolation & pti,
                       const real len, const real weight )
{
    assert_true( weight >= 0 );
    assert_true( len >= 0 );

    //index in the matrix mC:
    const index_t ii0 = DIM * pte.matIndex();
    const index_t ii1 = DIM * pti.matIndex1();
    const index_t ii2 = DIM * pti.matIndex2();

    if ( any_equal(ii0, ii1, ii2) )
        return;

    //force coefficients on the points:
    const real cc[] = {   -1.0,   pti.coef2(),   pti.coef1() };
    const real ww[] = { weight, -weight*cc[1], -weight*cc[2] };

    Vector off, axi = pte.pos() - pti.pos();

    if ( modulo )
        modulo->foldOffset(axi, off);
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pti.mecable()->signature()).load();
        gle::drawLink(pti.pos(), axi, len);
    }
#endif
    
    const real abn = axi.norm();
    if ( abn < REAL_EPSILON ) return;
    
    axi /= abn;

    axi.add_to(ww[0], vBAS+ii0);
    axi.add_to(ww[1], vBAS+ii1);
    axi.add_to(ww[2], vBAS+ii2);

    real lab = len / abn;

    /* To stabilize the matrix with compression, we remove the negative eigenvalues
        This is done by using len = 1 in the formula if len > 1.0. */
    const bool cooked = ( len > abn );
    
    MatrixBlock T;

    if ( cooked )
        T = MatrixBlock::outerProduct(axi);
    else
        T = MatrixBlock::offsetOuterProduct(1.0-lab, axi, lab);

    Matrix33 W = Matrix33::outerProduct(cc, ww);
    
    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii2, ii2, W(2,2), T);
#endif

    if ( modulo && off.is_not_zero() )
    {
        if ( cooked )
            off = dot(off, axi) * axi;
        else
            off = ( lab * dot(off, axi) ) * axi + ( 1.0 - lab ) * off;
        
        off.add_to(ww[0], vBAS+ii0);
        off.add_to(ww[1], vBAS+ii1);
        off.add_to(ww[2], vBAS+ii2);
    }
}


/**
 Link `pta` (A) and `ptb` (B),
 The force is affine with non-zero resting length: 
 
     force_A = weight * ( B - A ) * ( len / |AB| - 1 )
     force_B = weight * ( A - B ) * ( len / |AB| - 1 )

 */

void Meca::addLongLink(const Interpolation & pta,
                       const Interpolation & ptb,
                       const real len,
                       const real weight )
{
    assert_true( weight >= 0 );
    assert_true( len >= 0 );

    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();

    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;
    
    //force coefficients on the points:
    const real cc[] = { pta.coef2(),   pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real ww[] = {-weight*cc[0],-weight*cc[1],-weight*cc[2],-weight*cc[3] };
    
    Vector off, axi = ptb.pos() - pta.pos();

    if ( modulo )
        modulo->foldOffset(axi, off);
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), axi, len);
    }
#endif

    const real abn = axi.norm();
    if ( abn < REAL_EPSILON ) return;
    
    axi /= abn;
    
    axi.add_to(len*ww[0], vBAS+ii0);
    axi.add_to(len*ww[1], vBAS+ii1);
    axi.add_to(len*ww[2], vBAS+ii2);
    axi.add_to(len*ww[3], vBAS+ii3);

    real lab = len / abn;
    
    /* To stabilize the matrix with compression, we remove the negative eigenvalues
        This is done by using len = 1 in the formula if len > 1.0. */
    const bool cooked = ( len > abn );
    
    MatrixBlock T;
    
    if ( cooked )
        T = MatrixBlock::outerProduct(axi);
    else
        T = MatrixBlock::offsetOuterProduct(1.0-lab, axi, lab);
    
    Matrix44 W = Matrix44::outerProduct(cc, ww);
    
    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.block(ii3, ii0).add_full(W(3,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.block(ii3, ii1).add_full(W(3,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
    mC.block(ii3, ii2).add_full(W(3,2), T);
    mC.diag_block(ii3).add_half(W(3,3), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii3, ii0, W(3,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii3, ii1, W(3,1), T);
    add_block(ii2, ii2, W(2,2), T);
    add_block(ii3, ii2, W(3,2), T);
    add_block(ii3, ii3, W(3,3), T);
#endif

    if ( modulo && off.is_not_zero() )
    {
        if ( cooked )
            off = dot(off, axi) * axi;
        else
            off = ( lab * dot(off, axi) ) * axi + ( 1.0 - lab ) * off;

        off.add_to(ww[0], vBAS+ii0);
        off.add_to(ww[1], vBAS+ii1);
        off.add_to(ww[2], vBAS+ii2);
        off.add_to(ww[3], vBAS+ii3);
    }
}


//------------------------------------------------------------------------------
#pragma mark - Off-axis links between Mecable
//------------------------------------------------------------------------------

/**
 Link `pta` (A) and `ptb` (B),
 through an intermediate point S located on the side of the segment supporting A:
 S = A + len * N,
 where N is a unit vector that is orthogonal to the A-fiber in A.
 S is linearly related to the two vertices located on each side of A.
 The resulting force is linear of zero resting length, between B and S:
 
     force_S = weight * ( S - B )
     force_B = weight * ( B - S )
 
 @todo interSideLink2D should use block operations
 */


#if ( DIM == 2 )

void Meca::addSideLink2D(const Interpolation & pta,
                         const Mecapoint & ptb,
                         const real arm,
                         const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB:
    index_t ii0 = pta.matIndex1();
    index_t ii1 = pta.matIndex2();
    index_t ii2 = ptb.matIndex();

    if ( any_equal(ii0, ii1, ii2) )
        return;

    //force coefficients on the points:
    const real ca1 = pta.coef2();
    const real ca2 = pta.coef1();
    const real eps = arm / pta.len();
    
    const real ca1w = weight * ca1;
    const real ca2w = weight * ca2;
    const real epsw = weight * eps;
    const real e2sw = eps * epsw;
    
    //we put the isotropic terms in mB
    mB(ii0, ii0) -= ca1w * ca1 + e2sw;
    mB(ii0, ii1) -= ca1w * ca2 - e2sw;
    mB(ii1, ii1) -= ca2w * ca2 + e2sw;
    
    mB(ii2, ii2) -= weight;
    mB(ii0, ii2) += ca1w;
    mB(ii1, ii2) += ca2w;
    
    //index in the matrix mC:
    ii0 *= DIM;
    ii1 *= DIM;
    ii2 *= DIM;
    
    mC(ii0  , ii1+1) += epsw;
    mC(ii0+1, ii1  ) -= epsw;
    
    mC(ii0  , ii2+1) -= epsw;
    mC(ii0+1, ii2  ) += epsw;
    mC(ii1,   ii2+1) += epsw;
    mC(ii1+1, ii2  ) -= epsw;
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), cross(arm, pta.dir()), ptb.pos());
    }
#endif
    
    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        
        real offx = off.XX;
        if ( offx != 0 )
        {
            vBAS[ii0  ] += ca1w * offx;
            vBAS[ii0+1] += epsw * offx;
            vBAS[ii1  ] += ca2w * offx;
            vBAS[ii1+1] -= epsw * offx;
            vBAS[ii2  ] += weight * offx;
        }
        real offy = off.YY;
        if ( offy != 0 )
        {
            vBAS[ii0  ] -= epsw * offy;
            vBAS[ii0+1] += ca1w * offy;
            vBAS[ii1  ] += epsw * offy;
            vBAS[ii1+1] += ca2w * offy;
            vBAS[ii2+1] += weight * offy;
        }
    }
}

#elif ( DIM >= 3 )


/**
 This is experimental and should not be used.
 
 Link `pta` (A) and `ptb` (B)
 and a point S which is on the side of `pta`.
 
     S = pos_a + cross( arm, dir_a)

 arm must be perpendicular to link
 
 @todo interSideLink3D should use block operations
 */
void Meca::addSideLink3D(const Interpolation & pta,
                         const Mecapoint & ptb,
                         Vector const& arm,
                         const real weight)
{
    assert_true( weight >= 0 );

    // indices to mC:
    const index_t ia1 = pta.matIndex1();
    const index_t ia2 = pta.matIndex2();
    const index_t ib  = ptb.matIndex();
    
    if ( any_equal(ia1, ia2, ib) )
        return;
    
    const index_t inx[6] = { DIM*ia1, DIM*ia1+1, DIM*ia1+2, DIM*ia2, DIM*ia2+1, DIM*ia2+2 };

    real a = pta.coef2();
    real b = pta.coef1();
    real s = 1.0 / pta.len();
    
    real ex = s * arm.XX;
    real ey = s * arm.YY;
    real ez = s * arm.ZZ;
    
    /* The transfer matrix transforms the two Mecapoint in pta,
     to the side point S:
     S = aa * pt1 + bb * pt2 + cross(arm, normalize( pt2 - pt1 ))
     
     It was generated in Maxima:
     MVP: matrix([0, -ez, ey], [ez, 0, -ex], [-ey, ex, 0]);
     MD: addcol(-ident(3), ident(3));
     MC: addcol(aa*ident(3), bb*ident(3));
     T: MC+MVP.MD;
     */
    const real T[18] = {
        a,  ez, -ey,   b, -ez,  ey,
      -ez,   a,  ex,  ez,   b, -ex,
       ey, -ex,   a, -ey,  ex,   b
    };
    
    real a2 = a * a, ab = a * b;
    real b2 = b * b;
    
    real exx = ex * ex, exy = ex*ey, exz = ex*ez;
    real eyy = ey * ey, eyz = ey*ez;
    real ezz = ez * ez;
    
    // TT = transpose(T) * T is symmetric, and thus we only set half of it:
    /* Maxima code:
     TT: expand(transpose(T) . T);
     */
    real TT[36] = {
        eyy+ezz+a2,  0,           0,           0,           0,           0,
        -exy,        exx+ezz+a2,  0,           0,           0,           0,
        -exz,       -eyz,         exx+eyy+a2,  0,           0,           0,
        -ezz-eyy+ab, ez+exy,      exz-ey,      eyy+ezz+b2,  0,           0,
        -ez+exy,    -ezz-exx+ab,  eyz+ex,     -exy,         exx+ezz+b2,  0,
        exz+ey,      eyz-ex,     -eyy-exx+ab, -exz,        -eyz,         exx+eyy+b2
    };
    
    // we project to bring all forces in the plane perpendicular to 'arm'
    //real sca = arm.inv_norm();
    //real an = a * sca;
    //real bn = b * sca;
    // Maxima code: matrix([ex, ey, ez]) . T;
    //real TP[9] = { an*ex, an*ey, an*ez, bn*ex, bn*ey, bn*ez, -ex, -ey, -ez };
    //blas::xgemm('N','N', 6, 1, 3, sca, T, 6, arm, 3, 0.0, TP, 6);
    
    //blas::xsyrk('U','N', 6, 1, weight, TP, 6, -weight, TT, 6);
    blas::xscal(36, -weight, TT, 1);
    
    for ( int ii=0 ; ii<6; ++ii )
    for ( int jj=ii; jj<6; ++jj )
        mC(inx[ii], inx[jj]) += TT[ii+6*jj];
    
    //mB(ia1, ib) += -a * weight;
    //mB(ia2, ib) += -b * weight;
    mB(ib, ib) -= weight;
    
    for ( int ii=0; ii<6; ++ii )
    for ( int jj=0; jj<3; ++jj )
        mC(inx[ii], DIM*ib+jj) += weight * T[ii+6*jj];
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), cross(arm,pta.dir()), ptb.pos());
    }
#endif
    
    if ( modulo )
        throw Exception("interSideLink3D is not usable with periodic boundary conditions");
}


/**
 Link `pta` (A) and `ptb` (B),
 through an intermediate point S located on the side of the segment supporting A:
 S = A + len * N,
 where N is a unit vector that is orthogonal to the A-fiber in A.
 S is linearly related to the two vertices located on each side of A.
 The resulting force is linear of zero resting length, between B and S:
 
     force_S = weight * ( S - B )
     force_B = weight * ( B - S )
 
 */

void Meca::addSideLinkS(const Interpolation & pta,
                        const Mecapoint & ptb,
                        Vector const& arm,
                        const real len,
                        const real weight)
{
    assert_true( weight >= 0 );
    assert_true( len > REAL_EPSILON );

    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex();

    if ( any_equal(ii0, ii1, ii2) )
        return;

    MatrixBlock T;
    // vector 'a' is parallel to first Fiber
    {
        Vector a = pta.diff();
        Vector v = a / a.normSqr();
        Vector b = arm / len;
        
        // we can set directly the interaction coefficient matrix:
        T = MatrixBlock::outerProduct(a, v) + MatrixBlock::outerProduct(b);
    }
    
    // weights and indices:
    const real cc[3] = {   pta.coef2(),   pta.coef1(),   -1.0 };
    const real ww[3] = { -weight*cc[0], -weight*cc[1], weight };

    Matrix33 W = Matrix33::outerProduct(cc, ww);
    
    arm.add_to(ww[0], vBAS+ii0);
    arm.add_to(ww[1], vBAS+ii1);
    arm.add_to(ww[2], vBAS+ii2);

    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii2, ii2, W(2,2), T);
#endif

#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), arm, ptb.pos());
    }
#endif
    
    if ( modulo )
        throw Exception("interSideLinkS is not usable with periodic boundary conditions");
}
#endif


/// return a vector of norm 1.0, perpendicular to 'diff' and aligned with `off`:
Vector calculateArm(Vector off, Vector const& diff, real len)
{
    if ( modulo )
        modulo->fold(off);
    // remove component parallel to diff:
    off -= ( dot(off, diff) / diff.normSqr() ) * diff;
    real n = off.norm();
    if ( n > REAL_EPSILON )
        return off * ( len / n );
    else
        return diff.orthogonal(len);
}


void Meca::addSideLink(const Interpolation & pta,
                         const Mecapoint & ptb, 
                         const real len,
                         const real weight )
{
#if ( DIM == 1 )
    
    throw Exception("Meca::addSideLink is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    real arm = std::copysign(len,cross(pta.diff(), ptb.pos()-pta.pos()));
    addSideLink2D(pta, ptb, arm, weight);

#else
    
    // set 'arm' perpendicular to direction of the Fiber associated with `pta`:
    Vector arm = calculateArm(ptb.pos()-pta.pos(), pta.diff(), len);
    addSideLinkS(pta, ptb, arm, len, weight);
    
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Off-axis links between Mecable (Interpolation)

#if ( DIM == 2 )

void Meca::addSideLink2D(const Interpolation & pta,
                         const Interpolation & ptb,
                         const real arm,
                         const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mB and mC:
    const index_t ia0 = pta.matIndex1(),  ii0 = DIM * ia0;
    const index_t ia1 = pta.matIndex2(),  ii1 = DIM * ia1;
    const index_t ib2 = ptb.matIndex1(),  ii2 = DIM * ib2;
    const index_t ib3 = ptb.matIndex2(),  ii3 = DIM * ib3;
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;

    // weights and indices:
    const real w = -weight;
    const real cc0 =  pta.coef2(),  ww0 = w * cc0;
    const real cc1 =  pta.coef1(),  ww1 = w * cc1;
    const real cc2 = -ptb.coef2(),  ww2 = w * cc2;
    const real cc3 = -ptb.coef1(),  ww3 = w * cc3;
    
    const real ee = arm / pta.len(),  we = w * ee;

    Matrix22 A(cc0, -ee,  ee, cc0);
    Matrix22 B(cc1,  ee, -ee, cc1);

    mB(ia0, ia0) += ww0 * cc0 + we * ee;
    mB(ia1, ia1) += ww1 * cc1 + we * ee;
    mB(ib2, ib2) += ww2 * cc2;
    mB(ib3, ib3) += ww3 * cc3;
    mB(ib3, ib2) += ww3 * cc2;

#if USE_MATRIX_BLOCK
    mC.block(ii1, ii0).add_full(w, B.trans_mul(A));
    if ( ii2 > ii0 )
    {
        mC.block(ii2, ii0).add_full(ww2, A);
        mC.block(ii3, ii0).add_full(ww3, A);
        mC.block(ii2, ii1).add_full(ww2, B);
        mC.block(ii3, ii1).add_full(ww3, B);
    }
    else
    {
        Matrix22 At = A.transposed();
        Matrix22 Bt = B.transposed();
        mC.block(ii0, ii2).add_full(ww2, At);
        mC.block(ii1, ii2).add_full(ww2, Bt);
        mC.block(ii0, ii3).add_full(ww3, At);
        mC.block(ii1, ii3).add_full(ww3, Bt);
    }
#else
    // isotropic terms go in matrix mB:
    mB(ia0, ia1) += ww0 * cc1 - we * ee;
    mB(ia0, ib2) += ww0 * cc2;
    mB(ia0, ib3) += ww0 * cc3;
    mB(ia1, ib2) += ww1 * cc2;
    mB(ia1, ib3) += ww1 * cc3;
    
    // anistropic terms go in the matrix mC:
    mC(ii0  , ii1+1) -= we;
    mC(ii0+1, ii1  ) += we;
    
    const real wecc2 = we * cc2;
    const real wecc3 = we * cc3;
    
    mC(ii0, ii2+1) -= wecc2;
    mC(ii0, ii3+1) -= wecc3;
    
    mC(ii0+1, ii2) += wecc2;
    mC(ii0+1, ii3) += wecc3;
    
    mC(ii1, ii2+1) += wecc2;
    mC(ii1, ii3+1) += wecc3;
    
    mC(ii1+1, ii2) -= wecc2;
    mC(ii1+1, ii3) -= wecc3;
#endif
    
    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off *= -weight;
            A.trans_vecmul(off).add_to(vBAS+ii0);
            B.trans_vecmul(off).add_to(vBAS+ii1);
            off.add_to(cc2, vBAS+ii2);
            off.add_to(cc3, vBAS+ii3);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), cross(arm,pta.diff()), ptb.pos());
    }
#endif
}

#elif ( DIM >= 3 )

void Meca::addSideLinkS(const Interpolation & pta,
                        const Interpolation & ptb,
                        Vector const& arm,
                        const real len,
                        const real weight)
{
    assert_true( weight >= 0 );
    assert_true( len > REAL_EPSILON );

    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();

    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;

    MatrixBlock T;
    {
        Vector a = pta.diff();
        Vector v = a / a.normSqr();
        Vector b = arm / len;
        // Vector c = cross(a, b);
        
        // we can set directly the interaction coefficient matrix:
        T = MatrixBlock::outerProduct(a, v) + MatrixBlock::outerProduct(b);
    }
    
    // weights and indices:
    const real cc[4] = {   pta.coef2(),   pta.coef1(),  -ptb.coef2(),  -ptb.coef1() };
    const real ww[4] = { -weight*cc[0], -weight*cc[1], -weight*cc[2], -weight*cc[3] };
    
    Matrix44 W = Matrix44::outerProduct(cc, ww);
    
    arm.add_to(ww[0], vBAS+ii0);
    arm.add_to(ww[1], vBAS+ii1);
    arm.add_to(ww[2], vBAS+ii2);
    arm.add_to(ww[3], vBAS+ii3);
 
    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.block(ii3, ii0).add_full(W(3,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.block(ii3, ii1).add_full(W(3,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
    mC.block(ii3, ii2).add_full(W(3,2), T);
    mC.diag_block(ii3).add_half(W(3,3), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii3, ii0, W(3,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii3, ii1, W(3,1), T);
    add_block(ii2, ii2, W(2,2), T);
    add_block(ii3, ii2, W(3,2), T);
    add_block(ii3, ii3, W(3,3), T);
#endif

#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), arm, ptb.pos());
    }
#endif
    
    if ( modulo )
        throw Exception("interSideLinkS is not usable with periodic boundary conditions");
}

#endif


/**
 Link `pta` (A) and `ptb` (B),
 Which is taken between B and a point S located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in an.
 S is linearly related to the two vertices on the sides of A, P1 and P2
 In 3D S is choosen in the plane of P1, P2 and B.
 The force is linear of zero resting length:
 
     force_S = weight * ( S - B )
     force_B = weight * ( B - S )
 
 */

void Meca::addSideLink(const Interpolation & pta,
                       const Interpolation & ptb,
                       const real len,
                       const real weight)
{
#if ( DIM == 1 )
    
    throw Exception("Meca::addSideLink is meaningless in 1D");

#elif ( DIM == 2 )
    
    real arm = std::copysign(len, cross(pta.diff(), ptb.pos()-pta.pos()) );
    addSideLink2D(pta, ptb, arm, weight);
    
#else

    // set 'arm' perpendicular to direction of the Fiber associated with `pta`:
    Vector arm = calculateArm(ptb.pos()-pta.pos(), pta.diff(), len);
    addSideLinkS(pta, ptb, arm, len, weight);
    
#endif
}


//------------------------------------------------------------------------------
#pragma mark - Symmetric off-axis links
//------------------------------------------------------------------------------

#if ( DIM == 2 )

/*
void Meca::addSideSideLink2D(const Interpolation & pta,
                             const Interpolation & ptb,
                             const real len,
                             const real weight,
                             real side1, real side2 )
{
    assert_true( weight >= 0 );
 
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();
 
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;

    // weights and indices:
    const real w = -weight;
    const real cc0 =  pta.coef2(),  ww0 = w * cc0;
    const real cc1 =  pta.coef1(),  ww1 = w * cc1;
    const real cc2 = -ptb.coef2(),  ww2 = w * cc2;
    const real cc3 = -ptb.coef1(),  ww3 = w * cc3;

    const real ee1 = side1 / ( 2 * pta.len() ), we1 = w * ee1;
    const real ee2 = side2 / ( 2 * ptb.len() ), we2 = w * ee2;

    Matrix22 A(cc0, -ee1,  ee1, cc0);
    Matrix22 B(cc1,  ee1, -ee1, cc1);
    Matrix22 C(cc2, -ee2,  ee2, cc2);
    Matrix22 D(cc3,  ee2, -ee2, cc3);
    
    Matrix22 Aw(ww0, -ew1,  ew1, ww0);
    Matrix22 Bw(ww1,  ew1, -ew1, ww1);
    Matrix22 Cw(ww2, -ew2,  ew2, ww2);
    Matrix22 Dw(ww3,  ew2, -ew2, ww3);
    
    mC.diag_block(ii0).add_half(A.trans_mul(Aw));
    mC.diag_block(ii1).add_half(B.trans_mul(Bw));
    mC.diag_block(ii2).add_half(C.trans_mul(Cw));
    mC.diag_block(ii3).add_half(D.trans_mul(Dw));
 
    mC.block(ii1, ii0).add_full(B.trans_mul(Aw));
    mC.block(ii3, ii2).add_full(D.trans_mul(Cw));

    if ( ii2 > ii0 )
    {
        mC.block(ii2, ii0).add_full(C.trans_mul(Aw));
        mC.block(ii3, ii0).add_full(D.trans_mul(Aw));
        mC.block(ii2, ii1).add_full(C.trans_mul(Bw));
        mC.block(ii3, ii1).add_full(D.trans_mul(Bw));
    }
    else
    {
        mC.block(ii0, ii2).add_full(A.trans_mul(Cw));
        mC.block(ii1, ii2).add_full(B.trans_mul(Cw));
        mC.block(ii0, ii3).add_full(A.trans_mul(Dw));
        mC.block(ii1, ii3).add_full(B.trans_mul(Dw));
    }
 
    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            Aw.trans_vecmul(off).add_to(vBAS+ii0);
            Bw.trans_vecmul(off).add_to(vBAS+ii1);
            Cw.trans_vecmul(off).add_to(vBAS+ii2);
            Dw.trans_vecmul(off).add_to(vBAS+ii3);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), cross(ee1,pta.diff()), cross(ee2,ptb.diff()), ptb.pos());
    }
#endif
}
*/

void Meca::addSideSideLink2D(const Interpolation & pta,
                             const Interpolation & ptb,
                             const real len,
                             const real weight,
                             real side1, real side2 )
{
    assert_true( weight >= 0 );
    assert_true( len >= 0 );
    
    //index in the matrix mB:
    index_t ia1 = pta.matIndex1(), ia2 = pta.matIndex2();
    index_t ib1 = ptb.matIndex1(), ib2 = ptb.matIndex2();
    
    if ( any_equal(ia1, ia2, ib1, ib2) )
        return;

    const real ca1 =  pta.coef2(), ca2 =  pta.coef1();
    const real cb1 = -ptb.coef2(), cb2 = -ptb.coef1();
    
    const real ee1 = side1 * len / ( 2 * pta.len() );
    const real ee2 = side2 * len / ( 2 * ptb.len() );
    
    const real w = -weight;
    const real ca1w = ca1 * w, ca2w = ca2 * w;
    const real cb1w = cb1 * w, cb2w = cb2 * w;
   
    const real ee1w = ee1 * w, ee1ee1w = ee1 * ee1w;
    const real ee2w = ee2 * w, ee2ee2w = ee2 * ee2w;
    const real ee1ee2w = ee1 * ee2w;
    
    //we put the isotropic terms in mB
    mB(ia1, ia1) +=  ca1w * ca1 + ee1ee1w;
    mB(ia1, ia2) +=  ca1w * ca2 - ee1ee1w;
    mB(ia2, ia2) +=  ca2w * ca2 + ee1ee1w;
    
    mB(ib1, ib1) +=  cb1w * cb1 + ee2ee2w;
    mB(ib1, ib2) +=  cb1w * cb2 - ee2ee2w;
    mB(ib2, ib2) +=  cb2w * cb2 + ee2ee2w;
    
    mB(ia1, ib1) +=  ca1w * cb1 - ee1ee2w;
    mB(ia1, ib2) +=  ca1w * cb2 + ee1ee2w;
    mB(ia2, ib1) +=  ca2w * cb1 + ee1ee2w;
    mB(ia2, ib2) +=  ca2w * cb2 - ee1ee2w;
    
    //index in the matrix mC:
    ia1 *= DIM;
    ia2 *= DIM;
    ib1 *= DIM;
    ib2 *= DIM;
    
    mC(ia1  , ia2+1) -= ee1w;
    mC(ia1+1, ia2  ) += ee1w;
    
    mC(ib1  , ib2+1) -= ee2w;
    mC(ib1+1, ib2  ) += ee2w;
    
    const real ee1cb1w = ee1w * cb1;
    const real ee1cb2w = ee1w * cb2;
    const real ee2ca1w = ee2w * ca1;
    const real ee2ca2w = ee2w * ca2;
    
    mC(ia1, ib1+1) -=  ee2ca1w + ee1cb1w;
    mC(ia1, ib2+1) +=  ee2ca1w - ee1cb2w;
    
    mC(ia1+1, ib1) +=  ee2ca1w + ee1cb1w;
    mC(ia1+1, ib2) -=  ee2ca1w - ee1cb2w;
    
    mC(ia2, ib1+1) -=  ee2ca2w - ee1cb1w;
    mC(ia2, ib2+1) +=  ee2ca2w + ee1cb2w;
    
    mC(ia2+1, ib1) +=  ee2ca2w - ee1cb1w;
    mC(ia2+1, ib2) -=  ee2ca2w + ee1cb2w;
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), cross(ee1,pta.diff()), cross(ee2,ptb.diff()), ptb.pos());
    }
#endif
    
    if ( modulo )
        throw Exception("interSideSideLink2D is not usable with periodic boundary conditions");
}

#endif


/**
 Link `pta` (A) and `ptb` (B),
 but the links are maded between SA and SB which are located
 on the side of A and B, respectively:
 
     SA = A + len * N_A,
     SB = B + len * N_B,
 
 N_X is a normalized vector orthogonal to the fiber carrying X, in X:
 The force is linear of zero resting length,
 
     force_SA = weight * ( SA - SB )
     force_SB = weight * ( SB - SA )
 
 */

void Meca::addSideSideLink(const Interpolation & pta,
                           const Interpolation & ptb,
                           const real len,
                           const real weight )
{
#if ( DIM == 1 )
    
    throw Exception("Meca::addSideSideLink meaningless in 1D");
    
#elif ( DIM == 2 )
    
    Vector dir = ptb.pos() - pta.pos();
    real side1 = std::copysign((real)1.0, cross(pta.diff(), dir) );
    real side2 = std::copysign((real)1.0, cross(dir, ptb.diff()) );
    addSideSideLink2D(pta, ptb, len, weight, side1, side2);
    
#else
    
    throw Exception("Meca::addSideSideLink was not implemented in 3D");
    
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Frictionless Links
//------------------------------------------------------------------------------

/**
 Link `pta` (A) and `ptb` (B),
 The force is linear of zero resting length, but is anisotropic:
 The component of the force parallel to the fiber in A is removed

 If T is the normalized direction of the fiber in A:
 
     force_A = weight * ( 1 - T T' ) ( A - B )
     force_B = weight * ( 1 - T T' ) ( B - A )
 
 */

void Meca::addSlidingLink(const Interpolation & pta,
                          const Mecapoint & ptb,
                          const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex();
    
    if ( any_equal(ii0, ii1, ii2) )
        return;
    
    //force coefficients on the points:
    const real A = pta.coef2();
    const real B = pta.coef1();
    const real AA = A * A, AB = A * B, BB = B * B;
    
    Vector dir = pta.dir();

    /*
     Points are (a, b, e) with (ab) the Interpolation, and e the Mecapoint,
     P is the projection on the plane perpendicular to (ab)
         P = I - dir (x) dir / normSqr(dir)
     the interaction is  -weigth * transpose(bb, aa, -1) * P * ( bb, aa, -1 )
     we set only the upper part of this symmetric matrix:
     */

    // T = -weight * [ I - dir (x) dir ]
    MatrixBlock T = MatrixBlock::offsetOuterProduct(-weight, dir, weight);

#if USE_MATRIX_BLOCK
    
    mC.diag_block(ii0).add_half(AA, T);
    mC.diag_block(ii1).add_half(BB, T);
    mC.diag_block(ii2).add_half(T);

    mC.block(ii0, ii1).add_full(AB, T);
    mC.block(ii0, ii2).add_full(-A, T);
    mC.block(ii1, ii2).add_full(-B, T);

#else

    add_block(ii0, ii0, AA, T);
    add_block(ii1, ii1, BB, T);
    add_block(ii2, ii2, T);
    
    add_block(ii0, ii1, AB, T);
    add_block(ii0, ii2, -A, T);
    add_block(ii1, ii2, -B, T);

#endif
    
    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off = weight * ( off - dot(dir, off) * dir );
            off.sub_to(A, vBAS+ii0);
            off.sub_to(B, vBAS+ii1);
            off.add_to(vBAS+ii2);
        }
    }
}


/**
Link `pta` (A) and `ptb` (B),
 The force is linear of zero resting length, but is anisotropic:
 The component of the force parallel to the fiber in A is removed
 
 If T is the normalized direction of the fiber in A:
 
     force_A = weight * ( 1 - T T' ) ( A - B )
     force_B = weight * ( 1 - T T' ) ( B - A )
 
 */

void Meca::addSlidingLink(const Interpolation & pta,
                          const Interpolation & ptb,
                          const real weight)
{
    assert_true( weight >= 0 );
 
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;
    
    //interpolation coefficients
    const real cc[4] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    
    // on points (a, b, e), (ab) being the Interpolation, and e the Mecapoint,
    // P is the projection on the plane perpendicular to (ab): P.v= (v - (T.v)T/normSqr(T))
    // the interaction is  -wh' * P * h
    // we set only the upper part of this symmetric matrix:
    
    Vector dir = pta.dir();

    // T = -weight * [ I - dir (x) dir ]
    MatrixBlock T = MatrixBlock::offsetOuterProduct(-weight, dir, weight);

    Matrix44 W = Matrix44::outerProduct(cc);
    
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.block(ii3, ii0).add_full(W(3,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.block(ii3, ii1).add_full(W(3,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
    mC.block(ii3, ii2).add_full(W(3,2), T);
    mC.diag_block(ii3).add_half(W(3,3), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii3, ii0, W(3,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii3, ii1, W(3,1), T);
    add_block(ii2, ii2, W(2,2), T);
    add_block(ii3, ii2, W(3,2), T);
    add_block(ii3, ii3, W(3,3), T);
#endif

    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off = weight * ( off - dot(dir, off) * dir );
            off.add_to(cc[0], vBAS+ii0);
            off.add_to(cc[1], vBAS+ii1);
            off.add_to(cc[2], vBAS+ii2);
            off.add_to(cc[3], vBAS+ii3);
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark - Off-axis frictionless links (used for steric interactions)

#if ( DIM == 2 )

void Meca::addSideSlidingLink2D(const Interpolation & pta,
                                const Mecapoint & pte,
                                const real arm,
                                const real weight)
{
    assert_true( weight >= 0 );

    // indices
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * pte.matIndex();
    
    if ( any_equal(ii0, ii1, ii2) )
        return;
    
    Vector dir = pta.dir();
    const real aa = pta.coef2();
    const real bb = pta.coef1();
    const real ee = arm / pta.len();
    
    // the (symmetric) projection matrix:
    // P = -weight * [ I - dir (x) dir ]
    MatrixBlock P = MatrixBlock::offsetOuterProduct(-weight, dir, weight);

#if USE_MATRIX_BLOCK
    
    // anti-symmetric matrix blocks:
    const Matrix22 A( -aa,  ee, -ee, -aa );
    const Matrix22 B( -bb, -ee,  ee, -bb );

    /*
     We use block operations to set the matrix block by block:
     | A'PA  A'PB  A'P |
     | B'PA  B'PB  B'P |
     |   PA    PB    P |
     This matrix has symmetric and anti-symmetric blocks,
     since P' = P but A and B are not symmetric
     */
    assert_true( ii0 < ii1 );
    
    if ( ii2 > ii1 )
    {
        const Matrix22 PA = P.mul(A);
        const Matrix22 PB = P.mul(B);
        mC.diag_block(ii0).add_half(A.trans_mul(PA));
        mC.block(ii1, ii0).add_full(B.trans_mul(PA));
        mC.block(ii2, ii0).add_full(PA);
        mC.diag_block(ii1).add_half(B.trans_mul(PB));
        mC.block(ii2, ii1).add_full(PB);
        mC.diag_block(ii2).add_half(P);
    }
    else
    {
        // in this case, swap indices to address lower triangle
        const Matrix22 AtP = A.trans_mul(P);
        const Matrix22 BtP = B.trans_mul(P);
        mC.diag_block(ii2).add_half(P);
        mC.block(ii0, ii2).add_full(AtP);
        mC.block(ii1, ii2).add_full(BtP);
        mC.diag_block(ii0).add_half(AtP.mul(A));
        mC.block(ii1, ii0).add_full(BtP.mul(A));
        mC.diag_block(ii1).add_half(BtP.mul(B));
    }
    
    if ( modulo )
    {
        Vector off = modulo->offset( pte.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off = weight * ( off - dot(off, dir) * dir );
            A.trans_vecmul(off).add_to(vBAS+ii0);
            B.trans_vecmul(off).add_to(vBAS+ii1);
            off.add_to(vBAS+ii2);
        }
    }
    
#else

    // here we multiply whole matrices
    real TPT[6*6];
    {
        // matrix of coefficients 2x6:
        real PT[2*6], T[2*6] = { -aa,  ee, -ee, -aa,
                                 -bb, -ee,  ee, -bb,
                                 1.0, 0.0, 0.0, 1.0 };
        blas::xgemm('N','N', 2, 6, 2, 1.0, P.val, 2, T, 2, 0.0, PT, 2);
        blas::xgemm('T','N', 6, 6, 2, 1.0, T, 2, PT, 2, 0.0, TPT, 6);
    }
    
    //printf("\n"); VecPrint::print(std::clog, 6, 6, TPT, 6);
    
    const index_t ixx[] = { ii0, ii0+1, ii1, ii1+1, ii2, ii2+1 };
    
    for ( int x=0; x<6; ++x )
    for ( int y=x; y<6; ++y )
        mC(ixx[y], ixx[x]) += TPT[y+6*x];
    
    if ( modulo )
    {
        Vector off = modulo->offset( pte.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            for ( int n=0; n<6; ++n )
                vBAS[ixx[n]] -= TPT[n+6*4] * off.XX + TPT[n+6*5] * off.YY;
        }
     }
#endif
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), cross(arm, pta.dir()), pte.pos());
    }
#endif
}


/**
 Alternative 2D method in which we add an offset to vBAS
 */
void Meca::addSideSlidingLinkS(const Interpolation & pta,
                               const Mecapoint & pte,
                               const real arm,
                               const real weight)
{
    assert_true( weight >= 0 );
    
    // indices
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * pte.matIndex();
    
    if ( any_equal(ii0, ii1, ii2) )
        return;
    
    // set vector 'axi' perpendicular to Fiber:
    Vector axi = cross(1.0, pta.dir());
    
    MatrixBlock T = MatrixBlock::outerProduct(axi);
    
    // we set directly the transformed offset vector:
    axi *= arm;
    
    // weights and indices:
    const real cc[3] = {   pta.coef2(),   pta.coef1(),   -1.0 };
    const real ww[3] = { -weight*cc[0], -weight*cc[1], weight };
    
    Matrix33 W = Matrix33::outerProduct(cc, ww);
    
    axi.add_to(ww[0], vBAS+ii0);
    axi.add_to(ww[1], vBAS+ii1);
    axi.add_to(ww[2], vBAS+ii2);

    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii2, ii2, W(2,2), T);
#endif

    if ( modulo )
        throw Exception("interSideSlidingLinkS is not usable with periodic boundary conditions");
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), axi, pte.pos());
    }
#endif
}


#elif ( DIM >= 3 )

/**
 Vector 'arm' must be parallel to the link and orthogonal to 'pta'
 */

void Meca::addSideSlidingLinkS(const Interpolation & pta,
                               const Mecapoint & pte,
                               Vector const& arm,
                               const real len,
                               const real weight)
{    
    assert_true( weight >= 0 );
    assert_true( len > REAL_EPSILON );
    
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * pte.matIndex();

    if ( any_equal(ii0, ii1, ii2) )
        return;
    
    /*
     Without tangential force, a 'long link' is in the perpendicular direction.
     In the local reference frame, the matrix of interaction coefficients would be:
     real T[9] = { 0, 0, 0, 0, -weight, 0, 0, 0, 0 };
     we could transform it with a change-of-coordinates matrix R:
     Vector a = pta.dir();
     Vector b = dir;
     Vector c = cross(a, b);
     real R[9] = { a.XX, a.YY, a.ZZ, b.XX, b.YY, b.ZZ, c.XX, c.YY, c.ZZ };
     real TR[3*3];
     blas::xgemm('N','T', 3, 3, 3, 1.0, T, 3, R, 3, 0.0, TR, 3);
     blas::xgemm('N','N', 3, 3, 3, 1.0, R, 3, TR, 3, 0.0, T, 3);
     equivalently, we can set directly the interaction coefficient matrix: 
     */
    
    Vector axi = arm / len;
    
    MatrixBlock T = MatrixBlock::outerProduct(axi);
    
    // weights and indices:
    const real w = -weight;
    const real cc[3] = { pta.coef2(), pta.coef1(),  -1.0 };
    const real ww[3] = { w * cc[0], w * cc[1], w * cc[2] };
    
    Matrix33 W = Matrix33::outerProduct(cc, ww);
    
    arm.add_to(ww[0], vBAS+ii0);
    arm.add_to(ww[1], vBAS+ii1);
    arm.add_to(ww[2], vBAS+ii2);

    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii2, ii2, W(2,2), T);
#endif

    if ( modulo )
    {
        Vector off = modulo->offset( pte.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off = dot(axi, off) * axi;
            off.add_to(ww[0], vBAS+ii0);
            off.add_to(ww[1], vBAS+ii1);
            off.add_to(ww[2], vBAS+ii2);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), arm, pte.pos());
    }
#endif
    
}
#endif

/**
 Link `pta` (A) and `ptb` (B),
 This is a combination of a SideLink with a Sliding Link:
 The force is linear of zero resting length, but it is taken between B,
 and another point S located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in A, in the direction of B.
 In addition, the tangential part of the force is removed.
 
 If T is the normalized direction of the fiber in A:
 
     force_S = weight * ( 1 - T T' ) ( S - B )
     force_B = weight * ( 1 - T T' ) ( B - S )

 */
void Meca::addSideSlidingLink(const Interpolation & pta,
                              const Mecapoint & ptb,
                              const real len,
                              const real weight)
{
#if ( DIM == 1 )
    
    throw Exception("Meca::addSideLink is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    Vector as = ptb.pos()-pta.pos();
    if ( modulo )
        modulo->fold(as);
    real arm  = std::copysign(len, cross(pta.diff(), as) );
    addSideSlidingLink2D(pta, ptb, arm, weight);
    
#else
    
    // set 'arm' perpendicular to direction of the Fiber associated with `pta`:
    Vector arm = calculateArm(ptb.pos()-pta.pos(), pta.diff(), len);
    addSideSlidingLinkS(pta, ptb, arm, len, weight);
    
#endif
}

#pragma mark -


#if ( DIM == 2 )

// @todo interSideSlidingLink2D should use block operations

void Meca::addSideSlidingLink2D(const Interpolation & pta,
                                  const Interpolation & ptb, 
                                  const real arm,
                                  const real weight)
{
    assert_true( weight >= 0 );

    const index_t inx[] = { DIM*pta.matIndex1(),  DIM*pta.matIndex1()+1,
                            DIM*pta.matIndex2(),  DIM*pta.matIndex2()+1,
                            DIM*ptb.matIndex1(),  DIM*ptb.matIndex1()+1,
                            DIM*ptb.matIndex2(),  DIM*ptb.matIndex2()+1 };

    if ( any_equal(inx[0], inx[2], inx[4], inx[6]) )
        return;

    Vector dir = pta.dir();
    const real A1 =  pta.coef2(), A2 =  pta.coef1();
    const real B1 = -ptb.coef2(), B2 = -ptb.coef1();
    
    const real ee = arm / pta.len();

    //this is done the 'hard' way by multiplying all matrices
    //coefficient matrix:
    real T[2*8] = { A1, -ee, ee, A1, A2, ee, -ee,  A2,
                    B1,   0,  0, B1, B2,  0,   0,  B2 };
    
    //the projection matrix:
    const real P[4] = { 1-dir.XX*dir.XX, -dir.XX*dir.YY, -dir.XX*dir.YY, 1-dir.YY*dir.YY };
    
    real PT[2*8], TPT[8*8];
    blas::xgemm('N','N', 2, 8, 2, -weight, P, 2, T, 2, 0.0, PT, 2);
    blas::xgemm('T','N', 8, 8, 2, 1.0, T, 2, PT, 2, 0.0, TPT, 8);
    
    //printf("\n");  VecPrint::print(8,8, TPT);
    
    for ( int ii=0; ii<8; ++ii )
    for ( int jj=ii; jj<8; ++jj )
        mC(inx[ii], inx[jj]) += TPT[ii+8*jj];
    
    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            for ( int ii=0; ii<8; ++ii )
            {
                vBAS[inx[ii]] -= TPT[ii+8*4] * off.XX;
                vBAS[inx[ii]] -= TPT[ii+8*5] * off.YY;
                vBAS[inx[ii]] -= TPT[ii+8*6] * off.XX;
                vBAS[inx[ii]] -= TPT[ii+8*7] * off.YY;
            }
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), cross(arm, pta.dir()), ptb.pos());
    }
#endif
}

/**
 Alternative 2D method in which we add an offset to vBAS
 */
void Meca::addSideSlidingLinkS(const Interpolation & pta,
                               const Interpolation & ptb,
                               const real arm,
                               const real weight)
{
    assert_true( weight >= 0 );
    
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;

    // set vector 'axi' perpendicular to Fiber:
    Vector axi = cross(1.0, pta.dir());
    
    MatrixBlock T = MatrixBlock::outerProduct(axi);
    
    // we set directly the transformed offset vector:
    axi *= arm;

    // weights and indices:
    const real cc[4] = {   pta.coef2(),   pta.coef1(),  -ptb.coef2(),  -ptb.coef1() };
    const real ww[4] = { -weight*cc[0], -weight*cc[1], -weight*cc[2], -weight*cc[3] };
    
    Matrix44 W = Matrix44::outerProduct(cc, ww);
    
    axi.add_to(ww[0], vBAS+ii0);
    axi.add_to(ww[1], vBAS+ii1);
    axi.add_to(ww[2], vBAS+ii2);
    axi.add_to(ww[3], vBAS+ii3);
    
    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.block(ii3, ii0).add_full(W(3,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.block(ii3, ii1).add_full(W(3,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
    mC.block(ii3, ii2).add_full(W(3,2), T);
    mC.diag_block(ii3).add_half(W(3,3), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii3, ii0, W(3,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii3, ii1, W(3,1), T);
    add_block(ii2, ii2, W(2,2), T);
    add_block(ii3, ii2, W(3,2), T);
    add_block(ii3, ii3, W(3,3), T);
#endif

    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off = off - dot(axi, off) * axi;
            off.add_to(ww[0], vBAS+ii0);
            off.add_to(ww[1], vBAS+ii1);
            off.add_to(ww[2], vBAS+ii2);
            off.add_to(ww[3], vBAS+ii3);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), axi, ptb.pos());
    }
#endif
}


#elif ( DIM >= 3 )

    /**
     Vector 'arm' must be parallel to the link and orthogonal to 'pta'
     */
void Meca::addSideSlidingLinkS(const Interpolation & pta,
                               const Interpolation & ptb,
                               Vector const& arm,
                               const real len,
                               const real weight)
{
    assert_true( weight >= 0 );
    assert_true( len > REAL_EPSILON );
    
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t ii2 = DIM * ptb.matIndex1();
    const index_t ii3 = DIM * ptb.matIndex2();
    
    if ( any_equal(ii0, ii1, ii2, ii3) )
        return;

    /*
     Without tangential force, a 'long link' is in the perpendicular direction.
     In the local reference frame, the matrix of interaction coefficients would be:
     real T[9] = { 0, 0, 0, 0, -weight, 0, 0, 0, 0 };
     we could transform it with a change-of-coordinates matrix R:
     Vector a = pta.dir();
     Vector b = dir;
     Vector c = cross(a, b);
     real R[9] = { a.XX, a.YY, a.ZZ, b.XX, b.YY, b.ZZ, c.XX, c.YY, c.ZZ };
     real TR[3*3];
     blas::xgemm('N','T', 3, 3, 3, 1.0, T, 3, R, 3, 0.0, TR, 3);
     blas::xgemm('N','N', 3, 3, 3, 1.0, R, 3, TR, 3, 0.0, T, 3);
     equivalently, we can set directly the interaction coefficient matrix: 
     */
    
    Vector axi = arm / len;
    
    MatrixBlock T = MatrixBlock::outerProduct(axi);
    
    // weights and indices:
    const real cc[4] = {   pta.coef2(),   pta.coef1(),   -ptb.coef2(), -ptb.coef1() };
    const real ww[4] = { -weight*cc[0], -weight*cc[1], -weight*cc[2], -weight*cc[3] };
    
    Matrix44 W = Matrix44::outerProduct(cc, ww);

    arm.add_to(ww[0], vBAS+ii0);
    arm.add_to(ww[1], vBAS+ii1);
    arm.add_to(ww[2], vBAS+ii2);
    arm.add_to(ww[3], vBAS+ii3);

    // fill the matrix mC
#if USE_MATRIX_BLOCK
    mC.diag_block(ii0).add_half(W(0,0), T);
    mC.block(ii1, ii0).add_full(W(1,0), T);
    mC.block(ii2, ii0).add_full(W(2,0), T);
    mC.block(ii3, ii0).add_full(W(3,0), T);
    mC.diag_block(ii1).add_half(W(1,1), T);
    mC.block(ii2, ii1).add_full(W(2,1), T);
    mC.block(ii3, ii1).add_full(W(3,1), T);
    mC.diag_block(ii2).add_half(W(2,2), T);
    mC.block(ii3, ii2).add_full(W(3,2), T);
    mC.diag_block(ii3).add_half(W(3,3), T);
#else
    add_block(ii0, ii0, W(0,0), T);
    add_block(ii1, ii0, W(1,0), T);
    add_block(ii2, ii0, W(2,0), T);
    add_block(ii3, ii0, W(3,0), T);
    add_block(ii1, ii1, W(1,1), T);
    add_block(ii2, ii1, W(2,1), T);
    add_block(ii3, ii1, W(3,1), T);
    add_block(ii2, ii2, W(2,2), T);
    add_block(ii3, ii2, W(3,2), T);
    add_block(ii3, ii3, W(3,3), T);
#endif

    if ( modulo )
    {
        Vector off = modulo->offset( ptb.pos() - pta.pos() );
        if ( off.is_not_zero() )
        {
            off = dot(axi, off) * axi;
            off.add_to(ww[0], vBAS+ii0);
            off.add_to(ww[1], vBAS+ii1);
            off.add_to(ww[2], vBAS+ii2);
            off.add_to(ww[3], vBAS+ii3);
        }
    }
    
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        drawLink(pta.pos(), arm, ptb.pos());
    }
#endif
    
}

#endif

/**
 Link `pta` (A) and `ptb` (B),
 This is a combination of Side- and Sliding Links:
 The force is linear of zero resting length, but it is taken between B
 and another point S which is located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in A, in the direction of B.
 In addition, the part of the force tangential to A is removed.
 
 If T is the normalized direction of the fiber in A:
 
     force_S = weight * ( 1 - T T' ) ( S - B )
     force_B = weight * ( 1 - T T' ) ( B - S )
 
 */

void Meca::addSideSlidingLink(const Interpolation & pta,
                              const Interpolation & ptb,
                              const real len,
                              const real weight)
{
#if ( DIM == 1 )
    
    throw Exception("Meca::addSideSlidingLink is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    real arm = std::copysign(len, cross(pta.diff(), ptb.pos()-pta.pos()) );
    addSideSlidingLink2D(pta, ptb, arm, weight);
    
#else
    
    // set 'arm' perpendicular to direction of the Fiber associated with `pta`:
    Vector arm = calculateArm(ptb.pos()-pta.pos(), pta.diff(), len);
    addSideSlidingLinkS(pta, ptb, arm, len, weight);
    
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Links to fixed positions
//------------------------------------------------------------------------------

/**
 Link `pta` (A) and a fixed position `pos` (G)
 The force is linear:
 
     force_A = weight * ( G - A );
 
 There is no counter-force in G, since G is immobile.
 */

void Meca::addPointClamp(Mecapoint const& pta,
                         Vector pos,
                         const real weight)
{
    assert_true( weight >= 0 );
    const index_t inx = pta.matIndex();
    
    mB(inx, inx) -= weight;
    
    if ( modulo )
        modulo->fold(pos, pta.pos());

    pos.add_to(weight, vBAS+DIM*inx);
}


/**
 Link `pti` (A) and a fixed position `pos` (G)
 The force is linear:
 
     force_A = weight * ( G - A );
 
 The point G is not associated to a Mecable, and there is no counter-force in G.
 */

void Meca::addPointClamp(Interpolation const& pti,
                         Vector pos,
                         const real weight)
{
    assert_true( weight >= 0 );
    
    const index_t ii0 = pti.matIndex1();
    const index_t ii1 = pti.matIndex2();
    
    const real c1 = pti.coef2();
    const real c2 = pti.coef1();
    
    assert_true( 0 <= c1  &&  c1 <= 1 );
    assert_true( 0 <= c2  &&  c2 <= 1 );

    const real c2w = weight * c2;
    const real c1w = weight * c1;
    
    mB(ii0, ii0) -= c1w * c1;
    mB(ii0, ii1) -= c2w * c1;
    mB(ii1, ii1) -= c2w * c2;
    
    if ( modulo )
        modulo->fold(pos, pti.pos());
    
    pos.add_to(c1w, vBAS+DIM*ii0);
    pos.add_to(c2w, vBAS+DIM*ii1);
}


//------------------------------------------------------------------------------
#pragma mark - Links to fixed sphere and cylinder
//------------------------------------------------------------------------------

/**
 Link `pte` (P) and a fixed sphere of radius `rad` and center `center` (C)
 The force is affine with non-zero resting length:

     force = weight * ( C - P ) * ( 1 - rad / |PC| )

 The constant part is:
 
      weight * ( C - P ) * ( 1 - rad / |PC| )

 for a point inside, this is directed outward.
 There is no force on the center C, which is an immobile position.
 */

void Meca::addSphereClamp(Vector const& pos,
                          Mecapoint const& pte,
                          Vector const& center,
                          real rad,
                          const real weight)
{
    assert_true( rad >= 0 );
    assert_true( weight >= 0 );
    const index_t inx = DIM * pte.matIndex();
    
    Vector dir = pos - center;
    
    real len = dir.norm();
    
    if ( len > REAL_EPSILON )
        dir /= len;
    else
    {
        dir = Vector::randU();
        len = rad;
    }

    MatrixBlock T;
    if ( rad < len )
    {
        // point is outside sphere
        real R = weight * rad / len;
        // T = dia * I - len * [ I - dir (x) dir ]
        T = MatrixBlock::offsetOuterProduct(weight-R, dir, R);
        
        real facX = weight * rad + R * dot(dir, center);
        real facC = weight - R;
        ( facX * dir + facC * center ).add_to(vBAS+inx);
    }
    else
    {
        // point is inside sphere
        T = MatrixBlock::outerProduct(dir, weight);
        real facX = weight * ( rad + dot(dir, center) );
        ( facX * dir ).add_to(vBAS+inx);
    }
    
#if USE_MATRIX_BLOCK
    mC.diag_block(inx).sub_half(T);
#else
    sub_block(inx, inx, T);
#endif
    
    if ( modulo )
        throw Exception("addSphereClamp is not usable with periodic boundary conditions");
}


/**
 Link `pte` (P) to a cylinder of axis X and radius `rad`
 The force is affine with non-zero resting length:
 
     G = Vector(P.XX, 0, 0)
     force = weight * ( G - P ) * ( rad / |PG| - 1 )
 
 The force is only in the YZ plane.
 */

void Meca::addCylinderClampX(const Mecapoint & pte,
                             real  rad,
                             const real weight)
{
    assert_true( weight >= 0 );
    const index_t inx = DIM * pte.matIndex();
    
#if ( DIM == 2 )
    
    mC(inx+1, inx+1) -= weight;
    vBAS[inx+1]      += weight * std::copysign(rad, pte.pos().YY);
    
#elif ( DIM >= 3 )

    Vector pos = pte.pos();
    real dir_n = pos.normYZ();
    if ( dir_n < REAL_EPSILON )
        return;
    
    Vector dir(0, pos.YY/dir_n, pos.ZZ/dir_n);
    
    real facX;
    
    if ( rad < dir_n )
    {
        rad /= dir_n;
        facX = weight * rad * dir_n;
        
        mC(inx+1, inx+1) -= weight * ( 1.0 - rad * ( 1.0 - dir.YY * dir.YY ) );
        mC(inx+1, inx+2) -= weight * rad * dir.YY * dir.ZZ;
        mC(inx+2, inx+2) -= weight * ( 1.0 - rad * ( 1.0 - dir.ZZ * dir.ZZ ) );
    }
    else
    {
        facX = weight * rad;

        mC(inx+1, inx+1) -= weight * dir.YY * dir.YY;
        mC(inx+1, inx+2) -= weight * dir.YY * dir.ZZ;
        mC(inx+2, inx+2) -= weight * dir.ZZ * dir.ZZ;
    }
    
    // there should be no XX component here!
    vBAS[inx+1] += facX * dir.YY;
    vBAS[inx+2] += facX * dir.ZZ;
    
#endif
}


/**
 Link `pte` (P) to a cylinder of axis Z and radius `rad`
 The force is affine with non-zero resting length:
 
     G = Vector(0, 0, P.ZZ)
     force = weight * ( G - P ) * ( rad / |PG| - 1 )
 
 The force is constrained in the XY plane.
 */

void Meca::addCylinderClampZ(const Mecapoint & pte,
                             real  rad,
                             const real weight)
{
    assert_true( weight >= 0 );
    
#if ( DIM > 1 )

    const index_t inx = DIM * pte.matIndex();
    Vector pos = pte.pos();
    real dir_n = pos.normXY();
    if ( dir_n < REAL_EPSILON )
        return;
    
    Vector dir(pos.XX/dir_n, pos.YY/dir_n, 0);

    real facX;
    
    if ( rad < dir_n )
    {
        rad /= dir_n;
        facX = weight * rad * dir_n;
        
        mC(inx  , inx  ) -= weight * ( 1.0 - rad * ( 1.0 - dir.XX * dir.XX ) );
        mC(inx  , inx+1) -= weight * rad * dir.XX * dir.YY;
        mC(inx+1, inx+1) -= weight * ( 1.0 - rad * ( 1.0 - dir.YY * dir.YY ) );
    }
    else
    {
        facX = weight * rad;
        
        mC(inx  , inx  ) -= weight * dir.XX * dir.XX;
        mC(inx  , inx+1) -= weight * dir.XX * dir.YY;
        mC(inx+1, inx+1) -= weight * dir.YY * dir.YY;
    }
    
    vBAS[inx  ] += facX * dir.XX;
    vBAS[inx+1] += facX * dir.YY;
    // there should be no ZZ component here!

#endif
}


//------------------------------------------------------------------------------
#pragma mark - Off-axis links to fixed positions
//------------------------------------------------------------------------------


#if ( DIM == 2 )

void Meca::addSidePointClamp2D(Interpolation const& pta,
                               Vector const& pos,
                               const real arm,
                               const real weight)
{
    //force coefficients on the points:
    const real A = pta.coef2(),  wA = weight * A;
    const real B = pta.coef1(),  wB = weight * B;
    
    const real E = arm / pta.len();
    const real wE = weight * E;
    const real wEE = weight * E * E;
    
    //index in the matrix mB:
    index_t ii0 = pta.matIndex1();
    index_t ii1 = pta.matIndex2();
    
    //we put the isotropic terms in mB
    mB(ii0, ii0) -=  wA * A + wEE;
    mB(ii0, ii1) -=  wA * B - wEE;
    mB(ii1, ii1) -=  wB * B + wEE;
    
    //index in the matrix mC:
    ii0 *= DIM;
    ii1 *= DIM;
    
    mC(ii0  , ii1+1) += wE;
    mC(ii0+1, ii1  ) -= wE;
    
    //it seems to works also fine without the term in eew* below:
    vBAS[ii0  ] += wA * pos.XX - wE * pos.YY;
    vBAS[ii0+1] += wA * pos.YY + wE * pos.XX;
    vBAS[ii1  ] += wB * pos.XX + wE * pos.YY;
    vBAS[ii1+1] += wB * pos.YY - wE * pos.XX;

#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), cross(arm, pta.dir()), pos);
    }
#endif
    
    if ( modulo )
        throw Exception("addSidePointClamp2D is not usable with periodic boundary conditions");
}

#elif ( DIM >= 3 )

/**
 A link of stiffness `weight`, between offset_point on the side of `pta`, and the fixed position `pos` (G).

 This uses the vector product x -> cross(arm, x) to offset the point on which the link is attached:
 
     offset_point = fiber_point + cross(arm, fiber_dir)
 
 with fiber_point = pta.pos() and fiber_dir = pta.diff().normalized.
 `arm` must be perpendicular to link ( G - pta.pos() )

 F. Nedelec, March 2011
 
 @todo addSidePointClamp3D should use block operations
 */
void Meca::addSidePointClamp3D(Interpolation const& pta,
                               Vector const& pos,
                               Vector const& arm,
                               real const weight)
{
    real aa = pta.coef2();
    real bb = pta.coef1();
    
    real s = 1.0 / pta.len();

    real ex = s * arm.XX;
    real ey = s * arm.YY;
    real ez = s * arm.ZZ;
    
    // indices to mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();
    const index_t inx[6] = { ii0, ii0+1, ii0+2, ii1, ii1+1, ii1+2 };
    
    /* The transfer matrix transforms the two Mecapoint in pta,
     to the side point S:
     S = aa * pt1 + bb * pt2 + cross(arm, normalize( pt2 - pt1 ))
     
     It was generated in Maxima:
     MVP: matrix([0, -ez, ey], [ez, 0, -ex], [-ey, ex, 0]);
     MD: addcol(-ident(3), ident(3));
     MC: addcol(aa*ident(3), bb*ident(3));
     T: MC+MVP.MD;
     */
    const real T[18] = {
         aa,  ez, -ey,  bb, -ez,  ey,
        -ez,  aa,  ex,  ez,  bb, -ex,
         ey, -ex,  aa, -ey,  ex,  bb
    };
    
#if ( 0 )
    
    real TT[36];
    // TT = transpose(T) * T
    blas::xsyrk('U','N', 6, 3, 1.0, T, 6, 0.0, TT, 6);
    
#else
    
    real a2 = aa * aa;
    real b2 = bb * bb;
    real ab = aa * bb;
    
    real exx = ex * ex, exy = ex*ey, exz = ex*ez;
    real eyy = ey * ey, eyz = ey*ez;
    real ezz = ez * ez;
    
    // TT = transpose(T) * T is symmetric, and thus we only set half of it:
    /* Maxima code:
    TT: expand(transpose(T) . T);
     */
    real TT[36] = {
        eyy+ezz+a2,  0,           0,           0,           0,           0,
        -exy,        exx+ezz+a2,  0,           0,           0,           0,
        -exz,       -eyz,         exx+eyy+a2,  0,           0,           0,
        -ezz-eyy+ab, ez+exy,      exz-ey,      eyy+ezz+b2,  0,           0,
        -ez+exy,    -ezz-exx+ab,  eyz+ex,     -exy,         exx+ezz+b2,  0,
        exz+ey,      eyz-ex,     -eyy-exx+ab, -exz,        -eyz,         exx+eyy+b2
    };
    
#endif
    
    // we project to bring all forces in the plane perpendicular to 'arm'
    real sca = arm.inv_norm();
    real aan = aa * sca;
    real bbn = bb * sca;
    real TP[6] = { aan*ex, aan*ey, aan*ez, bbn*ex, bbn*ey, bbn*ez };
    
    //blas::xgemm('N','N', 6, 1, 3, sca, T, 6, arm, 3, 0.0, TP, 6);
    blas::xsyrk('U','N', 6, 1, weight, TP, 6, -weight, TT, 6);
    
    for ( int ii=0; ii<6; ++ii )
    for ( int jj=ii; jj<6; ++jj )
        mC(inx[ii], inx[jj]) += TT[ii+6*jj];
    
    // { gx, gy, gz } is the projection of `pos` in the plane perpendicular to 'arm'
    real ws = dot(arm, pos) * sca * sca;
    real gx = weight * ( pos.XX - ws * arm.XX );
    real gy = weight * ( pos.YY - ws * arm.YY );
    real gz = weight * ( pos.ZZ - ws * arm.ZZ );
    
    for ( int ii=0; ii<6; ++ii )
        vBAS[inx[ii]] += T[ii] * gx + T[ii+6] * gy + T[ii+12] * gz;
                 
#if DRAW_MECA_LINKS
    if ( drawLinks )
    {
        gle::bright_color(pta.mecable()->signature()).load();
        gle::drawLink(pta.pos(), cross(arm, pta.dir()), pos);
    }
#endif
    
    if ( modulo )
        throw Exception("addSidePointClamp3D is not usable with periodic boundary conditions");
}

#endif  

/**
 Update Meca to include a connection between `pta` (A) and a fixed position `pos` (G).
 The force is of zero resting length, but it is taken between G
 and another point S which is located on the side of the segment supporting A:
 
     S = A + len * N,
     force_S = weight * ( G - S )
 
 There is no counter-force in G, since G is immobile.
 */

void Meca::addSidePointClamp(Interpolation const& pta,
                             Vector const& pos,
                             const real len,
                             const real weight)
{
#if ( DIM == 1 )
    
    throw Exception("Meca::addSidePointClamp is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    // 'arm' is a vector in the Z direction
    real arm = std::copysign(len, cross(pta.diff(), pos-pta.pos()));
    addSidePointClamp2D(pta, pos, arm, weight);
   
#else
    
    // 'arm' perpendicular to link and fiber is obtained by vector product:
    Vector arm = cross( pta.pos()-pos, pta.diff() );
    real n = arm.norm();
    if ( n > REAL_EPSILON )
        addSidePointClamp3D(pta, pos, arm * ( len / n ), weight);

#endif  
}

//------------------------------------------------------------------------------
#pragma mark - Links to lines and planes
//------------------------------------------------------------------------------

/**
 Link `pta` (X) to the line defined by `G` and tangent vector `dir`.
 The force is linear with position with a stiffness `weight`, and its
 components parallel to `dir` are removed corresponding to a frictionless line:
 
     matrix M = 1 - dir (x) dir'
     force_X = weight * M * ( G - X )
 
 The vector `dir` should be of norm = 1.
 */

void Meca::addLineClamp(const Mecapoint & pta,
                        Vector const& pos,
                        Vector const& dir,
                        const real weight )
{
    assert_true( weight >= 0 );
    
    const index_t inx = DIM * pta.matIndex();
    
    
    // T = -weight * [ I - dir (x) dir ]
    MatrixBlock T = MatrixBlock::offsetOuterProduct(-weight, dir, weight);

#if USE_MATRIX_BLOCK
    mC.diag_block(inx).add_half(T);
#else
    add_block(inx, inx, T);
#endif
    
    Vector off = weight * ( pos - dot(pos, dir) * dir );
    off.add_to(vBAS+inx);
}


/**
 Link `pta` and the line defined by `pos` (C) and tangent vector `dir`.
 The force is linear with position with a stiffness `weight`, and its
 components parallel to `dir` are removed corresponding to a frictionless line:
 
     M = I - dir (x) dir'
     force = weight * M * ( C - P )
 
 The vector `dir` should be of norm = 1.
 */

void Meca::addLineClamp(const Interpolation & pta,
                        Vector const& pos,
                        Vector const& dir,
                        const real weight )
{
    assert_true( weight >= 0 );
    
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();

    //force coefficients on the points:
    const real A = pta.coef2();
    const real B = pta.coef1();

    // T = -weight * [ I - dir (x) dir ]
    MatrixBlock T = MatrixBlock::offsetOuterProduct(-weight, dir, weight);

#if USE_MATRIX_BLOCK

    mC.diag_block(ii0).add_half(A*A, T);
    mC.diag_block(ii1).add_half(B*B, T);
    mC.block(ii0, ii1).add_full(A*B, T);
    
#else

    add_block(ii0, ii0, A*A, T);
    add_block(ii1, ii1, B*B, T);
    add_block(ii0, ii1, A*B, T);

#endif
    
    //add the constant term:
    Vector off = weight * ( pos - dot(pos, dir) * dir );
    off.add_to(A, vBAS+ii0);
    off.add_to(B, vBAS+ii1);
}


/**
 Link `pta` (X) and the plane defined one of its point `G` and the normal `dir`.
 The force is linear and the components parallel to the plane are removed,
 corresponding to an interaction with a frictionless plane:
 
     matrix M = dir (x) dir'
     force_X = weight * M * ( G - X )
 
 The vector `dir` should be of norm = 1, or alternatively
 `weight` can be the true weigth divided by |dir|^2.
 */

void Meca::addPlaneClamp(const Mecapoint & pta, 
                         Vector const& pos,
                         Vector const& dir,
                         const real weight )
{
    assert_true( weight >= 0 );
    
    const index_t inx = DIM * pta.matIndex();
    
    // vBAS[inx] += dir * ( weigth * dot(pos,dir) );
    dir.add_to(weight*dot(pos, dir), vBAS+inx);
    
#if ( DIM == 1 )
    mC(inx, inx) -= weight;
#else
    MatrixBlock T = MatrixBlock::outerProduct(dir, -weight);
# if USE_MATRIX_BLOCK
    mC.diag_block(inx).add_half(T);
# else
    add_block(inx, inx, T);
# endif
#endif
}


/**
 Link `pta` (X) and the plane defined by `pos` (G) and the normal `dir`.
 The force is linear and the perpendicular forces are removed, to create a frictionless plane:
 
     M = dir (x) dir'
     force = weight * M * ( G - X )
 
 The vector `dir` should be of norm = 1, or alternatively 
 `weight` can be the true weigth divided by |dir|^2.
 */

void Meca::addPlaneClamp(const Interpolation & pta,
                         Vector const& pos,
                         Vector const& dir,
                         const real weight )
{
    assert_true( weight >= 0 );
    
    //index in the matrix mC:
    const index_t ii0 = DIM * pta.matIndex1();
    const index_t ii1 = DIM * pta.matIndex2();

    //force coefficients on the points:
    const real A = pta.coef2();
    const real B = pta.coef1();
    
    //add the constant term:
    Vector off = ( weight * dot(pos, dir)) * dir;
    off.add_to(A, vBAS+ii0);
    off.add_to(B, vBAS+ii1);
    
    MatrixBlock T = MatrixBlock::outerProduct(dir, -weight);
    
#if USE_MATRIX_BLOCK
    
    mC.diag_block(ii0).add_half(A*A, T);
    mC.diag_block(ii1).add_half(B*B, T);
    mC.block(ii0, ii1).add_full(A*B, T);
    
#else
    
    add_block(ii0, ii0, A*A, T);
    add_block(ii1, ii1, B*B, T);
    add_block(ii0, ii1, A*B, T);

#endif
}


//------------------------------------------------------------------------------
#pragma mark - Experimental interactions
//------------------------------------------------------------------------------

/**
Links { pt1, pt2, pt3 } to a dragless junction `X` with stiffness { w1, w2, w3 }.

                           pt2
                          /
                  pt1 -- X
                          \
                           pt3

The position of the virtual point `X` is always determined by force balance:

    0 = w1 * ( pt1 - X ) + w2 * ( pt2 - X ) + w3 * ( pt3 - X )

The force on `pt1` is then Hookean:

    w1 * ( X - pt1 )

and similarly for the other points.
 
We first derive:

    X = ( w1 * pt1 + w2 * pt2 + w3 * pt3 ) / sum

and for the first point:

    f1 = ( w1 / sum ) * [ w2 * ( pt2 - pt1 ) + w3 * ( pt3 - pt1 ) ]
*/
void  Meca::addTriLink(Interpolation const& pt1, const real w1,
                       Interpolation const& pt2, const real w2,
                       Interpolation const& pt3, const real w3)
{
    const real sum = w1 + w2 + w3;
    assert_true( sum > REAL_EPSILON );
    addLink(pt1, pt2, w1*w2/sum);
    addLink(pt1, pt3, w1*w3/sum);
    addLink(pt2, pt3, w2*w3/sum);
}

