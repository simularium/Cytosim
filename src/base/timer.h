// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University


#if 0
/* Using Intel's timer functions */
#include <x86intrin.h>

/* declare rdtsc from assembly if necessary
extern __inline unsigned long long
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
__rdtsc (void)
{
  return __builtin_ia32_rdtsc();
}
*/

/// keeping time using Intel's cycle counters
unsigned long long rdt_ = 0;

/// return current time value (64 bits)
inline unsigned long long timer() { return __rdtsc(); }

/// start timer
inline void tic() { rdt_ = __rdtsc(); }

/// return time since last 'tic()', divided by 'arg'
inline double toc(double arg = 1) { return double(__rdtsc()-rdt_) / arg; }

#elif ( 1 )

#include <sys/time.h>

// using real time
struct timeval tic_t;

/// return current time value
inline unsigned long long timer()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return double(1e6*tv.tv_sec + tv.tv_usec);
}

/// start timer
inline void tic()
{
    gettimeofday(&tic_t, nullptr);
}

/// return time since last 'tic()', divided by 'arg'
inline double toc(double arg = 1e3)
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return double(1e9*(tv.tv_sec-tic_t.tv_sec) + 1e3*(tv.tv_usec-tic_t.tv_usec))/arg;
}

#else
// using the CPU time

#include <ctime>
clock_t tic_t;

/// return current time value
inline clock_t timer()
{
    return clock();
}

/// start timer
void tic()
{
    tic_t = clock();
}

/// return time since last 'tic()', divided by 'arg'
double toc(double arg = CLOCKS_PER_SEC)
{
    return (1e3*( clock() - tic_t )) / arg;
}

#endif
