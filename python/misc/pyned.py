# pyned.py contains often used python subroutines
# it is not executable by itself
# Copyright F. Nedelec, 2017--2019

import os, subprocess, math, copy;


def random_color():
    R=random.random()
    G=random.random()
    B=random.random()
    return (R, G, B)


def sqr(x):
    return x * x;


def uncode(arg):
    try:
        return arg.decode('utf-8')
    except:
        return arg


def szudzik(a, b):
    """
        Matthew Szudzik's pairing function
    """
    if a >= b:
        return a * a + a + b
    else:
        return a + b * b


def kizduzs(z):
    """
        Reverse Matthew Szudzik's pairing function
    """
    b = math.floor(math.sqrt(z))
    a = z - b * b;
    if a < b:
        return ( a, b )
    else:
        return ( b, a - b )



def format_line(data):
    """
        Convert line of data to column-aligned string
    """
    res = ''
    for x in data:
        if isinstance(x, float):
            res += ' %9.4f' % x
        elif isinstance(x, str):
            res += ' %9s' % x
        else:
            res += ' %9s' % repr(x)
    return res


def extract_value(s):
    s = s.rstrip(',;')
    try:
        return 1, int(s)
    except:
        pass
    try:
        return 1, float(s)
    except:
        pass
    return 0, 0


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
    res = []
    sub = subprocess.Popen(['diff', left, right], stdout=subprocess.PIPE)
    for stuff in sub.stdout:
        try:
            line = stuff.decode('utf-8')
        except:
            line = stuff
        if line[0] == '>':
            for s in line.split():
                f, v = extract_value(s)
                if f == 1:
                    res.append(v)
    sub.stdout.close()
    return res


def get_column(file, jj):
    """
        Extract the jj-th word from line ii
        If jj is an array, the corresponding values will be concatenated
    """
    res = ' '
    for line in file:
        if not line or line[0] == '%':
            continue
        s = line.split();
        try:
            res += s[jj].rjust(7) + ' '
        except IndexError:
            pass
    return res


def frange(s, e, n):
    i = ( e - s ) / n
    return [ s+i*x for x in range(n) ]



def simple_linear_fit(arg):
    """
    Fit an linear law, minimizing squared residual with vertical offset
    This returns (0,B) where the best fit is B * x
    """
    data = copy.deepcopy(arg)
    s = 0
    sxx = 0
    sxy = 0
    for x, y in data:
        s += 1
        sxx += x * x
        sxy += x * y
    if s > 0:
        b = sxy / sxx
        return ( 0, b )
    else:
        return ()


def linear_fit(arg):
    """
    Fit an linear law, minimizing squared residual with vertical offset
    This returns (A,B) where the best fit is A + B * x
    """
    data = copy.deepcopy(arg)
    s = 0
    sx = 0
    sy = 0
    sxx = 0
    sxy = 0
    for x, y in data:
        s += 1
        sx += x
        sy += y
        sxx += x * x
        sxy += x * y
    if s > 0:
        n = ( s * sxx - sx * sx )
        a = ( sy * sxx - sx * sxy ) / n
        b = ( s * sxy - sx * sy ) / n
        return ( a, b )
    else:
        return ()


def exponential_fit(arg, ybase = 0):
    """
    Fit an exponential law, using robust (second) method described in
        http://mathworld.wolfram.com/LeastSquaresFittingExponential.html
    This returns (A,B) where the best fit is A * exp(B*x)
    """
    data = copy.deepcopy(arg)
    sy = 0
    sxy = 0
    sxxy = 0
    syly = 0
    sxyly = 0
    for x, y in data:
        if y > ybase:
            yb = y - ybase;
            sy += yb
            sxy += x * yb
            sxxy += x * x * yb
            syly += y * math.log(yb)
            sxyly += x * y * math.log(yb)
    if sy > 0:
        a = ( sxxy*syly - sxy*sxyly ) / ( sy*sxxy - sxy*sxy )
        b = ( sy*sxyly - sxy*syly ) / ( sy*sxxy - sxy*sxy )
        return ( math.exp(a), b )
    else:
        return ()


def exponential_model_fit(arg):
    """
    Fit an exponential curve  A*exp(B*x) + C to the data
    This returns (A,B,C)
    """
    data = copy.deepcopy(arg)
    mxy = min(data)
    best_err = math.inf
    best_fit = ()
    for m in range(0, 100):
        C = mxy-m
        A, B = exponential_fit(data, C)
        data = copy.deepcopy(arg)
        err = 0
        for x, y in data:
            err += abs(y-A*math.exp(B*x)-C)
            print(C, err)
        if err < best_err:
            best_err = err
            best_fit = (A,B,C)
    return best_fit


def power_fit(arg, power):
    """
    Fit a powerlaw with given exponent
    This returns (A) where the best fit is A * x^power
    """
    data = copy.deepcopy(arg)
    sxx = 0
    sxy = 0
    for x, y in data:
        sxx += ( x * x ) ** power
        sxy += y * ( x ** power )
    scale = sxy / sxx
    return scale


def powerlaw_fit(arg):
    """
    Fit a powerlaw, direct method as described in:
        http://mathworld.wolfram.com/LeastSquaresFittingPowerLaw.html
    This returns (A,B) where the best fit is A * x^B
    """
    from math import log, exp
    data = copy.deepcopy(arg)
    s = 0
    sx = 0
    sy = 0
    sxx = 0
    sxy = 0
    syy = 0
    for x, y in data:
        if x > 0 and y > 0:
            s   += 1
            sx  += log(x)
            sy  += log(y)
            sxx += log(x) * log(x)
            sxy += log(x) * log(y)
            syy += log(y) * log(y)
    if s > 0:
        n = s * sxy - sx * sy;
        d = s * sxx - sx * sx;
        power = n / d;
        scale = exp(( sy - power * sx ) / s);
        return ( scale, power )
    return ()
