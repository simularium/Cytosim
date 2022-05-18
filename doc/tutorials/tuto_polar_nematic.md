# Tutorial: Polar vs. nematic motor-microtubule organization

Authors: Jamie Rickman and FJN (10/2018)

### Objective
This tutorial is motivated by the question of how motor-microtubule networks self-organize into distinct network architectures. Specifically, we are interested in the conditions that lead to either **polar** or **nematic** network states. 
![alt text](images/nematic-vs-polar.png "Nematic vs. polar microtubule organizations")
In a nematic network the microtubule orientation is random and the microtubules point in all directions (above left). Whereas in a polar state all the microtubules are pointing in the same direction, either in an aligned array (above right) or in a radial array called an [aster](http://en.wikipedia.org/wiki/Aster_%28cell_biology%29) (above middle).

In the cell, molecular motors can organize microtubules into both polar and nematic networks. However, the molecular mechanisms that drive the formation of these different network types are not fully understood. 

Rather than considering a large network of motors and microtubules, we will start here by looking at the interaction between just two microtubules and some motors. We will see how this pair interaction is determined by certain properties of the microtubules and motors. In particular we will look at how the *microtubule growth speed* be and the *motor speed* affect the pair interaction.

We will then discuss how the collective effect of local interactions between microtubule pairs in a motor-microtubule network determine the large-scale organization of the network. We will see how polar and nematic network states can be understood as arising from two distinct types of microtubule pair interactions.

This tutorial is based on [this article](http://dummy):

>**Determinants of polar versus nematic organization in networks of dynamic microtubules and mitotic motors.** —
>Roostalu J, Rickman J, Thomas C, Nedelec F and Surrey T. — 
>*Cell 2018*


### Preamble

We assume here that you have already followed [Tutorial 1](tuto_introduction.md), and that you are now familiar with the general syntax of Cytosim's configuration files and you can run a live simulation from the command-line.

# Step 1 - Nucleation of dynamic microtubules
The following simulations should be run in 3D.  

### Basic set-up
You should first set up a basic configuration file. Since we are interested in the interactions between only two microtubules our 3D space can be small. Start by defining a 3D `rectangle` space and a `fiber` class called `microtubule`. Use a fresh empty directory and a new configuration file:

    set simul system
    {
        time_step = 0.01
        viscosity = 0.2
        steric = 1, 50
        display = ( style=2; line_width=3; point_size=2; )
    }

    set space cell
    {
        shape = rectangle
        display  = (visible = 0;)
    }

    new cell
    {
        length = 4, 0.4, 0.1
    }

    set fiber microtubule
    {
        rigidity = 30
        segmentation = 1
        confine = inside, 100
        steric = 1, 0.05
    }

    new 2 microtubule
    {
        length = 2.5
    }
    
    run 10000 simul *
    {
        nb_frames = 1000
    }

Check that this works as expected and verify that the values of the parameters are reasonable.

### Defining dynamic microtubules
Like microtubules *in vivo*, we want these microtubules to be dynamic. Set the activity of the microtubule to `classic` and define the parameters for two-step dynamic instability by adding these 8 lines of code.


    set fiber microtubule
    {
      ...
    activity = classic 
        growing_speed   = 0.05
        shrinking_speed  = -0.5
        catastrophe_rate = 0.03
        rescue_rate      = 0
        growing_force    = 1.67
        delete_stub      = 1
        min_length       = 0.02
      ...
    }
    

The growing speed here is in um/s. One needs to specify the initial state of the microtubule, by adding an additional line in the 'new' command where they are created, `end_state = grow`:

    new 2 microtubule
    {
        length = 1
        plus_end = 1
    }

We have also changed the initial length of the microtubules. Now the microtubules will grow to an average length of 2.5 um before undergoing a catastrophe. Rescues do not occur and once the microtubule is in its shrinking state and reaches the `min_length`, `delete_stub = 1` means that it will be deleted.

### Defining the nucleators

In the simulation above the microtubules will disappear once they have undergone a catastrophe. However, we want a simulation in which a fixed number of microtubules (in this case 2) can be maintained in a *steady-state*, mimicking the situation in the cell where there is constant turnover of microtubules. To do this we will create microtubule `nucleators` using the `hand` and `single` class.

    set hand nucleator
    {
        unbinding = 0, 3
        activity = nucleate
        nucleate = 100, microtubule, ( length = 1; plus_end = 1; )
        display = { visible = 0;}
    }

    set single creator
    {
        hand = nucleator
    }

The `nucleator` is the single hand of the `creator`. It nucleates microtubules at a rate of 100 /s (`nucleate = 100, microtubule`) and does not let them go (`unbinding = 0`). As before the microtubules are initialised with a length of 1 um in the growing state. Add the code above and replace the block

    new 2 microtubule {...}
    
with the code

    new 2 creator
    
and check that this works as expected. 

# Step 2 - Pair-wise interactions between motors and microtubules

### Defining the motors

We now need to define the motors that will interact with the microtubules. We will define the properties for a fast, processive plus end directed kinesin. For this we need a `couple` with two `hands` each of which can `move` along a microtubule.

    set hand kinesin
    {
        binding = 5, 0.1
        unbinding = 0.1, 5
        
        hold_growing_end = 1
        unbinding_rate_end = 0.1
    
        activity = move
        unloaded_speed = 0.05
        stall_force = 5
    }
    
    set couple motor
    {
        hand1 = kinesin
        hand2 = kinesin
        activity = bridge
    
        length = 0.08
        stiffness = 100
        fast_diffusion = 1
        diffusion = 10
    }
    
The `unloaded_speed` of these motors, 50 nm/s, is the same as the microtubule growth speed that we defined earlier. 

We want these motors to be able to dwell at the microtubule plus-end, rather than walking off the end when they reach it. This is done in the two lines of code:  

        hold_growing_end = 1
        unbinding_rate_end = 0.1

`hold_growing_end = 1` means that motors can hang on to the plus end and the parameter `unbinding_rate_end` allows for a detachment rate from the end which is different to the detachment rate from the side of the microtubule (although for these simulations both detachment rates are set to be equal). This *end-dependent unbinding rate* is a Cytosim feature that has to be explicitly enabled in the code. In `src/sim/hand_prop.h` you can find a line of code that says `#define NEW_END_DEPENDENT_DETACHMENT 1`, make sure this is set to 1.

Add some motors,

    new 100 motor

and see what happens!
    
    
### Setting up the pair-wise interaction

Hopefully you have seen that sometimes the pair of microtubules interact and become crosslinked by motors and sometimes they do not. Since motors can only crosslink microtubules that are a certain distance apart (in this case the second value in `binding = 5, 0.1` sets the binding distance to be 0.1 um) this will depend on where the microtubules are nucleated, their orientation, and their subsequent diffusion. Currently the position of the nucleators is random. We can greatly increase the chances of an interaction between our microtubule pair by positioning both nucleators at the centre of the simulation space. Do this by adding a line into the `new 2 creator` block,

    new 2 creator
    {
        position = 0 0 0
    }

Check that this works.

# Step 3 - Investigating the effect of motor speed and growth speed on the pair-wise interaction

### Types of motor crosslink
Now our simulation is set up we can start thinking about the different ways in which a pair of microtubules and motors can interact. It is useful to define the different geometries in which a motor can crosslink two microtubules. Below are five different types of motor crosslink,

![alt text](images/crosslinks.png "Five different types of motor crosslink")

defined by the connection made between the two microtubules:  

 * *T links* connects one microtubule's end to the others side
 * *V links* connects two microtubule ends 
 * *Hap links* connect microtubule sides in an anti-parallel fashion
 * *Hp links* connect microtubule sides in an parallel fashion 
 * *X links* connect microtubule sides at an internal angle between 60 and 120 degrees.

For the next steps it will be useful to visualize in `play live` the different types of motor crosslinks. There is a feature in Cytosim that will display crosslinks color-coded as in the image above, it needs to be explicitly enabled. In `src/play/display2.h` find a line of code that says `#define NEW_COLOR_HAND_BY_LINK_TYPE 1`, make sure this is set to 1. **NB**: this will work only when `style = 2` is set as a `simul` `display` parameter. 

The visualization of the different crosslink types is even better if only *bridging* motors are displayed i.e. the motors that form crosslinks and not motors that are attached at one end. To do this toggle through the `couple` display modes by pressing the **8** key when the simulation is running (press three times to get to the *bridge only* mode).

It is also helpful to visualize the orientation of the microtubules. To do this you can display microtubule plus-ends as arrows by pressing SHIFT + ! in `live` mode.

### 3.1 What happens when the motor speed is comparable to the growth speed?
In the current simulation configuration the motor speed and the microtubule growth speed are set to the same value.   
Run the simulation live a few times and see which types of crosslinks form.  

You can also try storing the simulation with `sim` and reporting on the results. There is a special `report` function to read out numbers of the different types of crosslinks:

    report couple:type:motor
    
You should see that depending on the initial orientation of the microtubule pair *Hp links* (blue) or *Hap links* (magenta) form between the microtubules and dominate the interaction. There are very few *T links* and  *V links* in these simulations. This is because the microtubules are growing as fast the motors are walking and so motors cannot efficiently reach microtubule ends. Instead the motors predominantly walk along the microtubules sides. 


![alt text](images/H.png "Link formation when microtubule growth speed is comparable to motor speed.")

As shown in the cartoon above the *Hp links* connecting parallel microtubules will process along the microtubules' sides as they grow, bundling them together. The *Hap links* connecting anti-parallel microtubules will cause relative sliding as both motors walk towards their respective plus-ends in opposite directions. Since the initial orientation of the microtubule pair is random, there is equal likelihood of finding a simulation dominated by *Hp links* or *Hap links*, neither crosslink type is preferred although they perform different roles.

### 3.2 What happens when the motor speed is faster than the growth speed?
What happens now if we change the motor speed such that the motors walk much faster than the microtubules grow? We can do this by setting;

        unloaded_speed = 0.3

in the `set hand kinesin {...}` block. Now the motors should be able to efficiently reach the microtubule end. 

**Which link type will dominate now? Test this out!**

Hopefully you have seen that *X links*, *Hp links* and *Hap links* that form between the microtubules sides now become *T links* and *V links*, which cluster microtubule plus-ends together like in the cartoon below.  
![alt text](images/XTV.png "Link formation when motor speed is faster than microtubule growth speed.")

Again you can check that the numbers confirm this by storing a simulation and reporting. 

Experiment further by changing the speeds of both the microtubule growth and the motor speed. 

**Can you find a rule, based on the motor speed and the microtubule growth speed, that can predict which type of crosslink will dominate the pairwise interaction?**

#Step 4 - Scaling up to a large motor-microtubule network

We have seen how depending on the relative speed of the motor to the microtubule growth different crosslink types will dominate the pair-wise interaction in two different speed regimes:

* When the microtubule growth speed and motor speed are comparable, motors cannot efficiently reach microtubule plus ends. *Hp* and *Hap links* dominate the interaction, resulting in microtubule bundling and relative sliding (by *Hap links)*.
* When the motor speed is faster than the microtubule growth speed, motors can efficiently reach microtubule plus ends. *V* and *T links* dominate the interaction resulting in the clustering together of microtubule plus-ends.

What happens if we scale this two-microtubule system up to thousands of microtubules? 
![alt text](images/aster-vs-nematic.png "Types of crosslinks in nematic and aster network states.")

In the cartoon above we can see that in the nematic network state, where microtubules are aligned and randomly oriented, *Hp* and *Hap links* are dominating in equal numbers. While the polar aster state, where plus-ends are clustered together and microtubules point radially inwards, is maintained by *T* and *V links*. 

**Which speed regime will result in which network state?**

We can scale up the simulations and see! Because these simulations will have thousands of components they will take a lot longer to run (from several hours to a day). Try this yourself by downloading the simulation configuration files for Figure 4 of [this article](http://dummy):

>**Determinants of polar versus nematic organization in networks of dynamic microtubules and mitotic motors.**  
>Roostalu J, Rickman J, Thomas C, Nedelec F and Surrey T,
>*Cell 2018* 

You can download them [here](http://doi:mendeleyXXXX).

### The end!

Congratulations, you have completed the tutorial.

