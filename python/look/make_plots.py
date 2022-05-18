#!/usr/bin/env python3
#
# Make a plot using matplotlib
#
# F. Nedelec, 13-15 November 2013
#
# How I installed matplotlib in Jan 2015 (Mac osx 10.10):
#
# brew install python3 --framework
# pip3 install numpy
# pip3 install nose
# pip3 install matplotlib
#
# Alternative:
# git clone git://github.com/matplotlib/matplotlib.git
# cd matplotlib
# python3 setup.py build
# sudo python3 setup.py install
# python3 setup.py clean
#


"""
    Make plots
    
Syntax:
    
    make_plots.py DIRECTORY-PATH
    
Description:
    
"""

import sys, os, math, subprocess
import numpy
import matplotlib.pyplot as plt
import read_config
#matplotlib.use('SVG')

fts = 14
out = sys.stderr

def load_data(file):
    try:
        return numpy.loadtxt(file, comments='%', unpack=True)
    except:
        return []

def image(x, p):
	while x > p:
		x = x - 2*p
	while x < -p:
		x = x + 2*p
	return x


def plot_diameter(cdata, rdata):
    plt.figure(figsize=(3.84, 2.56), dpi=100)
    plt.plot(cdata[0], cdata[1], 'ko')
    plt.plot(rdata[0], rdata[1], 'b-', linewidth=4)
    plt.xlabel('Time (s)', fontsize=fts)
    plt.ylabel('Diameter (um)', fontsize=fts)
    #plt.title('Closure', fontsize=fts)
    plt.tight_layout()
    #plt.xlim([-5, 5])
    #plt.grid(True)


def plot_positions(xdata, xlim, xinfo):
    plt.figure(figsize=(3.84, 2.56), dpi=100)
    n, bins, patches = plt.hist(xdata, 20, normed=1)
    ax = plt.gca()
    ax.yaxis.set_visible(False)
    plt.xlabel('Position (%s)' % xinfo, fontsize=fts)
    plt.ylabel('Amount', fontsize=fts)
    plt.title('Histogram', fontsize=fts)
    plt.tight_layout()
    plt.xlim(xlim)
    #plt.grid(True)


def parse(dirpath):
    """
    Work in current directory
    """
    pile = read_config.parse('config.cym')
    shape = read_config.value(pile, ['set', 'space', 'cell', 'geometry'])

    if shape.startswith('wall'):
        subprocess.call(['reportW', 'space'], stdout=open('radius.txt', 'w'), stderr=None)
        rdata = load_data('radius.txt')
        if len(rdata) > 0:
            cdata = get_data('/Users/nedelec/tmp/closure_diameter.txt')
            data = [ rdata[2], [ 2*x for x in rdata[3] ] ]
            plot_diameter(cdata, data)
            plt.savefig('diameter.png')
            plt.close()
        #
        subprocess.call(['reportW', 'bead:all', 'frame=1999'], stdout=open('beads.txt', 'w'), stderr=None)
        data = load_data('beads.txt')
        data = [ math.atan2(data[3][n], data[2][n]) for n in range(len(data[0])) ]
        if len(data) > 0:
            plot_positions(data, [-3.1416, 3.1416], 'radian')
            plt.savefig('position.png')
            plt.close()

    if shape.startswith('strip'):
        subprocess.call(['reportW', 'bead:all', 'frame=999'], stdout=open('beads.txt', 'w'), stderr=None)
        data = load_data('beads.txt')
        if len(data) > 0:
            data = [ image(x, 5) for x in data[2] ]
            plot_positions(data, [-5, 5], 'um')
            plt.savefig('position.png')
            plt.close()


#------------------------------------------------------------------------

def main(args):
    paths = []
    
    for arg in args:
        if os.path.isdir(arg):
            paths.append(arg)
        else:
            out.write("  Error: unexpected argument `%s'\n" % arg)
            sys.exit()
    
    if not paths:
        parse('.')
    else:
        cdir = os.getcwd()
        for p in paths:
            os.chdir(p)
            sys.stdout.write('- '*32+p"\n")
            try:
                parse(p)
            except Exception as e:
                out.write("Error: %s\n" % repr(e));
            os.chdir(cdir)

#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])

