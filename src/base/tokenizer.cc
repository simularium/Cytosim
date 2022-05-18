// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 1/7/2009.

#include "assert_macro.h"
#include "tokenizer.h"
#include "exceptions.h"
#include <errno.h>

// Use the second definition to get some verbose reports:
#define VLOG(ARG) ((void) 0)
//#define VLOG(ARG) std::clog << ARG;


char Tokenizer::block_delimiter(char c)
{
    switch (c)
    {
        case '(': return ')';
        case '{': return '}';
        case '[': return ']';
        case '"': return '"';
    }
    return 0;
}

//------------------------------------------------------------------------------

std::string Tokenizer::get_line(std::istream& is)
{
    std::string res;
    res.reserve(1024);
    std::getline(is, res);
    return res;
}


int Tokenizer::skip_space(std::istream& is, bool eat_line)
{
    int c = is.get();
    while ( isspace(c) )
    {
        if ( c == '\n' && ! eat_line )
            break;
        c = is.get();
    }
    if ( c != EOF )
        is.unget();
    return c;
}


int Tokenizer::get_character(std::istream& is, bool eat_space, bool eat_line)
{
    int c = 0;
    do {
        c = is.get();
        if ( c == EOF )
            return EOF;
#if ( 0 )
        if ( c == COMMENT_START )
        {
            std::string line;
            std::getline(is, line);
            c = '\n';
        }
#endif
        if ( c == '\n' && ! eat_line )
            break;
    } while ( eat_space && isspace(c) );
    return c;
}

//------------------------------------------------------------------------------
#pragma mark -

std::string get_stuff(std::istream& is, bool (*valid)(int))
{
    std::string res;
    res.reserve(64);
    int c = is.get();
    while ( valid(c) )
    {
        res.push_back((char)c);
        c = is.get();
    }
    if ( c != EOF )
        is.unget();
    return res;
}


bool valid_symbol(int c)
{
    return isalnum(c) || c == '_';
}


/**
 get_symbol() reads consecutive characters:
 - starting with a alpha-character,
 - followed by alpha-numerical characters or '_'
 .
 */
std::string Tokenizer::get_symbol(std::istream& is, bool eat_line)
{
    int c = skip_space(is, eat_line);
    
    if ( !isalpha(c) )
        return "";
    
    std::string res = get_stuff(is, valid_symbol);
    
    VLOG("SYMBOL |" << res << "|\n");

    return res;
}


/**
 get_symbols() reads multiple symbols concatenated with ':'
 */
std::string Tokenizer::get_symbols(std::istream& is, bool eat_line)
{
    int c = skip_space(is, eat_line);
    
    if ( !isalpha(c) )
        return "";
    
    std::string res = get_symbol(is, eat_line);
    while ( is.peek() == ':' )
    {
        res += (char)is.get();
        res += get_symbol(is, false);
    }
    
    return res;
}


bool valid_filename_start(int c)
{
    return isalpha(c) || c=='/' || c=='\\' || c=='.';
}

bool valid_filename(int c)
{
    return isalnum(c) || c=='_' || c=='-' || c=='/' || c=='\\' || c=='.' || c==':';
}

/**
 Return next token if it looks likes a path to a file name
 */
std::string Tokenizer::get_filename(std::istream& is, bool eat_line)
{
    int c = skip_space(is, eat_line);
    
    if ( c == '*' )
    {
        is.get();
        return "*";
    }
    
    if ( !valid_filename_start(c) )
        return "";
    
    std::string res = get_stuff(is, valid_filename);
    
    VLOG(" FILENAME |" << res << "|\n");
    
    return res;
}


/**
 Read multiple forms of integer:
 - integer-constant: digit-sequence [exponent-part]
 - digit-sequence:   [digit-sequence] digit
 - digit         :   one of [0123456789]
 - exponent-part: ('e' or 'E') [sign] digit-sequence
 */
std::string Tokenizer::get_integer(std::istream& is)
{
    bool accept_expon = true;
    std::string res;
    
    if ( ! is.good() )
        return "";
    
    int c = is.get();
    
    if ( c == '+' || c == '-' )
    {
        int d = is.peek();
        if ( d == EOF || !isdigit(d) )
        {
            is.unget();
            return "";
        }
        res.push_back((char)c);
        c = is.get();
    }
    
    while ( c != EOF )
    {
        if ( isdigit(c) )
            res.push_back((char)c);
        else if (( c == 'e' || c == 'E' ) && accept_expon )
        {
            accept_expon = false;
            int d = is.peek();
            // accept an optional sign character
            if ( isdigit(d) )
                res.push_back((char)c);
            else if ( d == '+' || d == '-' )
            {
                res.push_back((char)c);
                res.push_back((char)is.get());
            }
            else
                break;
        }
        else
            break;
        c = is.get();
    }
    
    if ( c != EOF )
        is.unget();
    
    VLOG("INTEGER |" << res << "|\n");
    return res;
}

