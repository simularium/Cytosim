
# SIMD Vectorization

Many routines in cytosim are implemented using vectorized primitives to achieve significant speedup on processors that support this technology. It uses the SIMD instruction set, to perform operations on multiple scalar values in parallel.
For example 'AVX' allow to operate on vectors of 4 double precision floats.

There has been different generations of SIMD, of increased vector size:

    128 bits:  SSE3           ~2004
    256 bits:  AVX            ~2009
    256 bits:  AVX2 and FMA   ~2013
    512 bits:  AVX-512        ~2018

Typically, one can expect a 40% speed up of AVX compared to SSE3, and 
another speedup with the 'Fused Add-multiply' instructions of `FMA`.
The functions included in `AVX2` other than FMA are not used in cytosim.
To take advantage of this, you need to adjust the `makefile.inc`.

# Compilation

Edit ‘makefile.inc’ to set the correct option for your compiler.
For example, to use `AVX` and `FMA` with `gcc`:

    -mavx -mfma

And for SSE3:

    -msse3

With Intel compiler (icpc):

    # Intel advanced instruction sets:
    # '-xHost' to optimize for host machine
    # '-xAVX' for AVX
    # '-march=core-avx2' for Intel core i7 (ca. 2015)

# SLURM submission

If you compile with 'avx2' support on the EMBL SLURM cluster, you need to request nodes with `avx2` features. This is done with the SLURM option:

    --constraint=avx2

In `submit_slurm.py`, make sure this option is set in `sub()`

    def sub(exe):
        """return command that will submit one job"""
        # specify memory, shell, minimum number of cores and queue
        cmd  = [subcmd, '--nodes=1', '--ntasks=1']
        ...
        # request special hardware:
        cmd += ['--constraint=avx2']
  	   	...
  	   	 
and also for any job arrays in `array()`:

    def array(jobcnt):
        """return command that will submit a job-array"""
        # define parameters directly in the script:
        cmd  = ['#SBATCH --nodes=1']
        cmd += ['#SBATCH --ntasks=1']
        ...
        # request special hardware:
        cmd += ['#SBATCH --constraint=avx2']
        ...

# Troubleshooting

The job should run. Otherwise, you will get 'Illegal instruction' if the instructions are not supported by the CPU on which the program is running. In this case, the program will terminate with UNIX signal `SIGILL` of value 4.


FJN 27/04/2018
