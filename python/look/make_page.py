#!/usr/bin/env python
#
# make_page.py creates a HTML page with links to files in given directories
#
# Copyright FJ Nedelec, 14.12.2007 -- 4.2015 & 7.3.2022

"""
Synopsis:

    Generates an HTML page linking images & movies found in sub-directories

Usage:
    
    make_page.py [recursive=1] [tile=INT] [width=INT] [height=INT] [OUTPUT] DIRECTORIES/FILES

Info:
    
    If `tile` is set, a HTML table will be generated, and the value specified will be the
    number of entries in each line. The name of the ouput file ('page.html' by default), 
    can be specified on the command line (extension should be .html).
    `width` or `height` specify the size in pixels at which images will appear on the HTML.


Copyright F.J. Nedelec, Cambridge University
Created  14.12.2007
Modified 3.2010, 5.2012, 11.2012, 7.2013, 11.2013, 4.2015, 7.3.2022
"""

import sys, os, subprocess
import struct, imghdr

out  = 0
indx = 1
iarg = ''
tile = 0
curs = 0
excluded = []
output = 'page.html'


def writeHeader():
    global out;
    out.write('<html>\n')
    out.write('<head>\n')
    try:
        out.write('<title>%s</title>\n'%os.getcwd())
    except Exception:
        pass
    out.write('<meta name="ROBOTS" content="NOINDEX, NOFOLLOW">\n')
    out.write('<script type="text/JavaScript">\n')
    out.write('var zwin=0;\n')
    out.write('function mk_win()   { zwin=window.open("","see","resizable=1,width=1280,height=1024"); }\n')
    #out.write('function set(i, m)  { document.images[i].src=m; }\n')
    out.write('function zoom(m)    { mk_win(); zwin.location=m; }\n')
    #out.write('function stat(s)    { window.status=s; }\n')
    out.write('</script>\n')
    out.write('</head>\n')
    out.write('<body bgcolor="#AAAAAA">\n')
    out.write('<h2>\n')
    out.write('<a href="index.html">index</a>\n')
    out.write('</h2>\n')


def writeFooter():
    global out;
    out.write('</body>\n')
    out.write('</html>\n')


def getImageSize(filename):
    '''Determine the image type and return its size. from draco'''
    with open(filename, 'rb') as file:
        head = file.read(24)
        if len(head) != 24:
            return
        if imghdr.what(filename) == 'png':
            check = struct.unpack('>i', head[4:8])[0]
            if check != 0x0d0a1a0a:
                return
            W, H = struct.unpack('>ii', head[16:24])
        elif imghdr.what(filename) == 'gif':
            W, H = struct.unpack('<HH', head[6:10])
        elif imghdr.what(filename) == 'jpeg':
            try:
                file.seek(0) # Read 0xff next
                size = 2
                ftype = 0
                while not 0xc0 <= ftype <= 0xcf:
                    file.seek(size, 1)
                    byte = file.read(1)
                    while ord(byte) == 0xff:
                        byte = file.read(1)
                    ftype = ord(byte)
                    size = struct.unpack('>H', file.read(2))[0] - 2
                # We are at a SOFn block
                file.seek(1, 1)  # Skip `precision' byte.
                H, W = struct.unpack('>HH', file.read(4))
            except Exception: #IGNORE:W0703
                return
        else:
            return
        return W, H


def getMovieSize(file):
    """
    Call ffprobe, which is a tool that comes with ffmpeg,
    and parse output to extract size of videos or images
    """
    W = 256
    H = 256
    call = subprocess.run(['ffprobe', '-v', 'quiet', '-show_streams', file], capture_output=True)
    for line in call.stdout.decode():
        #print(line, end='')
        try:
            [key, equal, val] = line.partition('=')
            if key=="width":
                W = int(val)
            elif key=="height":
                H = int(val)
        except:
            pass
    #print(" `%s' is %i x %i\n" %(file, W, H))
    return W, H