/**
 Read an integer, or return false if this was not possible.
 
 Note that the value of `what` will not change, if the input fails.
 */
bool Tokenizer::get_integer(std::istream& is, unsigned& var)
{
    std::streampos isp = is.tellg();
    unsigned num;
    is >> num;
    if ( is.fail() )
    {
        is.clear();
        is.seekg(isp);
        return false;
    }
    if ( isspace(is.peek()) )
    {
        var = num;
        return true;
    }
    // declare error, if there is no spacing character:
    is.seekg(isp);
    throw InvalidParameter("an integer is expected");
}

/**
 Read an integer, or return false if that was not possible.
 
 Note that the value of `what` will not change, if the input fails.
 */
bool Tokenizer::get_integer(std::istream& is, int& var)
{
    std::streampos isp = is.tellg();
    int num;
    is >> num;
    if ( is.fail() )
    {
        is.clear();
        is.seekg(isp);
        return false;
    }
    if ( isspace(is.peek()) )
    {
        var = num;
        return true;
    }
    // declare error, if there is no spacing character:
    is.seekg(isp);
    throw InvalidParameter("an integer is expected");
}


std::vector<std::string> Tokenizer::split(std::string& str, char sep, bool get_empty_fields)
{
    std::vector<std::string> res;
    std::istringstream iss(str);
    while (!iss.eof())
    {
        std::string s;
        getline(iss, s, sep);
        if ( get_empty_fields || !s.empty() )
            res.push_back(s);
    }
    return res;
}

/**
 Split string `arg` into an integer, a space, and the remaining string.
 Any space after the integer is discarded. `arg` is truncated in the process.
 */
bool Tokenizer::split_integer(long& val, std::string& arg)
{
    char const* ptr = arg.c_str();
    char * end;
    errno = 0;
    long num = strtol(ptr, &end, 10);
    if ( !errno && end > ptr && ( *end==0 || isspace(*end) ) )
    {
        val = num;
        // skip any additional space characters:
        while ( isspace(*end) )
            ++end;
        // remove consumed characters:
        arg.erase(0, (size_t)(end-ptr));
        return true;
    }
    return false;
}

/**
 Split string `arg` into an integer, a space, and the remaining string.
 Any space after the integer is discarded. `arg` is truncated in the process.
 */
bool Tokenizer::split_integer(unsigned long& val, std::string& arg)
{
    char const* ptr = arg.c_str();
    char * end;
    errno = 0;
    unsigned long num = strtoul(ptr, &end, 10);
    if ( !errno && end > ptr && ( *end==0 || isspace(*end) ) )
    {
        val = num;
        // skip any additional space characters:
        while ( isspace(*end) )
            ++end;
        // remove consumed characters:
        arg.erase(0, (size_t)(end-ptr));
        return true;
    }
    return false;
}

/**
 Read a number specified in the standard (US) format with an optional '.':
 floating-point-constant: 
             [sign] fractional-constant [exponent-part]
             [sign] digit-sequence [exponent-part]
 fractional-constant: digit-sequence.digit-sequence
 digit-sequence:     [digit-sequence] digit
 digit         : one of [0123456789]
 exponent-part : ('e' or 'E') [sign] digit-sequence
 sign          : '+' or '–'
*/
std::string Tokenizer::get_real(std::istream& is)
{
    bool accept_point = true;
    bool accept_expon = true;
    std::string res;
    
    if ( ! is.good() )
        return "";

    int c = is.get();
    
    if ( c == '+' || c == '-' )
    {
        int d = is.peek();
        if ( d == EOF || !isdigit(d) )
        {
            is.unget();
            return "";
        }
        res.push_back((char)c);
        c = is.get();
    }

    while ( c != EOF )
    {
        if ( isdigit(c) )
            res.push_back((char)c);
        else if ( c == '.' && accept_point )
        {
            res.push_back((char)c);
            accept_point = false;
        }
        else if (( c == 'e' || c == 'E' ) && accept_expon )
        {
            accept_expon = false;
            // only accept integer within exponent
            accept_point = false;
            int d = is.peek();
            // accept an optional sign character
            if ( isdigit(d) )
                res.push_back((char)c);
            else if ( d == '+' || d == '-' )
            {
                res.push_back((char)c);
                res.push_back((char)is.get());
            }
            else
                break;
        }
        else
            break;
        c = is.get();
    }
    
    if ( c != EOF )
        is.unget();
    
    VLOG("REAL |" << res << "|\n");
    return res;
}


bool valid_hexadecimal(int c)
{
    return isxdigit(c) || c=='x';
}


