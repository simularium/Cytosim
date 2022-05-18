// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "sim.h"
#include <fstream>
#include <unistd.h>
#include "filepath.h"
#include "messages.h"
#include "parser.h"
#include "print_color.h"
#include "tictoc.h"

/**
 A number `currentFormatID` is used to define the format of trajectory files
 This is the initial value to Inputter::formatID()
 History of changes in file format:

 52: 18/10/2019 Space's shape is stored always on 16 characters
 51: 03/03/2019 Storing number of Aster links
 50: 19/12/2018 Fiber's birth time moved to Filament (now Chain)
 49: 12/12/2018 FiberSite writes the Lattice index but not the abscissa
 49: 22/11/2018 reference do not include mark, which is writen in object header
 48: 04/07/2018 Fiber stores its birth time
 47: 13/02/2017 Wrist and Aster built on a local reference frame of Solid
 46: 23/10/2015 GrowingFiber writes dynamic states, changed ClassicFiber:write()
 45: 18/09/2015 Indices of Properties are numbers starting from one, and not zero
 44: 25/07/2015 the dynamic state of fiber ends is stored with a separate TAG
 43: 24/04/2014 number of dimension in Space stored in 16 bits
 42: 09/11/2013 All fibers store end_state on 32 bits
     08/12/2012 FRAME_TAG was changed from "#frame " to "#Cytosim "
 41: 17/11/2012 Space stores its shape as a string in objects.cmo
 40: 11/09/2012 Aster format simplified
 39: 11/07/2012 Object::mark is stored on 32 bits instead of 16 previously
 38: 03/05/2012 Fiber stores length instead of segment-length
 37: 30/04/2012 Couple::Duo stores its activity state
 36: 22/02/2012 All Spaces store their dimensions in objects.cmo
 35: 15/09/2011 Some Spaces store their dimensions in objects.cmo
 34: 20/12/2010 Moved Fiber::mark to Object::mark
 33: 29/04/2010 Added Fiber::mark
 32: 15/04/2010 Space became an Object
 31: 01/04/2010 Fiber became a Mecable
 30: The Tag were reduced to 1 char: saves space & simplifies code
     26/05/2009 started cytosim-PI: a revolution!
 27: 22/03/2008 new Fiber::write(), called in Tubule::write()
 26: 03/11/2007 Hand do not record haEnd flag
 24: 14/12/2006 started cytosim 3, lots of changes
 23: 10/12/2005 new class Solid
 22: modified Sphere
 21: modified Sphere
 20: 12/07/2004
 19: introduced different kinds of Single
*/


//------------------------------------------------------------------------------
#pragma mark - Write Objects

/**
 This writes all objects of the current state to a trajectory file
*/
void Simul::writeObjects(Outputter& out) const
{
    // write a line identifying a new frame:
    fprintf(out, "\n\n#Cytosim  %i  %s", getpid(), TicToc::date());
    
    // record file format:
    fprintf(out, "\n#format %i dim %i", currentFormatID, DIM);
    
    // identify the file as binary, with its endianess:
    if ( out.binary() )
    {
        fprintf(out, "\n#binary ");
        out.writeEndianess();
    }
    
    /*
     An object should be written after any other objects that it refers to.
     For example, Aster is written after Fiber, Couple after Fiber...
     This makes it easier to reconstruct the state during input.
     */
    fprintf(out, "\n#time %.6f sec", prop->time);

    spaces.write(out);
    fields.write(out);
    fibers.write(out);
    solids.write(out);
    beads.write(out);
    spheres.write(out);
    singles.write(out);
    couples.write(out);
    organizers.write(out);
    //events.write(out);
    
    out.put_line("\n#section end");
    out.put_line("\n#end cytosim");
    fprintf(out, " %s\n\n", TicToc::date());
}


