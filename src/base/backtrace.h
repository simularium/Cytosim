// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 24/04/2010.
#ifndef BACKTRACE_H
#define BACKTRACE_H


/*
 Backtrace will print the current call-stack using functions from the GNU C Library:
 - backtrace()
 - backtrace_symbols()
 .
 Having backtrace enabled can help to identify the cause of a crash, 
 but it is not required to run.
 
 This GNU extension is available on some systems. 
 On Mac this is available since OSX 10.5.
 It can be disabled without harm, by editing 'backtrace.cc'
 */


#include <cstdio>


/// print the stack of function calls for the current thread
void print_backtrace(int fildes = 2);


#endif

