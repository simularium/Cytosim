#!/usr/bin/env python
#
# compare.py
#
# copyright F. Nedelec, December 14th 2007, 14.03.2018; 4.8.2020

"""
compare.py
    Compare files from two root directories
Usage:
    compare.py root1 root2 [opendiff]
"""

from __future__ import print_function
import sys, os, shutil, time, subprocess, tempfile

exe = "diff"
diff="diff --side-by-side -W200 -p --suppress-common-lines"



def print_spacer(arg):
    """print a line of width size, with 'arg' in the middle"""
    rows, cols = os.popen('stty size', 'r').read().split()
    sys.stdout.write(chr(27)+"[36;2m"); sys.stdout.flush()
    print(arg.center(int(cols), '-'))
    sys.stdout.write(chr(27)+"[0m"); sys.stdout.flush()


def compareFiles(fileL, fileR):
    if not os.path.isfile(fileL):
        return print("absent LEFT file %s" % fileL)
    if not os.path.isfile(fileR):
        return print("absent RIGHT file %s" % fileR)
    #print("compare %s %s" % (fileL, fileR))
    comp  = os.popen(diff+" -q "+fileL+" "+fileR)
    empty = ( len(comp.read()) == 0 )
    comp.close()
    if not empty:
        if exe == 'diff':
            file = os.path.basename(fileL);
            print_spacer(file)
            comp = os.popen(diff+" "+fileL+" "+fileR)
            for line in comp:
                print(line, end='')
            comp.close()
            
            sys.stdout.write(chr(27)+"[32;2m")
            print("This was %40s" % file)
            ans = raw_input('Action (return/left/right/open/q)? >'+chr(27)+'[0m')
            
            if ans == "left" or ans == "l":
                shutil.copyfile(fileL, fileR)
            elif ans == "right" or ans == "r":
                shutil.copyfile(fileR, fileL)
            elif ans == "swap":
                fid, file = tempfile.mkstemp('.txt', 'temp', '', True)
                print(os.getcwd(), os.path.isfile(file))
                os.close(fid);
                os.rename(fileL, file)
                os.rename(fileR, fileL)
                os.rename(file, fileR)
            elif ans == "open":
                os.system("opendiff "+fileL+" "+fileR+"&")
            elif ans == "q":
                sys.exit()
        else:
            subprocess.call(["opendiff", fileL, fileR])
            #we wait a bit for the application to start
            time.sleep(0.5)


def interesting(file):
    return ( file.endswith('.py') or file.endswith('.m')
        or file.endswith('.h') or file.endswith('.cc')
        or file.endswith('.md') or file.endswith('.txt')
        or file.startswith('makefile') or file.startswith('.cym') )


def process_dir(roots, pathL, files):
    """compare files in the current directory"""
    if pathL.startswith(roots[0]):
        path = pathL[len(roots[0]):]
    else:
        path = pathL
    pathR = os.path.normpath(roots[1] + '/' + path)
    #print("PATH %s %s" % (pathL, pathR))
    #print("FILES %s" % files)
    if path.endswith('.svn'):
        return
    if path.endswith('.git'):
        return
    if 'DerivedData/' in path:
        return
    if 0 <= path.find('/.git/'):
        return
    if path == 'DerivedData':
        return
    if path.startswith('bin'):
        return
    if path == 'build':
        return
    print_spacer('%s %s'%(pathL, pathR))
    for file in files:
        if interesting(file):
            fileL = os.path.join(pathL, file)
            fileR = os.path.join(pathR, file)
            compareFiles(fileL, fileR)

#------------------------------------------------------------------------

def main(args):
    """main"""
    global exe
    if len(args) < 2:
        print("Error: you must specify two root directories!")
        sys.exit()
    rootL = os.path.relpath(args[0])
    rootR = os.path.relpath(args[1])
    if not os.path.isdir(rootL):
        print("Error: `%s' is not a directory" % rootL)
        sys.exit()
    if not os.path.isdir(rootR):
        print("Error: `%s' is not a directory" % rootR)
        sys.exit()
    #parse command-line arguments:    
    for arg in args[2:]:
        if arg == 'opendiff':
            exe=arg
        else:
            print("unknown argument '%s'" % arg)
            sys.exit()
    # process all directories within 'root':
    #print("Comparing %s and %s" % (rootL, rootR))
    for path, dirs, files in os.walk(rootL, topdown=False):
        process_dir([rootL, rootR], path, files)

if __name__ == "__main__":
    if len(sys.argv) < 3 or sys.argv[1]=='help':
        print(__doc__)
    else:
        main(sys.argv[1:])



