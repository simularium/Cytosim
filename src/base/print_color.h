// Cytosim was created by Francois Nedelec. Copyright Cambridge University 2020
// FJN 03.09.2014.

/**
 This defines functions to print text in color on terminals that support it
 It is based on the original ANSI specification that defined 8 standard colors:
 https://en.wikipedia.org/wiki/ANSI_escape_code
 
 This can be disabled in `print_color.cc`
 */

#include <iostream>


/// True if terminal supports colors
bool has_colors();

/// print text in red
void print_red(std::ostream&, std::string const&);

/// print text in green
void print_green(std::ostream&, std::string const&);

/// print text in yellow
void print_yellow(std::ostream&, std::string const&);

/// print text in blue
void print_blue(std::ostream&, std::string const&);

/// print text in magenta
void print_magenta(std::ostream&, std::string const&);

/// print text in cyan
void print_cyan(std::ostream&, std::string const&);


