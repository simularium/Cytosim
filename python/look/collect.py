#!/usr/bin/env python
#
# collect.py renames files from subdirectories
#
# Copyright F. Nedelec, 2007--2021


"""
Synopsis:
    
    Rename files or folders following a pattern containing an integer index,
    as in 'image0001.png'. The file will be moved in the current directory
    
    The number in the file name is incremented automatically for each file, and
    also if files with this name already exist. Thus pre-existing files are not 
    overwritten, such that 'collect.py' can be used to pool together many similar
    files in a common directory.
    
Syntax:
    
    collect.py PATTERN [INTEGER] [--copy] PATH1 [PATH2] [PATH3] ...

Arguments:
    
    PATTERN specifies the name of the output files, and should contain a variable
    part that will be replaced by an integer. It can be a 'scanf' compatible 
    pattern such as '%i' or '%0Xi', for example 'image%04i.png'.
    A character '%' repeated multiple times, such as `%%%%` or `%%%%%%`, can
    also be used to specify the size of the integer portion of the name.
    
    The pattern can include a '/' that would indicate a directory, and if this
    directory does not exist, collect.py will create it before moving the file.

    if specified, `--copy` will copy the files/directory instead of moving them
    
    if specified, INTEGER is the first index to be used (default=0)

    PATH1, PATH2, etc. is a list of files or directories

Examples:
    
    collect.py image%%%%.png  *.png
       will rename image files to: image0000.png, image0001.png, etc.
    
    collect.py --copy image%%%%.png 1 run*/image.png
       will copy the image files, starting at index 1

    collect.py run%%%%/config.cym config*.cym
       will create directories `run????` and move the `config*.cym` files into them
    
F. Nedelec, 2012--2021. Last modified 6.10.2021
"""


import sys, shutil, os, curses.ascii


#------------------------------------------------------------------------

def cannonical_pattern(arg):
    """check for repeated '%' character, replacing printf syntax: %04i """
    c = arg.count('%')
    for n in range(c,1,-1):
        if arg.find('%'*n) > 0:
            return arg.replace('%'*n, '%0'+str(n)+'i', 1);
    return arg


def validate_pattern(arg):
    # check validity of the pattern
    if os.path.isfile(arg):
        sys.stderr.write("Error: first argument should be the pattern used to build output file name")
        sys.exit(1)
    try:
        res = ( arg % 0 )
    except:
        arg = cannonical_pattern(arg)
    try:
        res = ( arg % 0 )
    except:
        sys.stderr.write("Error: the pattern should accept an integer: eg. '%04i'\n")
        sys.exit(1)
    for c in res:
        if curses.ascii.isspace(c):
            sys.stderr.write("Error: the pattern includes or generates white space character\n")
            sys.exit(1)
    return arg


def main(args):
    """rename files"""
    do_copy = False
    arg = args[0]
    # check if 'copy' specified before pattern
    if arg=='-c' or arg=='--copy' or arg=='copy=1':
        do_copy = True
        args.pop(0);
    # get pattern for moving files:
    pattern = validate_pattern(args.pop(0))
    # parse arguments:
    idx = 0
    paths = []
    for arg in args:
        if arg=='-c' or arg=='--copy' or arg=='copy=1':
            do_copy = True
        elif args[0].isdigit():
            idx = int(args[0])
        elif os.path.isfile(arg) or os.path.isdir(arg):
            paths.append(arg)
        else:
            sys.stderr.write("Error: '%s' is not a file or directory" % arg)
            return 1
    # process all files:
    res = []
    for src in paths:
        while idx < 1000000:
            dst = pattern % idx
            idx += 1
            if dst == src:
                res.append(dst)
                break
            if not os.path.exists(dst):
                #make directory if name includes a non-existent directory:
                dir = os.path.dirname(dst)
                if dir and not os.path.isdir(dir):
                    os.mkdir(dir)
                # process file:
                if do_copy:
                    shutil.copytree(src, dst)
                else:
                    os.rename(src, dst)
                res.append(dst)
                print("%s -> %s" % (src, dst))
                break
    return res


#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])