/**
 This writes the current state to a trajectory file called `name`.
 If this file does not exist, it is created de novo.
 If `append == true` the state is added to the file, otherwise it is cleared.
 If `binary == true` a binary format is used, otherwise a text-format is used.
*/
void Simul::writeObjects(std::string const& name, bool append, bool binary) const
{
    Outputter out(name.c_str(), append, binary);
    
    if ( ! out.good() )
        throw InvalidIO("could not open output file `"+name+"' for writing");
    
    try
    {
        out.lock();
        writeObjects(out);
        out.unlock();
    }
    catch( InvalidIO & e )
    {
        std::cerr << "Error writing trajectory file: " << e.what() << '\n';
    }
}

//------------------------------------------------------------------------------
#pragma mark - Read Objects

/**
 The Object is not modified
 */
Object * Simul::readReference(Inputter& in, ObjectTag& tag)
{
    int c = 0;
    do
        c = in.get_char();
    while ( c == ' ' );
    
    if ( c == EOF )
        throw InvalidIO("unexpected end of file");

    tag = c & 127;
    // detect fat reference:
    int fat = ( c & 128 );
#ifdef BACKWARD_COMPATIBILITY  // formatID() < 50
    if ( c == '$' )
    {
        tag = in.get_char();
        if ( tag == EOF )
            throw InvalidIO("unexpected end of file");
        fat = 1;
    }
#endif
    
    // Object::TAG is the 'void' reference
    if ( tag == Object::TAG )
        return nullptr;

#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 32 )
    {
        ObjectID n = isupper(tag) ? in.readUInt32() : in.readUInt16();
        if ( n == 0 )
            return nullptr;
        ObjectSet const* set = findSetT(tolower(tag));
        if ( !set )
            throw InvalidIO("unknown ObjectTag in Simul::find()");
        Object * w =  set->findID(n);
        if ( !w )
            throw InvalidIO("unknown Object referenced (old format)");
        return w;
    }
#endif
    
    ObjectID id = 0;

    if ( in.binary() )
    {
        if ( fat )
        {
            // long format
#ifdef BACKWARD_COMPATIBILITY
            // skip property index
            if ( in.formatID() < 49 )
                in.readUInt16();
#endif
            id = in.readUInt32();
#ifdef BACKWARD_COMPATIBILITY
            // skip ObjectMark
            if ( in.formatID() < 34 )
                ;
            else if ( in.formatID() < 39 )
                in.readUInt16();
            else if ( in.formatID() < 49 )
                in.readInt32();
#endif
        }
        else
        {
            // short format
#ifdef BACKWARD_COMPATIBILITY
            // skip property index
            if ( in.formatID() < 49 )
                in.readUInt8();
#endif
            id = in.readUInt16();
        }
    }
    else
    {
        FILE * file = in.file();
#ifdef BACKWARD_COMPATIBILITY
        // skip property index
        if ( in.formatID() < 49 )
        {
            unsigned u;
            if ( 1 != fscanf(file, "%u", &u) )
                throw InvalidIO("readReference (compatibility) failed");
            if ( in.get_char() != ':' )
                throw InvalidSyntax("missing ':'");
        }
#endif
        if ( 1 != fscanf(file, "%u", &id) )
            throw InvalidIO("readReference failed");
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 49 )
        {
            // skip ObjectMark which is not used
            int h = in.get_char();
            if ( h == ':' )
            {
                unsigned long u;
                if ( 1 != fscanf(file, "%lu", &u) )
                throw InvalidIO("readReference (compatibility) failed");
            }
            else
            in.unget(h);
        }
#endif
    }

    if ( id == 0 )
        return nullptr;
    
    if ( !isalpha(tag) )
        throw InvalidIO("`"+std::string(1,tag)+"' is not a valid class TAG");

    const ObjectSet * set = findSetT(tag);
    
    if ( !set )
        throw InvalidIO("`"+std::string(1,tag)+"' is not a known class TAG");

    Object * res = set->findID(id);
    
    if ( !res )
        throw InvalidIO("unknown object `"+((char)tag+std::to_string(id))+"' referenced");
    
    return res;
}


