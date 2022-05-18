// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 A test for the Floating Point Exceptions (Signal)
*/

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <csignal>
#include <cmath>

#include "smath.h"

/*
 icpc --help
 
 -fp-trap=<arg>[,<arg>,...]
 control floating point traps at program start.  <arg> can be of the
 following values
 [no]divzero   - [Do not] trap on division by zero
 [no]inexact   - [Do not] trap on inexact result
 [no]invalid   - [Do not] trap on invalid operation
 [no]overflow  - [Do not] trap on overflow
 [no]underflow - [Do not] trap on underflow
 [no]denormal  - [Do not] trap on denormal
 all           - enable trap on all of the above
 none          - trap on none of the above
 common        - trap on most commonly used IEEE traps
 (invalid, division by zero, overflow)
 -fp-trap-all=<arg>[,<arg>,...]
 control floating point traps in every routine.  <arg> can be of the
 values specified in -fp-trap
 */

std::ostream& out = std::cout;

typedef double real;

void signal_handler(int sig)
{
    psignal(sig, "test");
    _exit(sig);
}

void modulo()
{
    out << "   x    fmod remainder";
    for ( real x = -4; x <= 4; x += 0.5 )
    {
        out << "\n" << std::setw(5) << x;
        out << "  " << std::setw(5) << fmod(x, 2.0);
        out << "  " << std::setw(5) << remainder(x, 2.0);
    }
    out << '\n';
}

void infinities()
{
    out << "0   < inf = " << ( 0 < INFINITY ) << '\n';
    out << "inf < inf = " << ( INFINITY < INFINITY ) << '\n';
    real z = 0;
    real y = 0.0 / z;
    real x = 1.0 / z;
    out << " 1.0/0.0 = " << x << '\n';
    out << " 0.0/0.0 = " << y << '\n';
}

void std_copysign()
{
    out << "copysign(1, +1) = " << std::copysign(1.0, +1.0) << '\n';
    out << "copysign(1, -1) = " << std::copysign(1.0, -1.0) << '\n';
    out << "copysign(1,  0) = " << std::copysign(1.0,  0.0) << '\n';
    out << "copysign(1, -0) = " << std::copysign(1.0, -0.0) << '\n';
}

void print_numbers()
{
    out << " 1.0 / 0 = " <<  1.0 / 0 << '\n';
    out << "-1.0 / 0 = " << -1.0 / 0 << '\n';
    out << " 0.0 / 0 = " <<  0.0 / 0 << '\n';
    out << "-log(0)  = " << -log(0.0) << '\n';
#if ( 0 )
    out << "absf(-2) = " << sMath::absf(-2.0) << '\n';
    out << "absf(-1) = " << sMath::absf(-1.) << '\n';
    out << "absf(+1) = " << sMath::absf(+1.) << '\n';
    out << "absf(+2) = " << sMath::absf(+2.) << '\n';
#endif
}


void read_numbers(std::string const& str)
{
    std::stringstream is(str);
    double x, y, z;
    if ( is >> x )
        out << "read x = " << x << '\n';
    if ( is >> y )
        out << "read y = " << y << '\n';
    if ( is >> z )
        out << "read z = " << z << '\n';
    
    char tmp[128] = { 0 };
    is.clear();
    is.readsome(tmp, sizeof(tmp));
    out << "remaining >" << tmp << '\n';
}


int main()
{
    //read_numbers("1 2 nnnn .0 aaaaa");
    
    if ( signal(SIGFPE, signal_handler) == SIG_ERR )
    {
        out << "Could not register SIGFPE handler\n";
        return EXIT_FAILURE;
    }
    std_copysign();
    infinities();
    print_numbers();
    out << "test completed" << '\n';
    return 0;
}
