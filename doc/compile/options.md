# Compiler and Optimizations
 
The parameters of the compilation are set by editing the file `makefile.inc`, located in the root of the distribution. By manually editing this file, you can change:

The compiler:

	COMPILER := gcc
 
The level of optimization:

	MODE := F
 
Use the `D=debug` mode to test new code, and the 'F=fast' mode to run extensive calculations. For optimal performance, you should also disable assertions (see below).

# Assertions

Assertions (a safety mechanism used for debugging) are turned on/off in `src/base/assert_macro.h`
To make the executable faster, you can disable assertions by defining NDEBUG:

	#define NDEBUG


# Floating-point Precision
 
The type of floating points numbers (float or double) used throughout Cytosim is set in `src/math/real.h`
 
	#define REAL_IS_DOUBLE 1
 
Using double precision is strongly advised. The code was not optimized for single precision, and cytosim is thus not faster, but some calculations may fail because of the reduced precision.

# PNG image support
 
You can install the PNG library (libpng) on your mac with [Homebrew](https://brew.sh):

	brew install libpng
 
You can then enable PNG support by editing the `makefile.inc`:

	HAS_PNG := 2

# Advanced features

* [Math Kernel Library](intel_mkl.md)
* [SIMD optimizations](vectorization.md)
* [Parallel execution](multithreading.md)

