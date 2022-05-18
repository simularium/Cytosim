// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "inventory.h"
#include "assert_macro.h"
#include "exceptions.h"


Inventory::Inventory()
{
    lowest_    = 1;
    highest_   = 0;
    allocated_ = 8;
    byNames    = new Inventoried*[allocated_];
    
    for ( ObjectID n = 0; n < allocated_; ++n )
        byNames[n] = nullptr;
}


Inventory::~Inventory()
{
    delete[] byNames;
}


void Inventory::allocate(size_t sz)
{
    constexpr size_t chunk = 32;
    sz = ( sz + chunk - 1 ) & ~( chunk -1 );
    
    Inventoried ** byNames_new = new Inventoried*[sz];
    
    ObjectID n = 0;
    for ( ; n < allocated_; ++n )
        byNames_new[n] = byNames[n];
    while ( n < sz )
        byNames_new[n++] = nullptr;
    
    delete[] byNames;
    byNames    = byNames_new;
    allocated_ = sz;
}


//------------------------------------------------------------------------------

ObjectID Inventory::first_assigned() const
{
    ObjectID n = 1;
    while ( n < allocated_ )
    {
        if ( byNames[n] )
            return n;
        ++n;
    }
    return 0;
}


ObjectID Inventory::last_assigned() const
{
    ObjectID n = allocated_-1;
    while ( n > 0 )
    {
        if ( byNames[n] )
            return n;
        --n;
    }
    return 0;
}


ObjectID Inventory::next_assigned(ObjectID n) const
{
    ++n;
    while ( n < allocated_ )
    {
        if ( byNames[n] )
            return n;
        ++n;
    }
    return 0;
}


ObjectID Inventory::first_unassigned()
{
    ObjectID n = lowest_;
    
    if ( n < allocated_ )
    {
        if ( !byNames[n] )
            return n;
    
        while ( n < allocated_  &&  byNames[n] )
            ++n;
    
        lowest_ = n;
    }
    
    return n;
}

//------------------------------------------------------------------------------

/**
 This will assign a new serial-number for `obj`, if it does not have one.
 */
void Inventory::assign(Inventoried * obj)
{
    ObjectID& n = obj->identity();
    
    if ( n == 0 )
        n = ++highest_;
    else if ( highest_ < n )
        highest_ = n;
    
    if ( n >= allocated_ )
        allocate(n+1);
    
    assert_true( !byNames[n] );
    
    byNames[n] = obj;
    //std::clog << "Inventory::store() assigned " << n << " to " << obj << "\n";
}


void Inventory::unassign(const Inventoried * obj)
{
    ObjectID n = obj->identity();
    assert_true( n < allocated_ );
    byNames[n] = nullptr;
    
    if ( lowest_ >= n )
        lowest_ = n;
    
    while ( !byNames[highest_]  &&  highest_ > 0 )
        --highest_;
}


Inventoried * Inventory::get(const ObjectID n) const
{
    if ( n < allocated_ )
    {
        assert_true( !byNames[n]  ||  byNames[n]->identity()==n );
        return byNames[n];
    }
    return nullptr;
}


Inventoried* Inventory::first() const
{
    ObjectID n = 1;
    while ( n < allocated_ )
    {
        if ( byNames[n] )
            return byNames[n];
        ++n;
    }
    return nullptr;
}


Inventoried* Inventory::last() const
{
    ObjectID n = highest_;
    while ( n > 0 )
    {
        if ( byNames[n] )
            return byNames[n];
        --n;
    }
    return nullptr;
}


Inventoried* Inventory::previous(Inventoried const* i) const
{
    ObjectID n = i->identity() - 1;
    while ( n > 0 )
    {
        if ( byNames[n] )
            return byNames[n];
        --n;
    }
    return nullptr;
}

#include <iostream>
Inventoried* Inventory::next(Inventoried const* i) const
{
    ObjectID n = i->identity() + 1;
    while ( n < allocated_ )
    {
        if ( byNames[n] )
            return byNames[n];
        ++n;
    }
    return nullptr;
}

//------------------------------------------------------------------------------
unsigned Inventory::count() const
{
    unsigned cnt = 0;
    for ( ObjectID n = 0; n < allocated_; ++n )
        if ( byNames[n] ) ++cnt;
    return cnt;
}


void Inventory::reassign()
{
    ObjectID max = last_assigned();
    ObjectID next = 1;
    ObjectID nn   = 1;
    
    while ( nn <= max )
    {
        while ( nn <= max  &&  !byNames[nn] )
            ++nn;
        if ( nn > max )
            break;
        if ( next < nn )
        {
            byNames[next] = byNames[nn];
            byNames[nn]   = nullptr;
            byNames[next]->identity(next);
        }
        ++next;
        ++nn;
    }
    
    lowest_ = next;
    highest_ = next-1;
}


void Inventory::clear()
{
    for ( ObjectID n = 0; n < allocated_; ++n )
        byNames[n] = nullptr;
    //std::clog << "Inventory::forgetAll() removed " << cnt << "numbers\n";
    lowest_ = 1;
    highest_ = 0;
}


//------------------------------------------------------------------------------
void Inventory::print(std::ostream& os) const
{
    os << "Inventory " << this << "\n";
    for ( ObjectID n = 0; n < allocated_; ++n )
        os << n << " -> " << byNames[n] << "\n";
}


std::ostream& operator << (std::ostream& os, Inventory const& obj)
{
    obj.print(os);
    return os;
}

