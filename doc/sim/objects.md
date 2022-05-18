# Cytosim's objects

For up-to-date reference, check the [code documentation](doxygen/md_doc_code_objects.html).

# Simul & Immobile objects 

- `simul`: unique master class with [global parameters](doxygen/group___simul_par.html)
- `space`: a volume element used to confined objects in space
- `field`: a scalar value as a function of position
- `event`: a code executed at stochastic times
 
[simul class reference](doxygen/class_simul.html)

# Mecables

 Cytosim uses vertices (points) to describe objects.
 `Mecables` are objects made of multiple points that can move or deform:

The `mecables` are created directly:

- `fiber`: a linear object with points, with [multiple classes](doxygen/group___fiber_group.html)
- `solid`: a set of points that move and rotate but conserves its shape
- `bead`: a single point (center) with a radius
- `sphere`: a center with mobile points on the surface
 
[class reference](doxygen/class_mecable.html)

# Fibers

The `fiber` can grow and shrink according to different models,
specified by specifying an `activity`:

- `none`: constant length (default)
- `grow`: can grow from both ends
- `dynamic`: follow a stochastic dynamic instability model with multiple states
- `classic`: follows the two-state T. Hill model of dynamic instability
- `treadmill`: grow at the plus end, shrink at the minus end
 
[class reference](doxygen/class_fiber.html) —
[group reference](doxygen/group___fiber_group.html)

Example:

	set fiber microtubule
	{
		rigidity = 20
		activity = grow
		growing_speed = 1.0
	} 

# Hands
 
 A `hand` represents the capacity to bind to a single fiber.
 It can only be used as a subpart of `single` (one hand) or `couple` (two hands).
 
The type of `hand` is selected by specifying an `activity`:

- `bind`: only bind and unbinds from fibers
- `move`: moves along fibers in a continuous manner
- `nucleate`: can create a new fiber
- `slide`: passively moves along fiber
- `track`: stays associated with the end of fiber
- `rescue`: induce rescue in a dynamic fiber
- `regulate`: freeze the growth of a fiber (unfinished)
- `cut`: sever fiber at the position of attachment
- `chew`: induce dissassembly of fiber end when reached
- `mighty`: do multiple things to fiber (unfinished)
- `act`: do nothing to fiber (unfinished)

Example:

	set hand kinesin
	{
		binding = 10, 2
		unbinding = 1, inf
		activity = move
		unloaded_speed = 1.0
		stall_force = 6.0
	} 

[class reference](doxygen/class_hand.html) —
[group reference](doxygen/group___hand_group.html)


### Digital Hands

Some hand type can only bind at discrete positions along a Fiber, corresponding to the Lattice associated with a fiber.

 Digital hand `activity`:
 
- `digit`: bind and unbind at discrete sites
- `walk`: moves along fiber following discrete steps
- `kinesin`: unfinished
- `dynein`: unfinished
- `myosin`: unfinished


# Singles
 
 A `single` contains one `hand`, for example:
 
	set single grafted
	{
		hand = kinesin
		stiffness = 100
		activity = fixed
	} 
 
The type of `single` is selected by specifying an `activity`:
 
- `diffuse`: diffuse in space
- `fixed`: remains attached to a fixed position in space
- `wrist`: remains anchored to a `mecable`
 
[class reference](doxygen/class_single.html) —
[group reference](doxygen/group___single_group.html)

# Couples
 
 A `couple` contains two `hand`, for example:
  
	set couple complex
	{
		hand1 = kinesin
		hand2 = kinesin
		stiffness = 100
		diffusion = 10
	} 

The type of `couple` is selected by specifying an `activity`:

- `diffuse`: moves in space following diffusion and crosslink fibers
- `crosslink`: uses a 'longlink'
- `bridge`:
- `duo`:
- `slide`:
- `fork`: connect two fibers with a angular stiffness
 
[class reference](doxygen/class_couple.html) —
[group reference](doxygen/group___couple_group.html)
 
# Organizers
 
An `organizer` is a composite object build from multiple `mecable`

The type of `organizer ` is selected directly:

- `aster`: a radial array of fiber
- `fake`: two connected asters
- `bundle`: a linear bundle of parallel/antiparallel fibers
- `nucleus`: a sphere with bundles attached to its surface

[class reference](doxygen/class_organizer.html) —
[group reference](doxygen/group___organizer_group.html)


 


