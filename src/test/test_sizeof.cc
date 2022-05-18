// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cstdio>
#include <sys/types.h>

#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <cmath>

template <typename T>
void print_sizeof(char const* msg)
{
    std::cout << std::setw(16) << msg;
    //std::cout << " : " << typeid(T).name();
    std::cout << "    " << sizeof(T) << " bytes" << std::endl;
}

void print_types()
{
    print_sizeof<bool>("bool");
    print_sizeof<char>("char");
    print_sizeof<short>("short");
    print_sizeof<int>("int");
    print_sizeof<unsigned>("unsigned");
    print_sizeof<long>("long int");
    print_sizeof<long long int>("long long int");
    
    print_sizeof<float>("float");
    print_sizeof<double>("double");
    print_sizeof<long double>("long double");
}

void print_sizes()
{
    printf("Size of Data Types:\n");

    printf( "bool          = %lu bytes\n", sizeof(bool) );
    printf( "char          = %lu bytes\n", sizeof(char) );
    printf( "short         = %lu bytes\n", sizeof(short) );
    printf( "int           = %lu bytes\n", sizeof(int) );
    printf( "unsigned      = %lu bytes\n", sizeof(unsigned) );
    printf( "long int      = %lu bytes\n", sizeof(long int) );
    printf( "long long int = %lu bytes\n", sizeof(long long int) );
    printf( "float         = %lu bytes\n", sizeof(float) );
    printf( "double        = %lu bytes\n", sizeof(double) );
    printf( "long double   = %lu bytes\n", sizeof(long double) );
    printf( "void *        = %lu bytes\n", sizeof(void *) );
    printf( "fpos_t        = %lu bytes\n", sizeof(fpos_t) );
    printf( "off_t         = %lu bytes\n", sizeof(off_t) );
    printf( "size_t        = %lu bytes\n", sizeof(size_t) );
 
}

int main ()
{
    print_types();
    printf("\n");
    print_sizes();
    printf("done\n");
    return 0;
}
