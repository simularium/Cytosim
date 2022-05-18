// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef PROPERTY_H
#define PROPERTY_H

#include "assert_macro.h"
#include <iostream>
#include <iomanip>
#include <string>

class Glossary;
class Simul;


/// A Property holds the parameters for a particular category of objects
/**
 A Property is a list of parameters associated with a class of objects in Cytosim.
 A Property is identified by:
     - category() indicating the class (eg. `fiber`, `hand`, `single`, etc.)
     - name() which is chosen by the user (eg.`actin`, `microtubule`).
 .
 The `name` should be unique globally, to avoid ambiguity in the config file.
 
 A few important methods handle the most critical operations:
     - clear() will reset parameters to their default values,
     - read() will input parameter from a Glossary
     - complete() will compute derived parameter values, and check their consistency
     - write() will save parameter values to a file.
 .
 
 Cytosim objects have a pointer `prop` to their associated Property.
 This pointer defines a class of Objects in Cytosim. If two objects A and B share
 the same Property, they are defacto of the same kind.
 */
class Property
{
private:
    
    /// disabled default constructor:
    Property();
    
    /// the name of the property
    std::string  name_;
    
    /// numerical identifier used in output file
    unsigned     number_;

    /// pad string by adding white-space on the right up to size 20
    static std::string format_(std::string const& str)
    {
        if ( str.size() < 20 )
            return " " + str + std::string(20-str.size(), ' ') + " = ";
        else
            return " " + str + " = ";
    }

public:
    
    /// constructor must provide a name
    explicit     Property(std::string const& n);

    /// destructor
    virtual     ~Property();
    
    //-------------------------------------------------------------------------------
    
    /// identifies the class of objects made with this Property
    virtual std::string category()        const { return "undefined"; }
    
    //-------------------------------------------------------------------------------
    
    /// return copy of name given to property
    std::string  name()                   const { return name_; }
    
    /// return name of property
    const char*  name_str()               const { return name_.c_str(); }

    /// change name
    void         rename(std::string const& n)   { name_ = n; }
        
    /// true if this->name() is `n`
    bool         is_named(std::string const& n) { return ( n == name_ ); }
    
    //-------------------------------------------------------------------------------
    
    /// index, unique among all Property of similar category()
    unsigned     number()                 const { return number_; }
    
    /// set index in the array of Properties
    void         renumber(unsigned x)           { number_ = x; }
    
    //-------------------------------------------------------------------------------
    
    /// clear parameters to default values
    virtual void clear() = 0;
    
    /// return new object of the same class with identical parameter values
    /**
     The new object is created with `new` and should be destroyed with `delete`.
     Together with `clear()`, this function allows to know which parameters have a
     values that are different from the default values.
     */
    virtual Property* clone() const = 0;
    
    /// true if at least one value is different from its default setting
    bool         modified() const;
    
    //-------------------------------------------------------------------------------
    
    /// set from a Glossary
    virtual void read(Glossary&) = 0;

    /// set from a `str` and indicate `msg` in errors/warnings
    void         read_string(std::string const& str, std::string const& msg);

    /// read a file specified by name
    void         read_file(const char filename[]);
    
    /// read a file specified by name
    void         read_file(std::string const& str) { read_file(str.c_str()); }
   
    //-------------------------------------------------------------------------------
    
    /// set variables derived from the parameters, and check consistency of values
    /**
     The arguments provide the global SimulProp, and the list of all known Property.
     Any Property created within this function should be added to `plist`.
     complete() is usually called after read()
     */
    virtual void complete(Simul const&) {}
    
    //-------------------------------------------------------------------------------

    /// formatted output of one parameter, one value
    template<typename C>
    static  void write_value(std::ostream& os, std::string const& name, C const& c)
    {
        os << format_(name) << c << ";\n";
    }

    /// formatted output of one array parameter, `cnt` values
    template<typename C>
    static  void write_value(std::ostream& os, std::string const& name, C const* c, int cnt)
    {
        assert_true( cnt > 0 );
        os << format_(name) << c[0];
        for ( int i = 1; i < cnt; ++i )
            os << ", " << c[i];
        os << ";\n";
    }

    /// formatted output of one parameter, two values
    template<typename C, typename D>
    static  void write_value(std::ostream& os, std::string const& name, C const& c, D const& d)
    {
        os << format_(name) << c << ", " << d << ";\n";
    }

    /// formatted output of one parameter, three values
    template<typename C, typename D, typename E>
    static  void write_value(std::ostream& os, std::string const& name, C const& c, D const& d, E const& e)
    {
        os << format_(name) << c << ", " << d << ", " << e << ";\n";
    }

    /// formatted output of one parameter, four values
    template<typename C, typename D, typename E, typename F>
    static  void write_value(std::ostream& os, std::string const& name, C const& c, D const& d, E const& e, F const& f)
    {
        os << format_(name) << c << ", " << d << ", " << e << ", " << f << ";\n";
    }
    
    //-------------------------------------------------------------------------------

    /// write values of all parameters stored in class
    virtual void write_values(std::ostream&) const = 0;
    
    /// write only values that differ from the ones specified in `ref`
    void         write_values_diff(std::ostream&, Property const* ref) const;

    /// if ( prune == true ), write values that differ from the default values
    void         write_values_diff(std::ostream&, bool prune) const;
    
    /// write header + data
    void         write(std::ostream&, bool prune = false) const;

};

/// printing operator
std::ostream& operator << (std::ostream&, const Property&);


#endif