/// InputLock is a helper class used to import a cytosim state from a file
class Simul::InputLock
{
private:
    
    /// pointer
    Simul * sim;

    /// state
    bool  frozen;
    
    /// value of flag
    static constexpr ObjectFlag FLAG = 777;
    
public:
    
    /// flag all objects with FLAG
    InputLock(Simul * s)
    : sim(s)
    {
        //Cytosim::log("Simul::InputLock created with %i objects\n", sim->nbObjects());
        sim->couples.freeze(FLAG);
        sim->singles.freeze(FLAG);
        sim->fibers.freeze(FLAG);
        sim->beads.freeze(FLAG);
        sim->solids.freeze(FLAG);
        sim->spheres.freeze(FLAG);
        sim->organizers.freeze(FLAG);
        sim->fields.freeze(FLAG);
        sim->spaces.freeze(FLAG);
        //sim->events.freeze(FLAG);
        frozen = true;
    }
    
    /// erase objects flagged with number '7'
    void prune()
    {
        //sim->events.prune(FLAG);
        sim->organizers.prune(FLAG);
        sim->couples.prune(FLAG);
        sim->singles.prune(FLAG);
        sim->beads.prune(FLAG);
        sim->solids.prune(FLAG);
        sim->spheres.prune(FLAG);
        sim->fibers.prune(FLAG);
        sim->spaces.prune(FLAG);
        sim->fields.prune(FLAG);
        frozen = false;
    }

    /// reset flags
    ~InputLock()
    {
        /*
         Attention: The order of the thaw() below is important:
         destroying a Fiber will detach any motor attached to it,
         and thus automatically move them to the 'unattached' list,
         as if they had been updated from reading the file.
         Destroying couples and singles before the fibers avoid this problem.
         */
        if ( frozen )
        {
            //sim->events.thaw();
            sim->organizers.thaw();
            sim->couples.thaw();
            sim->singles.thaw();
            sim->beads.thaw();
            sim->solids.thaw();
            sim->spheres.thaw();
            sim->fibers.thaw();
            sim->spaces.thaw();
            sim->fields.thaw();
            frozen = false;
        }
        //Cytosim::log("Simul::InputLock deleted with %i objects\n", sim->nbObjects());
    }
};


/**
 This will update the current state to make it identical to what has been saved
 in the file.
 
 Before reading, all objects are marked with flag().
 Every object found in the file is unflagged as it is updated.
 
 When the read is complete, the objects that are still marked are deleted.
 In this way the new state reflects exactly the system that was stored on file.
 
 @returns
 - 0 = success
 - 1 = EOF
 .
 */
int Simul::reloadObjects(Inputter& in, ObjectSet* subset)
{
    // set flag to erase any object that was not updated
    InputLock lock(this);

    // if no error occurred, erase objects that have not been updated
    if ( 0 == loadObjects(in, subset) )
        lock.prune();

    return in.eof();
}


/**
 Read Objects from a file:
 update the ones that were already present in the simulation world,
 and otherwise create new ones. The Simulation worlds is augmented.
 If 'subset!=0' only objects from this class will be imported.
 
 @returns
 - 0 = success
 - 1 = EOF
 .
 */
int Simul::loadObjects(Inputter& in, ObjectSet* subset)
{
    if ( in.eof() )
        return 1;
    
    if ( ! in.good() )
        throw InvalidIO("invalid file in Simul::loadObjects()");
    
    int res = 0;
    
    in.lock();
    try
    {
        res = readObjects(in, subset);
        //std::clog << "loadObjects returns " << res << std::endl;
    }
    catch(Exception & e)
    {
        in.unlock();
        throw;
    }
    
    in.unlock();
    return res;
}


/**
 Create an Inputter 'in' and call 'loadObjects(in)'
 */
