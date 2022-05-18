
# Multithreaded sim

(feature implemented on 22/04/2018)

The most critical functions of Meca were parallelized with OpenMP, to use a fixed number of threads in parallel. On a macbookpro, the speedup was approximately 3x using 4 threads.

# Compilation

Use revision after 6150 (most recent revision preferred)

In ‘meca.cc’ set 

    #define NUM_THREADS 4

The compiler needs a flag `-fopenmp` to understand the OpenMP code.
Edit `makefile.inc` and change the value of `CXXFLG`:

    CXXFLG := -fno-rtti -fno-inline -fopenmp

This should be sufficient. Note that only recent version of GCC support OpenMP. This works best with Intel MKL

# SLURM submission

On the EMBL SLURM cluster, one needs to request 4 threads.
This is done by adding an option to SLURM's `sbatch`:

  	 --cpus-per-task=4

If you are using the latest version of `submit_slurm.py`, you must specify `cpu=4`

	./submit.py sim_omp cpu=4 cym/*

Troubleshooting: in the code of `submit_slurm.py` look for:

    def array(jobcnt):
    	 ...
        # specify number of threads if executable is threaded:
        cmd += ['#SBATCH --cpus-per-task=4']
        ...

# Verification

Using `squeue` get the name of the node on which one of your jobs is running.
Log into this node (`ssh`) and use `top` to verify the job CPU level.
They should indicate a CPU usage significantly above 100%.
A single threaded program would at best achieve 100%. 
Depending on the conditions, you can expect between 300% and 400% (assuming NUM_THREADS is 4).
Some of the CPU cycles are wasted, and the actual speedup will be less.

If the CPU usage remains below 100%, something is probably wrong.


FJN 27/04/2018
