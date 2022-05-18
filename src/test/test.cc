// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cstdio>
#include <sys/types.h>

#include "array.h"
#include "random.h"


int comp(const void * a, const void * b)
{
    int av = *(static_cast<const int*>(a));
    int bv = *(static_cast<const int*>(b));
    if ( av < bv )
        return -1;
    else if ( av > bv )
        return 1;
    else
        return 0;
}


int main(int argc, char* argv[])
{
    RNG.seed();
    Array<int> a;
    
    for( unsigned cnt = 0; cnt < 10; ++cnt )
    {
        a.clear();
        unsigned n = RNG.poisson(8);
        for( int i=0; i < n; ++i )
            a.push_back(RNG.pint32(2));
        
        printf("\nsize %lu", a.size());
        {
            Array<int> b;
            b = a;
            a.deallocate();
            
            printf("\n   copy %2lu :", b.size());
            for( unsigned i=0; i < b.size(); ++i )
                printf(" %i", b[i]);

            b.remove_pack(0);
            
            printf("\n   pack %2lu :", b.size());
            for( unsigned i=0; i < b.size(); ++i )
                printf(" %i", b[i]);
            
            b.sort(comp);
            
            printf("\n   sort %2lu :", b.size());
            for( unsigned i=0; i < b.size(); ++i )
                printf(" %i", b[i]);
        }
    }
    
    printf("\ndone\n");
    return 0;
}
