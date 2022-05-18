
# Dimensionality

Cytosim can perform simulations in 1D, 2D or 3D, but the dimensionality is selected at compilation time!

This means that one executable will only be able to perform simulations for one dimensionality.

### Compilation

The dimension of the simulation is selected in `src/math/dim.h`

	#define DIM 2

Save the file and always recompile everything after changing `DIM`:

	make clean
	make

This will place the executables in `bin/`

You can check the dimensionality of the executable, with:

	bin/sim info
	bin/play info

Shortcuts are built into the makefile, and you can do:

	make bin2/sim
	make bin2/play

and similarly

	make bin3/sim
	make bin3/play

and also:

	make bin2
	make bin3

or

	make alldim

### Recommended practice

Compile the executables that you need and rename them accordingly:

	cp bin2/play ~/cytobin/play2
	cp bin3/play ~/cytobin/play3

Create shortcuts or alias if you use these often.