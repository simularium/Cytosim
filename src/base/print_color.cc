// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 03/09/2014.


#include "print_color.h"
#include <cstdio>
#include <cstdlib>


#if ( 1 )

// using ANSI escape sequence for UNIX-based systems

#define KNRM  "\x1B[0m"
#define KBLD  "\x1B[1m"
#define KUND  "\x1B[4m"
#define KREV  "\x1B[7m"

#define KBLK  "\x1B[30m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define KBLDRED  "\x1B[1m\x1B[31m"
#define KBLDGRN  "\x1B[1m\x1B[32m"
#define KBLDYEL  "\x1B[1m\x1B[33m"
#define KBLDBLU  "\x1B[1m\x1B[34m"
#define KBLDMAG  "\x1B[1m\x1B[35m"
#define KBLDCYN  "\x1B[1m\x1B[36m"
#define KBLDWHT  "\x1B[1m\x1B[37m"


/* Check the number of colors that the terminal supports */
bool has_colors()
{
    static long n_colors = 0;
    if ( n_colors == 0 )
    {
        long n = 1;
        FILE * fp = popen("tput cols 2> /dev/null", "r");
        if ( fp ) {
            char str[32] = { 0 };
            if ( fgets(str, sizeof(str), fp) )
                n = strtol(str, nullptr, 10);
            pclose(fp);
            //printf("terminal has %li colors: %s", n, str);
        }
        n_colors = n;
    }
    return n_colors > 7;
}


void print_red(std::ostream& os, std::string const& str)
{
    if ( has_colors() )
        os << KBLDRED << str << KNRM;
    else
        os << str;
}

void print_green(std::ostream& os, std::string const& str)
{
    if ( has_colors() )
        os << KBLDGRN << str << KNRM;
    else
        os << str;
}

void print_yellow(std::ostream& os, std::string const& str)
{
    if ( has_colors() )
        os << KBLDYEL << str << KNRM;
    else
        os << str;
}

void print_blue(std::ostream& os, std::string const& str)
{
    if ( has_colors() )
        os << KBLDBLU << str << KNRM;
    else
        os << str;
}

void print_magenta(std::ostream& os, std::string const& str)
{
    if ( has_colors() )
        os << KBLDMAG << str << KNRM;
    else
        os << str;
}

void print_cyan(std::ostream& os, std::string const& str)
{
    if ( has_colors() )
        os << KBLDCYN << str << KNRM;
    else
        os << str;
}

#else


void print_red(std::ostream& os, std::string const& str)
{
    os << str;
}

void print_green(std::ostream& os, std::string const& str)
{
    os << str;
}

void print_yellow(std::ostream& os, std::string const& str)
{
    os << str;
}

void print_blue(std::ostream& os, std::string const& str)
{
    os << str;
}

void print_magenta(std::ostream& os, std::string const& str)
{
    os << str;
}

void print_cyan(std::ostream& os, std::string const& str)
{
    os << str;
}

#endif

