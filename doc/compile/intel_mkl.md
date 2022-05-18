
# Linking with Intel MKL

(feature implemented on 10/04/2018)

[Intel's 'Math Kernel Library'](https://en.wikipedia.org/wiki/Math_Kernel_Library) provides a version of BLAS and LAPACK that can be used with Cytosim.

# Compilation

Use revision after 6134 (most recent revision preferred)

The code is not changed. In ‘makefile.inc’ set 
    
    HAS_MKL := 2

This should be sufficient. In case of problem, check that the options of the makefile define `sequential static linking`.

Linking MKL requires multiple pass, due to cross-dependency in the library.
You must specify a 'group' of libraries:

    MKLLIB := -Wl,--start-group $(MKLDIR)/libmkl_intel_lp64.a $(MKLDIR)/libmkl_sequential.a $(MKLDIR)/libmkl_core.a -Wl,--end-group

Make sure there is no other MKLLIB defined in the makefile that would overwrite this line.

To compile on the SLURM cluster, load the module containing the Intel MKL:

    module load imkl

Use also the latest compiler:
 
    module load foss

The rest is as usual.

For advice on linking, check [Intel's Link Line Advisor](https://software.intel.com/en-us/articles/intel-mkl-link-line-advisor).

Note that `MKLROOT` should be defined by the OS, when you load module `imkl`

# SLURM submission

On the EMBL SLURM cluster, the `imkl` module should not be needed if we used static linking as specifed with `HAS_MKL := 2`.


# Verification

Check the library needed by cytosim with `ldd`:

    ldd bin/sim


FJN 27/04/2018
