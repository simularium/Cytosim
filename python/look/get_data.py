#!/usr/bin/env python
#
# get_data.py
#
# simple extraction of data from files
#
# Copyright F. Nedelec, 2011 - 2016

"""
    Collect data from given run directories, and print them to standard output

Syntax:

    get_data.py DIRECTORIES
    
Example:

    get_data run???? > data.txt

Description:

    This script must be customized for any meaningful application,
    but it should be a useful template to start from.
    Please make a copy of the script with a different name.

F. Nedelec, Jan. 2016
"""

import sys, os, subprocess


#------------------------------------------------------------------------

def find_value(file, key):
    """
        Find values corresponding to 'key' in the file.
        Multiple values are concatenated.
    """
    res = ''
    for line in file:
        inx = line.find(key)
        if inx >= 0:
            r = line.find('=', inx)
            if r > inx:
                v = line[r+1:].strip()
                if len(res):
                    res += '|' + v
                else:
                    res = v
    return res


def get_cell(file, ii, jj):
    """
        Extract the jj-th word from line ii
        If jj is an array, the corresponding values will be concatenated
    """
    if len(jj) == 0:
        return '';
    cnt = 0
    for line in file:
        cnt = cnt + 1
        if cnt == ii:
            s = line.split()
            val = s[jj[0]]
            for n in jj[1:]:
                val = s[n] + ' '
            return val
    return ''


def get_column(file, jj):
    """
        Extract the jj-th word from line ii
        If jj is an array, the corresponding values will be concatenated
        """
    res = ' '
    for line in file:
        s = line.split();
        if not s or s[0] == '%':
            continue
        try:
            res += s[jj].rjust(7) + ' '
        except IndexError:
            pass
    return res


def find_differences(left, right):
    """
        Find differences between two files that are numeric values
        Multiple values are concatenated.
        This can be used to compare config files
        """
    if not os.path.isfile(left):
        return 'file not found: ' + left
    if not os.path.isfile(right):
        return 'file not found: ' + right
    res = ''
    sub = subprocess.Popen(['diff', left, right], stdout=subprocess.PIPE)
    for stuff in sub.stdout:
        try:
            line = stuff.decode('utf-8')
        except:
            line = stuff
        if line[0] == '>':
            for s in line.split():
                s = s.rstrip(',;')
                try:
                    int(s)
                except:
                    try:
                        float(s)
                    except:
                        continue
                res += ' %-10s' % s
    sub.stdout.close()
    return res


#------------------------------------------------------------------------

def process(path):
    """ 
        This extracts parameters from the config file,
        and values from 'mom.txt'
    """
    res = path + ' '
    try:
        res += find_differences('config.cym', path+'/config.cym')
    except IOError as e:
        sys.stderr.write("Error: %s\n" % repr(e))
        return ''
    res += ' nan'
    try:
        with open(path+'/mom.txt', 'r') as f:
            res += get_column(f, -1)
    except IOError as e:
        sys.stderr.write("Error: %s\n" % repr(e))
        res = ''
    return res


def main(args):
    paths = []
    
    for arg in args:
        if os.path.isdir(arg):
            paths.append(arg)
        else:
            sys.stderr.write("  Error: unexpected argument `%s'\n" % arg)
            sys.exit()
    
    if not paths:
        sys.stderr.write("  Error: you must specify directories\n")
        sys.exit()
    
    nb_columns = 0
    for p in paths:
        res = process(p)
        # check that the number of column has not changed:
        if nb_columns != len(res.split()):
            if nb_columns == 0:
                nb_columns = len(res.split())
            else:
                sys.stderr.write("Error: data size mismatch in %s\n" % p)
                return
        print(res)


#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])


