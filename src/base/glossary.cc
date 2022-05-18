// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "glossary.h"
#include "tokenizer.h"
#include "stream_func.h"
#include "print_color.h"
#include "filepath.h"
#include <fstream>
#include <cctype>
#include <iomanip>
#include "vector2.h"


// Use the second definition to get some reports:
#define VLOG0(ARG) ((void) 0)
//#define VLOG0(ARG) std::clog << ARG;

#define VLOG1(ARG) ((void) 0)
//#define VLOG1(ARG) std::clog << ARG;

#define VLOG2(ARG) ((void) 0)
//#define VLOG2(ARG) std::clog << ARG;


//------------------------------------------------------------------------------

Glossary::Glossary()
{
}

Glossary::Glossary(std::istream& in)
{
    read(in);
}

Glossary::Glossary(const std::string& arg)
{
    std::istringstream iss(arg);
    read(iss);
}


//------------------------------------------------------------------------------
#pragma mark - Formating

std::string format_value(std::string const& str)
{
    if ( std::string::npos != str.find_first_of(" ;") )
        return '(' + str + ')';
    else
        return str;
}


std::string format_count(size_t c)
{
    if ( c == 0 )
        return " (unused)";
    if ( c == 1 )
        return " (used once)";
    return " (used " + std::to_string(c) + " times)";
}
    

std::string format(Glossary::pair_type const& pair)
{
    std::string res = pair.first;
    if ( pair.second.size() > 0 )
    {
        res += " = " + format_value(pair.second[0].value_);
        for ( size_t i = 1; i < pair.second.size(); ++i )
            res += ", " + format_value(pair.second[i].value_);
        res += ";";
    }
    return res;
}

/**
 Write the usage-counter for each value.
 The width of each record will match what is printed by Glossary::write()
 */
std::string format_counts(Glossary::pair_type const& pair)
{
    std::string res = pair.first;
    if ( pair.second.size() > 0 )
    {
        res += " = " + pair.second[0].value_ + format_count(pair.second[0].count_);
        for ( size_t i = 1; i < pair.second.size(); ++i )
            res += ", " + pair.second[i].value_ + format_count(pair.second[i].count_);
        res += ";";
    }
    return res;
}


//------------------------------------------------------------------------------
#pragma mark - Access

bool Glossary::has_key(key_type const& k) const
{
    return ( mTerms.end() != mTerms.find(k) );
}


/**
 This will return 'false' if the key is not defined,
 and `true` if the key is defined without a value,
 and the value if the key was defined with a value.
 */
bool Glossary::use_key(key_type const& k)
{
    map_type::iterator w = mTerms.find(k);
    
    if ( w != mTerms.end() )
    {
        rec_type& rec = w->second;
        bool var = true;

        // if a value is defined, we return this value:
        if ( rec.size() > 0 )
        {
            val_type const& val = rec.at(0);
            if ( val.defined_ )
            {
                std::istringstream iss(val.value_);
                iss >> var;
                if ( iss.fail() )
                    var = false;
            }
        }
        mTerms.erase(w);
        return var;
    }
    return false;
}


void Glossary::clear(key_type const& key)
{
    map_type::iterator w = mTerms.find(key);
    
    if ( w != mTerms.end() )
        mTerms.erase(w);
}


void Glossary::clear_except(key_type const& key)
{
    map_type::iterator w = mTerms.find(key);
    
    if ( w == mTerms.end() )
        clear();
    else
    {
        rec_type rec = w->second;
        clear();
        mTerms[key] = rec;
    }
}


void Glossary::clear_counts() const
{
    for ( map_type::const_iterator i = mTerms.begin(); i != mTerms.end(); ++i )
    {
        for ( val_type const& v : i->second )
            v.count_ = 0;
    }
}


Glossary Glossary::extract(key_type const& key) const
{
    Glossary res;
    map_type::const_iterator w = mTerms.find(key);
    
    if ( w != mTerms.end() )
        res.mTerms[key] = w->second;
    
    return res;
}


