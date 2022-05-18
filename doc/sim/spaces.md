# Confinement

Filaments and particles can be confined within fixed boundaries using one of the `Space` class.
For example, with a stiffness of `100 pN/um`:

	set fiber microtubule
	{
	    confine = inside, 100
	}

Confinement is usually applied to the vertices that describe an object. 
So let’s say `V` is such a vertex.
Let's call `P` the projection of `V` on the edge of the Space. 
This is the closest point on the edge to `V` and `PV` is orthogonal to the margin of the Space in `P`.

### if `confine = inside`:

- V is inside: no force.
- V is outside: 

		force =  stiffness * ( P - V )

This corresponds to a potential that is flat inside, and raises quadratically outside the shape.

### if `confine = outside`:

- V is outside: no force.
- V is inside:

		force =  stiffness * ( P - V )

This corresponds to a potential that is flat outside, and raises quadratically inside the shape.

### if `confine = on`:

The force is always applied:

		force =  stiffness * ( P - V )

This corresponds to a potential that raises quadratically from the edge in both directions.

# Available spaces
 
 A Space is created by specifying a `shape` and `dimensions`:
 
     set space NAME
     {
        shape = SHAPE
     }
  
     new NAME
     {
        PARAMETER = REAL, REAL, REAL
     }

 The names of the parameters to specify the `dimensions` depend on the shape.
 Spaces with fixed goemetry:
 
 SHAPE                                               | PARAMETERS                           
 ----------------------------------------------------|--------------------------------------
 [`rectangle`](doxygen/class_space_square.html)      | length (X, Y and Z values)
 [`sphere`](doxygen/class_space_sphere.html)         | radius
 [`polygon`](doxygen/class_space_polygon.html)       | file height
 [`polygonZ`](doxygen/class_space_polygon_z.html)    | file
 [`capsule`](doxygen/class_space_capsule.html)       | length radius
 [`torus`](doxygen/class_space_torus.html)           | radius thickness
 [`banana`](doxygen/class_space_banana.html)         | length width radius
 [`dice`](doxygen/class_space_dice.html)             | radius length (X, Y and Z values)
 [`strip`](doxygen/class_space_strip.html)           | length (X, Y and Z values)
 [`periodic`](doxygen/class_space_periodic.html)     | length (X, Y and Z values)
 [`ellipse`](doxygen/class_space_ellipse.html)       | length (X, Y and Z values)
 [`cylinder`](doxygen/class_space_cylinder.html)     | length radius
 [`cylinderZ`](doxygen/class_space_cylinder_z.html)  | radius bottom top
 [`cylinderP`](doxygen/class_space_cylinder_p.html)  | length radius
 [`ring`](doxygen/class_space_ring.html)             | length radius
