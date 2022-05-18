// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "property_list.h"
#include "exceptions.h"
#include <iomanip>

void PropertyList::erase()
{
    for ( Property const* i : vec_ )
        delete(i);
    vec_.clear();
}

/**
 Add a property to the list. If ( p == 0 ) nothing is done.

 This function sets the index of `p` to follow the Properties of the same category,
 that are already present in the list.
 */
void PropertyList::deposit(Property * p)
{
    if ( p )
    {
        size_t cnt = 0;
        for ( Property* i : vec_ )
        {
            if ( i->category() == p->category() )
                ++cnt;
            if ( i->name() == p->name() )
                throw InvalidParameter("Property '"+p->name()+"' is already defined");
        }
                
        vec_.push_back(p);
        p->renumber(cnt+1);
    }
}


/**
 The size of the array will be reduced by one
 */
void PropertyList::remove(Property const* val)
{
    const_iterator last = vec_.end();
    for (iterator i = vec_.begin(); i != last; ++i)
    {
        if ( *i == val )
        {
            vec_.erase(i);
            return;
        }
    }
}


size_t PropertyList::size(std::string const& cat) const
{
    size_t res = 0;
    
    for ( Property const* i : vec_ )
        if ( i->category() == cat )
            ++res;
    
    return res;
}

/*
Property * PropertyList::operator[] (const size_t n) const
{
    if ( n >= vec_.size() )
    {
        InvalidSyntax e("out-of-range");
        e << " index " << n << " ( list-size = " << vec_.size() << " )";
        throw e;
    }
    return vec_[n];
}
*/

//-------------------------------------------------------------------------------

void PropertyList::for_each(void func(Property *)) const
{
    //std::clog << "Running function for "<<vec_.size()<<" properties"<<std::endl;
    for ( Property* i : vec_ )
        func(i);
}

void PropertyList::complete(Simul const& sim) const
{
    for ( Property* i : vec_ )
        i->complete(sim);
}

//-------------------------------------------------------------------------------
#pragma mark -

/** 
 returns the first Property named as 'nom' or zero
 */
Property * PropertyList::find(std::string const& nom) const
{
    //std::clog << this << "->find(" << nom << ")" << std::endl;
    for ( Property* i : vec_ )
    {
        if ( i->name() == nom )
            return i;
    }
    
    return nullptr;
}


/**
 returns the first Property named as 'nom'
 */
Property * PropertyList::find_or_die(std::string const& nom) const
{
    //std::clog << this << "->find_or_die(" << nom << ")" << std::endl;
    Property * p = find(nom);

    if ( !p )
    {
        InvalidSyntax e("unknown class `"+nom+"'");
        e << all_names(PREF);
        throw e;
    }
    return p;
}


/**
 returns the first match
 */
Property * PropertyList::find(std::string const& cat, std::string const& nom) const
{
    //std::clog << this << "->find(" << cat << ", " << nom << ")" << std::endl;

    for ( Property* i : vec_ )
    {
        if ( i->category()==cat  &&  i->name()==nom )
            return i;
    }
    
    return nullptr;
}


Property * PropertyList::find(std::string const& cat, const size_t num) const
{
    //std::clog << this << "->find(" << cat << ", " << idx << ")" << std::endl;
    if ( num <= 0 )
        return nullptr;
    
    for ( Property* i : vec_ )
        if ( i->category()==cat  &&  i->number()==num )
            return i;
    
    return nullptr;
}


Property * PropertyList::find_or_die(std::string const& cat, std::string const& nom) const
{
    Property * res = find(cat, nom);
    
    if ( !res )
    {
        InvalidSyntax e("unknown "+cat+" class `"+nom+"'");
        e << all_names(PREF);
        throw e;
    }
    
    return res;
}


Property * PropertyList::find_or_die(std::string const& cat, const size_t num) const
{
    Property * res = find(cat, num);
    
    if ( !res )
    {
        InvalidSyntax e("unknown class `"+cat+std::to_string(num)+"'");
        e << all_names(PREF);
        throw e;
    }
    
    return res;
}


PropertyList PropertyList::find_all(std::string const& cat) const
{
    //std::clog << this << "->find_all(" << cat << ") " << std::endl;

    PropertyList res;
    res.reserve(4);
    for ( Property* i : vec_ )
    {
        if ( i->category() == cat )
            res.push_back(i);
    }
    return res;
}


PropertyList PropertyList::find_all(std::string const& c1, std::string const& c2) const
{
    //std::clog << this << "->find_all(" << kd1 << "," << kd2 << ") " << std::endl;
    
    PropertyList res;
    for ( Property* i : vec_ )
    {
        if ( i->category() == c1 || i->category() == c2 )
            res.push_back(i);
    }
    return res;
}


PropertyList PropertyList::find_all(std::string const& c1, std::string const& c2, std::string const& c3) const
{
    //std::clog << this << "->find_all(" << kd1 << "," << kd2 << ") " << std::endl;

    PropertyList res;
    for ( Property* i : vec_ )
    {
        if ( i->category() == c1 || i->category() == c2 || i->category() == c3 )
            res.push_back(i);
    }
    return res;
}


Property* PropertyList::find_next(std::string const& cat, Property * p) const
{
    //std::clog << this << "->find_next(" << cat << ") " << std::endl;
    bool found = ( p );
    
    for ( Property* i : vec_ )
    {
        if ( i->category() == cat )
        {
            if ( found )
                return i;
            found = ( i == p );
        }
    }
    
    if ( ! found ) 
        return nullptr;
    
    for ( Property* i : vec_ )
    {
        if ( i->category() == cat )
            return i;
    }
    
    return nullptr;
}


PropertyList PropertyList::find_all_except(std::string const& cat) const
{
    //std::clog << this << "->find_all_expect(" << cat << ") " << std::endl;
    
    PropertyList res;
    for ( Property* i : vec_ )
    {
        if ( i->category() != cat )
            res.push_back(i);
    }
    return res;
}


//-------------------------------------------------------------------------------
#pragma mark -

void PropertyList::write_names(std::ostream& os, std::string const& pf) const
{
    os << pf << "Known classes:\n";
    for ( Property const* i : vec_ )
    {
        os << pf << std::setw(10);
        if ( i )
            os << i->category() << i->number() << " `"<< i->name() << "'";
        else
            os << "void";
        std::endl(os);
    }
}


std::string PropertyList::all_names(std::string const& pf) const
{
    std::ostringstream oss;
    write_names(oss, pf);
    return oss.str();
}
        
/**
 The values identical to the default settings are skipped if prune==1
 */
void PropertyList::write(std::ostream& os, const bool prune) const
{
    for ( Property const* i : vec_ )
        i->write(os, prune);
}

