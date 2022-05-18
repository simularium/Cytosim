// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//Some error conditions are handled by throwing exceptions.
//here we define a very primite Exception class for cytosim

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "assert_macro.h"
#include <string>
#include <sstream>


/// This is used to align text in the error messages
extern const char PREF[];


/// A mechanism to handle errors (see C++ manual)
/**
 The exception carry a 'message' and associated 'info', which are both strings.
 The message is set by the constructor, and the info is set by the << operator.
 
 Usage: Throw an Exception (not a pointer), and catch a reference to an exception.
 This ensures proper memory managment (coordinated calls of constructor / destructor)
*/
class Exception 
{
protected:
    
    /// brief description of the issue
    std::string msg_;

    /// background information
    std::string info_;
    
public:
    
    /// Creator with empty message
    Exception()
    {
    }
    
    /// constructor with given message
    Exception(std::string const& m)
    {
        msg_ = m;
        //printf("Exception(%s)\n", msg.c_str());
    }
    
    /// return the message
    std::string brief()
    {
        return "Error, " + msg_ + ":";
    }
    
    /// return supplementary message
    std::string info() const
    {
        return info_;
    }

    /// return copy of the message composed of brief and info
    std::string what() const
    {
        if ( info_.size() > 0 )
            return msg_ + ":\n" + info_;
        return msg_;
    }

    /// return copy of the message
    char const* msg() const
    {
        return msg_.c_str();
    }
    
    /// change the message
    std::string message() const
    {
        return msg_;
    }

    /// change the message
    void message(const std::string& m)
    {
        msg_ = m;
    }
    
    /// concatenate `s` and `a` to build message
    template <typename A>
    Exception(const std::string& s, const A& a)
    {
        std::ostringstream oss;
        oss << s << a;
        msg_ = oss.str();
    }
    
    /// concatenate `s`, `a` and `b` to build message
    template <typename A, typename B>
    Exception(const std::string& s, const A& a, const B& b)
    {
        std::ostringstream oss;
        oss << s << a << b;
        msg_ = oss.str();
    }

    /// concatenate `s`, `a`, `b` and `c` to build message
    template <typename A, typename B, typename C>
    Exception(const std::string& s, const A& a, const B& b, const C& c)
    {
        std::ostringstream oss;
        oss << s << a << b << c;
        msg_ = oss.str();
    }

    /// concatenate `s`, `a`, `b`, `c` and `d` to build message
    template <typename A, typename B, typename C, typename D>
    Exception(const std::string& s, const A& a, const B& b, const C& c, const D& d)
    {
        std::ostringstream oss;
        oss << s << a << b << c << d;
        msg_ = oss.str();
    }

    /// append string to info
    Exception& operator << (const std::string& arg)
    {
        info_.append(arg);
        return *this;
    }
    
    /// append C-string to info
    Exception& operator << (const char arg[])
    {
        info_.append(arg);
        return *this;
    }

    /// append string-representation of `x` to info
    template<typename T>
    Exception& operator << (const T& x)
    {
        std::ostringstream oss;
        oss << x;
        *this << oss.str();
        return *this;
    }
};


//------------------------------------------------------------------------------
/// This class is thrown if a parameter value is invalid
class InvalidParameter : public Exception 
{
    
public:
    
    /// constructor
    InvalidParameter() : Exception()
    {
        //printf("new InvalidParameter [%s]\n", m.c_str());
    }
    
    /// constructor
    InvalidParameter(const std::string m) : Exception(m)
    {
        //printf("new InvalidParameter [%s]\n", m.c_str());
    }

    /// concatenate all arguments to build message
    template <typename A>
    InvalidParameter(const std::string& s, const A& a) : Exception(s, a) {}
    
    /// concatenate all arguments to build message
    template <typename A, typename B>
    InvalidParameter(const std::string& s, const A& a, const B& b) : Exception(s,a,b) {}
 
    /// concatenate all arguments to build message
    template <typename A, typename B, typename C>
    InvalidParameter(const std::string& s, const A& a, const B& b, const C&c) : Exception(s,a,b,c) {}

    /// concatenate all arguments to build message
    template <typename A, typename B, typename C, typename D>
    InvalidParameter(const std::string& s, const A& a, const B& b, const C& c, const D& d) : Exception(s,a,b,c,d) {}
};


//------------------------------------------------------------------------------
/// InvalidSyntax is thrown while parsing config file
class InvalidSyntax : public Exception 
{
    
public :
    
    /// constructor
    InvalidSyntax(std::string const& m) : Exception(m)
    {
        //printf("new InvalidSyntax [%s]\n", m.c_str());
    }
};

//------------------------------------------------------------------------------
/// InvalidIO is thrown during file Input/Output
class InvalidIO : public Exception 
{
    
public :
    
    /// constructor
    InvalidIO(const std::string m) : Exception(m)
    {
        //printf("new InvalidIO [%s]\n", m.c_str());
    }
};


#endif
