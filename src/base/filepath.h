// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include <vector>
#include <string>

/// Functions to handle UNIX-like file/directory paths
namespace FilePath
{
    /// open C-file
    FILE * open_file(const char path[], const char mode[2]);
    
    /// return path to current working directory
    std::string get_cwd();

    /// true if 'path' is an existing readable file
    bool is_file(const char path[]);
    
    /// true if 'path' is an existing directory
    bool is_dir(const char path[]);
    
    /// change current working directory
    /** returns error code: 0 = success */
    int change_dir(const char path[]);

    /// change current working directory, creating it, if absent and 'make==true'
    /** returns file descriptor of current working directory */
    int change_dir(const char path[], bool make);
    
    /// change current working directory, using file descriptor returned by change_dir()
    void change_dir(int);
    
    /// change current working directory
    inline int change_dir(std::string const& str) { return change_dir(str.c_str()); }
    
    /// change current working directory
    inline int change_dir(std::string const& str, bool make) { return change_dir(str.c_str(), make); }

    /// true if 'path' is an existing directory
    inline bool is_dir(std::string const& str) { return is_file(str.c_str()); }

    /// true if 'path' is an existing readable file
    inline bool is_file(std::string const& str) { return is_file(str.c_str()); }

    /// create new directory, if it does not exists already
    int make_dir(const char name[]);
    
    /// list content of directory
    std::vector<std::string> list_dir(const char path[]);
    
    /// list files in directory `path` that have a name finishing with 'ext'
    std::vector<std::string> list_dir(const char path[], std::string const& ext);
   
    /// extract the directory part from the given path
    std::string dir_part(const char path[]);

    /// extract the file part from the given path
    std::string file_part(const char path[]);

    /// complete the file name using the given directory
    std::string full_name(std::string const& path, std::string const& file);

}

