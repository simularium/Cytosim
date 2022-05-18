# Cytosim's Documentation

*This is work in progress*

*  [**Overview**](main/overview.md)
*  [The configuration file](sim/config.md)
*  [Units in Cytosim `(s, um, pN)`](sim/units.md)
*  [Set of standard config files](main/examples.md)
*  [Introduction to the command line](main/starter.md)
*  [**Tutorials**](tutorials/index.md)
*  [Simulation engine](sim/index.md)
*  [Running simulations on your computer](main/runs.md)
*  [Running cytosim on a cluster](main/run_slurm.md)
*  [Getting numbers out of Cytosim with `report`](sim/report.md)
*  [Frequently asked questions](main/faq.md)
*  [The different executables](main/executables.md)
*  [Graphical rendering](sim/graphics.md)
*  [Making movies](main/movies.md)
*  [Prior work](examples/index.md)
*  [Syntax highlighting for config file](https://github.com/manulera/cytosimGrammar)
*  [File types](main/file_types.md)

# Installation

Cytosim is distributed as source code and [must be compiled](compile/index.md) before use. On Mac OS X and Linux this should be straightforward if you are familiar with compilation in general. On Windows, we suggest to [compile within Cygwin](compile/cygwin.md).

To compile, enter these commands in a terminal window:

	git clone https://gitlab.com/f.nedelec/cytosim
	cd cytosim
	make

Once cytosim is running on your machine, check the tutorials, the page on [running simulations](main/runs.md), and the examples contained in the folder `cym`. Inspect in particular the short configuration files (e.g. fiber.cym, self.cym). 

#### Troubleshooting

For more information, please check the [compilation guideline](compile/index.md).  

cytosimATcytosimDOTorg

# Advanced matter

*  [Code documentation](code/index.md)
*  [Doxygen documentation](doc/code/doxygen/index.html)

# Credits

[Credits and history](main/credits.md)

