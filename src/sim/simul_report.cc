// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "glossary.h"
#include "iowrapper.h"
#include "aster.h"
#include "field.h"
#include <iostream>
#include <numeric>
#include <list>
#include <set>

/// width of columns in formatted output, in number of characters
int column_width = 10;

/// use this macro at the beginning of a line of comment
#define COM "\n% " << std::setw(column_width-2)

/// use this macro at the beginning of a new line of data
#define LIN '\n' << std::setw(column_width)

/// use this macro to separate between values on a line of data
#define SEP ' ' << std::setw(column_width-1)

#include "accumulator.h"

/// pad string by adding white-space on the right up to size 'n * column_width - p'
std::string ljust(std::string const& str, unsigned n, unsigned p = 0)
{
    size_t s = n * column_width - p;
    if ( str.size() < s )
        return str + std::string(s-str.size(), ' ');
    else
        return str;
}

/// pad string by adding white-space on the left up to size 'n * column_width - p'
std::string rjust(std::string const& str, unsigned n, unsigned p = 1)
{
    size_t s = n * column_width - p;
    if ( str.size() < s )
        return std::string(s-str.size(), ' ') + str;
    else
        return str;
}

/// repeat string DIM times with X, Y, Z endings as appropriate
std::string repeatXYZ(std::string const& str, size_t s = column_width)
{
    std::string res(rjust(str+"X", 1, 1));
#if ( DIM > 1 )
    res += " " + rjust(str+"Y", 1, 1);
#endif
#if ( DIM > 2 )
    res += " " + rjust(str+"Z", 1, 1);
#endif
    return res;
}

/// remove any 's' at the end of the argument
void remove_plural(std::string & str)
{
    if ( str.size() > 2  &&  str.at(str.size()-1) == 's' )
        str.resize(str.size()-1);
}


/**
 @copydetails Simul::report0
 This can report multiple data, separated by ';', for example:
 
     report fiber:force;fiber:length
 
 */
void Simul::report(std::ostream& out, std::string arg, Glossary& opt) const
{
    int p = 4;
    opt.set(p, "precision");
    out.precision(p);
    opt.set(column_width, "column") || opt.set(column_width, "width");

    //out << "\n% start   " << prop->time; // historical
    out << "\n% time " << std::fixed << prop->time;
    out << "\n% report " << arg;
    try {
        std::string::size_type pos = arg.find(',');
        while ( pos != std::string::npos )
        {
            report0(out, arg.substr(0, pos), opt);
            arg = arg.substr(pos+1);
            pos = arg.find(';');
        }
        report0(out, arg, opt);
        out << "\n% end\n";
    }
    catch( Exception & e )
    {
        out << "\n% error: " << e.what();
        out << "\n% end\n";
        throw;
    }
}


/**
 
 WHAT            | Output
 ----------------|--------------------------------------------------------------
 `bead`          | Position of beads
 `couple`        | Summary with number of couples in each state
 `fiber`         | Length and position of the ends of fibers
 `single`        | Number and state of singles
 `solid`         | Position of center and first point of solids
 `sphere`        | Position of center and first point of spheres
 `organizer`     | Position of the center of asters and other organizers
 `field`         | Total quantity of substance in field and Lattices
 `time`          | Time
 `inventory`     | summary list of objects
 `property`      | All object properties
 `parameter`     | All object properties
 
 WHAT                    | Output
 ------------------------|------------------------------------------------------
 `fiber:position`        | Position and orientation of fibers
 `fiber:age`             | Average age of fibers
 `fiber:length`          | Average length and standard deviation of fibers
 `fiber:distribution`    | length distribution of fiber lengths (option: `max` and `interval`)
 `fiber:dynamic`         | Number of fiber classified by PLUS_END Dynamic state
 `fiber:point`           | coordinates of vertices of all fibers
 `fiber:displacement`    | mean squared displacement of fibers since the last call
 `fiber:moments`         | standard deviation of vertices of all fibers
 `fiber:speckle`         | coordinates of points randomly distributed along all fibers (option: `interval`)
 `fiber:sample`          | coordinates of points newly distributed along all fibers (option: `interval`)
 `fiber:segment`         | information about lengths of segments, number of kinks
 `fiber:end`             | Positions and dynamic states of all fiber ends
 `fiber:force`           | Position of vertices and Forces acting on vertices
 `fiber:tension`         | Internal stress along fibers
 `fiber:energy`          | Fiber's elastic bending energy
 `fiber:confinement`     | Force applied by fibers on their confinement Space
 `fiber:lattice`         | Total quantity on fiber's lattices
 `fiber:intersection`    | Intersections point of fibers
 `fiber:hand`            | Position of hands attached to fibers
 `fiber:link`            | Positions of attached hands for which stiffness > 0
 `fiber:cluster`         | Clusters made of fibers connected by Couples


 WHAT                    | Output
 ------------------------|------------------------------------------------------
 `bead:all`              | Position of beads
 `bead:single`           | Number of Beads with no single attached, 1 single attached etc.
 `solid:hand`            | Number of hands and number of attached hands on Solids
 `spindle:indice`        | Two scalar indices that caracterize the organization of fibers
 `spindle:profile`       | Number of right- and left-pointing fiber as a function of position
 `single:all`            | Position and force of singles
 `single:NAME`           | Position and force of singles of class NAME
 `single:position:NAME`  | Position and force of singles of class NAME
 `couple:state`          | Position and state of all couples
 `couple:NAME`           | Position and state of couples of class NAME
 `couple:link`           | detailed information on doubly-attached couples
 `couple:configuration`  | number of Couples in { X, P, A, V, T } states
 `couple:force`          | Histogram of tension in the couple links
 `couple:active`         | Position of active couples
 `couple:anatomy`        | Composition of couples
 `couple:NAME`           | Position of couples of class NAME
 `couple:hands`          | Composition of couples
 
 */