Glossary Glossary::extract_unused() const
{
    Glossary res;
    for ( map_type::const_iterator i = mTerms.begin(); i != mTerms.end(); ++i )
    {
        bool used = false;
        for ( val_type const& v : i->second )
            if ( v.count_ == 0 )
                used = true;
        
        if ( !used )
            res.mTerms[i->first] = i->second;
    }
    return res;
}

//------------------------------------------------------------------------------
#pragma mark - Values access

size_t Glossary::nb_values(key_type const& k) const
{
    map_type::const_iterator w = mTerms.find(k);
    if ( w != mTerms.end() )
        return w->second.size();
    else
        return 0;
}


bool Glossary::has_value(key_type const& key, size_t inx) const
{
    map_type::const_iterator w = mTerms.find(key);
    if ( w != mTerms.end() )
        return inx < w->second.size();
    return false;
}


Glossary::rec_type * Glossary::values(key_type const& key)
{
    map_type::iterator w = mTerms.find(key);
    return ( w == mTerms.end() ) ? nullptr : &( w->second );
}


Glossary::rec_type const* Glossary::values(key_type const& key) const
{
    map_type::const_iterator w = mTerms.find(key);
    return ( w == mTerms.end() ) ? nullptr : &( w->second );
}


std::string Glossary::value(key_type const& key, size_t inx) const
{
    map_type::const_iterator w = mTerms.find(key);
    if ( w != mTerms.end() )
    {
        if ( inx < w->second.size() )
        {
            w->second[inx].count_++;
            return w->second[inx].value_;
        }
    }
    return "";
}


/**
 This is equivalement to value(key, inx) == val, except
 that the counter is incremented only if there is a match
 */
bool Glossary::value_is(key_type const& key, size_t inx, std::string const& val) const
{
    map_type::const_iterator w = mTerms.find(key);
    if ( w != mTerms.end() )
    {
        if ( inx < w->second.size() )
        {
            if ( w->second[inx].value_ == val )
            {
                w->second[inx].count_++;
                return true;
            }
        }
    }
    return false;
}

//------------------------------------------------------------------------------
#pragma mark - Value definitions

/**
 This reads a KEY followed by the assignement operator
 @returns
 - 0 if no valid key is found
 - 1 if the key is immediately followed by the '=' sign
 - 2 otherwise
*/
int Glossary::read_key(Glossary::pair_type& res, std::istream& is)
{
    std::string k = Tokenizer::get_symbol(is, false);

    if ( k.empty() )
        return 1;

    res.first = k;
    
    int op = Tokenizer::get_character(is);

    if ( op == '=' )
    {
        VLOG2("Glossary::KEY     |" << res.first << "| = \n");
        return 0;
    }
    
    VLOG2("Glossary::KEY     |" << res.first << "|\n");
    return 2;
}


/**
 push value at the back of `res.second`
 */
void Glossary::add_value(Glossary::pair_type& res, std::string& str, bool def)
{
    //remove any space at the end of the string:
    std::string val = Tokenizer::trim(str);
    
    VLOG2("Glossary::SET" << std::setw(20) << res.first << "[" << res.second.size() << "] = |" << val << "|\n");

    res.second.emplace_back(val, def);
}


/**
 return true if `c` can constitute a value.
 space are allowed since vectors are read as sets of space-separated values
*/
bool valid_value(const int c)
{
   return isalnum(c) || c==' ' || c=='/' || c == '#' || c==':' || c=='\t' || c=='_' || c=='.' || c=='+' || c=='-';
}


/**
 read one right-hand side value of an assignment
 return 1 if parsing should continue with the same key
 */
