// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef  MESSAGES_H
#define  MESSAGES_H

#include <iostream>
#include <fstream>


/// This facility provides some control over output
/** F. Nedelec, 16.03.2018 */
namespace Cytosim
{
    /// a class representing an output stream
    class Output
    {
        /// prefix to all messages
        std::string   pref_;

        /// pointer to the current destination of output
        std::ostream* out_;
        
        /// file stream, if open
        std::ofstream ofs_;

        /// alias to /dev/null
        std::ofstream nul_;

        /// remaining number of output that will be performed
        unsigned cnt_;

    public:
        
        /// create stream directed to given stream with `max_output` allowed
        Output(std::ostream& os, unsigned n_out = 1<<16, std::string const& p = "") : pref_(p), out_(&os), cnt_(n_out)
        {
            nul_.open("/dev/null");
        }
        
        /// redirect output to given file
        void open(std::string const& filename)
        {
            ofs_.open(filename.c_str());
            out_ = &ofs_;
        }
        
        /// close file
        void close()
        {
            if ( ofs_.is_open() )
                ofs_.close();
            out_ = &std::cout;
        }
        
        /// return current output stream
        std::ostream* stream() const
        {
            return out_;
        }
        
        /// return current output
        operator std::ostream&()
        {
            return *out_;
        }
        
        /// flush
        void flush()
        {
            if ( out_ != &nul_ )
                out_->flush();
        }

        /// direct output to /dev/null
        void silent()
        {
            out_ = &nul_;
        }
        
        /// direct output to given stream
        void redirect(Output const& x)
        {
            out_ = x.stream();
        }
        
        /// std::ostream style output operator
        template < typename T >
        std::ostream& operator <<(T const& x)
        {
            if ( out_!=&nul_ && out_->good() && cnt_ )
            {
                --cnt_;
                (*out_) << pref_ << x;
                return *out_;
            }
            return nul_;
        }
        
        /// printf() style formatting
        void operator()(const char* fmt, ...);
        
    };
    
    /// for usual output
    extern Output out;

    /// for logs
    extern Output log;

    /// for warnings
    extern Output warn;

    /// suppress all output
    void all_silent();
}


/// a macro to print some text only once
#define LOG_ONCE(a) { static bool V=1; if (V) { V=0; Cytosim::log << a; } }

#endif
