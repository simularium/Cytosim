// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by F. Nedelec, Oct 2002


#ifndef QUATERNION_H
#define QUATERNION_H

#include "assert_macro.h"
#include "random.h"
#include <cmath>
#include <cstdio>
#include <iostream>

/** 
 Quaternions extend complex numbers to dimension 4.
 
 http://en.wikipedia.org/wiki/Quaternion

 The unit bases of the Quaternion space are called 1, i, j and k.
 A quaternion is therefore q[0] + i * q[1] + j * q[2] + k * q[3],
 where q[?] are four real scalars.
 q[0] is the real part, and the other parts are imaginary.
 
 While addition is standard, multiplication is anti-commutative:
 - i*i = -1,
 - i*j =  k,
 - j*i = -k,
 - etc,
 .
 
 Unit quaternions are handy to represent rotations in 3D space:
 The group of 3D rotations has three degrees of freedom,
 and is mapped directly onto the quaternions of norm 1.
 
 A rotation is represented by a symmetric matrix using 6 scalar numbers, 
 but only 4 scalars are used when a Quaternion is used.
 Thus one economize spurious scalars, and moreover quaternions are easier 
 to normalize than rotation matrices as necessary to correct for numerical errors.
 
 The rotation associated to a unit quaternion Q is:
 
     v -> Q.v.inv(Q)
 
 where the imaginary quaternion v = { 0, x, y, z } represents a 3D vector { x, y, z }.
 
 Note that it is more costly to calculate a rotated vector using this formula
 than with a 3x3 matrix-vector multiplication.
 
 The composition of two rotations thus corresponds to quaternion multiplication.
 For example, Q*P corresponds to the rotation P followed by the rotation Q.
 Thus 1/Q is the rotation that is inverse to the rotation associated with Q.
 
 The angle A of the rotation associated with the quaternion Q obeys:
 - real part of Q = cos(A/2),
 - norm of imaginary part of Q = sin(A/2).
 The rotation axis is defined by the imaginary components of Q.
 
 Quaternion<real> implements the standard mathematical operations, 
 conversions to and from 3x3 real matrices, 
 and to 4x4 transformation matrices used in OpenGL.
 */


/// a Quaternion is similar to a complex number, but in dimension four
template <typename R>
class Quaternion 
{

private:
    
    /// The four coordinates of a Quaternion
    /** this represents q[0] + i * q[1] + j * q[2] + k * q[3] */
    R q[4];
    
public:
    
    /// The default constructor does not reset any value
    Quaternion() {}
    
    /// Constructor which can be used to convert from a real
    Quaternion(R a, R b, R c, R d)
    {
        q[0] = a;
        q[1] = b;
        q[2] = c;
        q[3] = d;
    }
    
    /// Destructor (ATTENTION: non-virtual: do not derive from this class)
    ~Quaternion() {}
    
    /// setting the values from Cartesian coordinates
    void set(R a, R b, R c, R d)
    {
        q[0] = a;
        q[1] = b;
        q[2] = c;
        q[3] = d;
    }
    
    /// access to a modifiable coordinate
    R& operator[] ( int n )  { return q[n]; }
    
    /// access to a non-modifiable coordinate 
    R  operator[] ( int n ) const  { return q[n]; };
    
    /// conversion operator to a "real array"
    operator R*() { return q; }
    
    /// conversion to a 'real array'
    R *    data() { return q; }
    
    /// opposition: change sign in all coordinates 
    Quaternion operator - () const
    {
        return Quaternion(-q[0], -q[1], -q[2], -q[3]);
    }
    
    /// multiply by a real value
    Quaternion operator * ( R f ) const
    {
        return Quaternion(q[0]*f, q[1]*f, q[2]*f, q[3]*f);
    }
    
    /// divide by a real value
    Quaternion operator / ( R f ) const
    {
        return Quaternion(q[0]/f, q[1]/f, q[2]/f, q[3]/f);
    }
    
    /// add a real value in place
    void operator += ( R f )
    {
        q[0] += f;
    }
    
    /// subtract a real value in place
    void operator -= ( R f )
    {
        q[0] -= f;
    }
    
    /// multiply for a real value in place
    void operator *= ( R f )
    {
        q[0] *= f;
        q[1] *= f;
        q[2] *= f;
        q[3] *= f;
    }
    
