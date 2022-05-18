# Building an exhibit with live Cytosim simulations

Using the ability of `play` to [be controlled externally](controller.md), it is possible to mount an exhibit aimed to the larger public, where the simulation could be easily controlled with a MIDI controller.

This event took place during the "Festival of Plants" in Cambridge on 18.05.2019, with Aram Gurzadyan and Argyris Zardilis helping.
We used a 24' monitor standing on a table, two chairs and power outlet.

<img src="data/Cambridge-2019-05a.jpg"/>
<img src="data/Cambridge-2019-05b.jpg"/> 

We outline here the steps involved in building the installation.

# Equipement

- Mac Pro (Late 2013) "Six Core" 3.5 Ghz running Mac OSX 10.13.6
- Keyboard, mouse
- USB MIDI controller: The [Novation Launchpad Mk2](https://novationmusic.com/launch/launchpad).

This Mac Pro has a HDMI 1.4 UltraHD port. A technician adjusted the parameters of the display.
The computer was connected only to one display, and thus Cytosim could be started directly in full screen.
The installation did not use sound.

# Preparation

A single scenario was prepared and optimized for speed and attractivity:
Cytosim was compiled in 2D and 3D (see below) and for each scenario a copy of the corresponding executable was made (*play0*, *play1*, etc.).

A folder 'live' was created and all the files needed to run the simulations were copied within:

- `config0.cym` ... `config8.cym`
- `play0` ... `play8`

We also made shortcuts to start the simulations by a double-click:

- `0-live.command` ... `8-live.command`

These were not used during the show, but were useful during development.

### Compilation details

Check that `SimThread::readInput(int n)` is called in Cytosim
(this might be disabled)
Cytosim in live mode will print the commands it gets on the top right corner of the window.

To maximize the running speed:

	MODE := F
	#define NDEBUG
	#define GRID_HAS_PERIODIC 0

Enable multithreading, according to the machine:

	#define NUM_THREAD 6

And finally:

	#define DIM 2
	#define FIBER_HAS_LATTICE 0
	#define ADD_PROJECTION_DIFF 1

(if necessary search the project's code to find these lines)

# MIDI controler

We made a specific program `cytobuilder` to read the MIDI messages and pipe commands to Cytosim. It can also restart the simulation in another model, when some MIDI buttons are pressed.

`Cytobuilder` uses [RtMIDI by Gary Scavone](https://www.music.mcgill.ca/~gary/rtmidi/) for cross-platform access to the MIDI interface. 
You will need to adjust [`cytobuilder.cpp`](../../src/misc/installation) and recompile multiple times during preparation.

On Mac OSX you can make it as:

	g++ -Wall -o cytobuilder cytobuilder.cpp -D__MACOSX_CORE__ RtMidi.cpp -framework CoreMidi -framework CoreAudio -framework CoreFoundation

For Linux use:

	g++ -Wall -D__LINUX_ALSA__ -o cytobuilder cytobuilder.cpp RtMidi.cpp -lasound -lpthread 

This process is automated from the command line, you should be able to compile and copy `cytobuilder` to the `live` directory with:

	cd cytobuilder
	make install

### Enabling the MIDI port

Even if the MIDI device is plugged in your USB computer, it may not immediately be enabled by the system.
The procedure depends on the OS.

On Mac OSX, open an Application called "Audio MIDI Setup" (`Menu > Windows > Show MIDI Studio`)  
The MIDI device should show up there, allowing you to customize (double click)
and **activate the output port**.
You may need to create a new configuration.

### Identifying the MIDI port number

The MIDI device sends its messages to a port, which is identified by an integer.
To get a list of all the available ports, enter:

	cytobuilder scan

The output is for example:
	
	Invalid port specified (only 2 ports)
	  port 0 is [ IAC Driver Bus 1 ]
	  port 1 is [ IAC Driver Bus 1 ]

### Identifying MIDI messages sent by the controller

When `cytobuilder` is running and listening to the correct port, it will print the MIDI messages received to the terminal window. While preparing for the installation, activate the different keys of the MIDI controller one-by-one and note the 'Slider' number that is associated to them.

	cd live
	cytobuilder 0

With this information, edit `cytobuilder.cpp` to associate a command to these keys.
For this, adapt the function `makeCommand()` to respond appropriately to the MIDI keys.
A MIDI message consist in a slider ID number, and a value in (0 ... 127). The cytosim command is generated inside a `switch`:

    switch ( slider )
    {
        case 1:    case 95:
            return snprintf(str, len, "change system { viscosity=%f }\n", linear(value, 0.1, 1.0));
        case 2:    case 96:
            return snprintf(str, len, "change filament { rigidity=%f }\n", quadratic(value, 0.1, 20));
	...
	}
	
You need to adjust the `case` and the associated message.
When you are done, recompile `cytobuilder` and copy it to directory `live`.

# Starting the show

Once you have identified the port number (`MIDI_PORT_NUMBER`), you can start the show with:

	cd live
	cytobuilder MIDI_PORT_NUMBER
	
For example:
	
	cytobuilder 0 

This should then start `cytosim` live with model 0. 
When moving the sliders, a message should appear on the top right corner of the window.

# Contributors

- Gaelle Letort
- Nicolas Borghi


Francois Nedelec, 18.06.2019.

