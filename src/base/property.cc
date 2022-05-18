// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "property.h"
#include "property_list.h"
#include "tokenizer.h"
#include "glossary.h"
#include "stream_func.h"
#include <sstream>
#include <fstream>

//------------------------------------------------------------------------------

Property::Property(const std::string& n) : name_(n), number_(0)
{
    //std::clog << "new Property `" << mName << "'\n";
}


Property::~Property()
{
    //std::clog << "del Property `" << mName << "'\n";
}


//------------------------------------------------------------------------------
/**
 parse string `str` to set values of the property.
*/
void Property::read_string(std::string const& str, std::string const& msg)
{
    if ( str.size() > 0 )
    {
        //std::clog << "reading " << msg << "=(" << str << ")\n";
        try {
            Glossary glos(str);
            read(glos);
            if ( glos.has_warning(std::cerr) )
                std::cerr << " in `" << msg << "'\n";
        } catch(Exception & e) {
            std::clog << msg << ": " << e.what() << std::endl;
        }
    }
}


void Property::read_file(char const* filename)
{
    std::ifstream is(filename, std::ifstream::in);
    Glossary glos(is);
    read(glos);
}


//------------------------------------------------------------------------------

void Property::write_values_diff(std::ostream& os, Property const* def) const
{
    assert_true(def);
    std::stringstream val, ref;
    def->write_values(ref);
    write_values(val);
    StreamFunc::diff_stream(os, val, ref);
}


void Property::write_values_diff(std::ostream& os, const bool prune) const
{
    if ( prune )
    {
        Property * def = clone();
        if ( def )
        {
            def->clear();
            write_values_diff(os, def);
            delete(def);
            return;
        }
    }
    write_values(os);
}


bool Property::modified() const
{
    Property * def = clone();
    if ( def )
    {
        std::ostringstream oss;
        def->clear();
        def->write_values(oss);
        std::string str = oss.str();
        delete(def);
        oss.str("");
        write_values(oss);
        return str.compare(oss.str());
    }
    return true;
}


//------------------------------------------------------------------------------

/**
 This outputs the Properties in this format:

     set CATEGORY name
     {
       property_number = INTEGER
       key = values
       ...
     }
or
     set name display
     {
       property_number = INTEGER
       key = values
       ...
     }

 */
void Property::write(std::ostream& os, const bool prune) const
{
    /* Check for compound category, eg 'fiber:display'  */
    std::string cat = category();
    std::string::size_type pos = cat.find(':');
    if ( pos != std::string::npos )
        os << "\nset " << name_ << ' ' << cat.substr(pos+1);
    else
        os << "\nset " << category() << ' ' << name_;
    os << "\n{\n";
    if ( number() > 0 )
        write_value(os, "property_number", number_);
    write_values_diff(os, prune);
    os << "}\n";
}


std::ostream& operator << (std::ostream& os, const Property& p)
{
    p.write(os, 0);
    return os;
}