int Simul::loadObjects(char const* filename)
{
    Inputter in(DIM, filename, true);

    if ( ! in.good() )
        throw InvalidIO("Could not open specified file for reading");
    
    return loadObjects(in);
}


//------------------------------------------------------------------------------

/**
 Read file, updating existing objects, and creating new ones for those not 
 already present in the Simul.
 If 'subset!=0' only objects from this class will be imported.
 The Inputter should be locked in a multithreaded application
 
 @returns
 - 0 : success
 - 1 : EOF
 - 2 : the file does not appear to be a valid cytosim archive
 
  */
int Simul::readObjects(Inputter& in, ObjectSet* subset)
{
    ObjectSet * objset = nullptr;
    std::string section, line;
    int has_frame = 0;
    int tag = 0, c = 0;
    int fat = 0;

    while ( in.good() )
    {
        do {
            c = in.get_char();
            if ( c == '#' )
            {
                line = in.get_line();
                break;
            }
            tag = ( c & 127 );
            fat = ( c & 128 );
#ifdef BACKWARD_COMPATIBILITY
            // detect fat header, formatID() < 50
            if ( c == '$' )
            {
                fat = 1;
                tag = in.get_char();
            }
#endif
            if ( c == EOF )
                return 1;
        } while ( !isalpha(tag) );
        
        
        //check for meta-data, contained in lines starting with '#'
        if ( c == '#' )
        {
            //std::clog << "      |#" << line << "|" << std::endl;
            std::istringstream iss(line);
            std::string tok;
            iss >> tok;

            // section start
            if ( tok == "section" )
            {
                iss >> section;
                //std::clog << " section |" << section << "|\n";
                if ( prop->skip_free_couple )
                {
                    iss >> tok;
                    // this skips loading of Couple that are not bridging:
                    if ( section == "couple" && tok == "FF" )
                    {
                        in.skip_until("#section ");
                        std::clog << "skipped " << section << " " << tok << '\n';
                    }
                }
                objset = findSet(section);
                if ( !objset && section != "end" )
                    std::clog << " warning: unknown section |" << section << "|\n";
            }
            // frame start
            else if ( tok == "Cytosim" || tok == "cytosim" || tok == "frame" )
            {
                if ( has_frame )
                    return 2;
                has_frame = 1;
            }
            //binary signature
            else if ( tok == "binary" )
            {
                in.setEndianess(line.substr(7).c_str());
            }
            // info line "#format 48 dim 2"
            else if ( tok == "format" )
            {
                int d = 0, f = 0;
                iss >> f >> tok >> d;
                in.formatID(f);
                in.vectorSize(d);
                if ( d != DIM )
                    Cytosim::warn << "mismatch between file ("<<d<<"D) and executable ("<<DIM<<"D)\n";
                //if ( f != currentFormatID )
                //    std::clog << "Cytosim is reading data format "<<f<<"\n";
            }
            // time data "#time 1.2345"
            else if ( tok == "time" )
            {
                iss >> prop->time;
#ifdef BACKWARD_COMPATIBILITY
                // old format info line "#time 14.000000, dim 2, format 47"
                if ( iss.get() == ',' )
                {
                    int i = 0;
                    iss >> tok >> i;
                    if ( tok == "dim" )
                    {
                        in.vectorSize(i);
                        if ( i != DIM )
                            Cytosim::warn << "mismatch between file ("<<i<<"D) and executable ("<<DIM<<"D)\n";
                    }
                    if ( iss.get() == ',' )
                    {
                        iss >> tok >> i;
                        if ( tok == "format" )
                        {
                            in.formatID(i);
                            //if ( i != currentFormatID )
                            //    std::clog << "Cytosim is reading data format " << i << "\n";
                        }
                    }
                }
#endif
            }
            //detect the mark at the end of the frame
            else if ( tok == "end" )
            {
                iss >> tok;
                if ( tok == "cytosim" )
                    return 0;
#ifdef BACKWARD_COMPATIBILITY
                if ( tok == "frame" )
                    return 0;
#endif
            }
        }
        else
        {
            //std::clog << "OBJECT |" << (char)tag << "| " << (fat?"fat\n":"\n");
            assert_true( isalpha(tag) );

#ifdef BACKWARD_COMPATIBILITY
            // Compatibility with older format (before 2010)
            if ( in.formatID() < 32 )
            {
                ObjectSet * set = findSetT(tolower(tag));
                if ( set )
                {
                    ObjectID n = isupper(tag) ? in.readUInt32() : in.readUInt16();
                    if ( n == 0 )
                        throw InvalidIO("invalid (null) Object reference");
                    Object * obj = set->findID(n);
                    if ( obj )
                    {
                        if ( tag!='i'  &&  ( tag!='m' || in.formatID()!=31 ))
                            in.readUInt16();
                        obj->read(in, *this, tag);
                        obj->flag(0);
                    }
                    else
                    {
                        int pi = 0;
                        if ( tag!='i'  &&  ( tag!='m' || in.formatID()!=31 ))
                            pi = in.readUInt16();
                        obj = set->newObject(tolower(tag), pi);
                        obj->identity(n);
                        obj->read(in, *this, tag);
                        set->add(obj);
                    }
                    continue;
                }
            }
#endif
            try
            {
                if ( objset )
                {
                    // check that we are using the correct ObjectSet:
                    assert_true( objset == findSetT(tag) );
                    const bool discard = ( subset && subset!=objset );
                    objset->loadObject(in, tag, fat, discard, true);
                }
                else
                {
                    // this is the 'older' pathway
                    ObjectSet * set = findSetT(tag);
                    if ( set )
                    {
                        const bool discard = ( subset && subset!=set );
                        set->loadObject(in, tag, fat, discard, true);
                    }
                }
            }
            catch( Exception & e )
            {
                if ( section.size() )
                {
                    std::cerr << "Error in section " << section << ": " << e.what() << std::endl;
                    if ( objset )
                        in.skip_until("#section ");
                }
                else
                    std::cerr << "Error : " << e.what() << std::endl;
            }
        }
    }
    return 2;
}


