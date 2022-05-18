// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "filewrapper.h"
#include "exceptions.h"
#include <sys/param.h>
#include <libgen.h>


//------------------------------------------------------------------------------
FileWrapper::FileWrapper()
{
    mFile = nullptr;
}


FileWrapper::FileWrapper(FILE * f, const char *path)
{
    mFile = f;
    if ( path )
        mPath = path;
}


FileWrapper::FileWrapper(const char* name, const char* mode)
{
    mFile = nullptr;
    open(name, mode);
}


FileWrapper::~FileWrapper()
{
    close();
}


//------------------------------------------------------------------------------
#pragma mark -

void FileWrapper::operator =(FILE * f)
{
    close();
    mFile = f;
}


int FileWrapper::open(const char* name, const char* mode)
{
    if ( name[0] == 0 )
        throw InvalidIO("an empty file name was specified");

    if ( mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a' )
        throw InvalidIO("invalid file opening mode");

    if ( mFile )
        close();
    
    /// remember the path
    mPath = name;
    
    mFile = fopen(name, mode);

    if ( !mFile )
    {
        if ( mode[0] == 'w'  ||  mode[0] == 'a' )
            throw InvalidIO("output file could not be opened");
        return 1;
    }
    
    if ( ferror(mFile) )
    {
        fclose(mFile);
        mFile = nullptr;
        throw InvalidIO("input file opened with errors");
    }
    
    return 0;
}


void FileWrapper::close()
{
    if ( mFile )
    {
        fflush(mFile);
        
        if ( mFile!=stdout  &&  mFile!=stderr )
        {
            if ( fclose(mFile) )
                throw InvalidIO("failed to close input file: fclose() is true");
        }
        mFile = nullptr;
    }
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 This will write a null-terminated C-string to output stream.
 If it is non-zero, character 'end' is appended.
 */
void FileWrapper::put_line(const char * str, bool end)
{
    size_t s = strlen(str);
    fwrite(str, 1, s, mFile);
    if ( end )
        putc('\n', mFile);
    //printf("* putline |%s|\n", str);
}


void FileWrapper::put_line(const std::string& str, bool end)
{
    put_line(str.c_str(), end);
}


std::string FileWrapper::get_line(const char end)
{
    std::string res;

    if ( ferror(mFile) )
        return res;

    const size_t CHK = 32;
    char buf[CHK];
    
    fpos_t pos;
    char * m;
    
    while ( !feof(mFile) )
    {
        fgetpos(mFile, &pos);
        size_t s = fread(buf, 1, CHK, mFile);
        
        // search for separator:
        m = (char*)memchr(buf, end, s);
        
        if ( m )
        {
            s = (size_t)( m - buf );
            res.append(buf, s);
            //reposition at end of line:
            fsetpos(mFile, &pos);
            if ( s+1 != fread(buf, 1, s+1, mFile) )
                throw InvalidIO("unexpected error");
            //fprintf(stderr,"-|%s|-\n", line.c_str());
            break;
        }
        res.append(buf, s);
    }
    
    return res;
}


void FileWrapper::put_characters(std::string const& str, size_t cnt)
{
    // write characters from 'str':
    size_t s = std::min(cnt, str.size());
    cnt -= fwrite(str.c_str(), 1, s, mFile);
    // fill the rest with '0'
    const size_t CHK = 32;
    char buf[CHK] = { 0 };
    while ( cnt > 0 )
        cnt -= fwrite(buf, 1, std::min(CHK, cnt), mFile);
}


std::string FileWrapper::get_characters(size_t cnt)
{
    std::string res;
    const size_t CHK = 32;
    char buf[CHK] = { 0 };
    
    while ( cnt > 0 )
    {
        size_t s = fread(buf, 1, std::min(CHK, cnt), mFile);
        res.append(buf, s);
        cnt -= s;
    }

    // trim trailing zeros:
    std::string::size_type e = res.find((char)0);
    return res.substr(0, e);
}


std::string FileWrapper::get_word()
{
    std::string res;
    int c = get_char();
    while ( isspace(c) )
        c = get_char();
    do {
        res.push_back(c);
        c = get_char();
    } while ( !isspace(c) );
    return res;
}

/**
 This will search for the string and position the stream
 at the first character of the match.
 If the `str` is not found, the stream will be positionned
 at the end of the file, with a eof() state.
 
 The search may fail if `str` contains repeated sequences
 */
void FileWrapper::skip_until(const char * str)
{
    const size_t CHK = 64;
    char buf[CHK+2];

    fpos_t pos, match;
    size_t off = 0;
    
    const char ccc = str[0];
    const char * s = str;
    const char * b;

    while ( !eof() )
    {
        fgetpos(mFile, &pos);
        size_t nbuf = fread(buf, 1, CHK, mFile);
        
        if ( s == str )
        {
            // locate 'ccc' inside 'buf'
            b = (char*)memchr(buf, ccc, nbuf);
            if ( !b )
                continue;
            match  = pos;
            off = (size_t)(b - buf);
            ++s;
            ++b;
        }
        else
            b = buf;
        
        char *const end = buf + nbuf;
        
        while ( b < end )
        {
            //assert_true( s != str );
            if ( *b == *s )
            {
                ++s;
                if ( *s == 0 )
                {
                    fsetpos(mFile, &match);
                    if ( off != fread(buf, 1, off, mFile) )
                        throw InvalidIO("unexpected error");
                    return;
                }
            }
            else
            {
                s = str;
                b = (char*)memchr(b, ccc, nbuf-(size_t)(b - buf));
                if ( !b )
                    break;
                match  = pos;
                off = (size_t)(b - buf);
                ++s;
            }
            ++b;
        }
    }
}

