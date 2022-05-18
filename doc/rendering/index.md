# Cytosim to Blender export

For its display, Cytosim uses methods that are fast but not physically exact.
It uses [OpenGL](https://en.wikipedia.org/wiki/OpenGL), a library used extensively in computer games, that uses drawing techniques.
More realistic (and nicer looking) images can be obtained using [Ray tracing](https://en.wikipedia.org/wiki/Ray_tracing_(graphics)) a technique in which the trajectory of light rays through the scene are estimated.

Funded by the DivIDE consortium, we developped an output module for Cytosim, which creates files that can be read into [Blender](https://en.wikipedia.org/wiki/Blender_(software)), an OpenSource computer graphics software with Ray Tracing capacity.

This document describes the main steps needed to render a Cytosim simulation using Blender.

# Protocol

For rendering a simulation you will need to:

1. produce a simulation using the offline mode (you need a `objects.cmo` file)
2. export the coordinates of the objects into specific files, using `cymart`
3. import these files into `Blender`
4. use Blender to render and export image files

### Step 1

As usual, create a config file and run `sim` in a separate directory:

	mkdir run
	cd run
	edit config.cym
	./sim

This should create `object.cmo` and `properties.cmo` in the `run` directory.

### Step 2

Use cytosim’s tool `cymart` to makes text files from the `objects.cmo` files.
You may need to compile this tool (`cd cytosim; make cymart`).

The text files created by `cymart` contain the coordinates of many spheres to be imported in Blender, that will represent the filaments. Their positions are extrapolated from the coordinates of the filaments calculated by `Cytosim`.

Syntax:

	cymart fiber_class [style=filament|actin|microtubule] [INPUTFILE] [binary=1]

`fiber_class` is the name of the property of the fiber or `all` to export all fibers
Select `style=filament` or `style=actin` or `style=microtubule`.
Each frame of the trajectory file is sent to a separate file.

For example within your folder `run`, inkoke:

    cymart actin style=actin

to create two files for each frame in `objects.cmo`:

	actin0000.txt      actin0001.txt     actin0002.txt   ...
	link0000.txt       link0001.txt      link0002.txt    ...

Copy these files into a subfolder ‘cymart’ of your blender directory.
	

### Step 3

[Download](https://www.blender.org/download/) and install blender on your computer.

Open the `Blender` application, and import a startup file.

The startup files `actin.blend` and `cytosim.blend` are identical.
You should keep one as a backup, and double-click on the other.
This file defines objects used for the rendering.

Load the cytosim-specific python scripts into the Blender script panel:

There are two scripts to import Cytosim's objects info Blender:

- import-actin.py :   add monomers from files `cymart/actin####.txt`
- import-links.py :   add links from files `cymart/link####.txt`

And one to delete these objects:

- delete-actin.py :   delete stuff added by the two scripts above

To import all the files made in the previous step, run `import-actin.py`.

### Step 4

Describing how to use Blender is beyond this document, but many courses exist online.
You can start for example at [Blender guru](https://www.blenderguru.com).

# Troubleshooting

### Python error messages

On MacOS, Python error messages are not visible in the Blender GUI. However, they are sent to the terminal, if Blender is started from the command line. Hence, open a Terminal window, and enter something like this:

    /Applications/blender-2.79/blender.app/Contents/MacOS/blender actin.blend

You will then be able to see the error messages.

# More

There is another version of import-actin.py:

    import-actin-ico.py    Version that uses a icosahedral mesh

# Contributors

Initial development was made in 2017.
FJN wrote the export module for cytosim.
Richard van der Oost wrote the first version of the Blender import scripts.
Julio Belmonte helped developing and testing this module.
Serge Dmitrieff and Markus Mund used and further tested the module.
