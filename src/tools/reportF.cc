// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 This is a program to analyse simulation results:
 it reads a trajectory-file, and print selected data from it to text files.
*/

#include <fstream>
#include <sstream>

#include "stream_func.h"
#include "frame_reader.h"
#include "iowrapper.h"
#include "glossary.h"
#include "messages.h"
#include "splash.h"
#include "parser.h"
#include "simul.h"

int verbose = 1;

void help(std::ostream& os)
{
    os << "Cytosim-reportF "<<DIM<<"D, file version " << Simul::currentFormatID << '\n';
    os << "       generate reports/statistics about cytosim's objects\n";
    os << "\n";
    os << "Syntax:\n";
    os << "       reportF WHAT [verbose=0] [root=STRING]\n";
    os << "\n";
    os << "This tool must be invoked in a directory containing the simulation output\n";
    os << "It will generate the same reports as Simul::report()\n";
    os << "See the documentation of Simul::report() for a list of possible values for WHAT\n";
    os << "\n";
    os << "'reportF' is simular to 'report', but sends the output for each frame\n";
    os << "of the trajectory to a different file. These files are named:\n";
    os << "    ROOT####.txt\n";
    os << "where #### is the frame number and ROOT can be specified.\n";
}

//------------------------------------------------------------------------------

void report(Simul const& simul, std::ostream& os, std::string const& what, Glossary& opt)
{
    if ( verbose )
    {
        simul.report(os, what, opt);
    }
    else
    {
        std::stringstream ss;
        simul.report(ss, what, opt);
        StreamFunc::skip_lines(os, ss, '%');
    }
}

//------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    Cytosim::all_silent();
    
    if ( argc < 2 || strstr(argv[1], "help") )
    {
        help(std::cout);
        return EXIT_SUCCESS;
    }
    
    if ( strstr(argv[1], "info") || strstr(argv[1], "--version")  )
    {
        splash(std::cout);
        print_version(std::cout);
        return EXIT_SUCCESS;
    }

    std::string input = TRAJECTORY;
    std::string root = "report", str, what = argv[1];
    
    Glossary arg;
    if ( arg.read_strings(argc-2, argv+2) )
        return EXIT_FAILURE;
    arg.set(input, ".cmo") || arg.set(input, "input");
    if ( arg.use_key("-") ) verbose = 0;
    arg.set(verbose, "verbose");
    arg.set(root, "root");
    
    Simul simul;
    FrameReader reader;
    RNG.seed();

    try
    {
        simul.loadProperties();
        reader.openFile(input);
        
        unsigned frame = 0;
        char filename[256];
        
        // load all frames in the file:
        while ( 0 == reader.loadNextFrame(simul) )
        {
            if ( DIM != reader.vectorSize() )
            {
                std::cerr << "Error: dimensionality missmatch between `report` and file\n";
                return EXIT_FAILURE;
            }
            snprintf(filename, sizeof(filename), "%s%04i.txt", root.c_str(), frame);
            std::ofstream out(filename);
            report(simul, out, what, arg);
            ++frame;
        }
    }
    catch( Exception & e )
    {
        std::cerr << "Aborted: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
