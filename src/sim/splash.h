// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <iostream>
#include "assert_macro.h"
#include "fiber.h"
#include "grid.h"
#include "real.h"
#include "dim.h"


/// print header line identifying the project
inline void splash(std::ostream& os)
{
    os << " ------------------------------------------------------------- \n";
    os << "|  CytoSIM " <<DIM<<"D -  www.cytosim.org  -  version PI  - June 2019  |\n";
    os << " ------------------------------------------------------------- \n";
}


/// print general info about the program
inline void print_version(std::ostream& os)
{
    os << "   Dimension: " << DIM;
    os << "   Periodic: " << GRID_HAS_PERIODIC;
    os << "   Precision: " << sizeof(real) << " bytes\n";
    os << "   Fiber lattice " << FIBER_HAS_LATTICE << "\n";
    os << "   Built " <<__DATE__<< " " <<__TIME__<< " with " <<__VERSION__<< "\n";
    
#ifdef CODE_VERSION
    os << "   Code version " << CODE_VERSION;
#else
    os << "   Code version unknown";
#endif
    
#ifdef NDEBUG
    os << " (no assertions)\n";
#else
    os << " with assertions\n";
#endif
}

