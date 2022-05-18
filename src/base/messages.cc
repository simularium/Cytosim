// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "messages.h"
#include <cstdarg>

namespace Cytosim
{
    /// alias to standard output
    Output out(std::cout);
    
    /// for logs
    Output log(std::clog);
    
    /// for warnings
    Output warn(std::cerr, 32U, "WARNING: ");
    
    /// output operator with `printf()` syntax and flush
    void Output::operator()(const char* fmt, ...)
    {
        char str[2048] = { 0 };
        va_list args;
        va_start(args, fmt);
        vsnprintf(str, sizeof(str), fmt, args);
        va_end(args);
        operator<<(str);
        flush();
    }
    
    /// turn all output off
    void all_silent()
    {
        Cytosim::out.silent();
        Cytosim::log.silent();
        Cytosim::warn.silent();
    }
}
