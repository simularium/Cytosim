// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "movable.h"
#include "assert_macro.h"
#include "stream_func.h"
#include "exceptions.h"
#include "quaternion.h"
#include "iowrapper.h"
#include "tokenizer.h"
#include "glossary.h"
#include "modulo.h"
#include "random.h"
#include "space.h"
#include "simul.h"


/** The default implementation is invalid */
void Movable::translate(Vector const&)
{
    ABORT_NOW("Movable::translate() called for immobile Object");
}

/** The default implementation is invalid */
void Movable::rotate(Rotation const& rot)
{
    ABORT_NOW("Movable::rotate() called for immobile Object");
}

/*
 if possible, the Object is translated by `[ rot * Object::position() - Object::position() ]`
 
 Note that this has no effect if the Object is at the origin
*/
void Movable::rotateT(Rotation const& rot)
{
    assert_true( mobile() == 1 );
    Vector pos = position();
    translate(rot*pos-pos);
}

/**
revolve() implements:

    Vector G = position();
    translate( -G );
    rotate( T );
    translate(  G );
*/
void Movable::revolve(Rotation const& T)
{
    Vector G = position();
    translate( -G );
    rotate( T );
    translate(  G );
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 There are different ways to specify a position:
 
 keyword & parameters | Position (X, Y, Z)                                     |
 ---------------------|---------------------------------------------------------
 `A B C`              | The specified vector (A,B,C)
 `inside`             | A random position inside the current Space
 `edge E`             | At distance E from the edge of the current Space
 `surface E`          | On the surface of the current Space\n By projecting a point at distance E from the surface.
 `line L T`           | Selected randomly with -L/2 < X < L/2; norm(Y,Z) < T
 `sphere R T`         | At distance R +/- T/2 from the origin\n `R-T/2 < norm(X,Y,Z) < R+T/2`
 `ball R`             | At distance R at most from the origin\n `norm(X,Y,Z) < R`
 `disc R T`           | in 2D, a disc in the XY-plane \n in 3D, a disc in the XY-plane of thickness T in Z
 `discXZ R T`         | Disc in the XZ-plane of radius R, thickness T
 `discYZ R T`         | Disc in the YZ-plane of radius R, thickness T
 `equator R T`        | At distance R from the origin, and T from the XY plane:\n `norm(X,Y) < R` `norm(Z) < T`
 `circle R T`         | Circle of radius R and thickness T \n At distance T from the circle of radius R
 `cylinder W R`       | Cylinder of axis X, W=thickness in X, R=radius in YZ
 `ellipse A B C`      | Inside the ellipse or ellipsoid of main axes 2A, 2B and 2C
 `arc L Theta`        | A piece of circle of length L and covering an angle Theta
 `stripe L R`         | Random vector with L < X < R
 `square R`           | Random vector with -R < X < R; -R < Y < R; -R < Z < R;
 `rectangle A B C`    | Random vector with -A < X < A; -B < Y < B; -C < Z < C;
 `gradient S E`       | Linear density gradient along X, of value 0 at X=S and 1 at X=E
 `gradient S E R`     | Linear density gradient, contained inside a cylinder of radius R
 `exponential S L`    | Exponential density gradient of length scale L, starting at S
 `exponential S L R`  | Exponential density gradient, contained inside a cylinder of radius R
 
 Each primitive describes a certain area in Space, and in most cases the returned position is
 chosen randomly inside this area following a uniform probability.
 */

Vector Movable::readPosition0(std::istream& is, Space const* spc)
{
    int c = Tokenizer::skip_space(is, false);

    if ( c == EOF )
        return Vector(0,0,0);

    if ( isalpha(c) )
    {
        std::string tok = Tokenizer::get_symbol(is);
        
        if ( spc )
        {
            if ( tok == "inside" || tok == "random" )
                return spc->randomPlace();
            
            if ( tok == "edge" )
            {
                real R = 0;
                is >> R;
                if ( R < REAL_EPSILON )
                    throw InvalidParameter("distance R must be > 0 in `edge R`");
                return spc->randomPlaceNearEdge(R);
            }
            
            if ( tok == "surface" )
            {
                real R = 1;
                is >> R;
                if ( R < REAL_EPSILON )
                    throw InvalidParameter("distance R must be > 0 in `surface R`");
                return spc->randomPlaceOnEdge(R);
            }

            if ( tok == "outside_sphere" )
            {
                real R = 0;
                is >> R;
                if ( R < 0 )
                    throw InvalidParameter("distance R must be >= 0 in `outside_sphere R`");
                Vector P;
                do
                    P = spc->randomPlace();
                while ( P.norm() < R );
                return P;
            }
            
            if ( tok == "stripe" )
            {
                real s = -0.5, e = 0.5;
                is >> s >> e;
                Vector inf, sup;
                spc->boundaries(inf, sup);
                Vector pos = inf + (sup-inf).e_mul(Vector::randP());
                pos.XX = RNG.real_uniform(s, e);
                return pos;
            }
        
            if ( tok == "gradient" )
            {
                real S = -10, E = 10, R = 0;
                is >> S >> E >> R;
                if ( R == 0 )
                {
                    Vector vec;
                    real p;
                    do {
                        vec = spc->randomPlace();
                        p = ( vec.XX - S ) / ( E - S );
                    } while ( p < 0 || p > 1 || p < RNG.preal() );
                    return vec;
                }
                real x = sqrt(RNG.preal());
#if ( DIM < 3 )
                return Vector(S+(E-S)*x, R*RNG.sreal(), 0);
#else
                const Vector2 V = Vector2::randU();
                return Vector(S+(E-S)*x, R*V.XX, R*V.YY);
#endif
            }
        
            if ( tok == "exponential" )
            {
                real S = -10, E = 1, R = 0;
                is >> S >> E >> R;
                if ( R == 0 )
                {
                    Vector vec;
                    real p;
                    do {
                        vec = spc->randomPlace();
                        p = exp( ( S - vec.XX ) / E );
                    } while ( p < 0 || p > 1 || p < RNG.preal() );
                    return vec;
                }
                real x = RNG.exponential();
#if ( DIM < 3 )
                return Vector(S+E*x, R*RNG.sreal(), 0);
#else
                const Vector2 V = Vector2::randU();
                return Vector(S+E*x, R*V.XX, R*V.YY);
#endif
            }
        }
        
        if ( tok == "sphere" )
        {
            real R = -1, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `sphere R`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `sphere R T`");
            return Vector::randU(R) + Vector::randU(T*0.5);
        }
        
        if ( tok == "equator" )
        {
            real R = 0, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `equator R T`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `equator R T`");
            const Vector2 V = Vector2::randU();
            return Vector(R*V.XX, R*V.YY, T*RNG.shalf());
        }
       
        if ( tok == "cylinder" )
        {
            real L = -1, R = -1;
            is >> L >> R;
            if ( L < 0 )
                throw InvalidParameter("length L must be >= 0 in `cylinder L R`");
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `cylinder L R`");
            const Vector2 V = Vector2::randB(R);
            return Vector(L*RNG.shalf(), V.XX, V.YY);
        }
        
        if ( tok == "circle" )
        {
            real R = -1, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `circle R T`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `circle R T`");
            //StreamFunc::mark_line(std::cout, is, is.tellg(), ">>>>");
#if ( DIM >= 3 )
            const Vector2 V = Vector2::randU(R);
            return Vector3(V.XX, V.YY, 0) + (0.5*T) * Vector3::randU();
#endif
            return Vector::randU(R) + Vector::randU(T*0.5);
        }
        
        if ( tok == "ball" )
        {
            real R = -1;
            is >> R;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `ball R`");
            return Vector::randB(R);
        }
        
        if ( tok == "disc" || tok == "discXY" )
        {
            real R = -1, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `disc R`");
#if ( DIM >= 3 )
            //in 3D, a disc in the XY-plane of thickness T in Z-direction
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `disc R T`");
            const Vector2 V = Vector2::randB(R);
            return Vector(V.XX, V.YY, T*RNG.shalf());
#endif
            //in 2D, a disc in the XY-plane
            return Vector::randB(R);
        }
        
        if ( tok == "discXZ"  )
        {
            real R = -1, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `discXZ R`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `discXZ R T`");
            const Vector2 V = Vector2::randB(R);
            return Vector(V.XX, T*RNG.shalf(), V.YY);
        }
        
        if ( tok == "discYZ"  )
        {
            real R = -1, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("radius R must be >= 0 in `discYZ R`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `discYZ R T`");
            const Vector2 V = Vector2::randB(R);
            return Vector(T*RNG.shalf(), V.XX, V.YY);
        }
        
        if ( tok == "ellipse" )
        {
            real x = 1, y = 1, z = 0;
            is >> x >> y >> z;
            return Vector(x,y,z).e_mul(Vector::randB());
        }
        
        if ( tok == "ellipse_surface" )
        {
            real x = 1, y = 1, z = 0;
            is >> x >> y >> z;
            return Vector(x,y,z).e_mul(Vector::randU());
        }
        
        if ( tok == "line" )
        {
            real L = -1, T = 0;
            is >> L >> T;
            if ( L < 0 )
                throw InvalidParameter("length L must be >= 0 in `line L`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `line L T`");
#if ( DIM >= 3 )
            const Vector2 V = Vector2::randB(T);
            return Vector(L*RNG.shalf(), V.XX, V.YY);
#endif
            return Vector(L*RNG.shalf(), T*RNG.shalf(), 0);
        }
        
        if ( tok == "arc" )
        {
            real L = -1, A = 1.57;
            is >> L >> A;
            
            if ( L <= 0 )
                throw InvalidParameter("length must be L >= 0 in `arc L`");
            
            real x = 0, y = 0;
            if ( A == 0 ) {
                x = 0;
                y = L * RNG.shalf();
            }
            else {
                real R = L / A;
                real angle = A * RNG.shalf();
                x = R * cos(angle) - R; // origin centered on arc
                y = R * sin(angle);
            }
#if ( DIM < 3 )
            return Vector(x, y, 0);
#else
            // in 3D this distributes on a portion of the sphere, with a gradient
            real a = RNG.sreal() * M_PI;
            return Vector(x, y*cos(a), y*sin(a));
#endif
        }
        
        if ( tok == "square" )
        {
            real x = 1;
            is >> x;
            return Vector::randS(x);
        }
        
        if ( tok == "rectangle" )
        {
            real x = 0, y = 0, z = 0;
            is >> x >> y >> z;
            return Vector(x,y,z).e_mul(Vector::randH());
        }

#if ( 1 )
        /// A contribution from Beat Rupp
        if ( tok == "segment" || tok == "newsegment" )
        {
            real bending = 0, length = 0, thickness = 0, rotation = 0;
            is >> bending >> length >> thickness >> rotation;
            real x=0, y=0;
            
            // straight
            if ( bending == 0 ) {
                x = thickness * RNG.shalf();
                y = length * RNG.preal();
            } else {
                real radius = length / (bending * M_PI);
                real radiusInner = radius - thickness/2.0;
                real theta = fabs( length / radius );
                real angle = RNG.preal() * theta;
                // substract R to have the arc start from 0,0:
                x = (radiusInner + thickness * RNG.preal()) * cos( angle ) - radius;
                y = (radiusInner + thickness * RNG.preal()) * sin( angle );
            }
            
            real cr = cos(rotation);
            real sr = sin(rotation);
            
            // rotate
            return Vector(cr*x + sr*y , -sr*x + cr*y, 0 );
        }
#endif
        if ( tok == "center" || tok == "origin" )
            return Vector(0,0,0);
        
        if ( spc )
            throw InvalidParameter("Unknown position specification `"+tok+"'");
        else
            throw InvalidParameter("Unknown position specification `"+tok+"' (with no space defined)");
    }
    
    // accept a Vector:
    Vector vec(0,0,0);
    if ( is >> vec )
        return vec;
    throw InvalidParameter("expected a vector specifying a `position`");
}


//------------------------------------------------------------------------------
/**
 A position is defined with a SHAPE followed by a number of TRANSFORMATION.
 
 TRANSFORMATION         | Result                                               |
 -----------------------|-------------------------------------------------------
 `at X Y Z`             | Translate by specified vector (X,Y,Z)
 `add SHAPE`            | Translate by a vector chosen according to SHAPE
 `align VECTOR`         | Rotate to align parallel with specified vector
 `turn ROTATION`        | Apply specified rotation
 `blur REAL`            | Add centered Gaussian noise of variance REAL
 `to X Y Z`             | Interpolate with the previously specified position
 `or POSITION`          | flip randomly between two specified positions
 
 A vector is set according to SHAPE, and the transformations are applied one after
 the other, in the order in which they were given.\n

 Examples:
 
   position = 1 0 0
   position = circle 3 at 1 0
   position = square 3 align 1 1 0 at 1 1
 
 */ 
Vector Movable::readPosition(std::istream& is, Space const* spc)
{
    std::string tok;
    Vector pos(0,0,0);
    std::streampos isp = 0;
    
    try
    {
        if ( is.fail() )
            return pos;
        isp = is.tellg();
        pos = readPosition0(is, spc);
        is.clear();
        
        while ( !is.eof() )
        {
            isp = is.tellg();
            tok = Tokenizer::get_symbol(is);

            if ( tok.empty() )
                return pos;
            
            // Translation is specified with 'at' or 'move'
            if ( tok == "at"  ||  tok == "move" )
            {
                Vector vec(0,0,0);
                is >> vec;
                pos = pos + vec;
            }
            // Convolve with shape
            else if ( tok == "add" )
            {
                Vector vec = readPosition0(is, spc);
                pos = pos + vec;
            }
            // Alignment with a vector is specified with 'align'
            else if ( tok == "align" )
            {
                Vector vec = readDirection(is, pos, spc);
                Rotation rot = Rotation::randomRotationToVector(vec);
                pos = rot * pos;
            }
            // Rotation is specified with 'turn'
            else if ( tok == "turn" )
            {
                Rotation rot = readOrientation(is, pos, spc);
                pos = rot * pos;
            }
            // Gaussian noise specified with 'blur'
            else if ( tok == "blur" )
            {
                real blur = 0;
                is >> blur;
                pos += Vector::randG(blur);
            }
            // returns a random position between the two points specified
            else if ( tok == "to" )
            {
                Vector vec = readPosition0(is, spc);
                return pos + ( vec - pos ) * RNG.preal();
            }
            // returns one of the two points specified
            else if ( tok == "or" )
            {
                Vector alt = readPosition0(is, spc);
                if ( RNG.flip() ) pos = alt;
            }
            else
            {
                // unget last token:
                is.clear();
                is.seekg(isp);
#if 1
                /*
                We need to work around a bug in the stream extraction operator,
                which eats extra characters ('a','n','e','E') if a double is read
                19.10.2015
                */
                is.seekg(-1, std::ios_base::cur);
                int c = is.peek();
                if ( c=='a' || c=='b' )
                    continue;
                is.seekg(1, std::ios_base::cur);
#endif
                break;
                //throw InvalidParameter("unexpected `"+tok+"'");
            }
        }
    }
    catch ( InvalidParameter& e )
    {
        e << "\n" << StreamFunc::marked_line(is, isp, PREF);
        throw;
    }
    return pos;
}


//------------------------------------------------------------------------------
/**
 Reads a direction which is a unit vector (norm = 1):
 
 Keyword                                     | Resulting Vector                                          |
 --------------------------------------------|------------------------------------------------------------
 `REAL REAL REAL`                            | the vector of norm 1 co-aligned with given vector
 `parallel REAL REAL REAL`                   | one of the two vectors of norm 1 parallel with given vector
 `orthogonal REAL REAL REAL`                 | a vector of norm 1 perpendicular to the given vector
 `horizontal` \n `parallel X`                | (+1,0,0) or (-1,0,0), randomly chosen with equal chance
 `vertical`\n `parallel Y`                   | (0,+1,0) or (0,-1,0), randomly chosen with equal chance
 `parallel Z`                                | (0,0,+1) or (0,0,-1), randomly chosen with equal chance
 `parallel XY`\n`parallel XZ`\n`parallel YZ` | A random vector in the specified plane
 `radial`                                    | directed from the origin to the current point
 `antiradial`                                | directed from the current point to the origin
 `circular`                                  | perpendicular to axis joining the current point to the origin
 `or DIRECTION`                              | flip randomly between two specified directions

 
 If a Space is defined, one may also use:
 
 Keyword         | Resulting Vector                       |
 ----------------|-----------------------------------------
 `tangent`       | parallel to the surface of the Space
 `normal`        | perpendicular to the surface
 `inward`        | normal to the surface, directed outward
 `outward`       | normal to the surface, directed inward


 Note: when the rotation is not uniquely determined in 3D (eg. `horizontal`), 
 cytosim will pick uniformly among all the possible rotations that fulfill the requirements.
 */


Vector Movable::readDirection0(std::istream& is, Vector const& pos, Space const* spc)
{
    int c = Tokenizer::skip_space(is, false);
    
    if ( c == EOF )
        return Vector::randU();

    if ( isalpha(c) )
    {
        const std::string tok = Tokenizer::get_symbol(is);
        
        if ( tok == "random" )
            return Vector::randU();

        if ( tok == "X" )
            return Vector(RNG.sflip(), 0, 0);
        if ( tok == "Y" )
            return Vector(0, RNG.sflip(), 0);
        if ( tok == "XY" )
        {
            const Vector2 V = Vector2::randU();
            return Vector(V.XX, V.YY, 0);
        }
#if ( DIM >= 3 )
        if ( tok == "Z" )
            return Vector(0, 0, RNG.sflip());
        if ( tok == "XZ" )
        {
            const Vector2 V = Vector2::randU();
            return Vector(V.XX, 0, V.YY);
        }
        if ( tok == "YZ" )
        {
            const Vector2 V = Vector2::randU();
            return Vector(0, V.XX, V.YY);
        }
#endif

        if ( tok == "align" )
        {
            Vector vec;
            if ( is >> vec )
                return vec.normalized(RNG.sflip());
            throw InvalidParameter("expected vector after `align`");
        }
        
        if ( tok == "parallel" )
        {
            Vector vec;
            if ( is >> vec )
                return normalize(vec);
            throw InvalidParameter("expected vector after `parallel`");
        }

        if ( tok == "orthogonal" )
        {
            Vector vec;
            if ( is >> vec )
                return vec.randOrthoU(1);
            throw InvalidParameter("expected vector after `orthogonal`");
        }

        if ( tok == "horizontal" )
        {
#if ( DIM >= 3 )
            return Vector(Vector2::randU());
#else
            return Vector(RNG.sflip(), 0, 0);
#endif
        }
        
        if ( tok == "vertical" )
        {
#if ( DIM >= 3 )
            return Vector(0, 0, RNG.sflip());
#else
            return Vector(0, RNG.sflip(), 0);
#endif
        }
        
        if ( tok == "radial" )
            return normalize(pos);
        
        if ( tok == "antiradial" )
            return -normalize(pos);

        if ( tok == "circular" )
            return pos.randOrthoU(1.0);
        
        if ( tok == "orthoradial" )
            return pos.randOrthoU(1.0);
      
        if ( spc )
        {
            if ( tok == "tangent" )
                return spc->normalToEdge(pos).randOrthoU(1.0);
           
#if ( DIM >= 3 )
            Vector dir(0,0,1);
#elif ( DIM == 2 )
            real dir = 1;
#endif
            
#if ( DIM > 1 )
            if ( tok == "clockwise" )
                return cross(dir, spc->normalToEdge(pos));
            
            if ( tok == "anticlockwise" )
                return -cross(dir, spc->normalToEdge(pos));
#endif
            
            if ( tok == "normal" )
                return RNG.sflip() * spc->normalToEdge(pos);
            
            if ( tok == "inward" )
                return -spc->normalToEdge(pos);
            
            if ( tok == "outward" )
                return spc->normalToEdge(pos);
        }
        
        throw InvalidParameter("Unknown direction `"+tok+"'");
    }
    
    {
        // accept a Vector:
        Vector vec(0,0,0);
        if ( is >> vec )
        {
            real n = vec.norm();
            if ( n < REAL_EPSILON )
                throw InvalidParameter("direction vector appears singular");
            return vec / n;
        }
    }
    throw InvalidParameter("expected a vector specifying a `direction`");
}


Vector Movable::readDirection(std::istream& is, Vector const& pos, Space const* spc)
{
    std::string tok;
    Vector dir(1,0,0);
    std::streampos isp = 0;
    
    try
    {
        if ( is.fail() )
            return dir;
        isp = is.tellg();
        dir = readDirection0(is, pos, spc);
        is.clear();
        
        while ( !is.eof() )
        {
            isp = is.tellg();
            tok = Tokenizer::get_symbol(is);

            if ( tok.empty() )
                return dir;
            
            // Gaussian noise specified with 'blur'
            else if ( tok == "blur" )
            {
                real blur = 0;
                is >> blur;
                dir = Rotation::randomRotation(blur*RNG.gauss()) * dir;
            }
            // returns one of the two points specified
            else if ( tok == "or" )
            {
                Vector alt = readDirection0(is, pos, spc);
                if ( RNG.flip() ) dir = alt;
            }
            else
            {
                // unget last token
                is.clear();
                is.seekg(isp);
                break;
            }
        }
    }
    catch ( InvalidParameter& e )
    {
        e << "\n" << StreamFunc::marked_line(is, isp, PREF);
        throw;
    }
    return dir;
}


/**
 A rotation can be specified as follows:
 
 Keyword                 | Rotation / Result
 ------------------------|-----------------------------------------------------------
 `random`                | A rotation selected uniformly among all possible rotations
 `identity`, `off`       | The object is not rotated
 `X theta`               | A rotation around axis 'X' of angle `theta` in radians
 `Y theta`               | A rotation around axis 'Y' of angle `theta` in radians
 `Z theta`               | A rotation around axis 'Z' of angle `theta` in radians
 `angle A`               | Rotation around Z axis with angle of rotation in radians
 `angle A axis X Y Z`    | Rotation around given axis with angle of rotation in radians
 `axis X Y Z angle A`    | Rotation around given axis with angle of rotation in radians
 `degree A axis X Y Z`   | As specified by axis and angle of rotation in degrees
 `quat q0 q1 q2 q3`      | As specified by the Quaternion (q0, q1, q2, q3)
*/

Rotation Movable::readRotation(std::istream& is)
{
    std::streampos isp = is.tellg();
    std::string tok = Tokenizer::get_symbol(is);
    
    if ( tok == "random" )
        return Rotation::randomRotation();
    else if ( tok == "identity" || tok == "off" || tok == "none" )
        return Rotation::identity();
    else if ( tok == "angle" || tok == "angles" )
    {
        real ang = 0;
        is >> ang;
#if ( DIM >= 3 )
        Vector dir(0,0,1);
        if ( is.good() )
        {
            isp = is.tellg();
            if ( Tokenizer::get_symbol(is) == "axis" )
                is >> dir;
            else
            {
                is.clear();
                is.seekg(isp);
            }
        }
        return Rotation::rotationAroundAxis(normalize(dir), cos(ang), sin(ang));
#else
        return Rotation::rotation(cos(ang), sin(ang));
#endif
    }
#if ( DIM >= 3 )
    else if ( tok == "axis" )
    {
        real ang = 0;
        Vector dir(0,0,1);
        is >> dir;
        isp = is.tellg();
        tok = Tokenizer::get_symbol(is);
        if ( tok == "angle" )
            is >> ang;
        else if ( tok == "degree" )
        {
            is >> ang;
            ang *= M_PI/180.0;
        }
        else
        {
            is.clear();
            is.seekg(isp);
        }
        return Rotation::rotationAroundAxis(normalize(dir), cos(ang), sin(ang));
    }
#endif
    else if ( tok == "degree" )
    {
        real ang = 0;
        is >> ang;
        ang *= M_PI/180.0;
#if ( DIM >= 3 )
        Vector dir(0,0,1);
        if ( is.good() )
        {
            isp = is.tellg();
            if ( Tokenizer::get_symbol(is) == "axis" )
                is >> dir;
            else
            {
                is.clear();
                is.seekg(isp);
            }
        }
        return Rotation::rotationAroundAxis(normalize(dir), cos(ang), sin(ang));
#else
        return Rotation::rotation(cos(ang), sin(ang));
#endif
    }
#if ( DIM >= 3 )
    else if ( tok == "quat" )
    {
        Quaternion<real> quat;
        is >> quat;
        quat.normalize();
        Rotation rot;
        quat.setMatrix3(rot);
        return rot;
    }
    else if ( tok == "X" )
    {
        real ang = 0.5 * M_PI;
        is >> ang;
        return Rotation::rotationAroundX(ang);
    }
    else if ( tok == "Y" )
    {
        real ang = 0.5 * M_PI;
        is >> ang;
        return Rotation::rotationAroundY(ang);
    }
    else if ( tok == "Z" )
    {
        real ang = 0.5 * M_PI;
        is >> ang;
        return Rotation::rotationAroundZ(ang);
    }
#endif
    
    // rewind before token:
    is.clear();
    is.seekg(isp);
    throw InvalidSyntax("unexpected `"+tok+"' in rotation");
    return Rotation::randomRotation();
}


/**
 The initial orientation of objects is defined by a rotation, but it is usually
 sufficient to specify a unit vector:
 
 Keyword                 | Rotation / Result
 ------------------------|------------------------------------------------------
 ROTATION                | see @ref Movable::readRotation()
 DIRECTION               | see @ref Movable::readDirection
 DIRECTION or DIRECTION  | flip randomly between two specified directions
 
 When a DIRECTION is specified, a rotation will be built that transforms (1, 0, 0)
 into the given vector (after normalization). In 3D, this does not define a rotation
 uniquely, and cytosim will randomly pick one of the possible rotations, with equal
 probability among all the possible rotation, by rotating around (1, 0, 0) beforehand.
*/

Rotation Movable::readOrientation(std::istream& is, Vector const& pos, Space const* spc)
{
    int c = Tokenizer::skip_space(is, false);

    if ( isalpha(c) )
    {
        try {
            return readRotation(is);
        }
        catch ( Exception& e )
        {
            // Some keywords are handled by readDirection(), so we just warn here
            std::cerr << "Warning, " << e.message() << '\n';
        }
    }

    // normally a unit vector is specified:
    Vector vec = readDirection(is, pos, spc);
    
    /*
     A single Vector does not uniquely define a rotation in 3D:
     hence we return a random rotation that is picked uniformly
     among all possible rotations transforming (1,0,0) in vec.
     */
    return Rotation::randomRotationToVector(vec);
}

