# How to Run Simulations with Cytosim

This is a brief introduction to using the command line interface to run one or many simulations on a personal computer.
Although the command line may be unfamiliar material to many, it is present on most computers and is very convenient.
If you are not familiar with the command line, [follow this first](starter.md).

# Live Run

A *live run* will perform a simulation and display its result on the fly, without saving anything on the disk.
To run a program, you may need to specify the full path of the corresponding executable.
If cytosim's executable `play` is present in the current directory, you can type:
 
	./play live

this should open a new window and start a live simulation using the file `config.cym`, which is the default.
However, if this file does not exist in the current working directory you will get an error.
You can specify any other configuration file, here in a subdirectory `cym`, if its extension is `.cym`:

	./play cym/self.cym

In the following, only the executable name will be specified. If you get an error `command not found`, please check our [Unix starter](starter.md), as every details of the command matter. 

# Normal Run
 
The canonical way to use cytosim is to call `sim` to calculate a simulation,
and then `play` to display the results once `sim` has finished:
	 
	sim
	play
 
By default, `sim` uses the configuration file `config.cym` in the current directory. 
You can specify another file, but it is recommended to keep a copy of the config file for your records.
If you do not want to wait until `sim` finishes, you may run `sim` in the background:
 
	sim&
 
You can then use `play` to display the partial results at any time, even if `sim` is still running. 
This usually works fine because `sim` only writes the files, while `play` only reads them. 
You could also open a second terminal window to run `play` while `sim` is still running,
or do this on different computers if you use a shared disc space.
Learn [how to convert the results into a movie](making_movies.md).


# Overnight Run
 
You should not start two `sim` in the same directory, because the output files
are always named in the same way, and the results will be losts. The good practice
is to run every simulation in a separate directory.
You may however use `play` or `play live` in a directory where `sim` is working.

You can let `sim` run in its own terminal window, and wait for it to complete.
However, if the calculation requires an hour to complete or more,
you can use <a href="http://en.wikipedia.org/wiki/Nohup">nohup</a> to release the
terminal, and prevent `sim` from being automatically terminated when you log out:

Here is an example:
	
	mkdir run0
	cp config.cym run0
	cd run0
	nohup ../sim&

To automate this, you can use the script `/python/run/start.py`:
it will automatically create the new directory, and start the simulation:
 
	start.py sim config.cym
 
You can call `start.py` many times, and the new directories will be named `run0000`, `run0001`, etc.
Other python scripts can be used to run many simulations in parallel.


# Parametric Scan

The process of running many simulations can be automated with the python scripts
located in `/python/run`. Here we illustrate how to scan one parameter by using `preconfig.py`.
The same technique makes it possible to scan many parameters.

First create a template file from your existing config file:

	cp config.cym config.cym.tpl
 
Then edit this template file to replace the parameter value(s) that you would like
to vary with some python code surrounded in double-brackets.
This code will specify how the parameter is varied, for example:
 
	set hand binder
	{
	 	binding_rate = [[ random.uniform(0,1) ]]
	}
 
Here `binding_rate` will be set randomly between 0 and 1 following a uniform distribution.
Any plain python code should work, including functions from the <a href="http://docs.python.org/library/random.html">Random Module</a>.
It is also possible to use multiple bracketed expressions in the same file to vary several parameters.
Please check the help provided by the script itself by running `preconfig.py help`.

 
You are now ready to use `preconfig.py` to generate a set of config files:

	preconfig.py 100 config.cym.tpl
 
This should create 100 files called `config????.cym` and
you can use `go_sim.py` to run simulations with all these files sequentially:
 
	go_sim.py config????.cym
 
You may also want to run these jobs in parallel, if your machine has multiple cores.
For example the UNIX command 'xargs' can be used to run 4 processes in parallel:

	ls config????.cym | xargs -P 4 -L 1 go_sim.py sim
 
The script `start.py` can be used to start long-running jobs.
Please check the help provided by the scripts (`go_sim.py help` and `start.py help`).


# Visual Inspection

You should now have many completed simulations, each in a different directory.
You can look at them, using a few python scripts:

Script          |   Typical usage                                   |
----------------|----------------------------------------------------
`make_page.py`  | create a HTML page to easily view all the images
`scan.py`       | run a command in multiple directories


For example, create an HTML directory with an image for each `run`:

	scan.py '~/bin/play window_size=1024,128 frame=100 movie' run*
	make_page.py run*
 
One needs to provide the full path for the executable to *scan.py*, and here we refer
to a copy of 'play' in the directory 'bin/' located in the Home directory.

Create HTML directory with images every 100 frame:

	scan.py '~/bin/play movie period=100 size=256 label={}' run*
	make_page.py run*
 
Open `page.html` in a browser to view the results.
 
 
# Analysis

You can also analyse the runs in a non-visual way using 'report'.
This tool will generate text files from the trajectory file of a completed simulation.
You may use `scan.py` to analyse a set of directories.
Alternatively, you can include some reporting directly into the config file,
with [the command "report"](../sim/commands.md).


The tools and python scripts should be able provide up-to-date help, for example:

	make_page.py help
	scan.py help

# Restarting a simulation

To restart a simulation from a frame stored in a trajectory file,
follow these steps:

1. Build the accessory program `frametool` if necessary:

		make frametool

2. Using `frametool`, extract the desired frame (in this example, 30).
This will create a file ‘objects.cmi’. The frame index start at 0. 

		frametool objects.cmo 30 > objects.cmi
Note that ‘frametool objects.cmo’ will tell you how many frames are contained
in the file.

3. Adjust the `config.cym` to import this frame.
Use the ‘import’ command to read the file created in step 1:
	
		import objects objects.cmi
		
		% simulate as usual:
		run 1000000 system
		{
			nb_frames = 100
		}

The ‘import’ command replaces the objects of the simulation, without changing their Properties. The config file should thus define all the Properties with ’set’ as usual, before the ’import’ command. From an existing configuration, one simply adds the 'import' and deletes all the 'new'.

However, any ’new’ placed after ‘import’ will add objects as usual. The simulation should be started in a fresh directory, as ’sim’ will erase the ‘object.cmo’ file.

One can merge two trajectory files later with 'cat' if necessary:

	cat objects1.cmo objects2.cmo > objects.cmo

# Conversion

With `sim`, it is possible to extract a frame from a binary trajectory file,
and export it in the text-based format. The text-based format is a plain ascii
file containing all the simulation state variables.


Simply execute a following config file:

	read properties.cmo
	import objects.cmo { frame=10; }
	export objects objects.txt { binary=0; }

