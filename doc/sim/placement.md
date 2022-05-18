# Object's initial position

The position of an object is specified as it is created with the `new` command:
 
     new [POSITIVE_INTEGER] NAME
     {
       position     = POSITION[, SPACE]
       orientation  = ROTATION[, ROTATION]
       direction    = DIRECTION
		...
     }

All parameters are optional. See the specifications below.
Some POSITION primitives refer to the *current space*, but another space can be specified (`position = inside, my_space`). 

The rotation given in `orientation` is applied before the translation given by `position`.
If a second value is specified in `orientation`, this rotation will be applied after the translation. The parameter `direction` is a replacement to `orientation`, which is handy in the case of linear objects. If both are specified, `direction` will be ignored.


# POSITION

A position is defined with a `PRIMITIVE` optionally followed by `TRANSFORMATION`. A vector is first set according to the `PRIMITIVE`, and the transformations are applied one after the other, in the order in which they are given. Examples:

	   position = 1 0 0
	   position = circle 3 at 1 0
	   position = square 3 align 1 1 0 at 1 1

### Geometrical Primitives

Most primitives describe a certain area in Space, and the returned position is
 then chosen randomly inside this area following a uniform probability.


 Input                | Resulting position (X, Y, Z)                  
 ---------------------|---------------------------------------------------------
 `A B C`              | The specified vector (A,B,C)
 `inside`             | A random position inside the current Space
 `edge E`             | At distance E from the edge of the current Space
 `surface E`          | On the surface of the current Space; By projecting a point at distance E from the surface.
 `line L T`           | Selected randomly with -L/2 < X < L/2; norm(Y,Z) < T
 `sphere R T`         | At distance R +/- T/2 from the origin; `R-T/2 < norm(X,Y,Z) < R+T/2`
 `ball R`             | At distance R at most from the origin; `norm(X,Y,Z) < R`
 `disc R T`           | in 2D, a disc in the XY-plane; in 3D, a disc in the XY-plane of thickness T in Z
 `discXZ R T`         | Disc in the XZ-plane of radius R, thickness T
 `discYZ R T`         | Disc in the YZ-plane of radius R, thickness T
 `equator R T`        | At distance R from the origin, and T from the XY plane: `norm(X,Y) < R` `norm(Z) < T`
 `circle R T`         | Circle of radius R and thickness T; At distance T from the circle of radius R
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

### Transformations 

 `TRANSFORMATION`       | Result                                            
 -----------------------|-------------------------------------------------------
 `at X Y Z`             | Translate by specified vector (X,Y,Z)
 `add SHAPE`            | Translate by a vector chosen according to SHAPE
 `align VECTOR`         | Rotate to align parallel with specified vector
 `turn ROTATION`        | Apply specified rotation
 `blur REAL`            | Add centered Gaussian noise of variance REAL
 `to X Y Z`             | Interpolate with the previously specified position
 `or POSITION`          | flip randomly between two specified positions
 
 
# ROTATION

 The initial orientation of objects is defined by a rotation, which can be
 specified as follows:
 
 Keyword                    | Rotation / Result                                        |
 ---------------------------|-----------------------------------------------------------
 `random`                   | A rotation selected uniformly among all possible rotations
 `identity`                 | The object is not rotated
 `angle A B C`              | As specified by 3 (or 1 in 2D) Euler angles in radians
 `degree A B C`             | As specified by 3 (or 1 in 2D) Euler angles in degrees
 `quat q0 q1 q2 q3`         | As specified by the Quaternion (q0, q1, q2, q3)
 `DIRECTION`                | see below
 `DIRECTION` or `DIRECTION` | flip randomly between two specified directions
 
 In the last case, a rotation will be built that transforms (1, 0, 0) into the given vector,
 after normalization. 
 
 Note: when the rotation is not uniquely determined in 3D (eg. `horizontal`), 
 cytosim will pick uniformly among all the possible rotations that fulfill the requirements.

### DIRECTION

 For some objects (e.g. fiber) specifying a direction is sufficient. A direction is a unit vector (of norm = 1):
 
 Keyword                                     | Resulting Vector    
 --------------------------------------------|------------------------------------------------------------
 `REAL REAL REAL`                            | the vector of norm 1 co-aligned with given vector
 `parallel REAL REAL REAL`                   | one of the two vectors of norm 1 parallel with given vector
 `orthogonal REAL REAL REAL`                 | a vector of norm 1 perpendicular to the given vector
 `horizontal`    `parallel X`                | (+1,0,0) or (-1,0,0), randomly chosen with equal chance
 `vertical`   `parallel Y`                   | (0,+1,0) or (0,-1,0), randomly chosen with equal chance
 `parallel Z`                                | (0,0,+1) or (0,0,-1), randomly chosen with equal chance
 `parallel XY`  `parallel XZ`  `parallel YZ` | A random vector in the specified plane
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


 
# Object placement


The creation of an object involves three steps:

1. Drawing a random position and a random orientation, 
2. Creating the object at this position with this orientation
3. Testing if the object's vertices fulfills certain requirements: eg. are all points inside?

If step 3 fails, Cytosim starts again from 1, up to `nb_trials` times.

Step 1 is controlled by parameter `position`  
Step 3 is controlled by parameter `placement`  

Both in steps 1 and 3 you can specify a Space, and by default, these use the ‘master space’: the first one to be created.
If ‘placement’ is not specified, it is by default ‘inside’ the 'master space'.

Syntax:

     new POSITIVE_INTEGER NAME
     {
       position    = POSITION[, SPACE_NAME]
       orientation = ROTATION[, ROTATION]
       direction   = DIRECTION
       placement   = PLACEMENT[, SPACE_NAME[, CONDITION]]
       nb_trials   = INTEGER
     }
 
PLACEMENT can be:
 
- `inside` (default), it tries to find a place inside the Space
- `anywhere`, the position is returned
- `outside`, the object is created only if it is outside the Space
- `surface`, the position is projected on the edge of current Space
.
 
By default, these specifications are relative to the first Space to be defined,
but a different space can be specified as second argument of PLACEMENT.

### Examples

One can use `nb_trials=1` to control the density of objects:

     new 100 grafted
     {
          position = ( rectangle 10 10 )
          nb_trials = 1
     }
 
In this way an object will be created only if its randomly chosen position falls
inside the Space, and the density will thus be exactly what is specified from the
`position` range (here 100/10*10 = 1 object per squared micrometer).



To place objects at the intersection of two spaces

    new 100 sphere_mol
    {
        radius = 0.1
        position = inside, sphere
        placement = inside, cylinder
    }

To skip the ‘placement’ test:

    new 100 cyl_mol
    {
        radius = 0.1
        position = inside, cyl
        placement = anywhere
    }
