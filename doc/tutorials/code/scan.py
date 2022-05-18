#!/usr/bin/env python
#
# scan.py
#
# copyright  F. Nedelec and S. Dmitrieff 2007--2018

"""
    Execute specified command in given directories, sequentially using a given number of processes
 
Syntax:

    scan.py command directory1 [directory2] [directory3] [...] [jobs=N]
    
Example:
    
    scan.py 'play image' run* jobs=2
    
F. Nedelec, 02.2011, 09.2012, 03.2013, 01.2014, 06.2017
S. Dmitreff, 06.2017
"""

try:
    import sys, os, subprocess
except ImportError:
    sys.stderr.write("Error: could not load necessary python modules\n")
    sys.exit()

executable = 'pwd'
out = sys.stderr
njobs = 1

#------------------------------------------------------------------------

def execute(path):
    """
    run executable in specified directory
    """
    os.chdir(path)
    out.write('-  '*24+path+"\n")
    try:
        subprocess.call(executable, shell=True)
    except Exception as e:
        sys.stderr.write("Error: %s\n" % repr(e));


def execute_queue(queue):
    """
    run executable sequentially in directories specified in paths
    """
    while True:
        try:
            arg = queue.get(True, 1)
            execute(arg)
        except:
            break;


def main(args):
    """
        read command line arguments and process command
    """
    global executable
    try:
        executable = args[0]
    except:
        out.write("Error: you should specify a command to execute\n")
        return 1

    njobs = 1
    paths = []
    for arg in args[1:]:
        if os.path.isdir(arg):
            paths.append(os.path.abspath(arg))
        elif arg.startswith('nproc=') or arg.startswith('njobs='):
            njobs = int(arg[6:])
        elif arg.startswith('jobs='):
            njobs = int(arg[5:])
        else:
            out.write("  Warning: unexpected argument `%s'\n" % arg)
            sys.exit()

    if not paths:
        out.write("Error: you should specify at least one directory\n")
        return 2
    
    if njobs > len(paths):
        njobs = len(paths)
    
    #process in parallel with child threads:
    if njobs > 1:
        try:
            from multiprocessing import Process, Queue
            queue = Queue()
            for p in paths:
                queue.put(p)
            jobs = [Process(target=execute_queue, args=(queue,)) for n in range(njobs)]
            for job in jobs:
                job.start()
            for job in jobs:
                job.join()
            return 0
        except ImportError:
            out.write("Warning: multiprocessing unavailable\n")
    #process sequentially:
    for p in paths:
        execute(p)
    return 0

#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])
