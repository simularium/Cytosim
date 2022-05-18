// Cytosim was created by Francois Nedelec. Copyright 2019 Cambridge University.
//
//  evaluator.h
//
//  Created by Francois Nedelec on 08/02/2019.
//  Copyright 2019 Cambridge University. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>

/// a minimal math expression evaluator
/**
 This can evaluate boolean expressions like `X^2 + (Y-3)^3 < 4'
 To support more fancy expressions, we could link here `tinyexpr`:
 https://codeplea.com/tinyexpr
*/
class Evaluator
{
public:
    
    /// variable names must be a single letter
    typedef std::pair<int, real> variable;

    /// list of variables
    typedef std::vector<variable> variable_list;
    
    static void print_variables(std::ostream& os, variable_list const& list)
    {
        os << "Known variables:\n";
        for ( variable const& v : list )
            os << "   " << (char)v.first << " = " << v.second << "\n";
    }

private:
    
    /// pointer
    char const* ptr;
    
    /// list of variables
    const variable_list variables_;
    
    
    void skip_space()
    {
        while ( isspace(*ptr) )
            ++ptr;
    }

    real value_(char c)
    {
        if ( c == 0 )
            throw InvalidSyntax("empty variable?");
        for ( variable const& v : variables_ )
        {
            if ( c == v.first || toupper(c) == v.first )
                return v.second;
        }
        print_variables(std::clog, variables_);
        throw InvalidSyntax("unknown variable '"+std::string(1,c)+"'");
        return 0;
    }
    
    real number_()
    {
        errno = 0;
        char * end = nullptr;
        real d = strtod(ptr, &end);
        if ( errno )
            throw InvalidSyntax("expected a number");
        ptr = end;
        //std::clog << "number: " << d << "  " << ptr << "\n";
        return d;
    }
    
    real factor_()
    {
        //std::clog << "factor: " << ptr << "\n";
        skip_space();
        char c = *ptr;
        if ( '0' <= c && c <= '9' )
            return number_();
        else if ( c == '(' )
        {
            ++ptr; // '('
            //std::clog << " (   " << ptr << "\n";
            real result = expression_();
            skip_space();
            if ( *ptr != ')' )
                throw InvalidSyntax("missing closing parenthesis");
            ++ptr; // ')'
            return result;
        }
        else if ( c == '-' )
        {
            ++ptr;
            return -factor_();
        }
        else
            return value_(*ptr++);
    }
    
    real term_()
    {
        //std::clog << "term: " << ptr << "\n";
        real result = factor_();
        while ( 1 )
        {
            skip_space();
            char c = *ptr;
            if ( c == '*' )
            {
                ++ptr;
                result *= factor_();
            }
            else if ( c == '/' )
            {
                ++ptr;
                result /= factor_();
            }
            else if ( c == '^' )
            {
                ++ptr;
                result = pow(result, factor_());
            }
            else
                return result;
        }
    }
    
    real expression_()
    {
        //std::clog << "expression: " << ptr << "\n";
        real result = term_();
        while ( 1 )
        {
            skip_space();
            char c = *ptr;
            if ( c == '+' )
            {
                ++ptr;
                result += term_();
            }
            else if ( c == '-' )
            {
                ++ptr;
                result -= term_();
            }
            else if ( c == '<' )
            {
                ++ptr;
                return ( result < term_() );
            }
            else if ( c == '>' )
            {
                ++ptr;
                return ( result > term_() );
            }
            else if ( c == '&' )
            {
                ++ptr;
                real rhs = term_();
                return (( result != 0 ) && ( rhs != 0 ));
            }
            else if ( c == '|' )
            {
                ++ptr;
                real rhs = term_();
                return (( result != 0 ) || ( rhs != 0 ));
            }
            else
                return result;
        }
    }
    
public:
    
    Evaluator(std::initializer_list<variable> const& v) : variables_(v)
    {
        //print_variables(std::clog, variables_);
    }
    
    real evaluate(char const* str)
    {
        ptr = str;
        real res = expression_();
        //std::clog << "evaluate(" << str << ") = " << res << "\n";
        return res;
    }
};
