// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <iostream>

/// Simple operations on C++ streams
namespace StreamFunc
{
    
    /// remove non-conventional characters
    void clean_stream(std::ostream&, std::istream&);
    
    /// export lines of `val` that are not identical to `ref`
    void diff_stream(std::ostream&, std::istream& val, std::istream& ref);
    
    
    /// copy lines that do not start with character `skip`
    void skip_lines(std::ostream&, std::istream&, char skip);

    /// add `prefix` before every line, but skip lines starting with `skip`
    void prefix_lines(std::ostream&, std::istream&, const char prefix[], char keep, char skip);

    /// print the line of `istream` indicating the position `pos`, with a marker underline
    void mark_line(std::ostream&, std::istream&, std::streampos pos, const char prefix[]);
    
    /// print a line of `istream` indicating the current input position
    void mark_line(std::ostream&, std::istream&);

    /// same as `mark_line()`, but output is returned as a string
    std::string marked_line(std::istream&, std::streampos, const char prefix[]);
    
    
    /// extract line containing given `pos'
    std::string get_line(std::istream&, std::streampos pos);

    /// extract the lines located between `start` and `end`, with line numbers
    void print_lines(std::ostream&, std::istream&, std::streampos start, std::streampos end);
    
    /// same as print_lines(), but output is returned as a string
    std::string get_lines(std::istream&, std::streampos start, std::streampos end);
    
    
    /// return line number corresponding to `pos` (or current position if not specified)
    size_t line_number(std::istream&, std::streampos pos = -1);

    /// replace all occurences of `fnd` by `rep` in `src`. Returns number of replacements done
    int  find_and_replace(std::string & src, std::string const& fnd, std::string const& rep);
}

