#!/usr/bin/env python3
#
# compare_config.py
#
# compare two by two all the provided files and print the one with fewer differences than the cutoff
#
# Copyright F. Nedelec, 2011 - 2016

"""
    Compare two by two all the provided files and print the one with fewer differences

Syntax:

    compare_config.py THRESHOLD files
    
Example:

    compare_config 2 run????/config.cym

Description:

    Compare all config files two by two, and print the pair if they differ
    at most on THRESHOLD lines
    
F. Nedelec, Mar. 2016
"""

from __future__ import print_function

import sys, os, subprocess

threshold = 2

#------------------------------------------------------------------------

def uncode(arg):
    try:
        if isinstance(arg, unicode):
            return str(arg.decode('utf-8'))
    except:
        pass
    return arg


def find_differences(left, right):
    """
        Find differences between the file that are numeric values
        Multiple values are concatenated.
    """
    L = []
    R = []
    sub = subprocess.Popen(['diff', left, right], stdout=subprocess.PIPE)
    while sub.stdout:
        line = uncode(sub.stdout.readline())
        if len(line) < 2:
            break
        if line[0] == '<':
            L.append(line[0:len(line)-1])
        elif line[0] == '>':
            R.append(line[0:len(line)-1])
    sub.stdout.close()
    return L, R


def process(files):
    """
    Compare all file pairs
    """
    for f in files:
        for g in files:
            if g <= f:
                continue
            L, R = find_differences(f, g)
            if len(L)+len(R) <= threshold:
                fs = f.rstrip('.tpl').rstrip('/config.cym')
                gs = g.rstrip('.tpl').rstrip('/config.cym')
                print(fs.ljust(10) + ' ' + gs.ljust(10), end='')
                if len(L)+len(R) == 0:
                    print(': no differences')
                elif len(L) == 1 and len(R) == 1:
                    L = L[0][2:].strip()
                    R = R[0][2:].strip()
                    print(':   %-40s <>    %-40s' %(L, R))
                else:
                    print('')
                    for d in L:
                        print('                  '+d)
                    for d in R:
                        print('                  '+d)
 

#------------------------------------------------------------------------

def main(args):
    global threshold
    files = []
    
    for arg in args:
        if arg.isdigit():
            threshold = int(arg)
        elif os.path.isdir(arg):
            files.append(arg+'/config.cym')
        elif os.path.isfile(arg):
            files.append(arg)
        else:
            sys.stderr.write("  Error: unexpected argument `%s'\n" % arg)
            sys.exit()
    
    if len(files) < 2:
        sys.stderr.write("  Error: two files at least should be specified\n")
        sys.exit()
    
    process(files)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])