    /// divide by a real value in place
    void operator /= ( R f )
    {
        q[0] /= f;
        q[1] /= f;
        q[2] /= f;
        q[3] /= f;
    }
    
    /// sum two quaternions
    const Quaternion  operator + ( const Quaternion &a ) const
    {
        return Quaternion(q[0]+a.q[0], q[1]+a.q[1], q[2]+a.q[2], q[3]+a.q[3]);
    }
    
    /// subtract two quaternions
    const Quaternion  operator - ( const Quaternion &a ) const
    {
        return Quaternion(q[0]-a.q[0], q[1]-a.q[1], q[2]-a.q[2], q[3]-a.q[3]);
    }
    
    /// add another quaternion in place
    void operator += ( const Quaternion & a )
    {
        q[0] += a.q[0];
        q[1] += a.q[1];
        q[2] += a.q[2];
        q[3] += a.q[3];
    }
    
    /// subtract a quaternion in place
    void operator -= ( const Quaternion & a )
    {
        q[0] -= a.q[0];
        q[1] -= a.q[1];
        q[2] -= a.q[2];
        q[3] -= a.q[3];
    }
    
    /// multiplication from the right side
    void operator *= ( const Quaternion & a )
    {
        rightMult(a);
    }
    
    /// divide in place by another quaternion
    void operator /= ( const Quaternion & a )
    {
        rightMult( a.inverted() );
    }
    
    /// multiplication between quaternions
    const Quaternion operator * ( const Quaternion & a ) const
    {
        Quaternion result(q[0], q[1], q[2], q[3]);
        result.rightMult( a );
        return result;
    }
    
    /// division between quaternions
    const Quaternion operator / ( const Quaternion & a ) const
    {
        Quaternion  result(q[0], q[1], q[2], q[3]);
        result.rightMult( a.inverted() );
        return result;
    }
    