int Glossary::read_value(Glossary::pair_type& res, std::istream& is)
{
    // skip spaces, but do not eat lines
    int c = Tokenizer::get_character(is);
    bool delimited = 0;
    
    std::string k;
    if ( Tokenizer::block_delimiter(c) )
    {
        delimited = true;
        k = Tokenizer::get_block_text(is, 0, Tokenizer::block_delimiter(c));
        c = Tokenizer::get_character(is);
    }
    else
    {
        while ( valid_value(c) )
        {
            k.push_back(char(c));
            c = is.get();
        }
    }
    //std::clog << (char)c << "|";
        
    if ( c == EOF || c == '\n' || c == '\r' )
    {
        if ( k.size() || delimited )
            add_value(res, k, true);
        return 0;
    }
    
    if ( c == ';' )
    {
        add_value(res, k, k.size() || delimited);
        return 0;
    }

    if ( c == ',' )
    {
        add_value(res, k, k.size() || delimited);
        return 1;
    }

    if ( c == '%' )
    {
        if ( k.size() || delimited )
            add_value(res, k, true);
        Tokenizer::get_line(is);
        return 0;
    }
    
    if ( c == '\\' )
    {
        if ( k.size() || delimited )
            add_value(res, k, true);
        // go to next line:
        Tokenizer::skip_space(is, true);
        return 1;
    }
    
    is.unget();
    throw InvalidSyntax("syntax error: unexpected `"+std::string(1,(char)c)+"'");
}


/**
 The value of `mode' will determine how to handle duplicate values:
 - `0`, previous values are erased without warning,
 - `1`, a prexisting symbol in not altered, but no exception is thrown
 - `2`, an exception is thrown for any duplicate symbol
 */
void Glossary::add_entry(Glossary::pair_type& pair, int no_overwrite)
{
    VLOG0("Glossary::ENTRY" << pair.second.size() << "   " << pair << '\n');
    
    map_type::iterator w = mTerms.find(pair.first);
    
    if ( w == mTerms.end() )
    {
        mTerms.insert(pair);
    }
    else
    {
        // the key already exists:
        rec_type & rec = w->second;
        // check every value of the argument:
        for ( size_t i = 0; i < pair.second.size(); ++i )
        {
            if ( rec.size() <= i )
                rec.push_back(pair.second[i]);
            else
            {
                // the record already exists:
                if ( !rec[i].defined_  ||  !no_overwrite )
                    rec[i] = pair.second[i];
                else if ( pair.second[i].value_ != rec[i].value_  &&  no_overwrite > 1 )
                    throw InvalidSyntax("conflicting definition: "+format(*w)+" and "+format(pair));
            }
        }
    }
}


/// define one value for the key at specified index: `key[inx]=val`.
void Glossary::define(key_type const& key, size_t inx, const std::string& arg)
{
    std::string val = Tokenizer::trim(arg);
    map_type::iterator w = mTerms.find(key);
    
    if ( w == mTerms.end() )
    {
        if ( inx > 0 )
            throw InvalidSyntax("index out of range in Glossary::define");
        // add new key and its value at index 0:
        mTerms[key].emplace_back(val, true);

        VLOG1("Glossary::DEFINE    " << key << " = |" << val << "|\n");
    }
    else
    {
        // add new value to existing key:
        rec_type & rec = w->second;
        
        if ( rec.size() > inx )
            rec[inx] = val_type(val, true);
        else if ( rec.size() == inx )
            rec.emplace_back(val, true);
        else
            throw InvalidSyntax("index out of range in Glossary::define");
        
        VLOG1("Glossary::DEFINE    " << key << "[" << inx << "] = |" << val << "|\n");
    }
}


/**
 This should be equivalent to read('k = rhs')
 */
void Glossary::add_value(key_type const& key, const std::string& arg)
{
    std::string val = Tokenizer::trim(arg);
    map_type::iterator w = mTerms.find(key);
    // define a new key, or add value to existing key:
    if ( w == mTerms.end() )
        mTerms[key].emplace_back(val, true);
    else
        w->second.emplace_back(val, true);
}


/**
 This should be equivalent to read('k = rhs')
 */
