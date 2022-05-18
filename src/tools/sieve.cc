// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "simul.h"
#include "parser.h"
#include "glossary.h"
#include "iowrapper.h"
#include "exceptions.h"
#include "simul_prop.h"


void help()
{
    printf("Cytosim-sieve %iD\n", DIM);
    printf("    file version %i built on %s\n", Simul::currentFormatID, __DATE__);
    printf("Synopsis:\n");
    printf("   `sieve` let you to manipulate cytosim trajectory file.\n");
    printf("   It reads a trajectory files, and loads the objects in memory\n");
    printf("\n");
    printf("   The system is written in the latest format, in either binary or text.\n");
    printf("   A category of objects can be removed with option skip=WHAT.\n");
    printf("   If the specified output file already exists, data is appended to it.\n");
    printf("\n");
    printf("Usage:\n");
    printf("    sieve input_file output_file [options]\n\n");
    printf("Possible options:\n");
    printf("    binary=0     generate output in text format\n");
    printf("    binary=1     generate output in binary format\n");
    printf("    skip=WHAT    remove all objects of class WHAT\n");
    printf("    frame=INDEX  process only specified frame\n");
    printf("\n");
    printf("Example:\n");
    printf("    sieve objects.cmo objects.txt binary=0\n");
    printf("    sieve objects.cmo objects.txt binary=0 skip=couple\n");
}


int main(int argc, char* argv[])
{
    if ( argc < 3 )
    {
        help();
        return EXIT_SUCCESS;
    }

    Simul simul;
    Glossary arg;
    
    std::string input  = argv[1];
    std::string output = argv[2];
    if ( arg.read_strings(argc-3, argv+3) )
        return EXIT_FAILURE;
    
    ObjectSet * skip_set = nullptr;
    std::string skip;
    if ( arg.set(skip, "skip") )
       skip_set = simul.findSet(skip);
    
    bool binary = true;
    arg.set(binary, "binary");
    arg.set(simul.prop->skip_free_couple, "skip_free_couple");
    
    Inputter in(DIM);
    try {
        simul.loadProperties();
        in.open(input.c_str(), "rb");
    }
    catch( Exception & e ) {
        std::cerr << "Error opening input file `" << input << "' :" << std::endl;
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    
    std::clog << ">>>>>> Sieve `" << input << "' -> `" << output << "'" << std::endl;
    
    size_t frm = 0, frame = 0;
    
    // a frame index can be specified:
    bool has_frame = arg.set(frame, "frame");
    
    while ( in.good() )
    {
        try {
            if ( simul.reloadObjects(in) )
                return EXIT_SUCCESS;
        }
        catch( Exception & e ) {
            std::clog << "Error in frame " << frm << ":\n";
            std::clog << "    " << e.what() << std::endl;
        }

        if ( skip_set )
            skip_set->erase();
            
        /*
        simul.reportInventory(std::cout);
        std::clog << "\b\b\b\b\b" << std::setw(5) << cnt;
        */
        
        try {
            if ( !has_frame || ( frm == frame ) )
                simul.writeObjects(output, true, binary);
        }
        catch( Exception & e ) {
            std::clog << "Error writing `" << output << "' :\n";
            std::clog << "    " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        if ( has_frame && frm == frame )
            break;
        ++frm;
    }
    return 0;
}
