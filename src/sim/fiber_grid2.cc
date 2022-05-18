// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/** 
 This file implements a 'dummy' grid using STL code, which can be used as a reference.
 For each position, it calculates the geometrical distance to all fiber segments.
 This is algorithmically the slowest method, but it is simple and most likely correct!
 It is useful to get a ground truth and evaluate more advanced methods.
 */


#include <algorithm>

typedef std::vector <FiberSegment> SegmentVector;

/// a list containing all segments, as a global variable
SegmentVector allSegments;


unsigned FiberGrid::setGrid(Space const*, real)
{
    LOG_ONCE("Cytosim is using a crude method to localize fibers!\n");
    return 0;
}

void FiberGrid::paintGrid(const Fiber * first, const Fiber * last)
{
    allSegments.clear();
    // add all segments
    for ( const Fiber * f = first ; f != last ; f=f->next() )
    {
        for ( unsigned s = 0; s < f->nbSegments(); ++s )
            allSegments.push_back(FiberSegment(f,s));
    }
}


void FiberGrid::createCells()
{
}

size_t FiberGrid::hasGrid() const
{
    return 1;
}


void FiberGrid::tryToAttach(Vector const& place, Hand& ha) const
{
    // randomize the list order
    std::random_shuffle( allSegments.begin(), allSegments.end() );

    // test all segments:
    for ( FiberSegment const& seg : allSegments )
    {
        if ( RNG.test(ha.prop->binding_prob) )
        {
            real dis = INFINITY;
            // Compute the distance from the hand to the rod, and abscissa of projection:
            real abs = seg.projectPoint(place, dis);
            
            /*
             Compare to the maximum attachment range of the hand,
             and compare a newly tossed random number with 'prob'
             */
            if ( dis < ha.prop->binding_range_sqr )
            {
                Fiber * fib = const_cast<Fiber*>(seg.fiber());
                FiberSite pos(fib, seg.abscissa1()+abs);
                
                if ( ha.attachmentAllowed(pos) )
                {
                    ha.attach(pos);
                    return;
                }
            }
        }
    }
}


FiberGrid::SegmentList FiberGrid::nearbySegments(Vector const& place, const real DD, Fiber * exclude) const
{
    SegmentList res;
    
    for ( FiberSegment const& seg : allSegments )
    {
        if ( seg.fiber() != exclude )
        {
            real dis = INFINITY;
            // Compute the distance from the hand to the rod:
            seg.projectPoint(place, dis);
            
            if ( dis < DD )
                res.push_back(seg);
        }
    }
    
    return res;
}


FiberSegment FiberGrid::closestSegment(Vector const& place) const
{
    FiberSegment res(nullptr, 0);
    real hit = INFINITY;
    
    for ( FiberSegment const& seg : allSegments )
    {
        real dis = INFINITY;
        // Compute the distance from the hand to the rod:
        seg.projectPoint(place, dis);
        
        if ( dis < hit )
        {
            hit = dis;
            res = seg;
        }
    }
    
    return res;
}
