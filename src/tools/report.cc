// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 This is a program to analyse simulation results:
 it reads a trajectory-file, and print some data from it.
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
int prefix = 0;
size_t cnt = 0;


void help(std::ostream& os)
{
    os << "Cytosim-report "<<DIM<<"D, file version " << Simul::currentFormatID << '\n';
    os << "       generates reports/statistics from a trjacetory file\n";
    os << "Syntax:\n";
    os << "       report [time] WHAT [OPTIONS]\n";
    os << "Options:\n";
    os << "       precision=INTEGER\n";
    os << "       column=INTEGER\n";
    os << "       verbose=0\n";
    os << "       frame=INTEGER[,INTEGER[,INTEGER[,INTEGER]]]\n";
    os << "       period=INTEGER\n";
    os << "       input=FILE_NAME\n";
    os << "       output=FILE_NAME\n";
    os << "\n";
    os << "  This tool must be invoked in a directory containing the simulation output,\n";
    os << "  and it will generate reports by calling Simul::report(). The only required\n";
    os << "  argument `WHAT` determines what sort of data will be generated. Many options are\n";
    os << "  available, but are not listed here. Please check the HTML documentation.\n";
    os << "  By default, all frames in the file are processed in order, but a frame index,\n";
    os << "  or multiple indices can be specified (the first frame has index 0).\n";
    os << "  A periodicity can also be specified (ignored if multiple frames are specified).\n";
    os << "  The input trajectory file is `objects.cmo` unless otherwise specified.\n";
    os << "  The result is sent to standard output unless a file is specified as `output`\n";
    os << "  Attention: there should be no whitespace in any of the option.\n";
    os << "\n";
    os << "Examples:\n";
    os << "       report fiber:points\n";
    os << "       report fiber:points frame=10 > fibers.txt\n";
    os << "       report fiber:points frame=10,20 > fibers.txt\n";
    os << "       report fiber:points period=8 > fibers.txt\n";
}

//------------------------------------------------------------------------------

void report_raw(Simul const& simul, std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    if ( verbose > 0 )
    {
        os << "\n% frame   " << frm;
        simul.report(os, what, opt);
    }
    else
    {
        std::stringstream ss;
        simul.report(ss, what, opt);
        StreamFunc::skip_lines(os, ss, '%');
    }
}


void report_prefix(Simul const& simul, std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    char str[256] = { 0 };
    size_t str_len = 0;
    
    if ( prefix & 1 )
        str_len += snprintf(str, sizeof(str), "%9.3f ", simul.time());
    
    if ( prefix & 2 )
        str_len += snprintf(str+str_len, sizeof(str)-str_len, "%9i ", frm);
    
    std::stringstream ss;
 
    if ( verbose )
    {
        os << "% frame   " << frm << '\n';
        simul.report(ss, what, opt);
        StreamFunc::prefix_lines(os, ss, str, '%', 0);
    }
    else
    {
        simul.report(ss, what, opt);
        StreamFunc::prefix_lines(os, ss, str, 0, '%');
    }
}


void report(Simul const& simul, std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    ++cnt;
    try
    {
        if ( prefix )
            report_prefix(simul, os, what, frm, opt);
        else
            report_raw(simul, os, what, frm, opt);
    }
    catch( Exception & e )
    {
        std::cerr << "Aborted: " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}


//------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
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
    
    Glossary arg;

    std::string input = TRAJECTORY;
    std::string str, what;
    std::ostream * osp = &std::cout;
    std::ofstream ofs;

    // check for prefix:
    int ax = 1;
    while ( argc > ax+1 )
    {
        if ( strstr(argv[ax], "time") )
            prefix |= 1;
        else if ( strstr(argv[ax], "frame") )
            prefix |= 2;
        else
            break;
        ++ax;
    }
    
    what = argv[ax++];
    if ( arg.read_strings(argc-ax, argv+ax) )
        return EXIT_FAILURE;

#ifdef BACKWARD_COMPATIBILITY
    if ( arg.set(str, "prefix") && str=="time" )
        prefix = 1;
#endif
    
    unsigned frame = 0;
    unsigned period = 1;

    arg.set(input, ".cmo") || arg.set(input, "input");
    arg.set(verbose, "verbose");
    if ( arg.use_key("-") ) verbose = 0;

    Simul simul;
    FrameReader reader;

    try
    {
        RNG.seed();
        simul.loadProperties();
        reader.openFile(input);
    }
    catch( Exception & e )
    {
        std::clog << "Aborted: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    if ( arg.set(str, "output") )
    {
        try {
            ofs.open(str.c_str());
        }
        catch( ... )
        {
            std::clog << "Cannot open output file\n";
            return EXIT_FAILURE;
        }
        osp = &ofs;
    }
    
    Cytosim::all_silent();
    
    // get arguments:
    if ( arg.set(frame, "frame") )
        period = 0;
    arg.set(period, "period");
    
    // process first record, at index 'frame':
    if ( reader.loadFrame(simul, frame) )
    {
        std::cerr << "Error: missing frame " << frame << '\n';
        return EXIT_FAILURE;
    }
    if ( DIM != reader.vectorSize() )
    {
        std::cerr << "Error: dimensionality missmatch between `report` and file\n";
        return EXIT_FAILURE;
    }

    report(simul, *osp, what, frame, arg);

    if ( arg.nb_values("frame") > 1 )
    {
        // multiple record indices were specified:
        unsigned s = 1;
        while ( arg.set(frame, "frame", s) )
        {
            // try to load the specified frame:
            if ( 0 == reader.loadFrame(simul, frame) )
                report(simul, *osp, what, frame, arg);
            else
            {
                std::cerr << "Error: missing frame " << frame << '\n';
                return EXIT_FAILURE;
            }
            ++s;
        }
    }
    else if ( period > 0 )
    {
        // process every 'period' record:
        unsigned f = frame;
        while ( 0 == reader.loadNextFrame(simul)  )
        {
            ++f;
            if ( f % period == frame % period )
                report(simul, *osp, what, f, arg);
        }
    }
    
    if ( ofs.is_open() )
        ofs.close();

    /// check if all specified parameters were used:
    arg.print_warning(std::cerr, cnt, "\n");

    return EXIT_SUCCESS;
}
