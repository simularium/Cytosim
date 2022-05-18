#!/usr/bin/env python
#
# battery_test.py
#
# Copyright F. Nedelec, March 19 2011 --- 22.05.2018

"""
battery_test.py:

    run a battery of .cym files to check cytosim

Example - live:

    battery_test.py bin/play live cym/*.cym

Example - runs:

    battery_test.py bin/sim cym/*.cym
    make_image.py 'play frame=100 window_size=512,512' *_cym

F. Nedelec, March-June 2011 - Feb 2013
"""

import shutil, sys, os, subprocess

#------------------------------------------------------------------------

def run_live(file, executable):
    """run live test"""
    print(file.center(100, '~'))
    cmd = executable + ['live', file]
    val = subprocess.call(cmd)
    if val != 0:
        print('returned %i' % val)

#------------------------------------------------------------------------

def run(file, executable):
    """run test in separate directory"""
    cdir = os.getcwd()
    name = os.path.split(file)[1]
    wdir = 'run_'+name.partition('.')[0];
    
    try:
        os.mkdir(wdir)
    except OSError:
        print('skipping  '+file)
        return
    
    shutil.copyfile(file, os.path.join(wdir, 'config.cym'))
    os.chdir(wdir)
    print('- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - '+name)
    
    val = subprocess.call(executable)
    if val != 0:
        print('returned %i' % val)
    
    os.chdir(cdir);

#------------------------------------------------------------------------

def main(args):
    " run cytosim for many files"
    files = []
    live = False;
    err = sys.stderr;

    executable = args[0].split()
    if os.access(executable[0], os.X_OK):
        executable[0] = os.path.abspath(executable[0])
    else:
        err.write("Error: you must specify an executable on the command line\n")
        sys.exit()
    
    for arg in args[1:]:
        if os.path.isfile(arg):
            files.append(os.path.abspath(arg))
        elif arg=='live' or arg=='live=1':
            live = True
        else:
            err.write("Ignored`"+arg+"' on the command line\n")
    
    if not files:
        print("You must specify config files!")
        sys.exit()

    if live:
        for f in files:
            run_live(f, executable)
    else:
        executable.append('-')
        for f in files:
            run(f, executable)


#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv)<2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])