def writeImageLinks(files):
    global out, iarg
    for f in sorted(files):
        shot = os.path.basename(f)
        #out.write('<br>%s: ' % shot);
        out.write('<a href="javascript:zoom(\'%s\');">\n' % f);
        out.write('  <img %s src="%s" alt="%s">\n' % (iarg, f, shot));
        out.write('</a>\n')
    if files:
        out.write('\n')


def writeMovieLinks(files):
    global out
    for f in sorted(files):
        W, H = getMovieSize(f)
        shot = os.path.basename(f)
        out.write('<video controls="controls" width="%s" height="%s" loop="true" alt="%s">\n' % (W, H, shot));
        out.write('  <source src="%s" type="video/mp4">\n' % f);
        out.write('  This is a HTML5 VIDEO element\n');
        out.write('</video>\n');
    if files:
        out.write('\n')


def process(dirpath, subdir, files, images, movies):
    """
    Write HTML code for given directory
    """
    global out, indx, tile, curs
    if tile > 0:
        out.write('<td>\n')
    out.write('<h3 style="padding:3px;margin:3px"> '+dirpath.lstrip('./'))
    for f in files:
        out.write('\n  &mdash; <a href="%s/%s">%s</a>' % (dirpath, f, f));
    out.write('\n</h3>\n')
    writeImageLinks(images)
    writeMovieLinks(movies)
    if curs:
        for d in subdir:
            process_dir(d)
    if tile > 0:
        out.write('</td>\n')
        if indx % tile == 0:
            out.write('</tr><tr>\n')
    indx += 1


def process_dir(dirpath):
    """select relevant files and call process()"""
    subdir = []
    files = []
    images = []
    movies = []
    for f in os.listdir(dirpath):
        path = os.path.join(dirpath, f)
        if os.path.isdir(path):
            subdir.append(f)
        else:
            [name, ext] = os.path.splitext(f)
            if name in excluded:
                pass
            elif ext in ['.png', '.jpg', '.gif', '.tif', '.svg']:
                images.append(path)
            elif ext in ['.mp4', '.mov']:
                movies.append(path)
            elif f in ['config.cym', 'movie.mp4']:
                files.append(f)
    process(dirpath, subdir, files, images, movies)


#------------------------------------------------------------------------

def main(args):
    """generates HTML page"""
    global output, out, iarg, tile, curs
    paths = []
    
    for arg in args:
        [key, equal, val] = arg.partition('=')
        if key and equal=='=' and val:
            if key=='width' or key=='height':
                iarg = arg
            elif key=='table':
                tile = int(val)
            elif key=='tile':
                tile = int(val)
            elif key=='recursive':
                curs = int(val)
            elif key=='exclude':
                excluded.append(val)
            else:
                sys.stderr.write("ignored '%s' on command line\n" % arg)
        else:
            if os.path.isdir(arg):
                paths.append(arg)
            elif arg.endswith('.html'):
                output = arg
            else:
                sys.stderr.write("ignored '%s' on command line\n" % arg)

    if not paths:
        sys.stderr.write("You must specify a path on the command line\n")
        sys.exit()

    try:
        out = open(output, 'w')
    except Exception as e:
        sys.stderr.write("Error creating file `%s': %s\n" % (output, repr(e)))
        out = sys.stdout
    writeHeader()
    
    if tile > 0:
        out.write('<table border="0" align="center" cellpadding="1">\n')
        out.write('<tr>\n')

    for p in paths:
        process_dir(p)
    
    if tile > 0:
        out.write('</tr>\n')
        out.write('</table>\n')
    
    writeFooter()
    if out != sys.stdout:
        out.close()
        print("generated '%s' with %i entries" % (output, indx-1))


#------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv)>1 and sys.argv[1].endswith("help"):
        print(__doc__)
    else:
        main(sys.argv[1:])

