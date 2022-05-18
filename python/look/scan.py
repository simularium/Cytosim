#!/usr/bin/env python
#
# scan.py executes a given command sequentially or in parallel, in specified directories
#
# Copyright  F. Nedelec and S. Dmitrieff; 2007--2022

"""
    Execute specified command in given directories, sequentially or in parallel,
    using independent threads.
 
Syntax:

    scan.py command [-][+] directory1 [directory2] [directory3] [...] [jobs=INTEGER]
    
    if '-' is specified, output is limited to what the command does
    if '+' is specified, the directory path is printed without decoration
    if 'jobs' is set, run in parallel using specified number of threads

Examples:
    
    scan.py 'play image' run*
    scan.py 'play image' run* jobs=2
    
    
F. Nedelec, 02.2011, 09.2012, 03.2013, 01.2014, 06.2017, 07.2021, 21.03.2022
S. Dmitreff, 06.2017
"""

try:
    import sys, os, subprocess
except ImportError:
    sys.stderr.write("Error: could not load necessary python modules\n")
    sys.exit()

out = sys.stdout
verbose = 2

#------------------------------------------------------------------------

def execute(tool, path):
    """
    run executable in specified directory
    """
    os.chdir(path)
    try:
        subprocess.call(tool, shell=True)
    except Exception as e:
        sys.stderr.write("Error: %s\n" % repr(e));


def worker(queue):
    """
    run executable taking argument from queue
    """
    while True:
        try:
            t, p = queue.get(True, 1)
        except:
            break;
        if verbose == 1:
            out.write(os.path.basename(p)+" ")
            out.flush()
        execute(t, p)
        if verbose == 2:
            out.write("done "+p+"\n")


def main(args):
    """
        read command line arguments and process command
    """
    global verbose
    
    if args[0] == '-':
        verbose = 0
        args.pop(0)
    elif args[0] == '+':
        verbose = 1
        args.pop(0)
    
    try:
        tool = args[0]
    except:
        out.write("Missing command: scan.py command [-][+] directory1 [directory2]...\n")
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
        elif arg == '-':
            verbose = 0
        elif arg == '+':
            verbose = 1
        else:
            out.write("  Warning: unexpected argument `%s'\n" % arg)
            sys.exit()

    if not paths:
        out.write("Missing directories: scan.py command [-][+] directory1 [directory2]...\n")
        out.write(" (scan.py would execute `%s`)\n"%tool)
        return 2
    
    njobs = min(njobs, len(paths))
    
    if njobs > 1:
        #process in parallel with child threads:
        try:
            from multiprocessing import Process, Queue
            queue = Queue()
            for p in paths:
                queue.put((tool, p))
            jobs = []
            for n in range(njobs):
                j = Process(target=worker, args=(queue,))
                jobs.append(j)
                j.start()
            # wait for completion of all jobs:
            for j in jobs:
                j.join()
            return 0
        except ImportError:
            out.write("Warning: multiprocessing module unavailable\n")
    #process sequentially:
    for p in paths:
        if verbose == 2:
            out.write('-  '*24+p+"\n")
        elif verbose == 1:
            out.write(os.path.basename(p)+" ")
            out.flush()
        execute(tool, p)
    return 0

#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 3 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])