    /// extract the square of the norm, i.e. norm*norm
    R normSqr() const
    {
        return q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3];
    }
    
    /// extract the norm 
    R norm() const
    {
        return sqrt( q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3] );
    }
    
    /// return the normalized quaternion
    const Quaternion normalized(R n = 1.0) const
    {
        R s = n / norm();
        return Quaternion(q[0]*s, q[1]*s, q[2]*s, q[3]*s );
    }
    
    /// return the normalized Quaternion
    friend Quaternion normalize(Quaternion a)
    {
        R s = 1.0 / a.norm();
        return Quaternion(a[0]*s, a[1]*s, a[2]*s, a[3]*s );
    }

    /// scale in place to obtain norm = `n`
    void normalize(R n = 1.0)
    {
        R s = n / norm();
        q[0] *= s;
        q[1] *= s;
        q[2] *= s;
        q[3] *= s;
    }
    
    /// conjugated quaternion +, -, -, -
    const Quaternion conjugated() const
    {
        return Quaternion(q[0], -q[1], -q[2], -q[3]);
    }
    
    /// conjugate in place +, -, -, -
    void conjugate()
    {
        q[1] = -q[1];
        q[2] = -q[2];
        q[3] = -q[3];
    }
    
    /// inversed quaternion:  1/*this
    const Quaternion inverted() const
    {
        R x = -normSqr();
        return Quaternion( -q[0]/x, q[1]/x, q[2]/x, q[3]/x );
    }
    
    /// inverse in place
    void inverse()
    {
        R x = -normSqr();
        q[0] /= -x;
        q[1] /=  x;
        q[2] /=  x;
        q[3] /=  x;
    }
    
    /// the opposed quaternion:  -*this
    const Quaternion opposed() const
    {
        return Quaternion( -q[0], -q[1], -q[2], -q[3] );
    }
    
    /// oppose in place
    void oppose()
    {
        q[0] = -q[0];
        q[1] = -q[1];
        q[2] = -q[2];
        q[3] = -q[3];
    }
    
    /// this * this
    const Quaternion squared() const
    {
        return Quaternion(q[0]*q[0] - q[1]*q[1] - q[2]*q[2] - q[3]*q[3],
                          2*q[0]*q[1],
                          2*q[0]*q[2],
                          2*q[0]*q[3]);
    }
    
    /// this = this * this
    void square()
    {
        R a = q[0], b = q[1], c = q[2], d = q[3];
        q[0] = a*a - b*b - c*c - d*d;
        a += a;
        q[1] = a*b;
        q[2] = a*c;
        q[3] = a*d;
    }
    
    /// multiplication from the right side ( this <= this * a )
    void rightMult( const Quaternion & a )
    {
        R q0 = q[0],   q1 = q[1],   q2 = q[2],   q3 = q[3];
        
        q[0] = q0 * a.q[0] - q1 * a.q[1] - q2 * a.q[2] - q3 * a.q[3];
        q[1] = q0 * a.q[1] + q1 * a.q[0] + q2 * a.q[3] - q3 * a.q[2];
        q[2] = q0 * a.q[2] - q1 * a.q[3] + q2 * a.q[0] + q3 * a.q[1];
        q[3] = q0 * a.q[3] + q1 * a.q[2] - q2 * a.q[1] + q3 * a.q[0];
    }
    
    /// multiplication from the left side ( this <= a * this )
    void leftMult( const Quaternion & a )
    {
        R q0 = q[0],   q1 = q[1],   q2 = q[2],   q3 = q[3];
        
        q[0] = q0 * a.q[0] - q1 * a.q[1] - q2 * a.q[2] - q3 * a.q[3];
        q[1] = q0 * a.q[1] + q1 * a.q[0] - q2 * a.q[3] + q3 * a.q[2];
        q[2] = q0 * a.q[2] + q1 * a.q[3] + q2 * a.q[0] - q3 * a.q[1];
        q[3] = q0 * a.q[3] - q1 * a.q[2] + q2 * a.q[1] + q3 * a.q[0];
    }
    
    /// multiplication from the right side, different implementation
    void rightMult_fast( const Quaternion & a )
    {
        R E = (q[3] + q[1]) * (a.q[1] + a.q[2]);
        R F = (q[3] - q[1]) * (a.q[1] - a.q[2]);
        R G = (q[0] + q[2]) * (a.q[0] - a.q[3]);
        R H = (q[0] - q[2]) * (a.q[0] + a.q[3]);
        R A = F - E;
        R B = F + E;
        R C = (q[0] + q[1]) * (a.q[0] + a.q[1]);
        R D = (q[0] - q[1]) * (a.q[2] + a.q[3]);
        E = (q[3] + q[2]) * (a.q[0] - a.q[1]);
        F = (q[3] - q[2]) * (a.q[2] - a.q[3]);
        q[0] = F + (A + G + H) * 0.5;
        q[1] = C + (A - G - H) * 0.5;
        q[2] = D + (B + G - H) * 0.5;
        q[3] = E + (B - G + H) * 0.5;
    }
    
    /// multiplication from the left side, different implementation
    void leftMult_fast( const Quaternion & a )
    {
        R E = (a.q[3] + a.q[1])*(q[1] + q[2]);
        R F = (a.q[3] - a.q[1])*(q[1] - q[2]);
        R G = (a.q[0] + a.q[2])*(q[0] - q[3]);
        R H = (a.q[0] - a.q[2])*(q[0] + q[3]);
        R A = F - E;
        R B = F + E;
        R C = (a.q[0] + a.q[1])*(q[0] + q[1]);
        R D = (a.q[0] - a.q[1])*(q[2] + q[3]);
        E = (a.q[3] + a.q[2])*(q[0] - q[1]);
        F = (a.q[3] - a.q[2])*(q[2] - q[3]);
        q[0] = F + (A + G + H) * 0.5;
        q[1] = C + (A - G - H) * 0.5;
        q[2] = D + (B + G - H) * 0.5;
        q[3] = E + (B - G + H) * 0.5;
    }
    
    
    /// generate the associated 3x3 rotation matrix for unit Quaternion
    /** This assumes that norm(*this) = 1 */
    void setMatrix3( R mat[], int ldd ) const
    {
        R rx, ry, rz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
        
        x2 = q[1] + q[1];
        y2 = q[2] + q[2];
        z2 = q[3] + q[3];
        
        rx = q[0] * x2; ry = q[0] * y2; rz = q[0] * z2;
        xx = q[1] * x2; xy = q[1] * y2; xz = q[1] * z2;
        yy = q[2] * y2; yz = q[2] * z2; zz = q[3] * z2;
        
        mat[0      ] = 1.0 - (yy + zz);
        mat[1      ] = xy + rz;
        mat[2      ] = xz - ry;
        
        mat[0+ldd  ] = xy - rz;
        mat[1+ldd  ] = 1.0 - (xx + zz);
        mat[2+ldd  ] = yz + rx;
        
        mat[0+ldd*2] = xz + ry;
        mat[1+ldd*2] = yz - rx;
        mat[2+ldd*2] = 1.0 - (xx + yy);
    }
    
    /// generate the associated 3x3 rotation matrix for unit Quaternion
    /** This assumes that norm(*this) = 1 */
    template < typename Matrix >
    void setMatrix3( Matrix & mat ) const
    {
        R rx, ry, rz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
        
        x2 = q[1] + q[1];
        y2 = q[2] + q[2];
        z2 = q[3] + q[3];
        
        rx = q[0] * x2; ry = q[0] * y2; rz = q[0] * z2;
        xx = q[1] * x2; xy = q[1] * y2; xz = q[1] * z2;
        yy = q[2] * y2; yz = q[2] * z2; zz = q[3] * z2;
        
        mat(0,0) = 1.0 - (yy + zz);
        mat(1,0) = xy + rz;
        mat(2,0) = xz - ry;
        
        mat(0,1) = xy - rz;
        mat(1,1) = 1.0 - (xx + zz);
        mat(2,1) = yz + rx;
        
        mat(0,2) = xz + ry;
        mat(1,2) = yz - rx;
        mat(2,2) = 1.0 - (xx + yy);
    }
    
    /// Rotate a 3D vector: des = Q * src * Q.conjugated()
    /** This assumes that norm(*this) = 1 */
    void rotateVector( R des[3], const R src[3] ) const
    {
        R two(2.0);
        
        R rx =  q[0]*q[1];
        R ry =  q[0]*q[2];
        R rz =  q[0]*q[3];
        R xx = -q[1]*q[1];
        R xy =  q[1]*q[2];
        R xz =  q[1]*q[3];
        R yy = -q[2]*q[2];
        R yz =  q[2]*q[3];
        R zz = -q[3]*q[3];
        
        des[0] = two * ( (yy + zz)*src[0] + (xy - rz)*src[1] + (ry + xz)*src[2] ) + src[0];
        des[1] = two * ( (rz + xy)*src[0] + (xx + zz)*src[1] + (yz - rx)*src[2] ) + src[1];
        des[2] = two * ( (xz - ry)*src[0] + (rx + yz)*src[1] + (xx + yy)*src[2] ) + src[2];
    }
    
    
    /// set from given 3x3 rotation matrix `m`
    void setFromMatrix3( const R m[9] )
    {
        R  s, trace = m[0] + m[4] + m[8];
        
        // check the diagonal
        if ( trace > 0 ) {
            s = sqrt( trace + 1.0 );
            q[0] = s * 0.5;
            s = 0.5 / s;
            q[1] = (m[5] - m[7]) * s;
            q[2] = (m[6] - m[2]) * s;
            q[3] = (m[1] - m[3]) * s;
        }
        else {
            // trace is negative
            // find biggest coefficient on diagonal:
            int i = 0;
            if (m[1+3*1] > m[0+3*0]) i = 1;
            if (m[2+3*2] > m[i+3*i]) i = 2;
            
            s = sqrt( 1.0 + 2*m[i+3*i] - trace );
            q[i+1] = s * 0.5;
            if (s != 0) s = 0.5 / s;
            int j = (i+1) % 3;
            int k = (j+1) % 3;
            q[j+1] = s * ( m[j+3*i] + m[i+3*j] );
            q[k+1] = s * ( m[i+3*k] + m[k+3*i] );
            q[0]   = s * ( m[k+3*j] - m[j+3*k] );
        }
    }
    
    /// generate OpenGL transformation matrix, translation followed by rotation
    void setOpenGLMatrix( float m[16], const float trans[3] ) const
    {
        //this code assumes that the quaternion has norm = 1,
        float rx, ry, rz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
        
        x2 = float(q[1]+q[1]);
        y2 = float(q[2]+q[2]);
        z2 = float(q[3]+q[3]);
        
        rx = float(q[0]*x2); ry = float(q[0]*y2); rz = float(q[0]*z2);
        xx = float(q[1]*x2); xy = float(q[1]*y2); xz = float(q[1]*z2);
        yy = float(q[2]*y2); yz = float(q[2]*z2); zz = float(q[3]*z2);
        
        m[0+4*0] = 1 - (yy + zz);
        m[1+4*0] = xy + rz;
        m[2+4*0] = xz - ry;
        m[3+4*0] = 0;

        m[0+4*1] = xy - rz;
        m[1+4*1] = 1 - (xx + zz);
        m[2+4*1] = yz + rx;
        m[3+4*1] = 0;

        m[0+4*2] = xz + ry;
        m[1+4*2] = yz - rx;
        m[2+4*2] = 1 - (xx + yy);
        m[3+4*2] = 0;

        m[0+4*3] = trans[0];
        m[1+4*3] = trans[1];
        m[2+4*3] = trans[2];
        m[3+4*3] = 1;
    }

    /// generate OpenGL transformation matrix, translation followed by rotation
    void setOpenGLMatrix( double m[16], const double trans[3] ) const
    {
        //this code assumes that the quaternion has norm = 1,
        double rx, ry, rz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
        
        x2 = q[1] + q[1];
        y2 = q[2] + q[2];
        z2 = q[3] + q[3];
        
        rx = q[0] * x2; ry = q[0] * y2; rz = q[0] * z2;
        xx = q[1] * x2; xy = q[1] * y2; xz = q[1] * z2;
        yy = q[2] * y2; yz = q[2] * z2; zz = q[3] * z2;
        
        m[0+4*0] = 1.0 - (yy + zz);
        m[1+4*0] = xy + rz;
        m[2+4*0] = xz - ry;
        m[3+4*0] = 0.0;

        m[0+4*1] = xy - rz;
        m[1+4*1] = 1.0 - (xx + zz);
        m[2+4*1] = yz + rx;
        m[3+4*1] = 0.0;

        m[0+4*2] = xz + ry;
        m[1+4*2] = yz - rx;
        m[2+4*2] = 1.0 - (xx + yy);
        m[3+4*2] = 0.0;

        m[0+4*3] = trans[0];
        m[1+4*3] = trans[1];
        m[2+4*3] = trans[2];
        m[3+4*3] = 1.0;
    }
    
    /// set from polar coordinates (r, phi, theta, psi)
    void setFromPolar( const R v[4] )
    {
        R a    = v[0] * sin(v[1]);
        q[0]   = v[0] * cos(v[1]);   //r*cos(phi)
        R b    = a * sin(v[2]);
        q[1]   = a * cos(v[2]);      //r*sin(phi)*cos(theta)
        q[2]   = b * cos(v[3]);      //r*sin(phi)*sin(theta)*cos(psi)
        q[3]   = b * sin(v[3]);      //r*sin(phi)*sin(theta)*sin(psi)
    }
    
    /// return new quaternion with polar coordinates (r, phi, theta, psi)
    static const Quaternion newFromPolar( const R v[4] )
    {
        Quaternion result;
        result.setFromPolar(v);
        return result;
    }
    
    /// calculate the polar coordinates (r, phi, theta, psi)
    void getPolar( R v[4] ) const
    {
        // r,  phi, theta, psi
        v[0] = norm();
        v[1] = acos( q[0] / v[0] );
        v[2] = acos( q[1] / (v[0] * sin(v[1])) );
        v[3] = atan2( q[3], q[2] );
    }
    

    /// set as rotation of axis v, with angle = v.norm() in radian;
    void setFromAxis( const R v[3] )
    {
        /** for small angles, we assume here angle ~ v.norm() */
        R n  = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
        R sd = sin( n * 0.5 );
        if ( n > 0 )
            sd /= n;
        q[0] = cos( n * 0.5 );
        q[1] = v[0] * sd;
        q[2] = v[1] * sd;
        q[3] = v[2] * sd;
    }
    
    /// set from rotation of axis v, and angle 'angle' in radian around this axis
    /** argument `v` is normalized for more security */
    void setFromAxis( const R v[3], R angle )
    {
        R  n = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
        R sd = sin( angle * 0.5 ) / n;
        q[0] = cos( angle * 0.5 );
        q[1] = v[0] * sd;
        q[2] = v[1] * sd;
        q[3] = v[2] * sd;
    }
    
    /// set as rotation of angle 'angle' and axis X, Y or Z (axis=0,1,2)
    /** along one of the unit axis specified by `axis`: ( 0: X, 1: Y, 2: Z ) */
    void setFromPrincipalAxis( int axis, R angle )
    {
        R  a = angle * 0.5;
        q[0] = cos(a);
        q[1] = 0.0;
        q[2] = 0.0;
        q[3] = 0.0;
        q[axis+1] = sin(a);
    }
    
    /// return angle of the rotation
    R getAngle() const
    {
        R n = sqrt( q[1]*q[1] + q[2]*q[2] + q[3]*q[3] );
        return  2 * atan2(n, q[0]);
    }
    
    /// compute the axis and return the angle of the rotation
    R getAngle( R v[3] ) const
    {
        R n = sqrt( q[1]*q[1] + q[2]*q[2] + q[3]*q[3] );
        if ( n > 0 )
        {
            R a = 2 * atan2(n, q[0]);
            n = 1.0 / n;
            v[0] = q[1] * n;
            v[1] = q[2] * n;
            v[2] = q[3] * n;
            return a;
        }
        v[0] = 0;
        v[1] = 0;
        v[2] = 1;
        return 0;
    }
    
    /// compute the axis and return the angle of the rotation
    void getAxis( R v[3] ) const
    {
        R n = sqrt( q[1]*q[1] + q[2]*q[2] + q[3]*q[3] );
        if ( n > 0 )
        {
            n = 1.0 / n;
            v[0] = q[1] * n;
            v[1] = q[2] * n;
            v[2] = q[3] * n;
        }
        else
        {
            v[0] = 0;
            v[1] = 0;
            v[2] = 1;
        }
    }
    
    /// multiply the angle of the rotation by `s`
    const Quaternion scaledAngle( R s ) const
    {
        R n = sqrt( q[1]*q[1] + q[2]*q[2] + q[3]*q[3] );
        if ( n > 0 )
        {
            R a = s * atan2(n, q[0]);
            n = sin(a) / n;
            return Quaternion(cos(a), n*q[1], n*q[2], n*q[3]);
        }
        return Quaternion(1, 0, 0, 0);
    }
    
    /// Linear interpolation between rotations 'this' and 'b'.
    const Quaternion slerp( const Quaternion &b, const R u ) const
    {
        // code from Jonathan Blow
        // Calculate the cosine of the angle between the two vectors
        R dot = q[0]*b.q[0] + q[1]*b.q[1] + q[2]*b.q[2] + q[3]*b.q[3];
        
        // If the angle is significant, use the spherical interpolation
        if ( dot > 0.9995 ) {
            // use cheap linear interpolation
            return normalize( (*this) + (b - (*this))*u );
        }
        
        R tmp = acos(dot) * u;
        //build v2 ortogonal to *this:
        Quaternion v2 = normalize( b - (*this)*dot );
        return (*this)*cos(tmp) + v2*sin(tmp);
    }
    
    /// printf
    void print( FILE* out = stdout, bool parenthesis = false ) const
    {
        if ( parenthesis )
            fprintf( out, "( %+6.3f %+6.3f %+6.3f %+6.3f )", q[0], q[1], q[2], q[3]);
        else
            fprintf( out, "  %+6.3f %+6.3f %+6.3f %+6.3f", q[0], q[1], q[2], q[3]);
    }
    
    /// printf with a new-line
    void println( FILE* out = stdout, bool parenthesis = false ) const
    {
        if ( parenthesis )
            fprintf( out, "( %+6.3f %+6.3f %+6.3f %+6.3f )\n", q[0], q[1], q[2], q[3]);
        else
            fprintf( out, "  %+6.3f %+6.3f %+6.3f %+6.3f\n", q[0], q[1], q[2], q[3]);
    }
    
    /// Human friendly ouput
    void print(std::ostream& os) const
    {
        os << q[0] << " " << q[1] << " " << q[2] << " " << q[3];
    }

    
#ifdef RANDOM_H
    
    /// returns a quaternion, uniformly sampling all possible rotations
    /** James Arvo, Fast random rotation matrices. in Graphics Gems 3. */
    static const Quaternion randomRotation()
    {
        R u1 = RNG.preal();
        R u2 = M_PI*RNG.sreal();
        R u3 = M_PI*RNG.sreal();
        R s1 = sqrt(1-u1), s2 = sqrt(u1);
        
        return Quaternion<R>(s1*sin(u2), s1*cos(u2), s2*sin(u3), s2*cos(u3));
    }
    
#endif
    
};


/// input operator
template <typename T>
std::istream& operator >> (std::istream& is, Quaternion<T> & q)
{
    is >> q[0] >> q[1] >> q[2] >> q[3];
    return is;
}

/// output operator
template <typename T>
std::ostream& operator << (std::ostream& os, const Quaternion<T> & q)
{
    q.print(os);
    return os;
}

#endif
