// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "object.h"
#include "iowrapper.h"
#include "exceptions.h"
#include "property.h"
#include "object_set.h"
#include "sim.h"


Simul& Object::simul() const
{
    return set_->simul;
}


Object::~Object()
{
    //std::clog << "~Object " << this << '\n';

    if ( set_ )
    {
        //std::clog << "unlinking deleted Object " << this << '\n';
        set_->remove(this);
    }
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 This is an accessory function used in ObjectSet::collect()
 */
bool match_all(Object const*, void const*)
{
    return true;
}

/**
 This is an accessory function used in ObjectSet::collect()
 */
bool match_mark(Object const* obj, void const* mrk)
{
    return ( obj->mark() == *((ObjectMark*)mrk) );
}

/**
 This is an accessory function used in ObjectSet::collect()
 */
bool match_property(Object const* obj, void const* prop)
{
    return ( obj->property() == prop );
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 The ASCII reference has the format `XP:N` where:
 - X is one ascii character, indicating the category (tag),
 - P > 0 is the index of the property, indicating the type of object,
 - N > 0 is the ID of the object within its category.
 .

 For example 'f0:021' is the fiber number 21 of class 0 (ie. property index = 0),
 For example 'f1:010' is the fiber number 10 of class 1 (ie. property index = 1)
*/
std::string Object::reference(ObjectTag tag, unsigned pix, ObjectID id)
{
    assert_true( pix >= 0 );
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%c%u:%04u", tag, pix, id);
    return std::string(tmp);
}


/**
 The ASCII reference has the format `XP:N` where:
 - X is one ascii character, indicating the category (ObjectSet::tag()),
 - P > 0 is the index of the property, indicating the type of object,
 - N > 0 is the ID of the object within its category.
 .
 
 For example 'f0:21' is the fiber of property 0, number 21
 For example 'f1:10:1' is the fiber of property 1, number 10, and it is marked as '1'
 */
std::string Object::reference() const
{
    if ( property() )
        return reference(tag(), property()->number(), identity());
    else
        return reference(tag(), 0, identity());
}


/**
 Two binary formats are used:
 - A slim format:
     - 1 byte for the tag()
     - 2 bytes for the identity
     .
 - A fat format:
     - 1 byte containing tag() + 128
     - 4 bytes for the identity
     .
 .
 The ascii based format always the same.
 All formats are read by Simul::readReference()
 */
void Object::writeReference(Outputter& out, ObjectTag g, ObjectID id)
{
    assert_true( id > 0 );

    if ( id > 65535 )
    {
        // long format (5 bytes)
        // set the highest bit of the byte, which is not used by ASCII codes
        out.writeChar(g, 128);
        out.writeUInt32(id, 0);
    }
    else
    {
        // short format (3 bytes)
        out.put_char(g);
        out.writeUInt16(id, 0);
    }
}


void Object::writeNullReference(Outputter& out)
{
    out.put_char(TAG);
}


void Object::writeReference(Outputter& out, Object const* i)
{
    if ( i )
        writeReference(out, i->tag(), i->identity());
    else
        writeNullReference(out);
}


/**
Writes the info that is common to all objects to file
 Two binary formats are used:
 - A slim format:
     - 1 byte for the tag()
     - 1 byte for the index of the property
     - 2 bytes for the identity
     .
 - A fat format:
     - the character '$' in binary mode
     - 1 byte for the tag()
     - 2 bytes for the index of the property
     - 4 bytes for the identity
     - 4 bytes for the mark
     .
 .
 The ascii based format is invariant.
 */
void Object::writeHeader(Outputter& out, ObjectTag g) const
{
    if ( ! out.binary() )
        out.put_char('\n');
    if ( identity() > 65535 || property()->number() > 255 || mark() )
    {
        // set the highest bit of the byte, which is not used by ASCII codes
        out.writeChar(g, 128);
        out.writeUInt16(property()->number(), 0);
        out.writeUInt32(identity(), ':');
        out.writeUInt32(mark(), ':');
    }
    else
    {
        out.put_char(g);
        out.writeUInt8(property()->number(), 0);
        out.writeUInt16(identity(), ':');
    }
}


void Object::readHeader(Inputter& in, bool fat, unsigned& ix, ObjectID& id, ObjectMark& mk)
{
    if ( in.binary() )
    {
        // read header in binary format
        if ( fat )
        {
            ix = in.readUInt16();
            id = in.readUInt32();
#ifdef BACKWARD_COMPATIBILITY
            if ( in.formatID() < 34 )
                ;
            else if ( in.formatID() < 39 )
                mk = in.readUInt16();
            else
#endif
            mk = in.readUInt32();
        }
        else
        {
            ix = in.readUInt8();
            id = in.readUInt16();
        }
    }
    else
    {
        // read header in text format
        FILE * file = in.file();
        if ( 1 != fscanf(file, "%u", &ix) )
            throw InvalidIO("invalid Object header");
        if ( in.get_char() != ':' )
            throw InvalidIO("invalid Object header");
        if ( 1 != fscanf(file, "%u", &id) )
            throw InvalidIO("invalid Object header");
        int c = in.get_char();
        if ( c == ':' )
        {
            if ( 1 != fscanf(file, "%lu", &mk) )
            throw InvalidIO("invalid Object header");
        }
        else
            in.unget(c);
    }
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 45 )
        ++ix;
#endif
    if ( id == 0 )
        throw InvalidIO("Invalid ObjectID referenced in file");
}



/// print a list of objects
std::ostream& operator << (std::ostream& os, ObjectList const& list)
{
    os << "ObjectList " << &list << " of size " << list.size() << "\n{\n";
    for ( Object * obj : list )
        os << "   " << obj->property()->name() << " " << obj->reference() << '\n';
    os << "}" << '\n';
    return os;
}

