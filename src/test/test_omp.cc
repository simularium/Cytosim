// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// F. Nedelec, 22.04.2018

#include <cstdio>
#include <omp.h>

typedef double real;

const int max = 1000;
const int num = 4;

void process(int val)
{
    printf("%i %i\n", omp_get_thread_num(), val);
}

/*
void pointers(real * vec, real * end)
{
#pragma omp parallel
    {
        #pragma omp single private(p)
        {
            real * p = vec;
            while ( p < end )
            {
                #pragma omp task
                process(p);
                p += 10;
            }
        }
    }
}
*/

void pooh(int off, real * vec, int inc)
{
    printf("thread %i\n", off);
    for ( int i = off; i < max; i += inc)
        vec[i] = 0;
}


int main(int argc, char* argv[])
{
    double vec[1000];
    
    omp_set_num_threads(num);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        pooh(id, vec, num);
    }
 
    #pragma omp parallel num_threads(6)
    {
        int inc = omp_get_num_threads();
        int id = omp_get_thread_num();
        pooh(id, vec, inc);
    }
    
    #pragma omp parallel for num_threads(4)
    for ( int i = 0; i < 11; ++i )
        process(i);
 
    printf("done\n");
    return 0;
}