//------------------------------------------------------------------------------
#pragma mark - Write/Read Properties


/**
 The order of the output is important, since properties may depend
 on each other (eg. SingleProp and CoupleProp use HandProp).
 Luckily, there is no circular dependency in Cytosim at the moment.
 
 Thus we simply follow the order in which properties were defined,
 and which is the order in which properties appear in the PropertyList.
 */

void Simul::writeProperties(std::ostream& os, const bool prune) const
{
    //std::clog << "Writing properties" << std::endl;
    os << "% Cytosim property file, pid " << getpid() << '\n';
    os << "% " << TicToc::date() << '\n';

    prop->write(os, prune);
    properties.write(os, prune);
}


/**
 At the first call, this will write all properties to file, 
 and save a copy of what was written to a string `properties_saved`.
 
 The next time this is called, the properties will be compared to the string,
 and the file will be rewritten only if there is a difference.
 */
void Simul::writeProperties(char const* name, bool prune) const
{
    std::ostringstream oss;
    writeProperties(oss, prune);
    if ( oss.str() != properties_saved )
    {
        properties_saved = oss.str();

        // use default file name if 'name' is empty or not provided
        if ( !name || *name==0 )
            name = prop->property_file.c_str();
        
        std::ofstream os(name);
        //this should be equivalent to: writeProperties(os, prune);
        os << properties_saved << std::endl;
        os.close();
        //std::clog << "Writing properties at frame " << currentFrame() << std::endl;
    }
}


void Simul::loadProperties()
{
    Parser(*this, 1, 1, 0, 0, 0).readConfig(prop->property_file);
}
