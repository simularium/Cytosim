// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "object_set.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"
#include "modulo.h"
#include "space.h"
#include "simul.h"
#include <errno.h>

extern Modulo const* modulo;

//------------------------------------------------------------------------------

/**
 The object is added at the front of the list
 */
void ObjectSet::link(Object * obj)
{
    assert_true( !obj->objset() );
    obj->objset(this);
    nodes.push_front(obj);
}


void ObjectSet::unlink(Object * obj)
{
    assert_true( obj->objset() == this );
    nodes.pop(obj);
    obj->objset(nullptr);
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Translate all listed movable objects ( Object::mobile()==true ) by `vec`
 */
void ObjectSet::translateObjects(ObjectList const& objs, Vector const& vec)
{
    for ( Object * obj : objs )
        if ( obj->mobile() & 1 )
            obj->translate(vec);
}

/**
 Apply Rotation around the origin to all movable objects in list
 */
void ObjectSet::rotateObjects(ObjectList const& objs, Rotation const& rot)
{
    for ( Object * obj : objs )
        if ( obj->mobile() & 2 )
            obj->rotate(rot);
}

/**
 Apply isometry to all objects
 */
void ObjectSet::moveObjects(ObjectList const& objs, Isometry const& iso)
{
    //std::clog << "moving " << objs.size() << " objects" << std::endl;
    for ( Object * obj : objs )
    {
        switch ( obj->mobile() )
        {
            case 1: obj->rotateT(iso); obj->translate(iso); break;
            case 2: obj->rotate(iso); break;
            case 3: obj->rotate(iso); obj->translate(iso); break;
        }
    }
}


void ObjectSet::flagObjects(ObjectList const& objs, ObjectFlag f)
{
    for ( Object * obj : objs )
        obj->flag(f);
}


/**
 Translate movable objects in list if ( obj->flag() != f )
 */
void ObjectSet::translateObjects(ObjectList const& objs, Vector const& vec, ObjectFlag f)
{
    for ( Object * obj : objs )
    {
        if ( obj->mobile() & 1 && obj->flag() != f )
        {
            obj->translate(vec);
            obj->flag(f);
        }
    }
}

/**
 Apply Rotation around the origin to objects in list if ( obj->flag() != f )
 */
void ObjectSet::rotateObjects(ObjectList const& objs, Rotation const& rot, ObjectFlag f)
{
    for ( Object * obj : objs )
    {
        if ( obj->mobile() & 2 && obj->flag() != f )
        {
            obj->rotate(rot);
            obj->flag(f);
        }
    }
}

/**
Apply isometry to objects in list if ( obj->flag() != f )
 */
void ObjectSet::moveObjects(ObjectList const& objs, Isometry const& iso, ObjectFlag f)
{
    //std::clog << "moving " << objs.size() << " objects" << std::endl;
    for ( Object * obj : objs )
    {
        if ( obj->flag() != f )
        {
            //std::clog << "    moving " << obj->reference() << std::endl;
            switch ( obj->mobile() )
            {
                case 1: obj->rotateT(iso); obj->translate(iso); break;
                case 2: obj->rotate(iso); break;
                case 3: obj->rotate(iso); obj->translate(iso); break;
            }
            obj->flag(f);
        }
        //else std::clog << "    already moved " << obj->reference() << std::endl;
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void ObjectSet::add(Object * obj)
{
    if ( !obj->linked() )
    {
        inventory.assign(obj);
        link(obj);
        //std::clog << "ObjectSet::add(" << obj->reference() << ")\n";
    }
    else
    {
        std::cerr << "Warning: attempted to re-link "+obj->reference()+" \n";
    }
}


void ObjectSet::add(ObjectList const& list)
{
    for ( Object * obj : list )
        add(obj);
}


void ObjectSet::remove(Object * obj)
{
    //std::clog << "ObjectSet::remove " <<  obj->reference() << '\n';
    inventory.unassign(obj);
    if ( obj->linked() )
        unlink(obj);
}


void ObjectSet::remove(ObjectList const& list)
{
    for ( Object * obj : list )
        remove(obj);
}


void ObjectSet::erase(NodeList & list)
{
    Node * n = list.front();
    while ( n )
    {
        Node * p = n->next();
        list.pop(n);
        static_cast<Object*>(n)->objset(nullptr);
        delete(n);
        n = p;
    }
}


void ObjectSet::erase(Object * obj)
{
    //std::clog << "ObjectSet::erase " << obj->reference() << '\n';
    remove(obj);
    delete(obj);
}


void ObjectSet::erase()
{
    erase(nodes);
    inventory.clear();
}


Object* ObjectSet::findObject(std::string spec, long num, const std::string& title) const
{
    //std::clog << "findObject(" << spec << "|" << num << "|" << title << ")\n";
    // check for a string starting with the class name (eg. 'fiber'):
    if ( spec == title )
    {
        Inventoried * inv = nullptr;
        if ( num > 0 )
        {
            inv = inventory.get(num);
        }
        else
        {
            // start from the end of the list:
            inv = inventory.last();
            while ( inv  &&  ++num <= 0 )
                inv = inventory.previous(inv);
        }
        return static_cast<Object*>(inv);
    }
    
    // check if string starts with 'first'
    if ( spec == "first" )
    {
        Inventoried* inv = inventory.first();
        while ( inv  &&  --num >= 0 )
            inv = inventory.next(inv);
        return static_cast<Object*>(inv);
    }
    
    // check if string starts with 'last'
    if ( spec == "last" )
    {
        Inventoried* inv = inventory.last();
        while ( inv  &&  ++num <= 0 )
            inv = inventory.previous(inv);
        return static_cast<Object*>(inv);
    }
    
    // finally search for a property name:
    Property * pp = simul.findProperty(title, spec);

    if ( pp )
    {
        Inventoried* inv = nullptr;
        if ( num > 0 )
        {
            // 'microtubule1' would return the first created microtubule
            // std::clog << "findObject -> highest pick `" << spec << num << "'\n";
            inv = inventory.first();
            while ( inv )
            {
                num -= ( static_cast<Object*>(inv)->property() == pp );
                if ( num <= 0 )
                    break;
                inv = inventory.next(inv);
            }
        }
        else
        {
            // 'microtubule0' would return the last created microtubule
            //std::clog << "findObject -> highest pick `" << spec << num << "'\n";
            inv = inventory.last();
            while ( inv )
            {
                num += ( static_cast<Object*>(inv)->property() == pp );
                if ( num > 0 )
                    break;
                inv = inventory.previous(inv);
            }
        }
        return static_cast<Object*>(inv);
    }
    
    return nullptr;
}


// split into a word and a number, without a space:
bool splitObjectSpec(std::string& str, long& num)
{
    size_t pos = str.find_first_of("0123456789+-");
    if ( pos != std::string::npos )
    {
        char const* ptr = str.c_str() + pos;
        char * end;
        errno = 0;
        num = strtol(ptr, &end, 10);
        if ( errno || ( *end && !isspace(*end) ))
            throw InvalidParameter("expected a number in `"+str+"'");
        str.resize(pos);
        //std::clog << "splitObjectSpec |" << str << "|" << num << "|\n";
        return true;
    }
    return false;
}

/*
 There are several ways to designate an object.
 For example, if the class name (title) is 'fiber', one may use:
 - `fiber1`  indicates fiber number 1
 - `fiber2`  indicates fiber number 2, etc.
 - `first`   indicates the oldest fiber remaining
 - `first+1` indicates the second oldest fiber remaining
 - `last`    indicates the last fiber created
 - `last-1`  indicates the penultimate fiber created
 - `fiber0`  the last fiber created,
 - `fiber-1` the penultimate fiber, etc.
 .
 */
Object* ObjectSet::findObject(std::string spec, const std::string& title) const
{
    //std::clog << "ObjectSet::findObject " << spec << std::endl;
    
    if ( spec == "first" )
        return static_cast<Object*>(inventory.first());
    
    if ( spec == "last" )
        return static_cast<Object*>(inventory.last());
 
    // try to split into a word and a number:
    long num = 0;
    if ( splitObjectSpec(spec, num) )
        return findObject(spec, num, title);

    // check category name, eg. 'fiber':
    if ( spec == title )
    {
        ObjectList all = collect();
        //std::clog << "findObject -> random pick among " << sel.size() << " " << title << "\n";
        if ( all.size() > 0 )
            return all.pick_one();
    }
    
    // check property name:
    Property * p = simul.findProperty(title, spec);
    if ( p )
    {
        ObjectList sel = collect(match_property, p);
        //std::clog << "findObject -> random pick among " << sel.size() << " " << spec << "\n";
        if ( sel.size() > 0 )
            return sel.pick_one();
    }

    return nullptr;
}


/**
 return the first object encountered with the given property,
 but it can be any one of them, since the lists are regularly
 shuffled to randomize the order in the list.
 */
Object * ObjectSet::findObject(Property const* p) const
{
    for ( Object* obj=first(); obj; obj=obj->next() )
        if ( obj->property() == p )
            return obj;
    return nullptr;
}


unsigned ObjectSet::count(const NodeList & list,
                          bool (*func)(Object const*, void const*), void const* arg)
{
    unsigned res = 0;
    Node const* n = list.front();
    while ( n )
    {
        Object const* obj = static_cast<Object const*>(n);
        n = n->next();
        res += func(obj, arg);
    }
    return res;
}


ObjectList ObjectSet::collect(const NodeList & list)
{
    ObjectList res;
    for ( Node* n = list.front(); n; n=n->next() )
        res.push_back(static_cast<Object*>(n));
    return res;
}


ObjectList ObjectSet::collect(const NodeList & list,
                              bool (*func)(Object const*, void const*), void const* arg)
{
    ObjectList res;
    Node * n = list.front();
    while ( n )
    {
        Object * obj = static_cast<Object*>(n);
        n = n->next();
        if ( func(obj, arg) )
            res.push_back(obj);
    }
    return res;
}


ObjectList ObjectSet::collect() const
{
    return collect(nodes);
}


ObjectList ObjectSet::collect(bool (*func)(Object const*, void const*), void const* arg) const
{
    return collect(nodes, func, arg);
}


ObjectList ObjectSet::collect(Property const* p) const
{
    return collect(match_property, p);
}


unsigned ObjectSet::count(bool (*func)(Object const*, void const*), void const* arg) const
{
    return count(nodes, func, arg);
}

//------------------------------------------------------------------------------
#pragma mark - I/O


void ObjectSet::flag(NodeList const& list, ObjectFlag f)
{
    for ( Node * n=list.front(); n; n=n->next() )
        static_cast<Object*>(n)->flag(f);
}


void ObjectSet::prune(NodeList const& list, ObjectFlag f, ObjectFlag g)
{
    Node * n = list.front();
    
    while ( n )
    {
        Node * p = n->next();
        Object * o = static_cast<Object*>(n);
        if ( o->flag() == f )
            delete(o);
        else
            o->flag(g);
        n = p;
    }
}


/**
 Write Reference and Object's data, for all Objects in `list`
 */
void ObjectSet::writeNodes(Outputter& out, NodeList const& list)
{
    for ( Node const* n=list.front(); n; n=n->next() )
    {
        Object const* o = static_cast<const Object*>(n);
        //std::clog << "writeObject " << o->reference() << '\n';
        o->writeHeader(out, o->tag());
        o->write(out);
    }
}


/**
 Load an object from file
 
 If 'update==true', the corresponding object is changed, otherwise a new object is created.
 If 'load==false', the record is not kept
 */
void ObjectSet::loadObject(Inputter& in, const ObjectTag tag, bool fat, bool discard, bool update)
{
    unsigned ix = 0;
    ObjectID id = 0;
    ObjectMark mk = 0;
    Object * obj = nullptr;

    Object::readHeader(in, fat, ix, id, mk);
    
    if ( update )
    {
        obj = findID(id);
        // check that property index has not changed:
        if ( obj && obj->property()->number() != ix )
        {
            Property const* P = obj->property();
#if ( 1 )
            std::clog << "Property mismatch: " << P->name() << "' is " << P->category() << P->number();
            std::clog << " but loaded object has property #" << ix << '\n';
#endif
            erase(obj);
            obj = nullptr;
        }
    }
    
    if ( !obj )
    {
        assert_true(isprint(tag));
        // create new object of required class
        obj = newObject(tag, ix);
        if ( !obj )
        {
            std::string str = std::to_string(tag);
            if ( isprint(tag) )
                str += " ("+std::string(1,tag)+")";
            throw InvalidIO("invalid ObjectTag "+str+" referenced in file");
        }
        obj->identity(id);
        //std::clog << "- new " << Object::reference(tag, ix, id) << '\n';
    }
    assert_true( obj->identity() == id );
    assert_true( obj->property() );
    
    try {
        //std::clog << "- loading " << Object::reference(tag, ix, id) << " at " << in.pos() << '\n';
        // read object data:
        obj->read(in, simul, tag);
    }
    catch( Exception & e )
    {
        e << ", while loading " << Object::reference(tag, ix, id);
        throw;
    }
    
    if ( discard )
        delete(obj);
    else
    {
        if ( !obj->linked() )
            add(obj);
        // clear flag to indicate that object was refreshed:
        obj->flag(0);
        obj->mark(mk);
    }
}


//------------------------------------------------------------------------------


void ObjectSet::writeAssets(std::ostream& os, const std::string& title) const
{
    if ( size() > 0 )
    {
        os << '\n' << title;
        PropertyList plist = simul.properties.find_all(title);
        if ( plist.size() > 0 )
        {
            for ( Property * p : plist )
            {
                size_t cnt = count(match_property, p);
                os << '\n' << std::setw(10) << cnt << " " << p->name();
            }
            if ( plist.size() > 1 )
                os << '\n' << std::setw(10) << size() << " total";
        }
        else
        {
            os << '\n' << std::setw(10) << size() << " " << title;
        }
    }
}