void Simul::report0(std::ostream& out, std::string const& arg, Glossary& opt) const
{
    std::string who = arg, what, which;
    
    // split argument string into 3 parts separated by ':': who:what:which
    std::string::size_type pos = arg.find(':');
    if ( pos != std::string::npos )
    {
        who  = arg.substr(0, pos);
        what = arg.substr(pos+1);
        std::string::size_type pas = what.find(':');
        if ( pas != std::string::npos )
        {
            which = what.substr(pas+1);
            what.resize(pas);
        }
    }
    
    // allow for approximate spelling (missing 's'):
    remove_plural(who);
    remove_plural(what);

    //std::clog << "report("<< what << "|" << who << "|" << which << ")\n";

    if ( who == "fiber" )
    {
        if ( !which.empty() )
            return reportFibers(out, which);

        if ( what.empty() || what == "position" )
            return reportFibers(out);
        if ( what == "end" )
            return reportFiberEnds(out);
        if ( what == "point" )
            return reportFiberPoints(out);
        if ( what == "displacement" )
            return reportFiberDisplacement(out);
        if ( what == "moment" )
            return reportFiberMoments(out);
        if ( what == "speckle" )
            return reportFiberSpeckles(out, opt);
        if ( what == "sample" )
            return reportFiberSamples(out, opt);
        if ( what == "segment" )
            return reportFiberSegments(out);
        if ( what == "length" )
            return reportFiberLengths(out);
        if ( what == "distribution" )
            return reportFiberLengthDistribution(out, opt);
        if ( what == "tension" )
            return reportFiberTension(out, opt);
        if ( what == "energy" )
            return reportFiberBendingEnergy(out);
        if ( what == "dynamic" )
            return reportFiberDynamic(out);
        if ( what == "force" )
            return reportFiberForces(out);
        if ( what == "confine_force" )
            return reportFiberConfineForce(out);
        if ( what == "confinement" )
            { reportFiberConfinement(out); return; }
        if ( what == "cluster" )
            return reportClusters(out, opt);
        if ( what == "age" )
            return reportFiberAge(out);
        if ( what == "intersection" )
            return reportFiberIntersections(out, opt);
        if ( what == "hand" )
            return reportFiberHands(out);
        if ( what == "link" )
            return reportFiberLinks(out);
        if ( what == "lattice" )
            return reportFiberLattice(out, false);
        if ( what == "lattice_density" )
            return reportFiberLattice(out, true);

        throw InvalidSyntax("I only know fiber: position, end, point, moment, speckle, sample, segment, dynamic, length, distribution, tension, force, cluster, age, energy, hand, link");
    }
    if ( who == "bead" )
    {
        if ( what.empty() )
            return reportBeadPosition(out);
        if ( what == "single" )
            return reportBeadSingles(out);
        if ( what == "position" )
            return reportBeadPosition(out);
        throw InvalidSyntax("I only know bead: position, single");
    }
    if ( who == "solid" )
    {
        if ( what == "hand" )
            return reportSolidHands(out);
        else if ( what == "position" || what.empty() )
            return reportSolidPosition(out);
        throw InvalidSyntax("I only know `solid'");
    }
    if ( who == "space" )
    {
        if ( what == "force" )
            return reportSpaceForce(out);
        else if ( what.empty() )
            return reportSpace(out);
        throw InvalidSyntax("I only know `space'");
    }
    if ( who == "sphere" )
    {
        if ( what == "position" || what.empty() )
            return reportSpherePosition(out);
        throw InvalidSyntax("I only know `sphere'");
    }
    if ( who == "single" )
    {
        if ( what.empty() )
            return reportSingle(out);
        if ( what == "state" || what == "force" )
            return reportSingleState(out, which);
        if ( what == "position" )
            return reportSinglePosition(out, which);
        if ( what == "attached" )
            return reportAttachedSingle(out, which);
        throw InvalidSyntax("I only know single: state, force, position, NAME");
    }
    if ( who == "couple" )
    {
        if ( what.empty() )
            return reportCouple(out);
        else if ( what == "state" )
        {
            if ( which.empty() )
                return reportCoupleState(out);
            else
                return reportCoupleState(out, which);
        }
        else if ( what == "link" )
            return reportCoupleLink(out, which);
        else if ( what == "configuration" )
            return reportCoupleConfiguration(out, which, opt);
        else if ( what == "force" )
            return reportCoupleForce(out, opt);
        else if ( what == "active" )
            return reportCoupleActive(out, which);
        else if ( what == "anatomy" )
            return reportCoupleAnatomy(out);
        else
            return reportCoupleState(out, what);
        throw InvalidSyntax("I only know couple: state, link, active, force, anatomy, NAME");
    }
    if ( who == "organizer" )
    {
        if ( what.empty() )
            return reportOrganizer(out);
        throw InvalidSyntax("I only know `organizer'");
    }
    if ( who == "aster" )
    {
        if ( what.empty() )
            return reportAster(out);
        throw InvalidSyntax("I only know `aster'");
    }
    if ( who == "field" )
    {
        return reportField(out);
    }
    if ( who == "time" )
    {
        if ( what.empty() )
            return reportTime(out);
        throw InvalidSyntax("I only know `time'");
    }
    if ( who == "inventory" )
    {
        if ( what.empty() )
            return reportInventory(out);
        throw InvalidSyntax("I only know `inventory'");
    }
    if ( who == "system" )
    {
        return reportSystem(out);
    }
    if ( who == "property" || who == "parameter" )
    {
        if ( what.empty() )
            return writeProperties(out, false);
        else
        {
            Property * p = findProperty(what);
            if ( !p )
                throw InvalidSyntax("unknown Property `"+what+"'");
            p->write(out);
            return;
        }
    }
    if ( who == "spindle" )
    {
        if ( what == "indice" )
            return reportIndices(out);
        if ( what == "profile" )
            return reportProfile(out);
        throw InvalidSyntax("I only know spindle: indices, profile");
    }
    if ( who == "network" )
    {
        if ( what == "size" )
            return reportNetworkSize(out);
    }
    if ( who == "ring" )
        return reportRing(out);
    if ( who == "platelet" )
        return reportPlatelet(out);
    if ( who == "ashbya" )
        return reportAshbya(out);
    if ( who == "custom" )
        return reportCustom(out);

    if ( !who.empty() )
        throw InvalidSyntax("Unknown requested report `"+arg+"'");
}

//------------------------------------------------------------------------------
#pragma mark - Fiber Aggregated Properties

/**
 Export average length and standard-deviation for each class of fiber
 */
void Simul::reportFiberAge(std::ostream& out) const
{
    out << COM << ljust("class", 2, 2) << SEP << "count" << SEP << "avg_birth";
    out << SEP << "dev_birth" << SEP << "avg_age" << SEP << "min_age" << SEP << "max_age";
    
    unsigned cnt;
    real avg, dev, mn, mx;
    const real now = prop->time;

    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp * fp = static_cast<FiberProp*>(i);
        ObjectList objs = fibers.collect(fp);
        fibers.infoBirthtime(objs, cnt, avg, dev, mn, mx);
        out << LIN << ljust(fp->name(), 2);
        out << SEP << cnt;
        out << SEP << avg;
        out << SEP << dev;
        out << SEP << now-mx;
        out << SEP << now-avg;
        out << SEP << now-mn;
    }
}

/**
 Export average length and standard-deviation for each class of fiber
 */
void Simul::reportFiberLengths(std::ostream& out) const
{
    out << COM << ljust("class", 2, 2) << SEP << "count" << SEP << "avg_len" << SEP << "std_dev";
    out << SEP << "min_len" << SEP << "max_len" << SEP << "total";

    unsigned cnt;
    real avg, dev, mn, mx;
    
    std::streamsize p = out.precision();
    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp * fp = static_cast<FiberProp*>(i);
        
        ObjectList objs = fibers.collect(fp);
        fibers.infoLength(objs, cnt, avg, dev, mn, mx);
        
        out << LIN << ljust(fp->name(), 2);
        out << SEP << cnt;
        out.precision(3);
        out << SEP << std::fixed << avg;
        out << SEP << std::fixed << dev;
        out << SEP << std::fixed << mn;
        out << SEP << std::fixed << mx;
        out.precision(1);
        out << SEP << std::fixed << avg*cnt;
    }
    out.precision(p);
}


/**
 Export average length and standard-deviation for each class of fiber
 */
void Simul::reportFiberLengthDistribution(std::ostream& out, Glossary & opt) const
{
    const size_t BMAX = 256;
    unsigned cnt[BMAX+1];
    
    real delta = 1;
    size_t nbin = 32;
    opt.set(delta, "interval");
    opt.set(nbin, "interval", 1);
    nbin = std::min(nbin, BMAX);
    
    std::streamsize p = out.precision();

    if ( 1 )
    {
        out.precision(2);
        out << COM << "length_distribution (`scale` indicates the center of each bin)";
        out << LIN << ljust("scale", 2);
        for ( unsigned u = 0; u <= nbin; ++u )
            out << " " << std::setw(5) << delta * ( u + 0.5 );
    }
    out.precision(4);

    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp * fp = static_cast<FiberProp*>(i);
        
        for ( unsigned u = 0; u <= nbin; ++u )
            cnt[u] = 0;
        
        for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
        {
            if ( obj->prop == fp )
            {
                size_t u = size_t(floor( obj->length() / delta ));
                if ( u < nbin )
                    ++cnt[u];
                else
                    ++cnt[nbin];
            }
        }

        out << LIN << ljust(fp->name(), 2);
        for ( unsigned u = 0; u <= nbin; ++u )
            out << " " << std::setw(5) << cnt[u];
    }
    out.precision(p);
}


/**
 Export number of fiber, classified according to dynamic state of one end
 */
void Simul::reportFiberDynamic(std::ostream& out, FiberEnd end) const
{
    const int MAX = 5;
    int cnt[MAX] = { 0 };
    int sum = 0;
    
    for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
    {
        ++sum;
        unsigned s = obj->dynamicState(end);
        if ( s < MAX )
            ++cnt[s];
    }

    if ( end == PLUS_END )
        out << LIN << ljust("plus_end", 1);
    else if ( end == MINUS_END )
        out << LIN << ljust("minus_end", 1);
 
    out << SEP << sum;
    for ( int ii = 0; ii < MAX; ++ii )
        out << SEP << cnt[ii];
}

/**
 Export number of fiber, classified according to dynamic state of one end
 */
void Simul::reportFiberDynamic(std::ostream& out) const
{
    out << COM << "fiber_end" << SEP << "total" << SEP << "static";
    out << SEP << "green" << SEP << "yellow" << SEP << "orange"  << SEP << "red";
    reportFiberDynamic(out, PLUS_END);
    reportFiberDynamic(out, MINUS_END);
}


void Simul::reportFiberSegments(std::ostream& out) const
{
    out << COM << ljust("class", 2, 2) << SEP << "nb_fibers" << SEP << "nb_joints";
    out << SEP << "nb_kinks" << SEP << "min_seg" << SEP << "max_seg";
    
    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp * fp = static_cast<FiberProp*>(i);
        
        unsigned cnt, joints;
        real mn = 0, mx = 0;
        
        ObjectList objs = fibers.collect(fp);
        fibers.infoSegments(objs, cnt, joints, mn, mx);
        out << LIN << ljust(fp->name(), 2);
        out << SEP << cnt;
        out << SEP << joints;
        out << SEP << fibers.nbKinks(objs);
        out << SEP << std::fixed << mn;
        out << SEP << std::fixed << mx;
    }
}