void Glossary::define(key_type const& key, const std::string& rhs)
{
    define(key, 0, rhs);
}


//------------------------------------------------------------------------------
#pragma mark - Output


void Glossary::write(std::ostream& os, std::string const& prefix) const
{
    for ( map_type::const_iterator i = mTerms.begin(); i != mTerms.end(); ++i )
    {
        os << prefix << format(*i);
        std::endl(os);
    }
}


std::ostream& operator << (std::ostream& os, Glossary::pair_type const& arg)
{
    os << "  " << format(arg);
    return os;
}


std::ostream& operator << (std::ostream& os, Glossary const& arg)
{
    arg.write(os, "  ");
    return os;
}

//------------------------------------------------------------------------------
#pragma mark - Input

/**
@copydetails Glossary::add_entry
 */

void Glossary::read_entry(std::istream& is, int no_overwrite)
{
    int c = Tokenizer::skip_space(is, true);
    std::streampos isp = is.tellg();

    if ( c == EOF )
        return;
        
    // skip comments:
    if ( c == '%' )
    {
        std::string skip;
        std::getline(is, skip);
        return;
    }
    
    pair_type pair;
    try
    {
        if ( read_key(pair, is) )
            throw InvalidParameter("syntax error");
        
        while ( read_value(pair, is) );
        
        if ( pair.second.empty() )
            throw InvalidSyntax("expected value in assignement");
    }
    catch( Exception& e )
    {
        e << StreamFunc::marked_line(is, isp, PREF);
        throw;
    }
    
    add_entry(pair, no_overwrite);
}


/**
 @copydetails Glossary::add_entry
 */
void Glossary::read(std::istream& is, int no_overwrite)
{
    std::istream::sentry s(is);
    if ( s )
        while ( is.good() )
            read_entry(is, no_overwrite);
}


void Glossary::read(std::string const& str, int no_overwrite)
{
    VLOG2("Glossary::READ STR |" << str << "|\n");
    std::istringstream iss(str);
    read(iss, no_overwrite);
}


void Glossary::read_file(const char path[], int no_overwrite)
{
    std::ifstream is(path, std::ifstream::in);
    if ( is.good() )
        read(is, no_overwrite);
    else
        throw InvalidIO("could not open Glossary file");
    is.close();
}


/**
 This is useful to parse the command-line strings given to main().
 
 The following syntax will be accepted:
 FILE.EXT
 and recorded as:
 EXT = FILE.EXT
 
 Strings corresponding to existing directories
 */
void Glossary::read_string(const char arg[], int no_overwrite)
{
    pair_type pair;
    VLOG0("Glossary::ARG      |" << arg << "|\n");
    if ( strchr(arg, '=') )
    {
        /*
         Here is a key specified with one of more value:
         */
        std::istringstream iss(arg);
        if ( 0 == read_key(pair, iss) )
        {
            while ( read_value(pair, iss) );
            if ( pair.second.empty() )
                throw InvalidSyntax("expected value in assignement");
            add_entry(pair, no_overwrite);
        }
    }
    else
    {
        // Handle a key specified without any value
        if ( FilePath::is_dir(arg) )
            add_value("directory", arg);
        else
        {
            /*
             Find last occurence of '.' to identify a potential file name,
             while anything else is treated as a orphan string */
            char const* c = strrchr(arg, '.');
            if ( c )
                add_value(c, arg);
            else
            {
                pair.first = arg;
                add_entry(pair, no_overwrite);
            }
        }
    }
}


/**
 This is useful to parse the command-line strings given to main().
 
 The following syntax will be accepted:
 FILE.EXT
 and recorded as:
 EXT = FILE.EXT
 
 Strings corresponding to existing directories
 */
int Glossary::read_strings(int argc, char* argv[], int no_overwrite)
{
    int res = 0;
    for ( int i = 0; i < argc; ++i )
    {
        try
        {
            read_string(argv[i], no_overwrite);
        }
        catch( Exception & e )
        {
            print_magenta(std::cerr, e.brief());
            std::cerr << e.info() << '\n';
            res = 1;
        }
    }
    return res;
}


