# Frequently Asked Questions

<details>
<summary>
**Can I buy Cytosim?**
</summary>
Cytosim is a free software and also an Open Source project [hosted on GitLab](https://gitlab.com/f.nedelec/cytosim).
</details>


<details>
<summary>
**According to what I have read, the documentation seems really helpfull. Are you interested in feedback?**
</summary>
Yes, of course, your feedback is essential to improve Cytosim. Please send it to `feedbackATcytosimDOTorg`.
</details>


<details>
<summary>
**Can I install Cytosim on Windows?**
</summary>
Compiling "natively" on windows would require dealing with `/` becoming `\` and different end-of-lines, and other annoying issues. You can however run Cytosim on your Windows computer, within [Cygwin](https://cygwin.com) which is a Unix emulator for Windows. We provide [instructions to compile on Cygwin](compile/cygwin.md).
</details>


<details>
<summary>
**Is there a detailed explanation of what the different parameters in the code mean (including units)?**
</summary>
The parameters associated with the objects are defined in a dedicated file, which includes documentation for each parameter.
For example the parameters of `hand` are in file `hand_prop.h`, in which you will find:
 
    /// binding rate when the Hand is within `binding_range` (also known as `binding[0]`)
    real         binding_rate;
    
    /// maximum distance at which the Hand can bind (also known as `binding[1]`)
    real         binding_range;

In this case, it refers to the work of [Leduc et al. PNAS 2004 vol. 101 no. 49 17096-17101](http://www.pnas.org/content/101/49/17096.abstract), in which the molecular binding rate of kinesin was determined to be 4.7 +/- 2.4 /s. Usually, the name of a parameter in the configuration file is also the name of this parameter in the source code, which makes it easy to find the lines where this parameter is used. Try for example to search for `binding_range` in the source code.

Cytosim has [many objects](sim/objects.md), and the documentation is distributed. If a class is called `foo.h`, check for its parameter class that would be called `foo_prop.h`. All parameters use the same [system of units](sim/units.md).

</details>


<details>
<summary>
**Do you have a reference of all of the possible functions and configurations possible?**
</summary>
The source code is the ultimate reference of what can be done with it. Unfortunately, the documentation is often laging behind, because Cytosim is constantly evolving to address new challenges. Cytosim is a simulation platform, and there is an infinite number of possible configurations. 
Please, check the examples in `cym`.
</details>


<details>
<summary>
**My simulations take several hours. Can I monitor the progress of `sim` while it is running?**
</summary>
When `sim` is running, it continuously writes to `messages.cmo` and you can read this file to check progression (we recommand using the command line, for example with `cat` or `tail`)
</details>


# Simulation #################################################


<details>
<summary>
**How can I run simulations in 3D?**
</summary>
The executables `sim`, `play`, `report`, etc. are built for a specific dimension: 1D, 2D or 3D.
Hence to change the dimension, you need to select the right executable. To set the dimension of the executables, set `DIM=3` in the `src/math/dim.h`, enter `make clean` to remove the old files and start the compilation with `make` as usual. You can query the dimension with `sim info`.
</details>


<details>
<summary>
**How is the movement of objects related to the overall forces acting on them?**
</summary>
Cytosim calculates a effective drag coefficient, from the size of the objects and the viscosity of the medium.
For a spherical object, this is Stokes’ law: 

	drag_coefficient = 6 * PI * viscosity * radius

A similar formula is used for elongated objects (filaments).
Cytosim allows you to set a different viscosity for the Fiber or the Solid (by default it is using the global viscosity). In this way, you can control where the `drag` of your objects more finely.
For instance, you can set the viscosity of the Solid higher, and this affects only the drag coefficient of this Solid.

The speed of an object is then proportional to the total force vector acting on it:

	speed = total_force / drag_coefficient
	
</details>


<details>
<summary>
**What is the spring constant of the links, what is their unit?**
</summary>
The force of a link is proportional to its extension: `force = k * delta_x`.
All sprint constant (stiffness) are in pico-Newton / micro-meter. 
Please check our [system of units](sim/units.md).
</details>


<details>
<summary>
**What are the units of kT that are used for temperature?**
</summary>
The temperature is expressed as an energy, in micro-meter x pico-Newton. 
kT is the product of the Boltzmann constant kB by the absolute temperature in Kelvin.
Please check our [system of units](sim/units.md).
</details>


<details>
<summary>
**Do you have a suggestion how to control the .cym file with parameters externally?** <br>**Is it possible to control/override parameters in .cym file from the command line?**
</summary>
We recommend using [preconfig](https://github.com/nedelec/preconfig) with a template. For more details on this approach, [read this](https://openresearchsoftware.metajnl.com/articles/10.5334/jors.156/). Check also [the tutorial](https://github.com/nedelec/cytosim/blob/master/tutorials/tutorial5.md) dedicated to this topic.
</details>


<details>
<summary>
**Can I change parameters throughout the simulation (i.e. lower the motor speed) or add in filaments at specific time points?**
</summary>
You can already change most parameters in the config file (see cym/overlap.cym)
This is a discrete abrupt change. You can add filament with `new` at any time in the same way, between multiple `run`. If you want a more continuous change, we may have to implement it, but it is possible.

	set hand kinesin
	{
	   binding_rate = 10
	   binding_range = 0.06
	   unbinding_rate = 0.3
	
	   activity = motor
	   unloaded_speed = 0
	   stall_force = 6
	
	   unbinding_force = 3
	}
	…
	run system 
	{
	   nb_steps = 100
	   solve = 0
	}
	
	change hand kinesin { unloaded_speed = 0.1 }
	
	run 5000 system
	{
	   nb_frames = 100
	}
</details>


<details>
<summary>
**Can I make a deformable space?**
</summary>
No, this is not possible currently, and it is a very challenging programming task in general.
Cytosim has no deformable generic Space, only static ones. You can do discrete changes, like changing the radius of the sphere, the same way you can change any parameter.
</details>


### Fibers #############################################


<details>
<summary>
**Are the rigidity and segmentation parameters related when assigning parameters to the filament?**
</summary>
Yes, the effective elastic coupling between the vertices of the filaments depends on the bending rigidity parameter, and on the distance between the points, which itself is determined by the segmentation parameter.
The distance between points is not equal to the segmentation, because there is the constraint that a segment should be described by an integral number of points, but it is as close as it can be, given this constraint. In the code the coupling is set as:

	rfRigidity = prop->rigidity / segmentationCube();

(that is rigidity divided by the power 3 of the distance between points.)

You can easily measure the `buckling force` by putting a filament in a circular space, and varying the segmentation and the rigidity parameters. There is a lot of noise in the system, but after averaging many runs, you should recover Euler`s formula. Hopefully these results should be independent of `segmentation`.

For this work, you need to start from `fiber.cym` and vary parameter values using [preconfig](https://github.com/nedelec/preconfig)
</details>


<details>
<summary>
**Can I have microtubules grow to an average length and then stop growing??**
</summary>
Yes, there are two ways to do this:

1. set `max_length` and then all individual MTs will stop growing when they reach this length.
2. set `total_polymer` and this will limit the sum of all the lengths, but not individual ones.
It works by scaling the growth speed by `1 - sum_of_all_MT_length / total_polymer`

These are parameters of `Fiber` and in addition, you should set the catastrophe rate to zero, and have all MTs created in the growing state.

With 2 the growth speed will decrease gradually, so that is probably the more realistic way.
</details>


<details>
<summary>
**If I have two fiber types (actin, microtubule), is there a way to make hands selective for one or the other fiber?**
</summary>
Yes, you can use the parameters `binding_key` for this:

IN short, set the binding keys of the filaments to be binary exclusive, eg. 1 and 2 
and then set the binding keys of the hands equal to the fiber to which they may bind.

There is an example for this: cym/fiber_both.cym
</details>


<details>
<summary>
**Can I bundle fibers without using Couples?**
</summary>

You might want to use the steric interaction to induce bundling, because this can also be configured with an attractive component.
Check the example `cym/steric_bundling.cym`, and check [the documentation](sim/steric.md). 
</details>


<details>
<summary>
**Is there a way to fix one end of fiber to any given point?**
</summary>
Yes, you can create a `single` at the desired location, and attach them directly where you want on the filaments. With one pivot the fiber can rotate, but using two pivots, you will restrict the rotation of the fiber as well. Please check how this is done in `cym/fiber_anchor.cym`.
</details>


<details>
<summary>
**Is it possible to add microtubules with different properties eg acetylated microtubules, for kinesin properties like binding rate or speed.?**
</summary>
No, but if you know C++, you could code this feature.
</details>


<details>
<summary>
**Can I include severing as an option in the filament properties?**
</summary>

*I noticed there is already code in place to model severing, however, how to include it as an option in the filament properties is not covered in the tutorials.*

The code to sever a filament is included, but at the moment it is only used for the Hand with activity=cut.
Please, check the example `cym/hand_cut.cym`, as it is quite simple:

    activity = cut
    cutting_rate = 1         % rate of cutting when bound to a filament
    new_end_state = 4, 4     % state of the new PLUS_END, and new MINUS_END

The hand binds, and sever the filament at the position where it is bound.
We have never really use this code, and I cannot garantee that it works under all conditions.
It is possible to do other sort of cutting: It could be controlled by curvature, etc. 
You would need to dig into the code to use this.
</details>


<details>
<summary>
**The filaments extend outside the simulation space. What is going on?**
</summary>
Cytosim will initially put the filaments at random, such that their middle will be inside the box, but not necessarily the ends. This default behavior can be changed with `placement=all_inside`.

If you must enable confinement, the filaments we be brought inside, but this takes some time steps (with solve=1), depending on the stiffness, viscosity, time_step, etc.
</details>


<details>
<summary>
**Can I do a simulation in which filaments spontaneously spawn in time to simulate the addition of microtubules to the system? (it can be after a fixed time or stochastically)**
</summary>
Yes, you can do this in three ways:

1. use command `new` at any time to add objects:

		run 1000 system
		new 1 filament
		{
		    length = 6;
		    orientation = horizontal;
		}
		run 5000 system
		{
		    nb_frames = 10
		}


2. use the event parameter of `run` to create objects:

		run 100000 system
		{
		    nb_frames = 10
		    event = 2, ( new microtubule { position=(rectangle 2 5); length=0.05; plus_end=grow; } )
		}

3. use Hand's `activity=nucleate` to create fibers:

		set hand nucleator
		{
		    unbinding = 0, 3
		    activity = nucleate
		    nucleate = 0.1, microtubule, ( length=0.1; plus_end=grow; )
		}
		set single protein
		{
		    hand = nucleator
		    activity = fixed
		    stiffness = 1000
		}

Method (1) is not stochastic, but you can chose the time and number of fibers.
Method (2) is stochastic, and you only provide a `rate` (here it is equal to 2).
With (3) the new fiber is created at the position of the Nucleator.
</details>


### Motors ##########################################
 
<details>
<summary>
**I simulate a gliding assay. How can I convert the value for surface density in the simulation to a filament line density that we know from experiments?**
</summary>
The best way to calibrate is indeed to output the actual number of attached motors in the simulation and to match this number with the desired value.
You can get the number of bound motor using `report`.
</details>


<details>
<summary>
**I simulate a gliding assay. Can I predict the density of bound motors along the filaments from the parameter of the simulation?**
</summary>
Yes the number of bound motors can be predicted:

	The capture area = A = length_of_filament * 2 * binding_range
	Number of motors in this area = A * density_of_immobilized_motors

For this N motors you have an equilibrium:

	U = unbound motors
	B = bound motors
	U + B = N
	U -> B with rate `binding_rate`
	B -> U with rate `unbinding_rate`.

You can easily solve the equilibrium for this system analytically.
If the unbinding depends of force, and the effect is significant, you will see a disagreement with the simulation.
</details>


<details>
<summary>
**Can I simulate motors that can bind microtubules, undertake a run during which they stop running and with some probability either pause or undergo 1D diffusion along the microtubule (or fall off), and after pausing either reinitiate motility or fall off with some probability.**
</summary>

There is nothing that does exactly this, buit it should not be too difficult to write.
There are a few possible ways to approach the problem, depending on how the position of the motor is recorded :

- as a single continuous scalar: “the abscissa”
- as a single discrete integer: the index of the tubulin monomer.
- as multiple discrete integers, for example recording the two `heads` of a motor.

Cytosim has classes that use A or B, but not C.
Once you have decided what is the best way to go, all the events (stop, diffuse, unbind, etc) can be treated using stochastic methods, following standard practice (eg. Gillespie or just tossing random numbers).
</details>

### Single / Couple #################################

<details>
<summary>
**Can I attach Single whose activity is fixed to each Fiber when it is created to make them immobile?**
</summary>
Yes, and that is the recommended method. please check how this is done in `cym/fiber_anchor.cym`.
You can adjust the stiffness to tune the effect.
</details>


<details>
<summary>
**How can I relate the binding rate (s<sup>-1</sup>) to the macroscopic biochemical association rate constant (in µM<sup>-1</sup> s<sup>-1</sup>)?**
</summary>
You know already the difference between the affinity of the reaction, as defined by the equilibrium constant, and the molecular association rate constant. The equilibrium constant is the ratio of association/dissociation rate constants.
I will focus on the association rate constant:

Now the parameters in Cytosim are essentially the biochemical association rate, just with a bit of math.
The best is to do this math yourself to understand how it works. What you should calculate is the number of molecules that will bind per unit time (tau = 1 second) and per unit length of filament (L = 1 micro-meter).

Noting C the concentration of unbound molecules in solution. Attention: C is in units of molecules / m^3.
In cytosim, the `capture volume` of the filament is a cylinder of volume `V = L * PI * capture_radius^2`, and within this volume, there will be `C * V` molecules unbound.
And as they all bind at `binding_rate`, the number of binding events is:

    N = tau * binding_rate * C * V

The units work out as they should and N is a dimensionless number.
Now the macroscopic approach should give the same:

    N` = tau * kon * C` 

Except that here C` is in micro-mols-per-liter, and kon also uses mols traditionally.
So you need to convert to physical units using Avogadro`s and convert to molecules / meter^3.

Then the two values should be equal, one needs:

    converted_kon = binding_rate * V 

Importantly, because we are dealing with binding to a filament. That is why you get a `L` and it is not straightforward to convert into a concentration of molecules. There is no simple way around it, because binding to a filament is different from binding to the equivalent number of monomers, dispersed in solution. The law of mass action does not hold in the filament case. That is true also experimentally, and it is well possible that the estimate found in the literature may not consider this fact correctly, but they can still give you a useful order of magnitude. For more on this topic, you can read Hackney, Biophysical J. Vol 68, pp 267-270; 1995 - PMC1281942
</details>


<details>
<summary>
**Can I create a Myosin 2 motor and crosslinkers that are able to reorient the fibers in the antiparallel direction?**
</summary>
It is not very easy to reorient long filaments, because the lever arm is limited.
Also they may be entangled with other things, and then it is hopeless.

I would say that if two filaments are parallel, it is not possible to flip one in the other direction. That would require too much rotation.

A generally better strategy is to nucleate filaments in a particular way, which works well, but it requires that the filaments are dynamic in the system.

If you have entities (Couple) with the same two motors: M=M then indeed they move on two parallel filaments, and that will just maintain the parallel bundle.

You could use however B=M entities, where B is a binder, which bind but is not motile. This will create a tug of war between the couples that are bound in different configurations (with a drawing you will see what happens). If the unbinding rate is force dependent, the tug of war may be resolved, and this might induce the filaments to slide relative to each other (the outcome will depend on the parameters).
</details>


<details>
<summary>
**Can I simulate a microtubule on a motor surface that consists of two parts with different properties, e.g. maximal velocities, and maybe even different motor properties?**
</summary>
The easiest is to define two type of motors, and to put them at different places.
There is an example `glide_stripe.cym` showing you how to place Single into simple regions.
</details>


<details>
<summary>
**Can I have different densities of motors in microtubule gliding assays?**
</summary>
Yes, you can easily setup a gradient in the command used to place the motors used in the gliding assay:

	new 16000 grafted
	{
	     position= gradient -5 5
	}
</details>


<details>
<summary>
**Is it possible to fix a `couple` to the position where it is at first time?**
</summary>
No, you cannot fix a Couple at a given position.
You can set the diffusion constant to zero, and it will not move, but only until it binds to a filament. As long as it is bound, it will move with the filament.
</details>


<details>
<summary>
**I know how to make immobile Single. Can I make immobile Couple?**
</summary>
In cytosim you cannot make immobile Couple. However, you can instead create two Single, and if they are both immobile, the result would be similar. The Couple is made of two Hands, and you give these Hands to each of the Single. So instead of: 
	
	set couple complex
	{
	    hand1 = kinesin
	    hand2 = dynein
	    stiffness = 100
	    diffusion = 10
	}
	new 2000 complex

you set:

	set single fixedK
	{
	    hand = kinesin
	    stiffness = 100
	    activity = fixed
	}
	
	set single fixedD
	{
	    hand = dynein
	    stiffness = 100
	    activity = fixed
	}
	
	new 2000 fixedK
	new 2000 fixedD
</details>

<details>
<summary>
**I used the "fast_diffusion=1" setting for my diffusing couples, so assuming their concentration is uniform. Should it make the simulation faster?**
</summary>
Yes, it should be a faster, because the diffusion of free couple is not simulated. How much you gain depends on the number of unbound Couples in the simulation, but usually that is not a big gain. The main reason for using this approximation is to make the model simpler, because there cannot be any accumulation of unbound couple anywhere in the Space, and that is a good reason to use it.
</details>

### Solids / Beads ####################################

<details>
<summary>
**I am using a Solid consisting of spheres of different sizes that are fixed together to make a complex shape. The viscosity of the simulation space is 1 Pa.S. What is the drag coefficient of my object?**
</summary>

The drag coefficient, for translation and rotation, are calculated considering all the spheres, assuming that Stokes’ law applies to each of them, and that there is no hydrodynamic interactions.

The translation drag coefficient is thus simply the sum of the bead’s translation drag coefficients. The rotation coefficient further depends on the spatial distribution of the beads. If spheres are more distant, the object is harder to rotate.

The math is described on page 13 of [*Collective Langevin Dynamics of Flexible Cytoskeletal Fibers*](http://www.doi.org/10.1088/1367-2630/9/11/427); New Journal of Physics 9 (11) 427, 2007.

This means that it will be overestimated, because hydrodynamics in general will make things easier to move.

If 2 beads of similar size overlap, Cytosim estimates the total drag to be 2x the bead's drag.
With hydrodynamics, this might be closer to 1x.

However, cytosim could be modified to change this. One can also print the drag coefficient that is calculated, and change the effective viscosity of the spheres (the one that is defined in "set solid {}") to adjust it to a level that is deemed realistic.
</details>


<details>
<summary>
**I understand that it is possible to spontaneously spawn molecules in time, but is it also possible to spontaneously remove molecules from the system, at some fixed rate? And if so, can it be done by specifying also the precise region to remove them from?**
</summary>

Yes, the function ‘delete’ can be called in a stochastic manner using an ‘event’, and you can specify that the object should be inside or outside a Space. Please, see the example `fountain.cym'.
This approach is generic and it should work for most object class in Cytosim
</details>


# Misc ###################################


<details>
<summary>
**Is it possible to restart the simulation from the same configuration at which it stopped?**
</summary>
Yes, you will need to extract the frame you want to restart from, with the program `frametool`, which you first need to compile, in cytosim source directory:

	make frametool

then navigate to the old run dir, and run:
	
	frametool objects.cmo 30 > objects.cmi

in this example, we extracted frame #30 (index start at 0), to create the file `objects.cmi`. Run `frametool objects.cmo` to know how many frames are in the file.

2. Use a fresh directory, copy `objects.cmi` and `config.cym` from the old simulation.
Edit `config.cym` and add the `import` command to read the frame.
	
	set …
	
	import objects.cmi
	
	run 1000000 system
	{
	    nb_frames = 100
	}

the `import` command replaces all the objects of the simulation, without affecting their properties. You should remove all the `new` since the `import` will erase all objects anyhow. Any `new` after `import` will add objects to the imported state.
 
It is important to do this in a fresh directory, as `sim` will create a new `object.cmo` file, erasing the old one.

You can later merge two object files later if you want to display them continuously in play. Make sure you copy all the files before you start experimenting, but normally this works with the standard unix `cat`.
</details>

<details>
<summary>
**Is there a way run the exact same simulation again, for debugging or other purpose?**
</summary>

*My HPC team suggested that in order to debug the code, they would need to know if there is a way to make it run with the same seed for the random number generator to make reproducible simulation results. Could you please help me with that?*

There is a parameter `random_seed` in `simul`. By default, it is set to zero, and then cytosim seeds using the timer. The value that is used is then reported in `properties.cmo`.
So you can rerun the same simulation by copy-pasting that value into config.cym (and using the same machine). However, you can also set `random_seed=1` from the start.
</details>


<details>
<summary>
**What is the drag coefficient of an aster?**
</summary>

The forces add up and thus the drag coefficients also mostly do.
As an aster is made of one Solid and N Fibers, the drag coefficient in general will be:

	drag_solid + sum( drag_fiber )

However, that is true only if all objects move at the same speed, which is not the case if the fiber bend, typically. If filaments bend, the force diminishes. In general, hydrodynamic interactions would reduce the drag compared with this formula.
That is when the fluid can go around the aster, it is easier to do so, than to go through the aster.

For the Solid, this is Stokes’ law: drag_solid = 6*PI*eta*R
For the Fiber, we use a different law, which is for a cylinder:
	
	cylinder_drag = 3*PI*length*viscosity / ( log(length/diameter) + 0.312 )

So the drag of a fiber depends on the length, and is not the sum of the polymer length.

You can check the formula in:

	void Solid::setDragCoefficient()
	void Fiber::setDragCoefficient()

You can also add a printout in these functions to get values:

    std::clog << “Solid " << reference() << " has drag " << soDrag << "\n";

Change the formula for the drag coefficient would not break the code.
</details>


<details>
<summary>
**I get this warning "Stagnant solver? residuals: ... at  iteration ...".
Is this something to worry about?**
</summary>
This message indicates that the task of solving the system of linear equations, to simulate the filaments is getting hard, but cytosim is still able to do it.
Generally, it is a sign that the program would run better if the time_step was smaller.
I would suggest that you reduce `time_step` to half its current value, and check the results.
Normally, the results should look the same, and if the CPU time is not too high, then switch to this smaller time_step.
</details>


<details>
<summary>
**Can I re-run the Loughlin et al. simulations of the mitotic spindle?**
</summary>
Cytosim has changed completely in 2012, breaking backward compatibility, and if you compare the config files, you will understand the magnitude of the changes... we have not maintained some of the functionalities implemented for this model. Hence we cannot run this model in the up-to-date version of cytosim. Please ask us for the code used back in 2011, and we will send it. It compiled and still workson Mac OSX 10.12.6, in july 2018.
</details>


<details>
<summary>
**Can I re-run the Belmonte et al. simulations of contractile networks?**
</summary>
Yes, everything works and all the code was included in the supplementary material of this paper. 
</details>


# Graphics #################################################


<details>
<summary>
**Can I save what is rendered during `play live XXXX.cym` into a movie file?**
</summary>
No, `play live` will discard the past simulation state and can only save the current state, which makes it very difficult to produce a proper animation. You can however run the simulation using `sim`, saving regular snapshots, and then use `play movie` to convert the output of `sim` into images. These images can then be assembled into a movie in different ways (ffmpeg).
Please, [check this](making_movies.md).
</details>


<details>
<summary>
**I have two different sized cells, say 10 um and 100um and I make movies of the same size, 1000x1000 pixels. However, microtubules and motors in the 10um cell appear bigger than in the other one. Can I make them scale correctly without losing resolution?**
</summary>
By default, Cytosim adjusts the `zoom` factor to fit the entire simulation space, but you can disable this and define the dimension covered by the window like this:

    bin/play live auto_scale=0 view_size=5

Hence I would recommend you keep the `view_size` and the number of pixels the same to make all your movies. The image with the small cell will be small in the middle of a black image, but the pixels per micrometer ratio will be the same for all the movies.
Alternatively, you could also scale the movie size with the cell size.
</details>


<details>
<summary>
**Is there a way to customize the colors, size of filaments, etc?**
</summary>
You can use a `setup` file you want to change some parameters of the display.

To create this file, follow these steps:
1. Adjust the parameters to what you like in `play`. 
2. Press `R` to output the parameters values on the terminal
3. Copy-paste to a new file that you call `style.cyp`

You can then start play with this file, and you should recover the display the way it was:

	play style.cyp live

You can of course edit the file and change the settings.
</details>


# Reporting #################################################

<details>
<summary>
**Can I get the coordinates for the filaments for quantification from the output files?**
</summary>

Cytosim has a tool called `report` to do this type of work.

1. Compile the tool:  `make report`
You should use the same settings as `sim` and `play`, in particular the DIM defined in `dim.h`
2. You can then invoke `report` and get filament coordinates.
For example `report fiber:points` gives me the coordinates of the vertices that make up the fibers.

There are many options to `report` and you can find a list in `src/sim/simul_report.cc`
	
	%   fiber f1:100
	100     -5.88    -6.137
	100    -6.023    -5.658
	100    -6.165    -5.178
	100    -6.308    -4.699
	100    -6.453    -4.221
	100      -6.6    -3.743
	100    -6.749    -3.266
	100    -6.902     -2.79
	100     -7.06    -2.315
	100     -7.22    -1.842
	100    -7.382    -1.369
	100    -7.546   -0.8961
	100    -7.711    -0.424
	100    -7.876   0.04795
	100    -8.041    0.5199
	100    -8.207    0.9915
	100    -8.375     1.463
	100    -8.543     1.933
	100    -8.711     2.404
	% end
	
</details>

<details>
<summary>
**What are the different ways to get the positions of the filaments?**
</summary>

	report fiber:point

gives the coordinates of the points that are used to model the fibers:
first_point = minus-end
last_point = plus-end

This gives the exact location of the filaments in the model, but these points can change, especially when a filament is lengthening or shortening, as cytosim adds and redistributes these points automatically.

	report fiber:speckle

Gives points that are distributed randomly over the filaments, but which are fixed relative to the `lattice` and stable over time. If the filament lengthen, you will get more `speckles` but the existing one will not move or disapear. Speckles disappear when the fiber is shortening. So the speckles indicate the movement of the filaments.

</details>

<details>
<summary>
**In what referential is this data? What are the units?**
</summary>
The origin (0,0) is in the centre of the simulation volume. The reference frame can be displayed in 'play' by pressing 'x'. A scale bar appears if you press 'b'. Press these keys again to make the items disapear.
The position and length are given in micro-metres.
</details>


<details>
<summary>
**How can I extract the position of both hands in a couple?**
</summary>
You can get the position of the attachment point on the fibers for all `bridging` couple like this:

	bin/report couple:link

Output:

	% frame   0
	% start   0.4
	%     class  identity    fiber1 abscissa1    fiber2 abscissa2 cos_angle
	        0      1294        34   1.72501        26   11.7828 -0.937117
	        0       311        63   9.08312         2   8.04673  0.925618
	        0       882        58   10.5219        62   3.29681   0.63335
	        0       244        38   5.61696        44   4.47288  0.352854
	        0      1613        63   13.2303        31   13.4573  0.646246
	        0       842        52   9.06932        22   2.12533  0.999798
	        0      1532        37   10.1407         3    4.8218  0.879076
	        0       360        31  0.563402        49      10.8 -0.419071
	        0       741        24   9.46205        10   11.9597  0.303854
	        0       231        72   12.4499        21   3.67777 -0.653629
	        0      1285        52   8.99386        22   2.05029  0.999798
	        0      1084        32   1.05265         2   13.3123 -0.876371
	        0      1979         8   6.15876        11   2.11755  0.956354
	% end
</details>


<details>
<summary>
**Can I run the simulations and then extract information afterwards from the .cmo files?**
</summary>
In addition to `play` and `sim` there is a tool called `report`, which should be compiled by ´maké together with the other. You can use it to read the output of `sim` and get the data out. It works like the report command but on a completed run:

    ./report fiber:point > data.dat
    
Here the output of the command is [redirected](https://en.wikipedia.org/wiki/Redirection_(computing)) to a file with `>`.
</details>


<details>
<summary>
**Is it possible to generate report output corresponding to a particular frame?**
</summary>
Yes, it is possible to extract the information for only a subset of frames:

    ./report fiber:point frame=10 > data.dat
    
    ./report fiber:point frame=10,20,30 > data.dat
    
You can also instruct `sim` to create the report directly, by including a `report` command in the config file:
	
	run 10000 system
	{
	    nb_frames = 10
	}
	
	report fiber:points fibers.txt 

This will store the coordinates at this particular time of the simulation, into a file `fibers.txt`.
</details>


<details>
<summary>
**Can I use either Python or MATLAB to read the coordinate of all the system components?**
</summary>
We recommend using `report` to make easy-to-parse files. For example, to get the coordinates of fibers:

	bin/report fiber:point > fiber.txt

There are plenty of output modules, and please check `simul_report.cc` to see what is there.
However, if you want to get the whole thing, you can also generate `objects.cmo` in text mode:
	
	run 10000 system
	{
	    nb_frames = 10
	    binary = 0;
	}
	
	report fiber:points points.txt 

And the output trajectory `objects.cmo` will contain everything in text-mode:

	#Cytosim  Mon Mar 12 23:10:02 2018
	#time 10.000000, dim 2, format 41
	#section space
	e0:1ellipse  2 5.9991 4.1673 10 78.5398 0.9997 0.0233 0.0000 -0.0233 0.9997 0.0000 0.0000 0.0000 0.0000
	#section fiber
	f0:1 3735833971 12.0000 0.5000 0.0000 25
	 5.9954 0.2700
	 5.4955 0.2593
	 4.9956 0.2486
	 4.4957 0.2381
	 3.9958 0.2280
	 3.4959 0.2181
	 2.9960 0.2087
	 2.4961 0.1998
	 1.9962 0.1913
	 1.4962 0.1831
	 0.9963 0.1748
	 0.4964 0.1659
	 -0.0035 0.1566
	 -0.5034 0.1467
	 -1.0033 0.1365
	 -1.5032 0.1258
	 -2.0031 0.1148
	 -2.5030 0.1037
	 -3.0028 0.0923
	 -3.5027 0.0809
	 -4.0026 0.0693
	 -4.5024 0.0577
	 -5.0023 0.0459
	 -5.5022 0.0341
	 -6.0020 0.0223
	#section end
	#end cytosim Mon Mar 12 23:10:02 2018
</details>


<details>
<summary>
**How can I export the XY positions of the data, e.g. the strain force?**
</summary>
There is an `export` function, and even an accessory program to export things.
For example, I get the force of every motor in the simulation from `report single:force`, like this:

	% frame   49
	% start   50
	% class id state position force
	1      2635 1   7.83849  2.92708  -4.89505 0.531159
	0       234 1   4.09032 -3.34168  -1.15568  1.53319
	0      2140 1   4.20264 -3.96491  0.568488  1.80256
	1      4202 1  -6.24798 -3.34674  -0.154147 0.786457
	0      1604 1   4.54602 0.150717   3.67875 0.452888
	0      1161 1   4.07268 -1.88066  -1.94039  1.92252
	0      1540 1  -3.35479 0.761841  0.557642 0.742035
	1      4853 1    -6.562 -0.354874   3.08978  1.60128
	1      3854 1  -6.25633  -3.2128  -0.372595 0.736061
	0      2428 1   4.27979 -1.14094   4.37166 0.734957
	0       256 1   4.04292 -2.62947  -0.217265  1.60336
	1      3591 1    6.9669  2.37945   -1.4706 -6.03942
	0      2430 1  -3.48165  1.06843   0.20585 -0.0465097
	0      2481 1   4.25996 -4.30718  0.761503   1.1907
	0      1859 1  -3.10476 -0.020653    1.6283 0.917287
	1      4275 1  -6.57178 -0.473411  -0.528856 0.309855
	0      1878 1  -3.86934  1.75798  -3.08386 -1.19756
	0       290 1  -3.03989 -0.432839  -1.79529 0.0626133
	% end

You can get the position of the microtubules with `report fiber:position`
There are many other outputs possible, listed in the file `simul_report.cc`.
</details>


<details>
<summary>
**I am performing a parameter sweep with `scan.py`. How can I get different names for the report file?**
</summary>
You can use `preconfig` to template the file name:

	report fiber:force force_[[nb]].txt

Normally, these files will be created in the local run folder, but you may also use:

	report fiber:force ../report.txt
	
</details>

# Compilation #################################################

<details>
<summary>
**Do you have a config file where you utilize 3D rendering and rotation?**
</summary>
You cannot select 2D versus 3D from the config file.
To run a simulation in 3D, you need to edit the file `dim.h` and recompile Cytosim.
You can call the 3D executable `sim3` and then you run this one to get a 3D simulation.
</details>


<details>
<summary>
**I need to have Lapack on our server to compile Cytosim, any idea how to install it?**
</summary>
You can find a precompiled BLAS/LAPACK distributions for Linux. Ask you system administrator to deploy it. If you really need to compile BLAS/LAPACK, the [reference code is on netlib](http://www.netlib.org/lapack/index.html).
</details>


# Algorithms #################################################

Cytosim uses [Langevin dynamics to simulate the system of filaments](https://iopscience.iop.org/article/10.1088/1367-2630/9/11/427/meta), with an implicit integration scheme.


<details>
<summary>
The time step in langevin evolution should depend on the fastest modes in the system. As actin fibers are extremely stiff, fast modes might mostly be associated with filament stretching. Does cytosim impose filament length conservation as a constraint rather than an additional potential in order to get around that bottleneck?
</summary>
Yess, because this method it more efficient, computationally. In short, the time-step is limited by a condition that looks something like this:

	mobility * time_step * stiffness < 1

so the time_step must be small, in a manner inversely proportional to stiffness.
And the actin filament stretching stiffness is very high indeed (stretching elasticity of a single 1 μm-long actin filament was reported to be ~35 pN/nm).
</summary>
</details>


<details>
<summary>
**Is the choice of time step inherently dependent on the variable that the user wishes to study?**
</summary>
Yes, I would advise anyone to produce a curve such as [Fig. 8](https://iopscience.iop.org/article/10.1088/1367-2630/9/11/427/meta), in which one monitors a well chosen and meaningful output as a function of the time-step.
This will permite a wise choice of time step!
</details>


<details>
<summary>
Even though actin filaments are extremely stiff, local tension loading can occur (on the order of ten piconewtons) from asymmetric myosin/crosslinker binding dynamics. Wouldn't such tensions might be crucial to hold a network stable as tensegrity based structures? 
</summary>
Certainly the force along the backbone, and the shape of filaments are very important, but this you get correctly with the constraint of non-extensibility. What we are talking about is wether the amount by which a filament extends under some force…  in principles that could be relevant as well, but I have not encountered a system where that was clearly the case. On the other hand, I think it is undesirable to model a filament that can easily stretch… but that is exactly what using potentials, to keep the filament’s length, encourages you to do, because of the computational costs. The main consideration for us is performance, as constraint allows to effectively remove some high stiffness modes (possibly the highest stiffness modes) that are of little interest to us.
</details>


<details>
<summary>
**My simulation is very slow, could it be a convergence issue?**
</summary>
Yes, it could be. To test this, you can do a few things:
- decrease your time step by 1/2 (and compensate with more steps).
- enable precondionning (set `precondionning=1` in *simul*) 
- start sim with verbose mode (set `verbose=1` in *simul*)
It will then print additional information in ‘messages.cmo', like this:

	Meca 2*3400 brick 17 MSS1x 0 MSSBx 4*125 precond 0 count 52 residual 0.00123246

Here the system is 2D, the matrix size is 2*3400. The block size is 17. The matrix types are “MSS1” and “MSSB” but MSS1 is not used. There system is solved without preconditionning, and it converged after 52 iterations to a residual of 0.00123.

Convergence issues would be indicated by a value of ‘count’ that becomes larger. Normally the number of iterations should stay below a few hundreds.
</details>

<details>
<summary>
**Is there a good way to find out how much time is spent in different submodules of the simulator?**
</summary>

In ‘interface.cc’ within ‘execute_run()’ cytosim prints CPU information when a frame is saved.
Cytosim alternates ’step()’ and ’solve()’ and you can easily add a bunch of printf() to report more detailed CPU time information.
You can do this at every time step, for example, by changing ‘execute_run’ in this way:

        hold();

        clk = clock();
        simul.step();
        
        static double clk = 0;
        double c_step = double(clock() - clk) / CLOCKS_PER_SEC;
        clk = clock();

        (simul.*solveFunc)();
        
        double c_solve = double(clock() - clk) / CLOCKS_PER_SEC;
        Cytosim::log("CPU  %6i  step %10.3fs  solve  %10.3fs\n", sss, c_step, c_solve);

        ++sss;
</details>


# Performance #################################################

<details>
<summary>
**Is there any way to improve calculation speed?**
</summary>
To speed up the calculation, you should compile with the `fast` option, and turn off assertions.

You can select the `fast` option by editing the file `makefile.inc`. That is a variable in the beggining that you need to set to `F`.

You can turn off assertion by editing the file `assert.h`. The keyword NDEBUG needs to be defined.
</details>


<details>
<summary>
**I saw cytosim includes pthreads, How can I enable the multi thread version?**
</summary>

*Compilation with multithreading support is explained [here](compile/multithreading.md)*

This is doable, but it may not give you any benefit:
If you use 4 threads, Cytosim will run between 2x and 3x faster, at best.
So if you have many simulations to run, which happens often for example if you want to vary parameters, you will make a better use of your resources by running 4 mono-threaded simulations in parallel, than by running 3 multi-threaded simulations sequentially. In the same time, you will get 4 jobs completed in the former case, versus 3 in the later one.
If you need to run many conditions, you can trivially parallelize the task, and in that case, it offers you the best performance.
</details>


<details>
<summary>
**I've noticed that only one core is used by sim... so I'm wondering if there is any fix for this.**
</summary>
Cytosim can be linked with a multithreaded version of BLAS/LAPACK, but in my experience, this will not increase performance much. For a good use of multiple cores, the calculation needs to be parallelized at a higher level, changing the C++ code. I this direction, we obtained [~3x gain using 4 cores, for some problems](compile/multithreading.md).
In principle, this can be scaled to a higher number of cores, but I do not have any machine to do the development. Nearly always, I run sequential simulations, using single-core, in parallel on the machine, and this anyway is a better use of the hardware, than multithreaded code, which has overheads. I hope this helps!
</details>


<details>
<summary>
**The 2D model was running in 2 minutes on Steve's gaming PC but the 3D model is taking 2 hrs. Why?**
</summary>
</summary>
First, you need to compare the 2D and 3D versions on the exact same machine.
In my experience, a 3D model may need 2–3x CPU of a SIMILAR 2D model.
This is generally true because you have 3/2 x more degrees of freedom and more complex calculations, but only if there is the same number of objects in both.

However, porting a model to 3D often involves boosting the number of objects, and that can slow things down dramatically.
In your case, do you have more filaments in 3D than in 2D?
</details>


<details>
<summary>
**Do you know if there is a way to allocate more CPU to Cytosim?**
</summary>
</summary>
Multithreading is not a solution, if you need to run multiple simulations (many more than the number of cores). That is because most likely you can use all your CPU cores by running multiple simulations in parallel. So while multithreading can make a single program finish earlier, it will overall increase the computation time needed to complete all the simulations.
</details>


<details>
<summary>
**Do you know of coding tricks for making 3D simulations run faster?**
</summary>
</summary>
You can investigate why it is slow by profiling the program. On Mac OSX, use Xcode's performance tools. On Linux if you compiled with gcc, check this:
https://www.thegeekstuff.com/2012/08/gprof-tutorial/
</details>


# Advanced topics ###########################################

<details>
<summary>
**Can I create custom shapes for cytosim?**
</summary>
The config files offers only very limited options, and you cannot change the shapes.
Yes, you can create a C++ class that will extend the class Space, and code the shape that you have in mind.
This would be the simplest way I can see at the moment to do what you want.
We have done this often in the past to encode special shapes. 
For someone who knows the syntax of C++, this typically may require 1 week of work.
</details>


<details>
<summary>
**Is it possible to make a capping protein that attached to the plus end and halt polymerization of fibers?**
</summary>
We never needed a `capper` activity, and have not implemented one,
but you could take the `Actor` class and modify it to do what you want.
Either you modify `Actor` or you clone it under a different name. 

You would need to copy 4 files: actor.h actor.cc actor_prop.h and actor_prop.cc,
eg. to `capper.h`, etc.
and then you rename all Actor to Capper,
and then link these new class by editing `hand_prop.cc`
There are only 3 lines to write (orange below), duplicating what is done with “actor”:

Near the top of the file:

	#include "mighty_prop.h"
	#include "actor_prop.h"
	#include “capper_prop.h"


	HandProp * HandProp::newProperty(const std::string& nm, Glossary& glos)
	{
	...
              if ( a == “act” )
	            return new ActorProp(nm);
	        if ( a == “cap” )
	            return new CapperProp(nm);
	        if ( a == "bind" )
	            return new HandProp(nm);
	        
             throw InvalidParameter("unknown hand:activity `"+a+"'");
	    }
	    
	    return new HandProp(name);
	}
This is a more work but it will guarantee that your code does not break someone`s else code.
</details>


<details>
<summary>
**I found a function whose name is hasKinks() in the Fiber class.
Can I use this function in my simulation?**
</summary>
The function was not called, and this is not a feature that can be access from the config.
You could for example cut filaments at the positions where they make a sharp angle like this:
	
	void Fiber::step()
	{
        #if ( 0 )
            unsigned p = hasKink(0);
            if ( p )
                objset()->add(severPoint(p));
        #endif
	...
	}
   
</details>


<details>
<summary>
**Can I make fibers severed when they are locally stretched with the tensile stress above a critical one?**
</summary>
That is feasible, using 

	real RigidFiber::tension(size_t p) const 

You could make this code dependent on a parameter, and link the value of the parameter to the config fie. This requires some work, but it is not difficult.
</details>


<details>
<summary>
**Can I fix some objects to the locations where they are created first throughout my simulation?**
</summary>
<p>
Currently, you cannot have some objects immobile and other not.
There is a parameter `solve` in `run simul`. If you set it to zero, objects are immobile but this applies to all objects.

It should however be possible to modify cytosim to do have some object mobile while others are not.
There is a function that calculates the speed, as a function of the force:

	virtual void Mecable::projectForces(const real* X, real* Y)

To make the corresponding object immobile, set `Y` to zero like this:
	
	void projectForces(const real*, real* Y) const
	{
	   for(size_t i=0; i < DIM*nbPoints(); ++i)
	   		Y[i] = 0;
	}

This is a virtual function, and you must modify it in a child class of Mecable.
Hence, if you want the Fibers to be immobile, you must do this in the RigidFiber class.
</p>
</details>


<details>
<summary>
**I want to make some fiber class fixed in space while others can move in space normally.**
</summary>
At the moment, `solve=0` applies to all mecable and simply turns all the mechanics off.

It is possible however to do what you describe, by modifying the code to do two things:

- NOT link these objects into the Meca,  
- mutate all the Couples effectively to call Meca::addPointClamp() instead of Meca::addLink().  
 
You can add a boolean parameter “mobile" into these classes, and test for it when you build the Meca:

	void Simul::setInteractions(Meca & meca) const

 Not having these fibers included in the Meca would speed up things certainly.
</details>


<details>
<summary>
**Is it possible to change functional form of bending stiffness/axial stiffness for a fiber?**
</summary>

The Rigidity term is calculated in class `RigidFiber`:

	void RigidFiber::addRigidity(const real* X, real* Y) const

The job is done in:

	add_rigidity1(X, DIM*(nbPoints()-2), Y, rfRigidity);

That is essentially a second differential, multiplied by the rigidity modulus divided by the cube of the segment length.
This correspond to standard elasticity, everywhere the same in the filament.

You can already define different classes of filaments with different rigidity.
You could make the rigidity dependent on some thing else, with a rigidity defined for each filament at each time point.
You would just need to plug in the value instead of `rfRigidity`.

That`s crazy but for fun you could do this:

	#include "object_set.h"
	#include "simul.h"
	
	#if ( DIM == 1 )
	
	void RigidFiber::addRigidity(const real* X, real* Y) const {}
	
	#else
	
	/**
	 calculate the second-differential of points,
	 scale by the rigidity term, and add to vector Y
	*/
	void RigidFiber::addRigidity(const real* X, real* Y) const
	{
	    if ( nbPoints() > 2 )
	    {
	        real time = simul().simTime();
	        add_rigidity1(X, DIM*(nbPoints()-2), Y, rfRigidity * (1 + cos(time)));
	    }
	}

The axial stiffness is `infinite` in cytosim, and that kind of built-in. It would be much more work to change.
</details>


<details>
<summary>
**I want to understand how Cytosim works and look at the code. Where should I start?**
</summary>
It depends on your levelin programming, but you could start for example by 

- Math concepts: Vector3, Solver, Random, Polygon, Quaternion, Rasterizer
- Simulation objects: Movable, Space, Mecable, Chain, Mecafil, Fiber, Hand, Single

To do this, I recommend printing the .h and .cc files on paper (yes, paper), and to read the code from top to bottom, in a quiet time and away from your computer. This will allow you to examine the structure of the code in detail. 

To understand how cytosim *really* works, please check `src/sim/meca1d.h`.
This is a 1D bare-bone solver, which is equivalent to what is done in Meca, but much simpler than the 2D and 3D version `src/sim/simul_solve.cc`.
</details>

<details>
<summary>
**Comment fixer un aster à une position donnée, puis mesurer la force nette due a cet ancrage?**
</summary>
Tu ajoute un 
	
	addPointClamp(Mecapoint const& pta, Vector pos, const real weight)

dans

	void Solid::setInteractions(Meca & meca) const

Il faudra ajouter un parametre dans SolidProp, pour controller cela.
Tu pourra alors obtenir la force sur le lien qui retient le `core`:

	weigth * (position_anchor - position_model_point )
	
</details>


<details>
<summary>
**I have a daughter filament branching off a mother filament (by a nucleator) and the mother filament gets depolymerized, how can I make sure the daughter filament is then also destroyed?**
</summary>
I guess you are using a Couple with an `activator` and a `nucleator`.
You want the nucleator to kill its Filament, if the `activator` detaches.
If `nucleator:addictive=1`, the filament catastrophes if the `nucleator` detaches.

Your condition is different, but if the mother filament vanished, then the `activator` detaches, and this is easy to detect. If the `activator` is attached, the function `stepLoaded()` of `nucleator` will be called, whereas if the `activator` is detached, `nucleator:stepUnloaded()` will be called. 

Hence two lines of code in `Nucleator::stepUnloaded()` should do the job:

	void Nucleator::stepUnloaded()
	{
	…    
	    /// delete entire fiber
	    if ( prop->addictive )
	    {
	        fiber()->objset()->erase(fiber());
	        return;
	    }
	…
	}

If you make the parameter `addictive` an integer so it can take multiple values.
In file `nucleator_prop.h`:

    int          addictive;

You could have multiple options like this:

	void Nucleator::stepUnloaded()
	{
	…    
	    
	    /// OPTION 2: delete entire fiber
	    if ( prop->addictive == 2 )
	    {
	        fiber()->objset()->erase(fiber());
	        return;
	    }
	…}

In this way you would keep backward compatibility with your older model.
</details>


<details>
<summary>
**Can I limit binding to microtubules once they are a certain minimum length?**
</summary>
That is very easy to implement.
There is a function `Hand::attachmentAllowed()` that returns true of false.
You simply need to add a test in there for the length. The quick and dirty way is this:

	bool Hand::attachmentAllowed(FiberSite& sit) const
	{
	    if ( sit.fiber()->length() < 1 )
	        return false;	 
	...
	}

</details>


<details>
<summary>
**Adding fluid: I’ve been told that this is an extremely difficult task and should not be attempted unless one is absolutely sure of its necessity?**
</summary>
I tend to agree with this. You could add hydrodynamic interactions using Oseen's tensors, and that would involve adding another matrix into the master equation. That is very serious work and will require a good knowledge of the topic, and of the inner working of Cytosim. Ultimately, performance will be significantly reduced (because everything interacts with everything else), such that you may be limited to small systems, or be obliged to spend effort on the parallelization/hardware side. I would be happy if someone did this, but it is a major endeavor!
</details>


<details>
<summary>
**Local conversion of Single motors to linked motor Couples**
</summary>
**I would like the ability to define a region (say a rectangle centered on the cell) such that any two single motors that moved into this region has some probability of following a dimer, given they are within close proximity of one another?**

That is doable in a few weeks. You could start with a naive method to detect which objects are within this rectangle, delete pairs of ’Single’ and create a corresponding number of ‘Couple’ with appropriate positions and properties to compensate for the ones that have been deleted. For you first attempt, do not worry about performance, and just implement a exhaustive scan: 

	( for all A ) x ( for all B ) : if ( A close to B )  …

However, before you do this, I would still advise to think hard wether you really need this in your model. There maybe a simpler solution!
</details>


# More questions? #########################################

<details>
<summary>
**You have a question that is not answered here?**
</summary>
Please write to feedbackATcytosimDOTorg
</details>


<details>
<summary>
** ?**
</summary>
</details>


FJN, 2.5.2019

