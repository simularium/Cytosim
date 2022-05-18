// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 This is a test of C++ extensions
 defined by ISOC++11
 
 http://en.wikipedia.org/wiki/C%2B%2B11
 
 it should be compiled with icc -std=gnu++11
 
 
 199711L
 201103L
 201300L
 201402L
 */

#include <iostream>

int main ()
{
    std::clog << "C++ version " << __cplusplus << std::endl;

    int my_array[] = { 1, 2, 3, 5, 7, 11, 13, 17, 23 };
    for (int &x : my_array) {
        std::cout << " " << x;
    }
    std::cout << std::endl;
    return 0;
}
