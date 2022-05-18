#!/usr/bin/env python
#
# collect.py
#
# Copyright F. Nedelec, 2007--2018


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
    A character '%' repeated multiple times, such as `%%%%` or `%%%%%%`, can be 
    used to directly specify the size of the integer portion of the name.
    
    The pattern can include a directory, and if this directory does not exist,
    collect.py will attemtps to create it before moving the file.

    if specified, --copy will copy the files/directory instead of moving them
    
    if specified, INTEGER is the first index to be used (default=0)

    PATH1, PATH2, etc. is a list of files or directories

Examples:
    
    collect.py image%%%%.png  *.png
       will rename image files to: image0000.png, image0001.png, etc.
    
    collect.py --copy image%%%%.png 1 run*/image.png
       will copy the image files, starting at index 1

    collect.py run%%%%/config.cym config*.cym
       will create directories `run????` and move the `config*.cym` files into them
    
F. Nedelec, 2012--2018. Last modified 2.10.2017
"""


import sys, shutil, os, curses.ascii


#------------------------------------------------------------------------

def copy_recursive(src, dst):
    """Copy directory recursively"""
    if os.path.isfile(src):
        shutil.copy2(src, dst)
    elif os.path.isdir(src):
        try:
            os.mkdir(dst)
        except OSError:
            pass
        files = os.listdir(src)
        for f in files:
            s = os.path.join(src, f)
            d = os.path.join(dst, f)
            copy_recursive(s, d)


def main(args):
    """rename files"""
    do_copy = False
    arg = args.pop(0);
    # check if 'copy' specified before pattern
    if arg=='-c' or arg=='--copy' or arg=='copy=1':
        do_copy = True
        pattern = args.pop(0);
    else:
        pattern = arg
    # check validity of the pattern
    if os.path.isfile(pattern):
        sys.stderr.write("Error: first argument should be the pattern used to build output file name")
        return 1
    try:
        res = ( pattern % 0 )
    except:
        # check for repeated '%' character:
        for n in range(10,0,-1):
            s = pattern.find('%'*n)
            if s > 0:
                pattern = pattern.replace('%'*n, '%0'+str(n)+'i', 1);
                break
        try:
            res = ( pattern % 0 )
        except:
            sys.stderr.write("Error: the pattern should accept an integer: eg. '%04i'\n")
            return 1
    for c in res:
        if curses.ascii.isspace(c):
            sys.stderr.write("Error: the pattern includes or generates white space character\n")
            return 1
    # go
    paths = []
    idx = 0
    # parse arguments:
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
    # process all files
    res = []
    for src in paths:
        while idx < 1000000:
            dst = pattern % idx
            idx += 1
            if dst == src:
                res.append(dst)
                break
            if not os.path.exists(dst):
                #make directory if name include a directory that does not exist:
                dir = os.path.dirname(dst)
                if dir and not os.path.isdir(dir):
                    os.mkdir(dir)
                # process file:
                if do_copy:
                    copy_recursive(src, dst)
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


