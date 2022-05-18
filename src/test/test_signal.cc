// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 A test for handling Systems signals
 F. Nedelec 17.02.2017
 This is pure C code
*/

#include <signal.h>
#include <stdio.h>
#include <unistd.h>


void signal_handler(int sig)
{
    fprintf(stderr, "--- Received signal %u\n", sig);
}


int main(int argc, char* argv[])
{
    // Catch interrupting signals:
    if ( signal(SIGINT, signal_handler) == SIG_ERR )
        fprintf(stderr, "Could not register SIGINT handler\n");
    
    if ( signal(SIGTERM, signal_handler) == SIG_ERR )
        fprintf(stderr, "Could not register SIGTERM handler\n");

	// do nothing for 10 minutes:
	for ( int m = 0; m < 10; ++m )
	{
		printf("--- runtime is %u min.\n", m);
		for ( int s = 0; s < 60; ++s )
			sleep(1);
	}
    
	return 0;
}
