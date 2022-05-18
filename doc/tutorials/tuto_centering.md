# Tutorial: Aster Positionning

Authors: Alessandro De Simone and Pierre Gonczy (2015) and Francois Nedelec (15.09.2017)

### Objective

This tutorial is inspired by the problem of centering: how can the cell define its center?
It considers a radial array of microtubules (an [aster](http://en.wikipedia.org/wiki/Aster_%28cell_biology%29)) confined within fixed boundaries, and examine the role of some of the important parameters of this system.

This tutorial can be performed with 2D or 3D simulations, equally.
Note that 2D simulations tend to be easier to visualize, but 3D simulations are more realistic.
To select 2D or 3D, you need to get the corresponding executables (sim and play) which are specifically made for one dimensionality.

This is the basis of [this article](http://dx.doi.org/10.1016/j.celrep.2016.01.077):

>**Dynein transmits polarized actomyosin cortical flows to promote centrosome separation.**  
>De Simone A, Nedelec F and Gönczy P  
>Cell Reports Vol 14 (9) 2016


### Preamble

We assume here that you have already followed [Tutorial 1](tuto_introduction.md), and that you are now familiar with the general syntax of Cytosim's configuration files, and you can run a live simulation from the command-line.

# Step 1 - Microtubules

You should first set a basic configuration file, defining a `fiber` class called `microtubule`.
Start with a fresh empty directory and a new configuration file:

    set simul system
    {
        time_step = 0.001
        viscosity = 0.01
        display = ( style=2; point_size=7 )
    }
    
    set space cell
    {
        shape = sphere
    }
    
    new cell
    {
        radius = 5
    }

    set fiber microtubule
    {
        rigidity = 20
        segmentation = 0.5
        confine = inside, 100
    
        display = ( line_width = 2 )
    }
    
    new microtubule
    {
        length = 1
    }
    
    run 100000 system
    {
        nb_frames = 100
    }

Check that this works as expected, and verify that the values of the parameters are reasonable.

### Use growing microtubules

Set the `activity` of the microtubule to make it grow smoothly, up to a maximum length of 10.
For this, add 4 lines to the paragraph that defines the fiber class:

    set fiber microtubule
    {
        ...
        activity = classic
        growing_speed = 1
        growing_force = 1.67
        total_polymer = 10
        ...
    }

In addition, one needs to specify the initial state of the microtubule, by adding one line in the 'new' command where they are created:

    new microtubule
    {
        length = 1
        end_state = grow
    }


The growing_speed is specified here in um/s. Since the amount of polymer is limited, the assembly rate will decrease as the filament gets longer. Specifically, growth rate will be linearly dependent on the availability of polymer:

    speed = growing_speed * free_polymer

Where `free_polymer` is a number in [0,1], representing the fraction of free monomers:

    free_polymer = 1.0 - sum(all_fiber_length) / total_polymer

You can press `r` in the live simulation to display detailed information about the length of the filaments.
You will notice that the growth slows down as the length approaches the 'total_polymer' value of 10um.

### Frictionless boundaries

Cytosim's boundaries are without friction. This means that the force exerted by the boundary on a filament is always exactly orthogonal to the boundary at the point of contact. This is thought to be appropriate to represent the plasma membrane, which is a fluid bilayer.
Note however that many cells have a cortex, which is a mesh of actin filaments anchored to the membrane. If the microtubule tip get entangled within the cortex, one could expect some effective friction.

What happens when the microtubule becomes as long as the box diameter?

What happens when it is longer?

### Stalled growth

The growth of a microtubule can stop because monomers are exhausted, or because force prevents elongation. The later case may happen in particular when the ends are pushing against the boundary.

The parameter `growing_force` defines the amount of force that polymerization can sustain. 
This is the level of force at which the microtubule will exhibit significantly slower growth.
Hence the microtubule will stall due to force when both ends touch the box, if the buckling force is higher than thand `growing_force`.

What is the buckling force of a microtubule of length L? What is the value for L = 10?

Can you adjust the parameters of the simulation such as to stall microtubule growth?


# Step 2 - Aster of microtubules

An aster is a composite objects, in which microtubules are anchored at their minus-end onto a object called a `solid`. The `solid` is an object that behaves like a rigid body: it can move by translation and rotation, but it will not deform. By anchoring the microtubules to a solid, we can establish a radial array that resembles a real aster. The structure of the aster is however imposed by the simulation, i.e. its structural integrity is completely independent from the action of motor complexes, which are absent here.

Remove the free microtubule:

    new 0 microtubule

and define a solid called `core` and the aster, by adding these paragraphs:

    set solid core
    {
        display = ( style = 3; )
    }
    
    set aster star
    {
        stiffness = 1000, 500
    }
    
    new star
    {
        solid = core
        radius = 0.5
        fibers = 25, microtubule, ( plus_end=grow; length = 0.1; )
    }

The solid named `core` is used for the construction of the aster, but only graphical properties are defined. For the `aster` object, we defined:

* `solid = core` specifies the attachment `solid` for the fibers,
* `fibers = 25, microtubule` defines the number and the class of fibers used to compose the aster,
* `radius` set the radius of the aster solid (here called `core`) which is at its center.

The aster has 25 microtubules, and they are all initially 0.1 micro-meter long. Their minus-ends are anchored onto the solid with links of stiffness `1000` and `500` (which will be explained shortly), while their plus end pointing out. 

The complete configuration file is here: [aster.cym](data/aster.cym).

### Microtubule length

Parameter `fiber_length` within the `new aster` block defines the initial length of the microtubules. The fiber will grow, but the sum of their lengths cannot exceed `total_polymer=10`, and thus they will not be able to extend longer than `~10/25 um`. You need to increase `total_polymer` to allow them to grow longer.

What happens when microtubules are able to grow up to about the radius of the box?

What happens when microtubules are able to grow longer than this?

### Links and stiffnesses

To physically connect Fibers and Solid, constraining both position and direction, each Fiber is attached to the Solid in two points:

* at the end of the Fiber (the minus-end)
* at an intermediate point of the Fiber at some distance from this end.

These different links can have different stiffnesses:

* stiffness[0] for the link 1 at the end of the Fiber
* stiffness[1] for the link 2 at the intermediate point of the Fiber. 

Set one of the stiffness value to zero, and observe the effect on the aster.

Can you observe centering? Is it stable?

### Hollow core

You can specify two values for `radius` to place the microtubules minus-end a little away from the center.
This can be useful to avoid the very high density of filaments in the center, which is not realistic.

    new star
    {
        radius = 0.5, 0.5
        ...
    }

Check the difference visually.

### Initial position

You can specify the initial position within he `new` command, in the same way as most objects:

    new star
    {
        ...
        position = center
    }

or to place it near the edge: 

    new star
    {
        ...
        position = 4 0 0
    }

### Drag coefficient

In the model, the drag of an aster is the sum of all the drag of the solid and microtubules attached to it. The drag is thus approximatly proportional to the total amount of microtubules-length. Asters with more microtubules, or with longer ones are more difficult to move than asters with less microtubules or shorter ones. This is a simplifying approximation which ignores hydrodynamic interactions.

The drag coefficient affect the speed at which an aster will move under some force, but this should not have much effect on the equilibrium position that will eventually be reached, if one waits long enough.

# Step 3 - Centering by Pushing

## Small cell

Microtubules growing from a centrosome will exert pushing forces against the cell edge; note that the compressive force that a microtubule can sustain before buckling decreases with its length squared:

    F = 2 * rigidity / length^2 

Position the aster 1 um away from the cell edge to start with, in a cell with a radius of 5 um and a total amount of polymer such that the microtubule lengths are equal to the cell radius . Does the aster center itself? How stable is the final position? 

## Large cell

Is the pushing mechanism efficient in a larger cell, say with a radius of 15 um? 
Change cell size by modifying the geometry line:

    new cell
    {
        radius = 15
    }

Place the aster 1 um away from the cell membrane by modifying the position line:
    
    new star
    {
        position = 14 0 0
    }

What happens? Adjust the total amount of polymer so that all fibers can growth up to a length equal to the radius of the cell

## Cell shape

Try different shapes, while keeping the size similar:

    set space cell
    {
        shape = capsule
    }
    new cell
    {
        length = 8, 2
    }

and:
    
    set space cell
    {
        shape = dice
    }
    new cell
    {
        length = 8, 8;
        radius = 2;
    }

Do you find that centration is different when the cells has 'corners'?


# Step 4 - Microtubule Dynamicity

Set the cell radius back to 5 um and the position to “4 0”. Now you will explore the importance of microtubule dynamics. Enable the standard two-state model of dynamic instability as follows:

    set fiber microtubule
    {
        ...
        activity         = classic
        growing_speed    = 0.5
        shrinking_speed  = -0.85
        catastrophe_rate = 0.05,  0.5
        rescue_rate      = 0
        growing_force    = 1.67
        min_length       = 0.5
        ...
    }

You can start with the complete configuration file [aster_dynamic.cym](data/aster_dynamic.cym).

Make sure that the fibers start with a length larger than `min_length`. 

## Nucleation

If the fiber class from which the aster is constructed is dynamic, you can specify a nucleation rate:
    
    set aster star
    {
        ...
        nucleate = 1, microtubule, ( length = 1 )
    }

This will be the rate a which an empty site will be re-populated, leading to a steady-state in which the number of microtubules may vary.
    
The mean length of the microtubules is then equal to `growing_speed/catastrophe_rate` at equilibrium.
Increase (and then decrease) microtubule catastrophe rate, and run the two simulations in turn. Describe the events.

# Step 5 - Cytoplasmic Motors

Reset `catastrophe_rate` to 0.1 and add 2000 motors distributed at fixed points throughout the cytoplasm:

    set hand dynein
    {
        binding_rate = 5
        binding_range = 0.02
        unbinding_rate = 1
        unbinding_force = 2
        
        activity = move
        unloaded_speed = -1
        stall_force = 5
        display = ( color = green, dark_gray )
    }
    
    set single grafted
    {
        hand = dynein
        stiffness = 100
        activity = fixed
    }
    
    new 2000 grafted

Do cytoplasmic motors help centration? Is the final position more stable? 
Can you imagine why? 

Is the result very different if these motors are minus-end directed or plus-end directed? 

Next, repeat this in the large cell. Compare the result with the situation without such motors.

### Placement of motors

Place the motors at a maximum distance of 0.1 from the edge of the cell:

    new 2000 grafted
    {
        position = edge 0.1
    }

Take a few minutes to guess what will happen with plus-end and with minus-end directed motors... write your predictions on paper and only then, run the corresponding simulations.
Check the outcome and verify your prediction.


## The end

Congratulation, you have completed the tutorial.
Please help us to improve this material by sending us your feedback.

