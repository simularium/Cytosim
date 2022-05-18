#!/usr/bin/env python
#
# FJN, Sainsbury Laboratory, Cambridge University
# 4.6.2019


"""
    Read data from file and save plot 
    
Syntax:
    
    plot.py [filename]
    
"""

import sys, math, os
from random import random

try:
    import matplotlib
    import matplotlib.pyplot as plt
except ImportError:
    print("  Error: could not load matplotlib in python " + sys.version)
    sys.exit()


#set default font size for plots
font = {'family':'arial', 'weight':'normal', 'size': 18}
matplotlib.rc('font', **font)


def random_color():
    """ Return a darkish random RGB triplet"""
    while True:
        R = random()
        G = random()
        B = random()
        if R + G + B < 2:
            break
    return (R, G, B)


def read(filename):
    """
        Read numeric data from file
    """
    data = []
    with open(filename, 'r') as f:
        for line in f:
            if line and line[0]!='%':
                s=(float(x) for x in line.split());
                data.append(s)
    return data


def plot_data(data):
    """ plot data on new graph """
    # repackage data:
    val = list(zip(*data))
    t = val[0]
    for v in val[1:]:
        plt.plot(t, v, '-', linewidth=2, color=random_color())
        #plt.ylim(0, max(v))


def plot(files):
    """ plot data from multiple files and save image """
    fig = plt.figure(figsize=(9, 6))
    ax = plt.axes()
    ax.xaxis.set_major_locator(plt.MaxNLocator(12))
    ax.yaxis.set_major_locator(plt.MaxNLocator(12))
    for f in files:
        data = read(f)
        plot_data(data)
    plt.xlabel('Time')
    plt.ylabel('Molecules')
    plt.title('Simulation')
    fig.tight_layout()
    plt.show()
    plt.savefig('data', dpi=100)
    plt.close()

#-------------------------------------------------------------------------------

def main(args):
    files = []
    # examine command line arguments
    for arg in args:
        if os.path.isfile(arg):
            files.append(arg)
        else:
            sys.stderr.write("Unexpected argument `%s'\n" % arg)
            sys.exit()
    # set default
    if not files:
        files = ['data.txt']
    # process all files
    plot(files)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1]=='help':
        print(__doc__)
    else:
        main(sys.argv[1:])

