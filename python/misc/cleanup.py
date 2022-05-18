#!/usr/bin/env python
#
# cleanup.py
#
# copyright F. Nedelec, December 14th 2007

"""
    Scan recursively from current directory,
    and clean up temporary cytosim files
    
Syntax:
    
    cleanup.py [walk=1] [commit=1] directory-list
    
Example:

    cleanup.py data*

F. Nedelec, April 2013
"""

import sys, os, shutil, subprocess

commit = 0

#------------------------------------------------------------------------

def process(path, dirs, files):
    """clean a directory"""
    print('- '*32+path)
    if not files:
        if not dirs:
            print('removed empty directory '+path)
            os.rmdir(path)
            return
        print('no files in directory '+path)
        return
    cdir = os.getcwd()
    os.chdir(path)
    if commit:
        for name in files:
            #print("FILE="+name)
            if name.endswith('~'):
                os.remove(name)
            elif name.endswith('.pyc'):
                os.remove(name)
            elif name == 'array.pbs':
                os.remove(name)
            elif name == "log.txt":
                os.remove(name)
            elif name.endswith('.o') or name.endswith('.a'):
                os.remove(name)
            elif name.startswith('makefile') and name.endswith('.dep'):
                os.remove(name)
            elif name.startswith('._'):
                try:
                    os.remove(name)
                except OSError:
                    pass
            elif name == '.DS_Store':
                os.remove(name)
            elif name == 'config.out' and os.path.exists('config.cym'):
                os.remove(name)
            else:
                try:
                    if 0 == os.path.getsize(name):
                        os.remove(name)
                except OSError:
                    pass
    os.chdir(cdir)


def process_dir(path):
    """call process() with appropriate arguments"""
    files = os.listdir(path)
    for f in files:
        if os.path.isdir(f):
            files.remove(f)
    process(path, os.path.basename(path), files)


def archive(path):
    """compress a directory"""
    process_dir(path)
    subprocess.call(['tar', '-czf', path+'.tar.gz', path])
    shutil.rmtree(path)


#------------------------------------------------------------------------

def main(args):
    
    global commit
    walk = 0    
    paths = []
    
    for arg in args:
        if os.path.isdir(arg):
            paths.append(arg)
        else:
            [key, equal, value] = arg.partition('=')
            if key=='' or equal!='=' or value=='':
                sys.stderr.write("Error: I do not understand `%s'\n" % arg)
            #sys.exit()
            if key=='commit':
                commit = int(value)
            elif key=='walk':
                walk = int(value)
    
    
    if not paths:
        sys.stderr.write("Error: you should specify at least one directory\n")
        sys.exit()
    
    for path in paths:
        try:
            if walk:
                for path, dirs, files in os.walk(path, topdown=False):
                    process(path, dirs, files)
            else:
                archive(path)
        except Exception as e:
            sys.stderr.write("Error in `%s': %s\n" % (path, repr(e)));


#------------------------------------------------------------------------


if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])