void Simul::reportFiberHands(std::ostream& out) const
{
    out << COM << "fib_type" << SEP << "fib_id" << SEP << "class" << SEP << "abs";
    for ( Fiber * fib=fibers.first(); fib; fib=fib->next() )
    {
        if ( fib->nbHands() > 0 )
        {
            out << COM << "on fiber " << fib->reference();
            fib->sortHands();
            for ( Hand * ha = fib->firstHand(); ha; ha = ha->next() )
            {
                out << LIN << fib->prop->number();
                out << SEP << fib->identity();
                out << SEP << ha->prop->number();
                out << SEP << ha->abscissa();
            }
        }
    }
}


void Simul::reportFiberLinks(std::ostream& out) const
{
    out << COM << "fib_type" << SEP << "fib_id" << SEP << "class" << SEP << "abs" << SEP << "position";
    for ( Fiber * fib=fibers.first(); fib; fib=fib->next() )
    {
        if ( fib->nbHands() > 0 )
        {
            out << COM << "on fiber " << fib->reference();
            fib->sortHands();
            for ( Hand * ha = fib->firstHand(); ha; ha = ha->next() )
            {
                if ( ha->linkStiffness() > 0 )
                {
                    out << LIN << fib->prop->number();
                    out << SEP << fib->identity();
                    out << SEP << ha->prop->number();
                    out << SEP << ha->abscissa();
                    out << SEP << ha->otherPosition();
                }
            }
        }
    }
}


/**
 Report quantity of substance in Field
 */
void Simul::reportFiberLattice(std::ostream& out, bool density) const
{
    out << COM << ljust("class", 2, 2);
    out << SEP << "total" << SEP << "avg" << SEP << "min" << SEP << "max" << SEP << "length";
    
    // report substance on Fiber Lattices
    unsigned cnt = 0;
    real len = 0, sm = 0, mn = INFINITY, mx = -INFINITY;
    
    for ( Fiber const* fib = fibers.first(); fib; fib = fib->next() )
        fib->infoLattice(len, cnt, sm, mn, mx);

    int p = out.precision();
    out << LIN << ljust("fiber:lattice", 2);
    out << SEP << sm;
    out << SEP << std::setprecision(4) << sm / cnt;
    out << SEP << std::fixed << std::setprecision(6) << mn;
    out << SEP << std::fixed << std::setprecision(6) << mx;
    out << SEP << std::setprecision(3) << len;
    out.precision(p);
}


//------------------------------------------------------------------------------
#pragma mark - Fiber Individual Properties


/**
 Export length, position and directions at center of fibers
 */
void Simul::reportFibers(std::ostream& out, Property const* sel) const
{
    out << COM << "class" << SEP << "identity" << SEP << "length";
    out << SEP << repeatXYZ("pos") << SEP << repeatXYZ("dir");
    out << SEP << "endToEnd" << SEP << "cosinus" << SEP << "organizer";
    //out << COM << "fiber class " + std::to_string(sel->number()) + " is " + sel->name();

    // list fibers in the order of the inventory:
    for ( Fiber const* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
    {
        if ( !sel || sel == fib->prop )
        {
            out << LIN << fib->prop->number();
            out << SEP << fib->identity();
            out << SEP << fib->length();
            out << SEP << fib->posEnd(CENTER);
            out << SEP << fib->dirEnd(CENTER);
            out << SEP << (fib->posEndM()-fib->posEndP()).norm();
            out << SEP << dot(fib->dirEndM(), fib->dirEndP());
            out << SEP << organizers.findOrganizerID(fib);
        }
    }
}


/**
 Export length, position and directions at center of fibers
 */
void Simul::reportFibers(std::ostream& out, std::string const& which) const
{
    Property * p = properties.find_or_die("fiber", which);
    reportFibers(out, static_cast<FiberProp*>(p));
}


/**
 Export length, position and directions at center of fibers
 */
void Simul::reportFibers(std::ostream& out) const
{
    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp const* fp = static_cast<FiberProp*>(i);
        out << COM << "fiber class " + std::to_string(fp->number()) + " is " + fp->name();
        reportFibers(out, fp);
    }
}


/**
 Export dynamic state, positions and directions of fiber at both ends
 */
