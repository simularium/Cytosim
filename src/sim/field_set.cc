// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "field_set.h"
#include "field_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"
#include "field.h"


// first object
Field * FieldSet::first() const
{
    return static_cast<Field*>(nodes.front());
}

// find object
Field * FieldSet::findObject(Property const* p) const
{
    return static_cast<Field*>(ObjectSet::findObject(p));
}

// return pointer to the Object of given ID, or zero if not found
Field * FieldSet::findID(ObjectID n) const
{
    return static_cast<Field*>(inventory.get(n));
}

//------------------------------------------------------------------------------

void FieldSet::prepare()
{
    for ( Field * f=first(); f; f=f->next() )
    {
        assert_true( f->hasField() );
        f->prepare();
    }
}


void FieldSet::step()
{
    for ( Field * f=first(); f; f=f->next() )
    {
        if ( f->hasField() )
        {
            LOG_ONCE("!!!! Field is active\n");
            f->step(simul.fibers);
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -

Property* FieldSet::newProperty(const std::string& cat, const std::string& nom, Glossary&) const
{
    if ( cat == "field" )
        return new FieldProp(nom);
    return nullptr;
}


Object * FieldSet::newObject(const ObjectTag tag, unsigned num)
{
    if ( tag == Field::TAG )
    {
        FieldProp * p = simul.findProperty<FieldProp>("field", num);
        return new Field(p);
    }
    return nullptr;
}


/**
 @ingroup NewObject

 Specify the initial value of the Field:
 
     new field NAME
     {
        value = 0
     }
 
 \todo: read the value of the field from a file, at initialization
 */
ObjectList FieldSet::newObjects(const std::string& name, Glossary& opt)
{
    Property * p = simul.properties.find_or_die("field", name);
    FieldProp * fp = static_cast<FieldProp*>(p);
        
    Field * obj = new Field(fp);
        
    // initialize field:
    obj->setField();
        
    // an initial concentration can be specified:
    Field::value_type val = 0;
    if ( opt.set(val, "value") || opt.set(val, "initial_value") )
    {
        std::string str;
        if ( opt.set(str, "value", 1) )
        {
            Space const* spc = simul.findSpace(str);
            if ( !spc )
                spc = obj->prop->confine_space_ptr;
            obj->setConcentration(spc, val, 0);
        }
        else
        {
            obj->setConcentration(val);
        }
    }

    ObjectList res;
    res.push_back(obj);
    return res;
}


void FieldSet::write(Outputter& out) const
{
    if ( size() > 0 )
    {
        out.put_line("\n#section "+title(), out.binary());
        writeNodes(out, nodes);
    }
}

