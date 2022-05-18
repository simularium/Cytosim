#!/usr/bin/env python
#
# reduce.py
#
# copyright F. Nedelec, 2010-2013

"""
reduce.py:
    reduce the size of cytosim file 'objects.cmo' in selected directories
    
Usage:
    reduce.py [min=INTEGER] [commit] DIRECTORIES
    
The word 'commit' must be given otherwise the directories will not be processed
Specifying 'min' sets the minimum number of frames in 'objects.cmo'

Each pass reduces the number of frames by a factor 10.
A file 'reduced' is created in each processed directory.
If such a file already exists, the directory is not processed.
This is a safety mechanism to ensure each directory is processed once only.

F. Nedelec, 28.04.2011 - 20.12.2013
"""

import sys, os, subprocess

commit = 0
min_cnt = 0

err = sys.stderr

#------------------------------------------------------------------------

def nbFrames(file='objects.cmo'):
    try:
        str = subprocess.check_output(['frametool', file])
    except:
        proc = subprocess.Popen(['frametool', file], stdout=subprocess.PIPE)
        line = proc.stdout.readline().split()
        proc.stdout.close()
    try:
        return int(line[0])
    except:
        return int(line[1])
    return 0


def process(path):
    """cleanup directory"""
    os.chdir(path)
    files = os.listdir('.')

    print('- '*32+path)
    if min_cnt > 0:
        cnt = nbFrames()
        if cnt <= min_cnt:
            print(' > objects.cmo has %i frames' % cnt)
            return

    if commit:
        if 'reduced' in files:
            return
        if 'objects.cmo' in files:
            out = open('objectsR.cmo', 'w')
            subprocess.call(['frametool', 'objects.cmo', '0:10:'], stdout=out)
            os.rename('objectsR.cmo', 'objects.cmo');
            print('reduced > objects.cmo')
        if 'messages.cmo' in files:
            out = open('messagesR.cmo', 'w')
            subprocess.call(['grep', '-v', '^F[0-9]*[123456789] ', 'messages.cmo'], stdout=out);
            os.rename('messagesR.cmo', 'messages.cmo');
            print('reduced > messages.cmo')
        subprocess.call(['touch', 'reduced'])
    else:
        if 'objects.cmo' in files:
            cnt = nbFrames('objects.cmo')
            print('  > objects.cmo  : %i frames' % cnt)
        if 'messages.cmo' in files:
            print('  > messages.cmo')


#------------------------------------------------------------------------

def main(args):
    global commit, min_cnt
    paths = []
    
    for arg in args:
        if arg == 'commit':
            commit = 1
        elif arg.startswith('commit='):
            commit = int(arg[7:])
        elif arg.startswith('min='):
            min_cnt = int(arg[4:])
        elif os.path.isdir(arg):
            paths.append(arg)
        else:
            err.write("ignored '%s' on command line\n" % arg)

    if not paths:
        paths.append('.')

    cdir = os.getcwd()
    for path in paths:
        os.chdir(cdir)
        try:
            process(path)
        except Exception as e:
            print(" Error in `%s': %s" % (path, repr(e)));


if __name__ == "__main__":
    if len(sys.argv)>1 and sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])