std::string Tokenizer::get_hexadecimal(std::istream& is)
{
    return get_stuff(is, valid_hexadecimal);
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 get_token() reads the next 'token' in stream.
 It can be:
 - a symbol starting with an alpha character
 - a block enclosed by '{}', '()' and '""',
 - a number 
 - the new line character ('\n') if 'eat_line=true'
 - a single character (except '\n')
 */

std::string Tokenizer::get_token(std::istream& is, bool eat_line)
{
    int c = skip_space(is, false);
 
    if ( c == EOF )
        return "";
    
    if ( c == '\n' )
    {
        if ( !eat_line )
            return "";
        is.get();
        return "\n";
    }
    
    if ( isalpha(c) )
        return get_symbol(is);

    if ( block_delimiter(c) )
        return get_block_text(is, (char)is.get(), block_delimiter(c));
    
    c = is.get();
    int d = is.peek();

    if ( d == EOF )
        return std::string(1,c);
    
    if ( c == '0' && d == 'x' )
    {
        is.unget();
        return get_hexadecimal(is);
    }
    
    if ( isdigit(c) || (( c == '-' || c == '+' ) && isdigit(d)) )
    {
        is.unget();
        return get_real(is);
    }
    
    // anything else is one character long:
    VLOG(" ASCII |" << c << "|\n");
    return std::string(1,c);
}

//------------------------------------------------------------------------------

/**
 This will read a block, assuming that opening delimiter has been read already.
 It will read characters until the given closing delimiter `c_out` is found.
 if `c_in` is not zero, the block is returned with `c_in` and `c_out` at the
 first and last positions.
 if `c_in` is null, the character `c_out` is read but not appended to the result.
 This throws an exception if `c_out` is not found.
 */
std::string Tokenizer::get_block_text(std::istream& is, char c_in, const char c_out)
{
    assert_true(c_out);
    std::string res;
    res.reserve(16384);
    char c = 0;
    
    if ( c_in )
        res.push_back(c_in);
    is.get(c);
    
    while ( is.good() )
    {
        if ( c == c_out )
        {
            if ( c_in )
                return res+c;
            else
                return res;
        }
        else if ( block_delimiter(c) )
            res.append( get_block_text(is, c, block_delimiter(c)) );
        else if ( c == ')' || c == '}' )
            throw InvalidSyntax("unclosed delimiter '"+std::string(1,c_in)+"'");
#if ( 0 )
        else if ( c == COMMENT_START )
        {
            // Read comments as lines, to inactivate symbols within: ')' and '}'
            std::string line;
            std::getline(is, line);
        }
#endif
        else
            res.push_back(c);
        is.get(c);
    }
    
    throw InvalidSyntax("missing '"+std::string(1,c_out)+"'");
    return "";
}


/**
 This will skip spaces and new-lines until a character is found.
 If this character is equal to `c_in`, then the block is read and returned.
 Otherwise returns empty string "".
 
 @returns content of the block without delimiters
 */
std::string Tokenizer::get_block(std::istream& is, char c_in, bool or_die)
{
    assert_true(c_in);
    
    int c = skip_space(is, true);
    
    if ( c == c_in )
    {
        is.get();
        std::string res = get_block_text(is, 0, block_delimiter(c_in));
        VLOG("BLOCK |" << res << "|\n");
        return res;
    }

    if ( or_die )
        throw InvalidSyntax("syntax error (expected `{...}')");

    return "";
}


std::string Tokenizer::get_block(std::istream& is)
{
    int c = skip_space(is, true);
    
    if ( block_delimiter(c) )
        return get_block_text(is, (char)c, block_delimiter(c));
    
    return "";
}


std::string Tokenizer::strip_block(std::string const& arg)
{
    size_t s = arg.size();
    
    if ( s < 2 )
        return arg;
    
    char c = block_delimiter(arg[0]);
    if ( c )
    {
        if ( arg[s-1] != c )
            throw InvalidSyntax("mismatched enclosing symbols");
        return arg.substr(1, s-2);
    }
    return arg;
}


//------------------------------------------------------------------------------
#pragma mark -


std::string Tokenizer::get_until(std::istream& is, std::string what)
{
    std::string res;
    res.reserve(16384);
    unsigned d = 0;
    char c = 0;
    is.get(c);
    
    while ( is.good() )
    {
        if ( c == what[d] )
        {
            ++d;
            if ( what[d] == '\0' )
                break;
        }
        else
        {
            if ( d == 0 )
            {
                res.push_back(c);
            }
            else
            {
                res.push_back(what[0]);
                if ( d > 1 ) {
                    is.seekg(-(int)d, std::ios_base::cur);
                    d = 0;
                } else {
                    if ( c == what[0] )
                        d = 1;
                    else {
                        res.push_back(c);
                        d = 0;
                    }
                }
            }
        }
        is.get(c);
    }
    //std::clog << "get_until|" << res << std::endl;
    return res;
}


std::string Tokenizer::trim(std::string const& str, const std::string& ws)
{
    std::string::size_type s = str.find_first_not_of(ws);
    if ( s == std::string::npos )
        return std::string();
    std::string::size_type e = str.find_last_not_of(ws);
    return str.substr(s, 1+e-s);
}

