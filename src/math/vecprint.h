// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECPRINT_H
#define VECPRINT_H

#include <iostream>
#include <iomanip>
#include <cmath>


/// Templated functions to print Vectors and Matrices with minimal formatting
namespace VecPrint
{
    /// print 'len' components of 'vec[]' on a line
    template< typename T >
    void print(FILE * file, size_t len, const T* vec, int digits = 2, size_t dim = 0)
    {
        if ( !vec || len == 0 )
            fprintf(file, " void");
        else
        {
            char fmt[32];
            snprintf(fmt, sizeof(fmt), " %%%i.%if", digits+5, digits);
            for ( size_t i = 0; i < len; ++i )
            {
                fprintf(file, fmt, vec[i]);
                if ( dim && ( dim-1 == i % dim )) putc(39, file);
            }
        }
    }

    /// print 'len' components of 'vec[]' on a line
    template< typename T >
    std::ostream& print(std::ostream& os, size_t len, const T* vec, int digits = 2)
    {
        if ( !vec || len == 0 )
            os << " void";
        else
        {
            char str[32], fmt[32];
            snprintf(fmt, sizeof(fmt), " %%%i.%if", digits+4, digits);
            for ( size_t i = 0; i < len; ++i )
            {
                snprintf(str, sizeof(str), fmt, vec[i]);
                if ( i % 3 )
                    os << str;
                else
                    os << " " << str;
            }
        }
        os.flush();
        return os;
    }
    
    
    /// print 'len' components of 'vec[]' on separate lines
    template< typename T >
    std::ostream& dump(std::ostream& os, size_t len, const T* vec, int digits = 8)
    {
        if ( !vec || len == 0  )
            os << " void";
        else
        {
            char str[32], fmt[32];
            snprintf(fmt, sizeof(fmt), " %%%i.%ie", 9, digits);
            for ( size_t i = 0; i < len; ++i )
            {
                snprintf(str, sizeof(str), fmt, vec[i]);
                os << str << '\n';
            }
        }
        os.flush();
        return os;
    }
    
    
    /// print matrix `mat[]` of size 'lin*col', and leading dimension `ldd` with precision 'digits'
    template< typename T >
    void print(std::ostream& os, size_t lin, size_t col, const T* mat, size_t ldd, int digits = 3)
    {
        if ( !mat || lin == 0 || col == 0  )
            os << " void";
        else
        {
            const T threshold = pow(0.1, digits);
            char str[32] = { 0 }, zer[32] = { 0 }, fmt[32] = " %4.0f";
            
            { // build format strings:
                snprintf(fmt, sizeof(fmt), " %%%i.%if", digits+5, digits);
                snprintf(zer, sizeof(zer), fmt, 0.0);
                bool dot = false; char * d = zer;
                for ( char * c = zer; *c; ++c )
                {
                    if ( *c == '0' ) { *c = ' '; d = c; }
                    dot |= ( *c == '.' );
                }
                if ( !dot ) *d = '.';
            }
            
            for ( size_t ii = 0; ii < lin; ++ii )
            {
                for ( size_t jj = 0; jj < col; ++jj )
                {
                    T val = mat[ii+ldd*jj];
                    if ( fabs(val) < threshold )
                        os << zer;
                    else
                    {
                        snprintf(str, sizeof(str), fmt, mat[ii+ldd*jj]);
                        os << str;
                    }
                }
                os << '\n';
            }
        }
        std::endl(os);
    }
    
    /// print matrix in sparse format: line_index, column_index, value
    template< typename T >
    void sparse(std::ostream& os, size_t lin, size_t col, const T* mat, size_t ldd, int digits = 8, T threshold = 0)
    {
        if ( !mat || lin== 0 || col == 0 )
            os << " void";
        else
        {
            char str[64], fmt[64];
            snprintf(fmt, sizeof(fmt), " %%3i %%3i %%9.%if\n", digits);
            for (size_t ii = 0; ii < lin; ++ii )
                for (size_t jj = 0; jj < col; ++jj )
                {
                    T val = mat[ii+ldd*jj];
                    if ( fabs(val) > threshold )
                    {
                        snprintf(str, sizeof(str), fmt, ii, jj, val);
                        os << str;
                    }
                }
        }
        std::endl(os);
    }
    
    
    /// print a matrix in sparse format, but adding `off` to all line and column indices
    template< typename T >
    void sparse_off(std::ostream& os, size_t lin, size_t col, const T* mat, size_t ldd, size_t off, int digits = 8)
    {
        if ( !mat || lin == 0 || col == 0 )
            os << " void";
        else
        {
            char str[32], fmt[32];
            snprintf(fmt, sizeof(fmt), " %%9.%if\n", digits);
            for (size_t ii = 0; ii < lin; ++ii )
                for (size_t jj = 0; jj < col; ++jj )
                {
                    snprintf(str, sizeof(str), fmt, mat[ii+ldd*jj]);
                    os << ii+off << " " << jj+off << str;
                }
        }
        std::endl(os);
    }
    
    /// print matrix `mat[]` of size lin*col, and leading dimension `ldd` with precision 'digits'
    template< typename T >
    void image(std::ostream& os, size_t lin, size_t col, const T* mat, size_t ldd, T scale)
    {
        if ( !mat || lin == 0 || col == 0 )
            os << " void";
        else
        {
            char str[] = ".:+*hTM$";
            
            const T threshold = 0.01 * scale;
            for ( size_t ii = 0; ii < lin; ++ii )
            {
                os << '|';
                for ( size_t jj = 0; jj < col; ++jj )
                {
                    T val = mat[ii+ldd*jj];
                    if ( val != val )
                        os << '@';
                    else if ( val < threshold )
                        os << ' ';
                    else
                    {
                        int x = std::max(7, 2 + log10( fabs(val) / scale ));
                        os << str[x];
                    }
                }
                os << "|\n";
            }
        }
        std::endl(os);
    }
}

#endif
