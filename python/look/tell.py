#!/usr/bin/env python
#
# tell.py prints parameters from config files in given directories
#
# F. Nedelec, Jan 24th, 2008, March 2010, July 2014, July 2017

"""
    Print parameters from multiple files in CSV column format
    
Syntax:
    
    tell.py parameter-names [directories/files]

"""


import sys, os
sep = ' '


def format_value(val):
    if isinstance(val, str):
        return val
    if isinstance(val, dict):
        r = ""
        s = "{ "
        for k in sorted(val):
            r += s + str(k) + " = " + format_value(val[k])
            s = "; "
        return r + "; }"
    try:
        r = ""
        s = ""
        for k in val:
            r += s + format_value(k)
            s = ", "
        return r
    except:
        return str(val)


def column_width(table):
    """calculate the maximum width of each column"""
    w = []
    for line in table:
        for i, l in enumerate(line):
            while i >= len(w):
                w.append(0)
            v = len(format_value(l))
            if v > w[i]:
                w[i] = v
    return w


def format_table(table):
    """print table in tidy column format"""
    width = column_width(table)
    indx = range(1,len(width))
    res = ""
    for line in table:
        val = format_value(line[0]).ljust(width[0])
        for i in indx:
            if not line[i]:
                v = "-"
            else:
                v = format_value(line[i])
            val += sep+v.rjust(width[i])
        res += val + '\n'
    return res


def config_file(arg):
    """return name of probable config-file"""
    if os.path.isdir(arg):
        file = os.path.join(arg, 'config.cym')
        if os.path.isfile(file):
            return file
    if os.path.isfile(arg):
        return arg
    return ""


def find_any(str, chars):
    r = len(str)
    for c in chars:
        x = str.find(c)
        if x >= 0 and x < r:
            r = x
    return r


def find_value(file, key):
    """find value corresponding to key. Multiple values are concatenated"""
    val = []
    for line in file:
        [k, e, values] = line.partition('=')
        if key == k.strip() and e == '=':
            #print(k, e, values)
            e = find_any(values, '\n%')
            val.append(values[0:e].strip())
    return val


def tell(files, keys):
    """print parameter from multiple files in columns"""
    res=[]
    line = ['% file']
    line.extend(keys)
    res.append(line)
    
    for f in files:
        try:
            file = open(f, "r")
            vals = []
            for key in keys:
                file.seek(0)
                vals.append(find_value(file, key))
            file.close()
            if f.endswith('/config.cym'):
                line = [f[:len(f)-11]]
            else:
                line = [f]
            line.extend(vals)
            res.append(line)
        except IOError:
            continue
    return res;


#------------------------------------------------------------------------

def main(args):
    """print parameter from multiple files in columns"""
    global sep
    paths = []
    keys = []
    for arg in args:
        if os.path.exists(arg):
            paths.append(arg)
        elif arg.startswith('sep='):
            sep = arg[4:]
        else:
            keys.append(arg)
    
    if not paths:
        print(__doc__)
        return

    files = []
    for p in paths:
        files.append(config_file(p))
    
    table = tell(files, keys)
    #table = [ ['a', '1'], ['b', '2'], ['c', '3'] ]
    print(format_table(table))


if __name__ == "__main__":
    if len(sys.argv)>1 and sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])

