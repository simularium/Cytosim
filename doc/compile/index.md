# How to compile Cytosim

The core of Cytosim is written in C++11 and it is necessary to recompile the programs after each modification of the source code. Some of the accessory tools use [Python](https://www.python.org).
 
Compilation requires a C++ compiler: e.g. [gcc](http://gcc.gnu.org/), [Clang](http://clang.llvm.org) or the [Intel compiler](http://en.wikipedia.org/wiki/Intel_C%2B%2B_Compiler), together with a few libraries.
Compilation is started from a terminal, with a program called [make](http://www.gnu.org/software/make/).

### Dimensionality

The dimensionality is backed into the executable during compilation. 
To change it, [follow these instructions](dimensionality.md).

### Mathematical libraries
 
Running cytosim with `sim` or `play` requires these mathematical libraries:

* [BLAS](http://netlib.org/blas)
* [LAPACK](http://netlib.org/lapack)

These libraries offer standard interface to linear algebra functions used in Cytosim.
There is a [public reference implementations](http://netlib.org). 
Compiling the reference code is possible with a [FORTRAN](http://en.wikipedia.org/wiki/Fortran) compiler.
Precompiled libraries are available for most platforms, within:

- [Intel Math Kernel Library](http://software.intel.com/en-us/articles/intel-mkl/)
- [Apple's vecLib](http://developer.apple.com/hardwaredrivers/ve/vector_libraries.html)
- [OpenBLAS](https://www.openblas.net)
- Also available for many Linux distributions.

Apple's veclib is preinstalled on Mac OSX, and [Intel's MKL](https://en.wikipedia.org/wiki/Math_Kernel_Library) is available free of charge. MKL has better performance on Intel processors.

### Graphical libraries
 
Cytosim's ***play*** relies on [OpenGL](http://www.opengl.org/) for display 
and uses [POSIX threads](http://en.wikipedia.org/wiki/POSIX_Threads) for multithreading.
 
Interactivity is provided by [GLUT](http://www.opengl.org/resources/libraries/glut/),
which can be replaced by [freeGLUT](http://freeglut.sourceforge.net/).
 
GLUT and OpenGL are included in Mac OSX:

	-framework GLUT -framework OpenGL 

Cytosim program `play` will be able to export images only if it is linked with the PNG graphical libraries, but everything else works fine. See the [compile options](options.md).

# Getting Ready 

### Mac-OSX

On Mac OSX, install [Xcode](https://developer.apple.com/technologies/tools/), which is available on the Mac App Store. After installing Xcode, install the Xcode 'Command-Line Tools', which is an optional package providing 'make'. All necessary libraries are already included in MacOSX.

We provide the Xcode project file for cytosim, which is a convenient way to access the code.

Optionally, Cytosim can use the mouse wheel to zoom in and out, if you use FreeGLUT, instead of Apple's GLUT, or [Renaud Blanch's GLUT patch](http://iihm.imag.fr/blanch/howtos/MacOSXGLUTMouseWheel.html).


### Linux

Check the [dedicated pages](linux.md).

On Linux, you need to install the GNU compiler collection, BLAS and LAPACK. This is sufficient to compile `sim`.
Recent Linux distributions provide precompiled BLAS/LAPACK as optional installation (check your distribution).

To make `play` install the OpenGL developer libraries and FreeGLUT or OpenGLUT. 


### Windows

Native compilation on Windows is a Herculean task. We thus recommend installing Cygwin, which emulates a Linux-like environment. A fresh installation of Cygwin is recommended to ensure that all required reference libraries are installed.  
You will need a compiler, the X window system, BLAS/LAPACK and GLUT.  
Please, [refer to the dedicated page](cygwin.md).


# Compilation

After installing a compiler and [gnu's make ](http://www.gnu.org/software/make/)
you are ready to compile from a terminal, with the following commands in the root directory of cytosim:

	make sim
	make play

The command `make` without arguments will build `sim` and `play`.  
If this does not work, you may need to manually edit `makefile.in` to adjust to your platform.

It is also possible to use [cmake](https://cmake.org), which should require no adjustment:

	mkdir build
	cd build
	cmake ..
	make sim play report

You can then check the resulting executables, normally located in subdirectory `bin`:

	bin/sim info
	bin/sim
	bin/play live

# Optimizations

To speed up the calculation, follow these steps:

On line 10 or so of  `makefile.inc`, set `MODE` to `F`:

	MODE := F

Turn off assertions by defining `NDEBUG` in `src/base/assert.h`:

	#define NDEBUG

Recompile Cytosim from scratch:

	make clean
	make

# Troubleshooting `sim`

Compilation is specified in `makefile` and `makefile.inc`, and these files may need to be adjusted manually.
Check  `makefile.inc` first and verify the [compile options](options.md).

Make attempts to automatically detect the platform:

	#---------------- MACHINE = {mac, linux, cygwin, auto}
	MACHINE := auto

But you may manually set `MACHINE` to `mac`, `linux` or `cygwin` depending on your platform,
and check the parameters set lower in the `makefile.inc`, for this platform, for example:

	ifeq ($(MACHINE),linux)
		...
	endif

To pinpoint the problem, try to build objects that have fewer depencies first:

#### Check your compiler for C++11 support and compilation switches

	make test_cxx

#### Check your BLAS/LAPACK directly:

	make test_blas
	
If you get an error at this stage, check that the compiler is linking the correct libraries.
To find the libraries on your system, this command may help:

	find /usr/lib -name libblas.*

For example, if the result is:

	/usr/lib/libblas.so

You should adjust `makefile.inc` to specify the corresponding path:

	LIBDIR := /usr/lib

Ensure that no other line changes the value of `LIBDIR`. For example, this one is commented out with the `#`:

	#LIBDIR := /usr/lib/x86_64-linux-gnu

#### Check if it can read a configuration file:

	make test_glos

#### At this stage, you can attempt to compile `sim`:

	make sim

# Troubleshooting `play`

If you are having trouble compiling `play`, check its requirements independently:

#### Check for thread support:

	make test_thread
	
#### Check for OpenGL support:

	make test_opengl
	
#### Check for GLUT support:

	make test_glut

#### Check for GLAP (our own extension of GLUT)):

	make test_glapp

#### At this stage, you can compile `play`:

	make play

### Finally: contact us!

Please, write to `feedbackATcytosimDOTorg`.

Please, describe what fails and what you have tried.
Attach your 'makefile.inc' and tell us the platform on which you compiled.

