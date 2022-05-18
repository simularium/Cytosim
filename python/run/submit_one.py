#!/usr/bin/env python
#
# A script to submit analysis jobs to the SLURM queuing system
#
# Derived from submit_slurm.py
# F. Nedelec, 18.11.2018

"""
    Submit a job to the SLURM system to be called in multiple directories
    
Syntax:
    
    submit_one.py ARG [mem=????] [queue=????] [hours=INT] [days=INT] dir1 [dir2] [dir3] [...]
    
    Unless specified otherwise, the queue is 'medium_priority'.
    The amount of requested memory (default=2G) should be specified in MB:
       mem=1024 (for 1 GB)
       mem=512  (for 512 MB)
       ...
    
Example:
    
    submit_one.py 'report platelet > platelet.txt' run????
    Submit one job to run command in the directories provided
    
F. Nedelec, Last updated 18.11.2018
"""


import sys, os, subprocess, tempfile

# default parameters for submission:
subcmd  = 'sbatch'
queue   = 'htc'
runtime = '1:00:00'   # 1 hour
memory  = '4096'      # in MB
ncpu    = 1           # nb of threads per job

# parameters of the program:
out     = sys.stderr

#-------------------------------------------------------------------------------

def execute(cmd):
    """execute given command with subprocess.call()"""
    try:
        val = subprocess.call(cmd)
        if val:
            out.write("ERROR: command failed with value %i\n" % val)
            print(cmd)
    except OSError:
        out.write("ERROR: command failed:\n")
        print('> '+' '.join(cmd))


def write_script(fid, cmd):
    """write that will run the job"""
    os.write(fid, '#!/bin/bash\n')
    os.write(fid, 'module -q purge\n')
    os.write(fid, 'module -q load LAPACK OpenBLAS\n')
    os.write(fid, 'module -q load foss\n')
    for c in cmd:
        os.write(fid, c+'\n')
    os.close(fid)


def sub(file):
    """return command that will submit one job"""
    # specify memory, shell, minimum number of cores and queue
    cmd  = [subcmd, '--nodes=1', '--ntasks=1']
    # specify number of threads if executable is threaded:
    if ncpu > 1:
        cmd += ['--cpus-per-task=%i' % ncpu]
    cmd += ['--partition='+queue]
    cmd += ['--time='+runtime] 
    cmd += ['--mem='+memory]
    # define signals sent if time is exceeded:
    cmd += ['--signal=15@120']
    cmd += ['--signal=2@60']
    # request special hardware:
    cmd += ['--constraint=avx2']
    # redirect stderr and sdtout to files:
    cmd += ['--output='+file+'.out']
    cmd += ['--error='+file+'.err']
    # call script:
    cmd += [file]
    #cmd += ['rm '+file]
    return cmd

#-------------------------------------------------------------------------------

def main(args):
    """submit jobs, depending on the arguments provided"""
    global subcmd, memory, runtime, queue, ncpu
    
    #find subcmd command:
    proc = subprocess.Popen(['which', subcmd], stdout=subprocess.PIPE)
    if proc.wait():
        out.write("Error: submit command `"+subcmd+"' not found!\n")
    else:
        subcmd = proc.stdout.readline().strip()

    # first argument is used for go_sim.py:
    cmd = args.pop(0)

    # create job script file:
    if not os.path.isdir('log'):
        os.mkdir('log')
    fid, file = tempfile.mkstemp('', '', 'log', True)

    job = []
    cwd   = os.getcwd()
    for arg in args:
        if os.path.isdir(arg) and os.access(arg, os.X_OK):
            job += ['cd '+os.path.abspath(arg)+' && '+cmd+';']
        elif arg.startswith('mem='):
            memory = arg[4:]
        elif arg.startswith('memory='):
            memory = arg[7:]
        elif arg.startswith('cpu='):
            ncpu = arg[4:]
        elif arg.startswith('ncpu='):
            ncpu = arg[5:]
        elif arg.startswith('days='):
            runtime = arg[5:]+'-00:00:00'
        elif arg.startswith('hours='):
            runtime = arg[6:]+':00:00'
        elif arg.startswith('minutes='):
            runtime = arg[8:]+':00'
        elif arg.startswith('time='):
            runtime = arg[5:]
        elif arg.startswith('queue='):
            queue = arg[6:]
        else:
            out.write("Error: I do not understand argument `%s'\n" % arg)
            sys.exit()

    if memory < 64:
        out.write("Error: requested memory (%s MB) seems too low\n" % memory)
        sys.exit()

    if ncpu < 1:
        out.write("Error: number of cpu/job must be >= 1\n")
        sys.exit()

    if job:
        write_script(fid, job)
        os.chmod(file, 0700)
        execute(sub(file))
    else:
        out.write("Error: you need to specify at least one directory\n" % arg)


#-------------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])