void Simul::reportFiberEnds(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << "length";
    out << SEP << "stateM" << SEP << repeatXYZ("posM") << SEP << repeatXYZ("dirM");
    out << SEP << "stateP" << SEP << repeatXYZ("posP") << SEP << repeatXYZ("dirP");
    
    for ( Fiber const* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
    {
        out << LIN << fib->prop->number();
        out << SEP << fib->identity();
        out << SEP << fib->length();
        out << SEP << fib->dynamicStateM();
        out << SEP << fib->posEndM();
        out << SEP << fib->dirEndM();
        out << SEP << fib->dynamicStateP();
        out << SEP << fib->posEndP();
        out << SEP << fib->dirEndP();
    }
}


/**
 Export Fiber-number, position of vertices
 */
void Simul::reportFiberPoints(std::ostream& out) const
{
    out << COM << "identity" << SEP << repeatXYZ("pos") << SEP << "curvature";

    // list fibers in the order of the inventory:
    for ( Fiber const* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
    {
        out << COM << "fiber " << fib->reference() << "  " << fib->segmentation();
        
        for ( unsigned p = 0; p < fib->nbPoints(); ++p )
        {
            out << LIN << fib->identity();
            out << SEP << fib->posP(p);
            out << SEP << fib->curvature(p);
        }
    }
}


/**
 Export positions of points taken randomly along all fibers,
 but that remain static with respect to the lattice of each fiber,
 during the life-time of this fiber.
 
 This is meant to simulate the `speckle microscopy` that is obtained
 in microcscopy with a low amount of fluorescent-monomers.
 
 The distance between the speckles follows an exponential distribution
 with an average defined by the parameter `spread`.
 */
void Simul::reportFiberSpeckles(std::ostream& out, Glossary& opt) const
{
    const real ONE_OVER_32 = 0x1p-32;
    real spread = 1;
    if ( opt.set(spread, "density") )
        spread = 1.0 / spread;
    else
        opt.set(spread, "interval");

    Fiber * fib = fibers.first();
    while ( fib )
    {
        out << COM << "fiber " << fib->reference();
        
        // generate speckles below the origin of abscissa
        if ( fib->abscissaM() < 0 )
        {
            uint32_t z = fib->signature();
            real a = spread * log(z*ONE_OVER_32);
            while ( a > fib->abscissaP() )
            {
                z = lcrng2(z);
                a += spread * log(z*ONE_OVER_32);
            }
            while ( a >= fib->abscissaM() )
            {
                out << '\n' << fib->pos(a);
                z = lcrng2(z);
                a += spread * log(z*ONE_OVER_32);
            }
        }
        // generate speckles above the origin of abscissa
        if ( fib->abscissaP() > 0 )
        {
            uint32_t z = ~fib->signature();
            real a = -spread * log(z*ONE_OVER_32);
            while ( a < fib->abscissaM() )
            {
                z = lcrng1(z);
                a -= spread * log(z*ONE_OVER_32);
            }
            while ( a <= fib->abscissaP() )
            {
                out << '\n' << fib->pos(a);
                z = lcrng1(z);
                a -= spread * log(z*ONE_OVER_32);
            }
        }
        
        fib = fib->next();
    }
}


/**
 Export positions of points taken randomly along all fibers,
 changing the distribution at every time.
 */
void Simul::reportFiberSamples(std::ostream& out, Glossary& opt) const
{
    real spread = 1;
    if ( opt.set(spread, "density") )
        spread = 1.0 / spread;
    else
        opt.set(spread, "interval");
    
    Array<FiberSite> loc(1024);
    fibers.uniFiberSites(loc, spread);
    
    Fiber * ofib = nullptr;
    for ( FiberSite & i : loc )
    {
        if ( ofib != i.fiber() )
        {
            out << COM << "fiber " << i.fiber()->reference();
            ofib = i.fiber();
        }
        
        out << LIN << i.pos();
    }
}


/**
 Export Mean Squared displacement of fibers since the last call
 to this function.
 */
void Simul::reportFiberDisplacement(std::ostream& out) const
{
    typedef std::map <ObjectID, Vector> fiber_map;
    static fiber_map positions;
    static real old_time = 0;
    
    out << COM << "delta_time nb_fibers mean_squared_displacement";
    
    real sum = 0;
    unsigned cnt = 0;
    for ( Fiber * fib=fibers.first(); fib; fib=fib->next() )
    {
        Vector pos = fib->posEndM();
        fiber_map::iterator i = positions.find(fib->identity());
        if ( i != positions.end() )
        {
            ++cnt;
            sum += distanceSqr(pos, i->second);
            i->second = pos;
        }
        else
        {
            positions[fib->identity()] = pos;
        }
    }
    
    if ( cnt > 0 )
        out << LIN << time() - old_time << SEP << cnt << SEP << sum / cnt;
    else
        out << LIN << time() - old_time << SEP << 0 << SEP << 0;
    
    old_time = time();
}


/**
 Export first and second-order moments of vertices
 */
void Simul::reportFiberMoments(std::ostream& out) const
{
    out << COM << ljust("class", 2, 2) << SEP << "sum";
    out << SEP << "avgX" << SEP << "avgY" << SEP << "avgZ";
    out << SEP << "varX" << SEP << "varY" << SEP << "varZ" << SEP << "var_sum";
    out << std::fixed;
    
    Accumulator accum;
    
    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp * fp = static_cast<FiberProp*>(i);
        
        accum.reset();
       
        for ( Fiber * fib=fibers.first(); fib; fib=fib->next() )
        {
            if ( fib->prop == fp )
            {
                const real w = fib->segmentation();
                accum.add(0.5*w, fib->posEndM());
                for ( unsigned n = 1; n < fib->lastPoint(); ++n )
                    accum.add(w, fib->posP(n));
                accum.add(0.5*w, fib->posEndP());
            }
        }
        
        accum.subtract_mean();
        out << LIN << ljust(fp->name(), 2);
        accum.print(out, 0);
    }
}


/**
 Export first and second-order moments of vertices for each class of Fiber
 */
void Simul::reportNetworkSize(std::ostream& out) const
{
    out << COM << "polymer" << SEP << "surface";
    out << std::fixed;
    Accumulator acc;
    for ( Fiber const* fib = fibers.first(); fib; fib = fib->next() )
    {
        const real w = fib->segmentation();
        acc.add(0.5*w, fib->posEndM());
        for ( size_t n = 1; n < fib->lastPoint(); ++n )
            acc.add(w, fib->posPoint(n));
        acc.add(0.5*w, fib->posEndP());
    }
    acc.subtract_mean();
    real S = 2 * M_PI * acc.total_variance();
    out << LIN << acc.total_length() << SEP << S;
}

//------------------------------------------------------------------------------
#pragma mark - Fiber forces

/**
 Export Fiber-number, position of vertices and tension in each segment
 */
void Simul::reportFiberForces(std::ostream& out) const
{
    computeForces();

    out << COM << "identity" << SEP << repeatXYZ("pos") << SEP << repeatXYZ("force") << SEP << "tension";
    
    // list fibers in the order of the inventory:
    for ( Fiber const* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
    {
        out << COM << "fiber " << fib->reference();
            
        for ( unsigned p = 0; p < fib->nbPoints(); ++p )
        {
            out << LIN << fib->identity();
            out << SEP << fib->posP(p);
            out << SEP << fib->netForce(p);
            if ( p == fib->lastPoint() )
                out << SEP << 0.0;
            else
                out << SEP << fib->tension(p);
        }
    }
}


/**
 Sum of the internal tensions from fiber segments that intersect a plane
 specified in `opt`.
 The plane is defined by <em> n.pos + a = 0 </em>

     plane = NORMAL, SCALAR

 */
void Simul::reportFiberTension(std::ostream& out, Glossary& opt) const
{
    computeForces();
    
    out << COM << "count" << SEP << "force";

    Vector n(1,0,0);
    real ten = 0;
    unsigned cnt = 0;
    if ( opt.value_is("plane", 0, "all") )
    {
        // extending the comments:
        for ( int d = 1; d < DIM; ++d )
            out << SEP << "count" << SEP << "force";
        
        // plane orthogonal to X:
        fibers.infoTension(cnt, ten, Vector(1,0,0), 0);
        out << LIN << cnt << SEP << ten;
#if ( DIM > 1 )
        // plane orthogonal to Y:
        fibers.infoTension(cnt, ten, Vector(0,1,0), 0);
        out << SEP << cnt << SEP << ten;
#endif
#if ( DIM > 2 )
        // plane orthogonal to Z:
        fibers.infoTension(cnt, ten, Vector(0,0,1), 0);
        out << SEP << cnt << SEP << ten;
#endif
    }
    else if ( opt.set(n, "plane") )
    {
        real a = 0;
        opt.set(a, "plane", 1);
        out << COM << "fiber tension orthogonal to plane: (" << n << ").pos = " << -a;
        fibers.infoTension(cnt, ten, n, a);
        out << LIN << cnt << SEP << ten;
    }
    else
    {
        // if no plane is specified, sum all tension from all segments
        fibers.infoTension(cnt, ten);
        out << LIN << cnt << SEP << ten;
    }
}


/**
 Export fiber elastic bending energy
 */
void Simul::reportFiberBendingEnergy(std::ostream& out) const
{
    out << COM << ljust("class",2,2) << SEP << "count" << SEP << "sum" << SEP << "avg" << SEP << "dev";
    
    unsigned cnt;
    real avg, dev;
    
    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp * fp = static_cast<FiberProp*>(i);
        ObjectList objs = fibers.collect(fp);
        fibers.infoBendingEnergy(objs, cnt, avg, dev);
        if ( cnt > 0 )
        {
            out << LIN << ljust(fp->name(), 2);
            out << SEP << cnt;
            out << SEP << avg*cnt;
            out << SEP << avg;
            out << SEP << dev;
        }
    }
}


/**
 Export total magnitude of force exerted by Fiber on the confinement
 */
void Simul::reportFiberConfineForce(std::ostream& out) const
{
    out << COM << "confinement forces";
    out << COM << "identity" << SEP << repeatXYZ("pos") << SEP << repeatXYZ("force");
     
     // list fibers in the order of the inventory:
     for ( Fiber const* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
     {
         out << COM << "fiber " << fib->reference();
         Space const* spc = findSpace(fib->prop->confine_space);
         const real stiff = fib->prop->confine_stiffness;

         for ( size_t p = 0; p < fib->nbPoints(); ++p )
         {
             out << LIN << fib->identity();
             Vector w, pos = fib->posP(p);
             out << SEP << pos;
             if ( spc->outside(pos) )
             {
                 w = spc->project(pos);
                 out << SEP << stiff * ( w - pos );
             }
             else
             {
                 out << SEP << Vector(0,0,0);
             }
         }
     }
}


/**
 Export total magnitude of force exerted by Fiber on the confinement
 */
real Simul::reportFiberConfinement(std::ostream& out) const
{
    out << COM << "count" << SEP << repeatXYZ("force") << SEP << "radial";
    size_t cnt = 0;
    Vector sum(0,0,0);
    real   rad = 0;
    
#if ( DIM > 1 )
    for ( Fiber * fib=fibers.first(); fib; fib=fib->next() )
    {
        Space const* spc = findSpace(fib->prop->confine_space);
        const real stiff = fib->prop->confine_stiffness;
        
        for ( unsigned p = 0; p < fib->nbPoints(); ++p )
        {
            Vector w, pos = fib->posP(p);
            if ( spc->outside(pos) )
            {
                ++cnt;
                w = spc->project(pos);
                Vector dir = normalize(Vector(pos.XX, pos.YY, 0));
                Vector vec = stiff * ( pos - w );
                sum += vec;
                rad += dot(vec, dir);
            }
        }
    }
#endif
    out << LIN << cnt << SEP << sum << SEP << rad;
    return rad;
}


//------------------------------------------------------------------------------
#pragma mark - Networks


void Simul::reportFiberIntersections(std::ostream& out, Glossary& opt) const
{
    int details = 2;
    real up = 0;
    opt.set(up, "threshold");
    opt.set(details, "details");
    
    const real sup = up * up;
    real abs1, abs2;
    
    if ( details == 2 )
    {
        out << COM << "id1" << SEP << "abs1";
        out << SEP << "id2" << SEP << "abs2" << SEP << repeatXYZ("pos");
    }
    Accumulator accum;
    
    for ( Fiber const* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
    {
        unsigned cnt = 0;
        for ( Fiber * fob = fibers.nextID(fib); fob; fob = fibers.nextID(fob) )
        {
            for ( unsigned ii = 0; ii < fib->nbSegments(); ++ii )
            {
                FiberSegment seg1(fib, ii);
                for ( unsigned jj = 0; jj < fob->nbSegments(); ++jj )
                {
                    FiberSegment seg2(fob, jj);
                    if ( seg1.shortestDistance(seg2, abs1, abs2) < sup )
                    {
                        if ( seg1.within(abs1) & seg2.within(abs2) )
                        {
                            ++cnt;
                            Vector pos1 = seg1.pos(abs1/seg1.len());
                            //Vector pos2 = loc2.pos(abs2/loc2.len());
                            if ( details == 2 )
                            {
                                out << LIN << fib->identity();
                                out << SEP << abs1 + seg1.abscissa1();
                                out << SEP << fob->identity();
                                out << SEP << abs2 + seg2.abscissa1();
                                out << SEP << pos1;
                            }
                            accum.add(pos1);
                        }
                    }
                }
            }
        }
        if ( cnt && details >= 1 )
        {
            out << COM << "total";
            out << SEP << fib->identity();
            out << SEP << cnt;
        }
    }
    accum.subtract_mean();
    accum.print_doc(out);
    accum.print(out, 1);
}


//------------------------------------------------------------------------------
#pragma mark - Beads, Solid, Space


void Simul::reportTime(std::ostream& out) const
{
    out << LIN << prop->time;
}


void Simul::reportInventory(std::ostream& out) const
{
    //out << COM << "properties:";
    //properties.write_names(out, "");
    //out << COM << "objects:";
    spaces.report(out);
    fields.report(out);
    fibers.report(out);
    spheres.report(out);
    beads.report(out);
    solids.report(out);
    singles.report(out);
    couples.report(out);
    organizers.report(out);
    events.report(out);
}


template < typename SET >
void reportSystemSet(std::ostream& out, SET& set, PropertyList const& properties)
{
    for ( Property const* i : properties.find_all(set.title()) )
    {
        unsigned points = 0, sup = 0;
        ObjectList objs = set.collect(match_property, i);
        for ( Object * o : objs )
        {
            Mecable * mec = Simul::toMecable(o);
            if ( mec )
            {
                points += mec->nbPoints();
                sup = std::max(sup, mec->nbPoints());
            }
        }
        if ( points > 0 )
        {
            out << LIN << ljust(i->name(), 2);
            out << SEP << objs.size();
            out << SEP << points << SEP << sup;
        }
    }
}


void Simul::reportSystem(std::ostream& out) const
{
    out << COM << ljust("class", 2, 2) << SEP << "count" << SEP << "vertices" << SEP << "largest";
    reportSystemSet(out,  fibers, properties);
    reportSystemSet(out,  solids, properties);
    reportSystemSet(out, spheres, properties);
    reportSystemSet(out,   beads, properties);
}


/**
 Export position of all organizers
 */
void Simul::reportOrganizer(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << repeatXYZ("pos");

    for ( Organizer * obj=organizers.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->property()->number();
        out << SEP << obj->identity();
        out << SEP << obj->position();
        out << SEP << obj->nbOrganized();
    }
}


/**
 Export position of Asters
 */
void Simul::reportAster(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << repeatXYZ("pos");
    
    for ( Organizer * obj=organizers.first(); obj; obj=obj->next() )
    {
        if ( obj->tag() == Aster::TAG )
        {
            out << LIN << obj->property()->number();
            out << SEP << obj->identity();
            out << SEP << obj->position();
        }
    }
}


/**
 Export position of Beads
 */
void Simul::reportBeadPosition(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << repeatXYZ("pos");
    
    for ( Bead * obj=beads.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->prop->number();
        out << SEP << obj->identity();
        out << SEP << obj->position();
    }
}


/**
 Export number of beads classified as a function of
 the number of grafted Single that are attached to Fibers
 */
void Simul::reportBeadSingles(std::ostream& out) const
{
    out << COM << "identity" << "amount(nb_attached_hands)";
    
    std::map<ObjectID, int> cnt;
    
    for ( Single * sig=singles.firstA(); sig; sig=sig->next() )
    {
        Bead const* obj = Bead::toBead(sig->base());
        if ( obj )
            ++cnt[ obj->identity() ];
    }

    const int max = 12;
    int nb[max] = { 0 };
    for ( Bead * obj=beads.first(); obj; obj=obj->next() )
        ++nb[ cnt[obj->identity()] ];
    
    for ( int c = 0; c < max; ++c )
        out << " " << std::setw(3) << nb[c];
}


/**
 Export position of Solids
 */
void Simul::reportSolidPosition(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << repeatXYZ("cen");
    out << SEP << repeatXYZ("point0") << SEP << repeatXYZ("point1");
    
    for ( Solid * obj=solids.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->prop->number();
        out << SEP << obj->identity();
        out << SEP << obj->centroid();
        out << SEP << obj->posP(0);
        if ( obj->nbPoints() > 1 )
            out << SEP << obj->posP(1);

        if ( modulo ) 
        {
            Vector pos = obj->centroid();
            modulo->fold(pos);
            out << SEP << pos;
        }
    }
}

/**
 Export position of Solids with counts of Hands and attached Hands
 */
void Simul::reportSolidHands(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << repeatXYZ("pos");
    out << SEP << "nb_hand" << SEP << "nb_link";
    
    for ( Solid const* obj = solids.firstID(); obj; obj = solids.nextID(obj) )
    {
        out << LIN << obj->prop->number();
        out << SEP << obj->identity();
        Vector pos = obj->centroid();
        if ( modulo ) modulo->fold(pos);
        out << SEP << pos;
        SingleList anchored = singles.collectWrists(obj);
        int cnt = 0;
        for ( Single const* s : anchored )
            cnt += s->attached();
        out << SEP << anchored.size() << SEP << cnt;
    }
}


/**
 Report position of Sphere
 */
void Simul::reportSpherePosition(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity";
    out << SEP << repeatXYZ("point0") << SEP << repeatXYZ("point1");
  
    for ( Sphere * obj=spheres.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->prop->number();
        out << SEP << obj->identity();
        out << SEP << obj->posP(0);
        if ( obj->nbPoints() > 1 )
            out << SEP << obj->posP(1);
    }
}


/**
 Report something about Space (incomplete)
 */
void Simul::reportSpace(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity";
    
    for ( Space * obj=spaces.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->prop->name();
        out << SEP << obj->identity();
        out << SEP << std::fixed << obj->prop->shape;
    }
}


/**
 Report force on Space (unfinished)
 */
void Simul::reportSpaceForce(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << "shape";
    
    for ( Space * obj=spaces.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->prop->name();
        out << SEP << obj->identity();
        out << SEP << obj->prop->shape;
    }
}


/**
 Report quantity of substance in Field
 */
void Simul::reportField(std::ostream& out) const
{
    out << COM << ljust("class", 2, 2);
    out << SEP << "total" << SEP << "avg" << SEP << "min" << SEP << "max";
    
    // report total substance in each Field
    for ( Field * obj=fields.first(); obj; obj=obj->next() )
    {
        real vol = obj->cellVolume();
        Field::value_type s, n, x;
        obj->infoValues(s, n, x);
        out << LIN << ljust(obj->prop->name(), 2);
        out << SEP << s;
        out << SEP << s / ( vol * obj->nbCells() );
        out << SEP << n / vol;
        out << SEP << x / vol;
    }
}


//------------------------------------------------------------------------------
#pragma mark - Single

void write(std::ostream& out, Single const* obj, Simul const* simul)
{
    out << LIN << obj->prop->number();
    out << SEP << obj->identity();
    out << SEP << obj->position();
    Fiber const* fib = obj->fiber();
    if ( fib )
    {
        out << SEP << obj->force();
        out << SEP << fib->identity();
        out << SEP << obj->abscissa();
        out << SEP << simul->organizers.findOrganizerID(fib);
    }
    else
    {
        out << SEP << Vector(0,0,0);
        out << SEP << "0";
        out << SEP << "nan";
        out << SEP << "0";
    }
}


/**
 Export details of Singles, possiby selecting for a certain kind
 */
void Simul::reportSingleState(std::ostream& out, std::string const& which) const
{
    Property * selected = nullptr;
    
    if ( which.size() )
        selected = properties.find_or_die("single", which);

    out << COM << "class" << SEP << "identity";
    out << SEP << repeatXYZ("pos") << SEP << repeatXYZ("force");
    out << SEP << "fiber" << SEP << "abscissa" << SEP << "aster";
    
    for ( Single const* obj = singles.firstID(); obj; obj = singles.nextID(obj) )
        if ( !selected || obj->prop == selected )
            write(out, obj, this);
}


/**
 Export details of Singles
 */
void Simul::reportSinglePosition(std::ostream& out, std::string const& which) const
{
    Property * selected = nullptr;
    
    if ( which.size() )
        selected = properties.find_or_die("single", which);

    out << COM << "class" << SEP << "identity" << SEP << repeatXYZ("pos");
    out << SEP << "fiber" << SEP << "abscissa";

    for ( Single const* obj = singles.firstID(); obj; obj = singles.nextID(obj) )
    {
        if ( !selected || obj->prop == selected )
        {
            out << LIN << obj->prop->number();
            out << SEP << obj->identity();
            out << SEP << obj->posFoot();
            if ( obj->attached() )
            {
                out << SEP << obj->fiber()->identity();
                out << SEP << obj->abscissa();
            }
            else
            {
                out << SEP << 0 << SEP << 0;
            }
        }
    }
}

/**
 Export details of attached Singles
 */
void Simul::reportAttachedSingle(std::ostream& out, std::string const& which) const
{
    Property * selected = nullptr;
    
    if ( which.size() )
        selected = properties.find_or_die("single", which);

    out << COM << "class" << SEP << "identity";
    out << SEP << repeatXYZ("pos") << SEP << repeatXYZ("force");
    out << SEP << "fiber" << SEP << "abscissa" << SEP << "aster";

    for ( Single const* obj = singles.firstID(); obj; obj = singles.nextID(obj) )
    {
        if ( obj->attached()  && ( !selected || obj->prop == selected ))
            write(out, obj, this);
    }
}


/**
 Export number of Single in each state
 */
void Simul::reportSingle(std::ostream& out) const
{
    const unsigned mx = 128;
    
    int free[mx] = { 0 }, bound[mx] = { 0 };
    
    for ( Single * si = singles.firstF(); si ; si = si->next() )
    {
        assert_true(!si->attached());
        unsigned ix = si->prop->number();
        if ( ix < mx )
            ++free[ix];
    }
    
    for ( Single * sig=singles.firstA(); sig ; sig=sig->next() )
    {
        assert_true(sig->attached());
        unsigned ix = sig->prop->number();
        if ( ix < mx )
            ++bound[ix];
    }
    
    if ( 1 )
    {
        out << COM << ljust("single", 2, 2);
        out << SEP << "total";
        out << SEP << "free";
        out << SEP << "bound";
    }
    
    for ( Property * i : properties.find_all("single") )
    {
        out << LIN << ljust(i->name(), 2);
        unsigned ix = i->number();
        if ( ix < mx )
        {
            out << SEP << free[ix] + bound[ix];
            out << SEP << free[ix];
            out << SEP << bound[ix];
        }
        else
            out << SEP << " out-of-range ";
    }
}


//------------------------------------------------------------------------------
#pragma mark - Couple


void write(std::ostream& out, Couple * obj)
{
    out << LIN << obj->prop->number();
    out << SEP << obj->identity();
    out << SEP << obj->active();
    out << SEP << obj->position();

    Fiber const* fib = obj->fiber1();
    if ( fib )
    {
        out << SEP << fib->identity();
        out << SEP << obj->abscissa1();
    }
    else
    {
        out << SEP << "0";
        out << SEP << "nan";
    }

    fib = obj->fiber2();
    if ( fib )
    {
        out << SEP << fib->identity();
        out << SEP << obj->abscissa2();
    }
    else
    {
        out << SEP << "0";
        out << SEP << "nan";
    }
}

/**
 Export position of Couples
 */
void Simul::reportCoupleState(std::ostream& out) const
{
    out << COM << "class" << SEP << "identity" << SEP << "active" << SEP << repeatXYZ("pos");
    out << SEP << "fiber1" << SEP << "abscissa1" << SEP << "fiber2" << SEP << "abscissa2";

    for ( Couple * obj=couples.firstFF(); obj ; obj=obj->next() )
        write(out, obj);
    
    for ( Couple * obj=couples.firstAF(); obj ; obj=obj->next() )
        write(out, obj);
    
    for ( Couple * obj=couples.firstFA(); obj ; obj=obj->next() )
        write(out, obj);
    
    for ( Couple * obj=couples.firstAA(); obj ; obj=obj->next() )
        write(out, obj);
}

/**
 Export position of Couples of a certain kind
 */
void Simul::reportCoupleState(std::ostream& out, std::string const& which) const
{
    Property * selected = properties.find_or_die("couple", which);
    
    out << COM << "class" << SEP << "identity" << SEP << "active" << SEP << repeatXYZ("pos");
    out << SEP << "fiber1" << SEP << "abscissa1" << SEP << "fiber2" << SEP << "abscissa2";
    
    for ( Couple * obj=couples.firstFF(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            write(out, obj);
    
    for ( Couple * obj=couples.firstAF(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            write(out, obj);
    
    for ( Couple * obj=couples.firstFA(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            write(out, obj);
    
    for ( Couple * obj=couples.firstAA(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            write(out, obj);
}


/**
 Export position of active Couples of a certain kind
 */
void Simul::reportCoupleActive(std::ostream& out, std::string const& which) const
{
    Property * selected = properties.find_or_die("couple", which);
    
    out << COM << "state" << SEP << repeatXYZ("pos");
    
    for ( Couple * obj=couples.firstFF(); obj ; obj=obj->next() )
        if ( obj->active()  &&  obj->prop == selected )
            out << "\n 0 " << obj->position();
   
    for ( Couple * obj=couples.firstAF(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            out << "\n 1 " << obj->position();
    
    for ( Couple * obj=couples.firstFA(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            out << "\n 2 " << obj->position();
    
    for ( Couple * obj=couples.firstAA(); obj ; obj=obj->next() )
        if ( obj->prop == selected )
            out << "\n 3 " << obj->position();
}


/**
 Export position and force of Couples that are bound to 2 filaments
 */
void Simul::reportCoupleLink(std::ostream& out, std::string const& which) const
{
    Property * selected = nullptr;
    
    if ( which.size() )
        selected = properties.find_or_die("couple", which);
   
    out << COM << "class" << SEP << "identity";
    out << SEP << "fiber1" << SEP << "abscissa1";// << SEP << repeatXYZ("pos1");
    out << SEP << "fiber2" << SEP << "abscissa2";// << SEP << repeatXYZ("pos2");
    out << SEP << "force" << SEP << "cos_angle";
    
    for ( Couple * obj=couples.firstAA(); obj ; obj=obj->next() )
    {
        if ( !selected || obj->prop == selected )
        {
            out << LIN << obj->prop->number();
            out << SEP << obj->identity();
            
            out << SEP << obj->fiber1()->identity();
            out << SEP << obj->abscissa1();
            //out << SEP << obj->posHand1();

            out << SEP << obj->fiber2()->identity();
            out << SEP << obj->abscissa2();
            //out << SEP << obj->posHand2();

            out << SEP << obj->force().norm();
            out << SEP << obj->cosAngle();
        }
    }
}


/**
 Export configuration of bridging couple, as
 P: parallel
 A: antiparallel
 X: other side-side links
 T: side-end
 V: end-end
 
 T and V are defined with respect to the `end`, at distance `threshold`,
 both can be set as parameters.
 
 by Jamie Li Rickman for
 Determinants of Polar versus Nematic Organization in Networks of Dynamic Microtubules
 and Mitotic Motors, Cell 2018
 */
void Simul::reportCoupleConfiguration(std::ostream& out, std::string const& which,
                                      Glossary& opt) const
{
    Property * selected = nullptr;
    
    if ( which.size() )
        selected = properties.find_or_die("couple", which);
    
    real threshold = 0.010;
    FiberEnd end = PLUS_END;
    
    opt.set(threshold, "threshold");
    opt.set(end, "end", {{"plus_end", PLUS_END}, {"minus_end", MINUS_END}});
    
    size_t T[6] = { 0 };
    for ( Couple * obj=couples.firstAA(); obj ; obj=obj->next() )
    {
        if ( !selected || obj->prop == selected )
            ++T[obj->configuration(end, threshold)];
    }
    size_t sum = T[0]+T[1]+T[2]+T[3]+T[4]+T[5];
    
    out << COM << "total" << SEP << "P" << SEP << "A" << SEP << "X" << SEP << "T" << SEP << "V";
    out << LIN << sum << SEP << T[0] << SEP << T[1] << SEP << T[2] << SEP << T[3] << SEP << T[4];
 }


/**
 Export histogram of Couples forces
 */
void Simul::reportCoupleForce(std::ostream& out, Glossary& opt) const
{
    const size_t IMAX = 8;
    const size_t BMAX = 256;
    int  cnt[IMAX][BMAX+1];

    real delta = 0.5;
    size_t nbin = 64;
    opt.set(delta, "interval");
    opt.set(nbin, "interval", 1);
    nbin = std::min(nbin, BMAX);

    // reset counts:
    for ( unsigned ii = 0; ii <  IMAX; ++ii )
    for ( unsigned jj = 0; jj <= nbin; ++jj )
        cnt[ii][jj] = 0;
    
    // accumulate counts:
    for ( Couple * cxi=couples.firstAA(); cxi ; cxi = cxi->next() )
    {
        unsigned ix = cxi->prop->number();
        if ( ix < IMAX )
        {
            unsigned f = (unsigned)( cxi->force().norm() / delta );
            if ( f < nbin )
                ++cnt[ix][f];
            else
                ++cnt[ix][nbin];
        }
    }
    
    if ( 1 )
    {
        out << COM << "force_distribution" << " (`scale` indicates the center of each bin)";
        out << LIN << ljust("scale", 2);
        for ( unsigned u = 0; u <= nbin; ++u )
            out << " " << std::setw(5) << delta * ( u + 0.5 );
    }
    
    for ( unsigned ii = 0; ii < IMAX; ++ii )
    {
        unsigned sum = 0;
        for ( unsigned jj = 0; jj <= nbin; ++jj )
            sum += cnt[ii][jj];
        if ( sum )
        {
            Property const* ip = properties.find_or_die("couple", ii);
            out << LIN << ljust(ip->name(), 2);
            for ( unsigned jj = 0; jj <= nbin; ++jj )
                out << ' ' << std::setw(5) << cnt[ii][jj];
        }
    }
}


/**
 Export number of Couples in each state
 */
void Simul::reportCouple(std::ostream& out) const
{
    const unsigned mx = 128;
    int act[mx] = { 0 }, cnt[mx][4];
    
    //reset counts:
    for ( unsigned ii = 0; ii < mx; ++ii )
    {
        cnt[ii][0] = 0;
        cnt[ii][1] = 0;
        cnt[ii][2] = 0;
        cnt[ii][3] = 0;
    }
    
    for ( Couple * cxi=couples.firstFF(); cxi ; cxi = cxi->next() )
    {
        assert_true(!cxi->attached1() && !cxi->attached2());
        unsigned ix = cxi->prop->number();
        if ( ix < mx )
        {
            if ( cxi->active() ) ++act[ix];
            ++(cnt[ix][0]);
        }
    }
    
    for ( Couple * cxi=couples.firstAF(); cxi ; cxi = cxi->next() )
    {
        assert_true(cxi->attached1() && !cxi->attached2());
        unsigned ix = cxi->prop->number();
        if ( ix < mx )
        {
            if ( cxi->active() ) ++act[ix];
            ++(cnt[ix][1]);
        }
    }
    for ( Couple * cxi=couples.firstFA(); cxi ; cxi = cxi->next() )
    {
        assert_true(!cxi->attached1() && cxi->attached2());
        unsigned ix = cxi->prop->number();
        if ( ix < mx )
        {
            if ( cxi->active() ) ++act[ix];
            ++(cnt[ix][2]);
        }
    }
    
    for ( Couple * cxi=couples.firstAA(); cxi ; cxi = cxi->next() )
    {
        assert_true(cxi->attached1() && cxi->attached2());
        unsigned ix = cxi->prop->number();
        if ( ix < mx )
        {
            if ( cxi->active() ) ++act[ix];
            ++(cnt[ix][3]);
        }
    }
    
    if ( 1 )
    {
        out << COM << ljust("couple", 2, 2);
        out << SEP << "total";
        out << SEP << "active";
        out << SEP << "FF";
        out << SEP << "AF";
        out << SEP << "FA";
        out << SEP << "AA";
    }
    
    for ( Property * i : properties.find_all("couple") )
    {
        out << LIN << ljust(i->name(), 2);
        unsigned ix = i->number();
        if ( ix < mx )
        {
            out << SEP << cnt[ix][0]+cnt[ix][1]+cnt[ix][2]+cnt[ix][3];
            out << SEP << act[ix];
            for ( unsigned int d = 0; d < 4; ++d )
                out << SEP << cnt[ix][d];
        }
        else
            out << SEP << "out-of-range";
    }
}


/**
 Export composition of Couple classes
 */
void Simul::reportCoupleAnatomy(std::ostream& out) const
{
    out << COM << "hand_id" << SEP << rjust("hand_name", 2, 1);
    
    for ( Property * i : properties.find_all("hand") )
    {
        HandProp * p = static_cast<HandProp*>(i);
        out << LIN << p->number();
        out << SEP << rjust(p->name(), 2);
    }
    out << '\n';

    out << COM << "class_id" << SEP << rjust("couple_name", 2, 1);
    out << SEP << rjust("hand1", 2) << SEP << rjust("hand2", 2);

    for ( Property * i : properties.find_all("couple") )
    {
        CoupleProp * p = static_cast<CoupleProp*>(i);
        out << LIN << p->number();
        out << SEP << rjust(p->name(), 2);
        out << SEP << rjust(p->hand1_prop->name(), 2);
        out << SEP << rjust(p->hand2_prop->name(), 2);
    }
}

//------------------------------------------------------------------------------
#pragma mark - Clusters


// set flag() to unique value for all fibers
void resetFlags(FiberSet const& set)
{
    for ( Fiber * fib = set.first(); fib; fib=fib->next() )
        fib->flag_to_identity();
}


// set flag() for all fibers to `f`
void resetFlags(FiberSet const& set, ObjectFlag f)
{
    for ( Fiber * fib = set.first(); fib; fib=fib->next() )
        fib->flag(f);
}


/**
 Substitute the values of fiber:flag() such that both `a` and `b`
 values are replaced by min(a, b).
 */
void reFlag(FiberSet const& set, ObjectFlag a, ObjectFlag b)
{
    // swap to ensure b < a
    if ( a < b )
        std::swap(a, b);

    // replace a -> b = min(a, b)
    for ( Fiber* fib = set.first(); fib; fib=fib->next() )
    {
        if ( fib->flag() == a )
            fib->flag(b);
    }
}


/**
 equalize flag() when fibers are connected by a Couple:
 */
void Simul::flagClustersCouples() const
{
    for ( Couple const* cx = couples.firstAA(); cx ; cx=cx->next() )
    {
        ObjectFlag f1 = cx->fiber1()->flag();
        ObjectFlag f2 = cx->fiber2()->flag();
        if ( f1 != f2 )
            reFlag(fibers, f1, f2);
    }
}

/**
 equalize flag() when fibers are connected by a Couple of given type:
 */
void Simul::flagClustersCouples(Property const* arg) const
{
    resetFlags(fibers);

    for ( Couple * cx = couples.firstAA(); cx ; cx=cx->next() )
    {
        if ( cx->prop == arg )
        {
            ObjectFlag f1 = cx->fiber1()->flag();
            ObjectFlag f2 = cx->fiber2()->flag();
            if ( f1 != f2 )
                reFlag(fibers, f1, f2);
        }
    }
}


/**
 equalize flag() when fibers are connected through blobs:
 */
void Simul::flagClustersSolids() const
{
    for ( Solid * obj=solids.first(); obj; obj=obj->next() )
    {
        SingleList anchored = singles.collectWrists(obj);
        // find the minimun flag value:
        ObjectFlag flg = 0;
        for ( Single const* s : anchored )
        {
            if ( s->attached() )
            {
                ObjectFlag f = s->fiber()->flag();
                if ( flg == 0 || f < flg )
                    flg = f;
            }
        }
        // reflag:
        if ( flg > 0 )
        {
            for ( Single const* s : anchored )
            {
                if ( s->attached() )
                    reFlag(fibers, s->fiber()->flag(), flg);
            }
        }
    }
}


/// class to store info about a Cluster
struct Cluster
{
    ObjectFlag flg;
    size_t     cnt;
    
    Cluster(ObjectFlag f, size_t n) { flg = f; cnt = n; }
        
    /// Compare function
    bool operator < (Cluster const&b) const
    {
        if ( cnt == b.cnt ) return flg < b.flg;
        return cnt > b.cnt;
    }
};


/*
Clusters are ordered from 1 to max, in decreasing order
Returns the number of clusters, `max`, and set the fiber's
flags according to the corresponding cluster index.
*/
int Simul::orderClusters(std::ostream& out, size_t threshold, int details) const
{
    typedef std::vector<Fiber*> list_t;
    typedef std::map<ObjectFlag, list_t> map_t;
    // the std::set will keep its elements ordered:
    typedef std::multiset<Cluster> set_t;
    map_t map;
    set_t clusters;

    // extract clusters in 'map' and reset fiber's flag:
    for ( Fiber* fib = fibers.firstID(); fib; fib = fibers.nextID(fib) )
    {
        //std::clog << fib->reference() << " " << fib->flag() << "\n";
        map[fib->flag()].push_back(fib);
        fib->flag(0);
    }
    
    // insert clusters with size information to get them sorted:
    for ( map_t::const_iterator c = map.begin(); c != map.end(); ++c )
    {
        size_t s = c->second.size();
        if ( s >= threshold )
            clusters.emplace(c->first, s);
    }
    
    if ( details > 0 )
        out << LIN << clusters.size() << " clusters:";
    
    // consider clusters by decreasing size:
    int idx = 0;
    for ( set_t::const_iterator cc = clusters.begin(); cc != clusters.end(); ++cc )
    {
        ++idx;
        list_t const& list = map[cc->flg];
        
        for ( Fiber * i : list )
            i->flag(idx);

        if ( details > 0 )
        {
            out << LIN << cc->flg << SEP << cc->cnt;
            
            if ( details > 1 )
            {
                out << " :";
                for ( Fiber * i : list )
                    out << " " << i->identity();
            }
        }
    }
    
    return idx;
}


void Simul::flagClusters(bool order) const
{
    resetFlags(fibers);
    flagClustersCouples();
    if ( order )
    {
        std::ofstream nos("/dev/null");
        orderClusters(nos, 2, 0);
    }
}


/**
 Export size of clusters found by Simul::flagClusters()
 Clusters are ordered in decreasing size.
 */
void Simul::reportClusters(std::ostream& out, Glossary& opt) const
{
    int details = 2, sol = 0, cop = 1;
    opt.set(details, "details");
    opt.set(cop, "couples");
    opt.set(sol, "solids");

    resetFlags(fibers);

    if ( cop )
        flagClustersCouples();
    
    if ( sol )
        flagClustersSolids();
    
    out << COM << "cluster" << SEP << "nb_fibers :" << SEP << "fiber_id";
    orderClusters(out, 2, details);
}


//------------------------------------------------------------------------------
#pragma mark - Ring

/**
 Evaluates if the Fiber distribution makes a connected ring around the Z-axis
 @returns number of rings
 FJN 8.07.2017, 8.11.2018, for Blood Platelet project
 */
size_t Simul::flagRing() const
{
    flagClusters(false);

    typedef std::list<ObjectFlag> list_t;
    list_t ring;
    
    // include all cluster into 'ring'
    for ( Fiber * fib = fibers.first(); fib; fib=fib->next() )
    {
        ObjectFlag f = fib->flag();
        if ( f > 0 )
        {
            list_t::const_iterator i = std::find(ring.begin(), ring.end(), f);
            if ( i == ring.end() )
                ring.push_back(f);
        }
    }
    
    // rotate plane around the Z-axis and find intersecting fibers
    for ( unsigned a = 0; a < 360; ++a )
    {
        real ang = a * M_PI / 180.0;
        Vector nor( cos(ang), sin(ang), 0.0);
        Vector dir(-sin(ang), cos(ang), 0.0);
        
        list_t sec;
        for ( Fiber const* fib=fibers.first(); fib; fib=fib->next() )
        {
            for ( unsigned s = 0; s < fib->nbSegments(); ++s )
            {
                // check that fiber intersect with plane:
                real abs = fib->planarIntersect(s, nor, 0);
                if ( 0 <= abs  &&  abs < 1 )
                {
                    // check that intersection is located on 'dir' side of Z-axis:
                    Vector pos = fib->interpolatePoints(s,s+1,abs);
                    if ( dot(pos, dir) > 0 )
                    {
                        // transfer cluster if it was already present before:
                        ObjectFlag f = fib->flag();
                        list_t::iterator i = std::find(ring.begin(), ring.end(), f);
                        if ( i != ring.end() )
                        {
                            ring.erase(i);
                            sec.push_back(f);
                        }
                    }
                }
            }
        }
        ring = sec;
    }
    
    if ( ring.size() == 1 )
    {
        ObjectFlag f = *ring.begin();
        for ( Fiber * fib = fibers.first(); fib; fib=fib->next() )
        {
            if ( fib->flag() == f )
                fib->flag(1);
            else
                fib->flag(0);
        }
    }
    else if ( ring.size() > 0 )
    {
        // unflag all fibers that are not part of a ring:
        for ( Fiber * fib = fibers.first(); fib; fib=fib->next() )
        {
            if ( std::find(ring.begin(), ring.end(), fib->flag()) == ring.end() )
                fib->flag(0);
        }
    }
    else
    {
        // unflag all
        for ( Fiber * fib = fibers.first(); fib; fib=fib->next() )
            fib->flag(0);
    }
    return ring.size();
}


/**
 Calculate the length of the ring and its mean radius
 FJN 15.09.2018, for Blood Platelet project
*/
void Simul::analyzeRing(ObjectFlag flg, real& length, real& radius) const
{
    length = 0.0;
    radius = 0.0;

    Vector cen_old;
    unsigned rad_cnt = 0;
    
    // rotate plane around the Z-axis and find intersecting fibers
    for ( unsigned a = 0; a <= 360; ++a )
    {
        real ang = a * M_PI / 180.0;
        Vector nor( cos(ang), sin(ang), 0);
        Vector dir(-sin(ang), cos(ang), 0);
        
        unsigned cen_cnt = 0;
        Vector cen(0,0,0);
        for ( Fiber const* fib=fibers.first(); fib; fib=fib->next() )
        {
            // only consider fiber that are part of the ring:
            if ( fib->flag() == flg )
            {
                for ( unsigned s = 0; s < fib->nbSegments(); ++s )
                {
                    // check that fiber intersect with plane:
                    real abs = fib->planarIntersect(s, nor, 0);
                    if ( 0 <= abs  &&  abs < 1 )
                    {
                        Vector pos = fib->interpolatePoints(s,s+1,abs);
                        // check that intersection is located on 'dir' side of Z-axis:
                        real H = dot(pos, dir);
                        if ( H > 0 )
                        {
                            radius += H;
                            ++rad_cnt;
                            cen += pos;
                            ++cen_cnt;
                        }
                    }
                }
            }
        }
        if ( cen_cnt > 0 )
        {
            cen /= cen_cnt;
            if ( a > 0 )
                length += ( cen - cen_old ).norm();
            cen_old = cen;
        }
    }
    radius /= rad_cnt;
}

/**
Calculate the length of the ring and its radius
*/
void Simul::reportRing(std::ostream& out) const
{
    out << COM << "nb_rings" << SEP << "length" << SEP << "radius";
    size_t nring = flagRing();
    if ( nring == 1 )
    {
        real len, rad;
        analyzeRing(1, len, rad);
        out << LIN << 1 << SEP << std::fixed << len<< SEP << rad ;
    }
    else
    {
        out << LIN << nring << SEP << 0.0 << SEP << 0.0;
    }
}


/**
 Evaluates if the Fiber distribution makes a connected ring around the Z-axis
 FJN 8.07.2017, for Blood Platelet project
 */
void Simul::reportPlatelet(std::ostream& out) const
{
    unsigned nfib;
    real pol, dev, mn, mx;
    fibers.infoLength(fibers.collect(), nfib, pol, dev, mn, mx);
    pol *= nfib;
    
    computeForces();

    real t, ten = 0;
    unsigned c, cnt = 0;

    // rotate plane around the Z-axis and find intersecting fibers
    for ( unsigned a = 0; a < 180; a += 10 )
    {
        real ang = a * M_PI / 180.0;
        Vector dir(cos(ang), sin(ang), 0);
        fibers.infoTension(c, t, dir, 0);
        cnt += 2;  // every plane should intersect the ring twice
        ten += t;
    }
    ten /= cnt;
    
    std::ofstream nos("/dev/null");
    real force = reportFiberConfinement(nos);

    real len = 0.0, rad = 0.0;
    if ( flagRing() == 1 )
        analyzeRing(1, len, rad);

    int p = out.precision();
    out.precision(std::min(p,3));
    out << COM << "nb_fiber" << SEP << "polymer" << SEP << "tension" << SEP << "force" << SEP << "length" << SEP << "radius";
    out << LIN << nfib << SEP << std::fixed << pol << SEP << ten << SEP << force << SEP << len << SEP << rad;
    out.precision(p);
}


//------------------------------------------------------------------------------
#pragma mark - Misc

/**
 Export indices calculated by FiberSet::infoSpindle
 */
void Simul::reportIndices(std::ostream& out) const
{
    out << COM << "amount" << SEP << "radial" << SEP << "polar";
    real ixa, ixp;
    fibers.infoSpindle(ixa, ixp, Vector(1,0,0), 0, 30, 1);
    out << LIN << fibers.size();
    out << SEP << ixa;
    out << SEP << ixp;
}


/**
 Export number of Fibers pointing left and right,
 that intersect a plane parallel to YZ.
 The planes are distributed regularly every 0.5 um along the X-axis.
 */
void Simul::reportProfile(std::ostream& out) const
{
    out << COM << "position" << SEP << "left-pointing" << SEP << "right-pointing";
    Vector n(1,0,0);
    real m = 40, dm = 0.5;
    int nr, nl;
    for ( real p = -m ; p <= m ; p += dm )
    {
        fibers.infoPlane(nr, nl, n, -p);
        out << LIN << p;
        out << SEP << nl;
        out << SEP << nr;
    }
}


/**
 l'angle entre un vecteur 1 (centre du noyau --> SPB)
 et un vecteur 2 (axe de l'hyphe; gauche --> droite = sens du flow).
 */
void Simul::reportAshbya(std::ostream& out) const
{
    out << COM << "class id point_0, vector_1, angle";
    for ( Solid * obj=solids.first(); obj; obj=obj->next() )
    {
        out << LIN << obj->prop->number();
        out << SEP << obj->identity();
        out << SEP << obj->posP(0);
        if ( obj->nbPoints() > 1 )
        {
            Vector vec = normalize(obj->diffPoints(0));
            Vector dir(1,0,0);
            out << SEP << vec;
            out << SEP << acos(dot(vec, dir));
        }
    }
}


/**
 Export end-to-end distance of Fiber
 */
void Simul::reportCustom(std::ostream& out) const
{
    for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
    {
        Vector ee = obj->posEndP() - obj->posEndM();
        out << ee.norm() << " ";
    }
}