std::istream& operator >> (std::istream& is, Glossary& glos)
{
    glos.read(is);
    return is;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 builds the warning message in `msg'.
 
 @returns:
 - 4 if the parameter was not read
 - 2 if one of the value was not read
 - 1 if some of the value were used multiple times
 - 0 otherwise
 .
 */
int Glossary::warning(Glossary::pair_type const& pair, std::string& msg, size_t threshold)
{
    int code = 0;
    const rec_type& rec = pair.second;
        
    for ( size_t i = 0; i < rec.size(); ++i )
    {
        val_type const& val = rec[i];
        if ( val.count_ > 0 )
            code |= 4;  // one value used
        if ( !val.count_ && val.defined_ )
            code |= 2;  // one value not used
        else if ( val.count_ > threshold )
            code |= 1;  // one value overused
    }
    
    code ^= 4;  // invert highest bit
    
    if ( code & 4 )
        msg = "Warning, the parameter `" + pair.first + "' was ignored";
    else if ( code & 2 )
        msg = "Warning, a value was ignored: " + format_counts(pair);
    else if ( code & 1 )
        msg = "Warning, some value might have been overused: " + format_counts(pair);
    
    return code;
}


/**
 @returns the type of warning associated with entire set of terms
 If the return value is not zero, a message was printed to 'os', and
 at least a terminating '\n' should be printed to 'os' by the calling function.
 */
int Glossary::has_warning(std::ostream& os, size_t threshold) const
{
    int res = 0;
    std::string msg;
    for ( map_type::const_iterator i = mTerms.begin(); i != mTerms.end(); ++i )
    {
        int val = warning(*i, msg, threshold);
        if ( val )
        {
            if ( res ) os.put('\n');
            print_yellow(os, msg);
            res |= val;
        }
    }
    return res;
}


void Glossary::print_warning(std::ostream& os, size_t threshold, std::string const& msg) const
{
    if ( has_warning(os, threshold) )
        os << msg;
}

//------------------------------------------------------------------------------

/**
 This copies the string, removing spaces
*/
template <>
void Glossary::set_value(std::string& var, key_type const& key, std::string const& val)
{
    //var = Tokenizer::trim(val);
    var = val;
    VLOG2("Glossary::SET STR   " << key << " = |" << var << "|\n");
}


/**
 This reads a floating point value,
 also accepting 'inf', '+inf' and '-inf' for INFINITY values
 */
template <>
void Glossary::set_value(float& var, key_type const& key, std::string const& val)
{
/*
    // Infinite values are normally handled by std::strtof()
    if ( val == "inf" || val == "+inf" )
    {
        var = INFINITY;
        return;
    }
    
    if ( val == "-inf" )
    {
        var = -INFINITY;
        return;
    }
*/
    char const* ptr = val.c_str();
    char * end = nullptr;
    float num = strtof(ptr, &end);
    if ( end == ptr || not_space(end) )
        throw InvalidSyntax("could not set scalar value `"+key+"' from `"+val+"'");
    var = num;
}

/**
 This reads a floating point value,
 also accepting 'inf', '+inf' and '-inf' for INFINITY values
 */
template <>
void Glossary::set_value(double& var, key_type const& key, std::string const& val)
{
/*
    // Infinite values are normally handled by std::strtod()
    if ( val == "inf" || val == "+inf" )
    {
        var = INFINITY;
        return;
    }
    
    if ( val == "-inf" )
    {
        var = -INFINITY;
        return;
    }
*/
    char const* ptr = val.c_str();
    char * end = nullptr;
    double num = strtod(ptr, &end);
    if ( end == ptr || not_space(end) )
        throw InvalidSyntax("could not set scalar value `"+key+"' from `"+val+"'");
    var = num;
}
