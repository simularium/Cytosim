// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "exceptions.h"
#include "glossary.h"

int i = 0;
float f = 0.0;
std::string s;


int main(int argc, char* argv[])
{
    Glossary arg;
    
    if ( arg.read_strings(argc-1, argv+1) )
        return EXIT_FAILURE;

    try {
        
        // read file if provided on command line:
        // the file name is recognized by its extension
        std::string str;
        if ( arg.set(str, ".cym") )
            arg.read_file(str);
        
        // print content of Glossary:
        printf("%lu keys:\n", arg.nb_keys());
        arg.write(std::cout, "    > ");
        
        // extract values from Glossary:
        if ( arg.set(i, "integer") )  printf("integer : %i\n", i);
        if ( arg.set(f, "float") )    printf("float : %f\n", f);
        if ( arg.set(s, "string") )   printf("string : %s\n", s.c_str());
        
        return EXIT_SUCCESS;
    }
    catch ( Exception& e )
    {
        std::cout << "Error : " << e.what();
        return EXIT_FAILURE;
    }
}
