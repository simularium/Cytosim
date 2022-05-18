// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "simul.h"
#include "parser.h"
#include "messages.h"
#include "glossary.h"
#include "exceptions.h"
#include "print_color.h"
#include "filepath.h"
#include "splash.h"
#include "tictoc.h"
#include <csignal>
#include "unistd.h"


void help(std::ostream& os)
{
    os << "sim [OPTIONS] [FILE]\n";
    os << "  FILE    run specified config file (FILE must end with `.cym')\n";
    os << "  *       print messages to terminal (and not `messages.cmo')\n";
    os << "  info    print build options\n";
    os << "  help    print this message\n";
}


void handle_signal(int sig)
{
    /*
     A signal handler is restricted to call only async-signal-safe-functions
     practically speaking, most syscalls(2) only
     */
    char str[128] = { 0 };
    strncpy(str, "Cytosim received signal   \n", 128);
    str[25] = (char)('0' + ( sig     % 10));
    str[24] = (char)('0' + ((sig/10) % 10));
    (void) write(STDERR_FILENO, str, 28);
    _exit(sig);
}


void handle_interrupt(int sig)
{
    Cytosim::out << "killed " << sig << "\n" << std::endl;
    _exit(sig);
}

//------------------------------------------------------------------------------
//=================================  MAIN  =====================================
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // register callback to catch interrupting signals:
    if ( signal(SIGINT, handle_interrupt) )
        std::cerr << "Could not register SIGINT handler\n";
    if ( signal(SIGTERM, handle_interrupt) )
        std::cerr << "Could not register SIGTERM handler\n";
    if ( signal(SIGSEGV, handle_signal) )
        std::cerr << "Could not register SIGSEGV handler\n";
    if ( signal(SIGILL,  handle_signal) )
        std::cerr << "Could not register SIGILL handler\n";
    if ( signal(SIGABRT, handle_signal) )
        std::cerr << "Could not register SIGABRT handler\n";

    Glossary arg;

    //parse the command line:
    if ( arg.read_strings(argc-1, argv+1) )
        return EXIT_FAILURE;

    if ( arg.use_key("help") || arg.use_key("--help") )
    {
        splash(std::cout);
        help(std::cout);
        return EXIT_SUCCESS;
    }

    if ( arg.use_key("info") || arg.use_key("--version")  )
    {
        splash(std::cout);
        print_version(std::cout);
        return EXIT_SUCCESS;
    }
    
    if ( ! arg.use_key("+") )
    {
        Cytosim::out.open("messages.cmo");
        Cytosim::log.redirect(Cytosim::out);
        Cytosim::warn.redirect(Cytosim::out);
    }
    
    // change working directory if specified:
    if ( arg.has_key("directory") )
    {
        FilePath::change_dir(arg.value("directory", 0));
        //std::clog << "Cytosim working directory is " << FilePath::get_cwd() << '\n';
    }

#ifdef CODE_VERSION
    Cytosim::out << "CYTOSIM PI version " << CODE_VERSION << "\n";
#else
    Cytosim::out << "CYTOSIM PI\n";
#endif

    Simul simul;
    try {
        simul.initialize(arg);
    }
    catch( Exception & e ) {
        print_magenta(std::cerr, e.brief());
        std::cerr << '\n' << e.info() << '\n';
        return EXIT_FAILURE;
    }
    catch(...) {
        print_red(std::cerr, "Error: an unknown exception occurred during initialization\n");
        return EXIT_FAILURE;
    }
    
    arg.print_warning(std::cerr, 1, " on command line\n");
    time_t sec = TicToc::seconds_since_1970();
    
    try {
        Parser(simul, 1, 1, 1, 1, 1).readConfig();
    }
    catch( Exception & e ) {
        print_magenta(std::cerr, e.brief());
        std::cerr << '\n' << e.info() << '\n';
        return EXIT_FAILURE;
    }
    catch(...) {
        std::cerr << "\nError: an unknown exception occurred\n";
        return EXIT_FAILURE;
    }
    
    Cytosim::out << "% " << TicToc::date() << "\n";
    sec = TicToc::seconds_since_1970() - sec;
    Cytosim::out << "end  " << sec << " s ( " << (real)( sec / 60 ) / 60.0 << " h )\n";
    Cytosim::out.close();
    return EXIT_SUCCESS;
}
