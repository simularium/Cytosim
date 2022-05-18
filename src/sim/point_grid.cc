// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "assert_macro.h"
#include "point_grid.h"
#include "exceptions.h"
#include "messages.h"
#include "modulo.h"
#include "space.h"
#include "meca.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------

PointGrid::PointGrid()
: max_diameter(0)
{
}


size_t PointGrid::setGrid(Space const* spc, real min_step)
{
    if ( min_step <= REAL_EPSILON )
        return 0;
    
    Vector inf, sup;
    spc->boundaries(inf, sup);
    
    int n_cell[3];
    for ( int d = 0; d < DIM; ++d )
    {
        real n = ( sup[d] - inf[d] ) / min_step;
        
        if ( n < 0 )
            throw InvalidParameter("invalid space:boundaries");
        
        if ( modulo  &&  modulo->isPeriodic(d) )
        {
            //adjust the grid to match the edges
            n_cell[d] = (int)floor(n);
            if ( n_cell[d] <= 0 )
                n_cell[d] = 1;
            pGrid.setPeriodic(d, true);
        }
        else
        {
            //add a border in any dimension which is not periodic
            n_cell[d] = (int)ceil(n) + 2;
            n = n_cell[d] * 0.5 * min_step;
            real mid = inf[d] + sup[d];
            inf[d] = mid - n;
            sup[d] = mid + n;
        }
    }
    
    //create the grid using the calculated dimensions:
    pGrid.setDimensions(inf, sup, n_cell);
    return pGrid.nbCells();
}

void PointGrid::createCells()
{
    pGrid.createCells();

    //Create side regions suitable for pairwise interactions:
    pGrid.createSideRegions(1);

    //The maximum allowed diameter of particles is half the minimum cell width
    max_diameter = pGrid.minimumWidth(1);

    //report the grid size used
    if ( pGrid.nbCells() > 4096 )
        pGrid.printSummary(std::clog, "StericGrid");
}


//------------------------------------------------------------------------------
#pragma mark -

/// include verifications that the grid is appropriate for the particule radius
#define CHECK_RANGE 0


#if ( NB_STERIC_PANES != 1 )

void PointGrid::add(unsigned pan, Mecapoint const& pe, real rd, real rg) const
{
    if ( pan == 0 || pan > NB_STERIC_PANES )
        throw InvalidParameter("object:steric is out-of-range");

    Vector w = pe.pos();
    point_list(w, pan).new_val().set(pe, rd, rg, w);
    
#if ( CHECK_RANGE )
    //we check that the grid would correctly detect collision of two particles
    if ( max_diameter < 2 * rg - REAL_EPSILON )
    {
        InvalidParameter e("simul:steric_max_range is too short\n");
        e << PREF << "steric_max_range should be greater than 2 * ( particle_radius + extra_range )\n";
        e << PREF << "= " << 2 * rg << " for some particles\n";
        throw e;
    }
#endif
}


void PointGrid::add(unsigned pan, FiberSegment const& fl, real rd, real rg) const
{
    if ( pan == 0 || pan > NB_STERIC_PANES )
        throw InvalidParameter("object:steric is out-of-range");

    // link in the cell containing the middle of the segment:
    Vector w = fl.center();
    segment_list(w, pan).new_val().set(fl, rd, rg);
    
#if ( CHECK_RANGE )
    //we check that the grid would correctly detect collision of two segments
    //along the diagonal, corresponding to the worst-case scenario
    real diag = sqrt( fl.len() * fl.len() + 4 * rg * rg );
    if ( max_diameter < diag - REAL_EPSILON )
    {
        InvalidParameter("simul:steric_max_range is too short\n");
        e << PREF << "steric_max_range should be greater than sqrt( sqr(segment_length) + 4*sqr(range) )\n";
        e << PREF << "where normally segment_length ~ 4/3 segmentation\n";
        e << PREF << "= " << diag << " for some fibers\n";
        throw e;
    }
#endif
}


#endif


//------------------------------------------------------------------------------
#pragma mark - Steric functions


/**
 This is used to check two spherical objects:
 Solid/Bead/Sphere or the terminal vertex (the tips) of a Fiber
 
 The force is applied if the objects are closer than the
 sum of their radiuses.
 */
void PointGrid::checkPP(Meca& meca, PointGridParam const& pam,
                        FatPoint const& aa, FatPoint const& bb) const
{
    //std::clog << "   PP- " << bb.pnt << " " << aa.pnt << std::endl;
    const real len = aa.radius + bb.radius;
    Vector vab = bb.pos - aa.pos;
    
    if ( modulo )
        modulo->fold(vab);
    
    if ( vab.normSqr() < len*len )
        meca.addLongLink(aa.pnt, bb.pnt, len, pam.stiff_push);
}


/**
 This is used to check a segment of a fiber against a spherical object:
 Solid/Bead/Sphere or Fiber-tip.
 
 The force is applied if the objects are closer than the sum of their radiuses.
 */
void PointGrid::checkPL(Meca& meca, PointGridParam const& pam,
                        FatPoint const& aa, FatSegment const& bb) const
{
    //std::clog << "   PL- " << bb.seg << " " << aa.pnt << std::endl;
    const real len = aa.radius + bb.radius;
    
    // get position of point with respect to segment:
    real dis2;
    real abs = bb.seg.projectPoint0(aa.pos, dis2);
    
    if ( 0 <= abs )
    {
        if ( abs <= bb.seg.len() )
        {
            if ( dis2 < len*len )
            {
                Interpolation bi(bb.seg, abs);
                meca.addSideSlidingLink(bi, aa.pnt, len, pam.stiff_push);
            }
        }
        else
        {
            if ( bb.isLast() )
                checkPP(meca, pam, aa, bb.fatPoint2());
        }
    }
    else
    {
        if ( bb.isFirst() )
            checkPP(meca, pam, aa, bb.fatPoint1());
        else
        {
            /* we check the projection to the previous segment,
             and if it falls on the right of it, then we interact with the node */
            Vector vab = aa.pos - bb.seg.pos1();
            
            if ( modulo )
                modulo->fold(vab);
            
            if ( dot(vab, bb.seg.fiber()->diffPoints(bb.seg.point()-1)) >= 0 )
            {
                if ( vab.normSqr() < len*len )
                    meca.addLongLink(aa.pnt, bb.seg.exact1(), len, pam.stiff_push);
            }
        }
    }
}


/**
 This is used to check a segment of a fiber against another segment of fiber,
 not including the terminal vertex of fibers.
 
 The interaction is applied only if the vertex projects 'inside' the segment.
 */
void PointGrid::checkLL1(Meca& meca, PointGridParam const& pam,
                         FatSegment const& aa, FatSegment const& bb) const
{
    //std::clog << "   LL1 " << aa.seg << " " << bb.point1() << std::endl;
    const real ran = aa.range + bb.radius;
    
    // get position of bb.point1() with respect to segment 'aa'
    real dis2 = INFINITY;
    real abs = aa.seg.projectPoint0(bb.seg.pos1(), dis2);
    
    if ( dis2 < ran*ran )
    {
        /*
         bb.point1() projects inside segment 'aa'
         */
        assert_true( 0 <= abs  &&  abs <= aa.seg.len() );
        const real len = aa.radius + bb.radius;
        Interpolation ai(aa.seg, abs);
        if ( dis2 > len*len )
            meca.addSideSlidingLink(ai, bb.seg.exact1(), len, pam.stiff_pull);
        else
            meca.addSideSlidingLink(ai, bb.seg.exact1(), len, pam.stiff_push);
    }
    else if ( abs < 0 )
    {
        if ( aa.isFirst() )
        {
            /*
             Check the projection of aa.point1(),
             on the segment represented by 'bb'
             */
            if ( &bb < &aa  &&  bb.isFirst() )
            {
                Vector vab = bb.seg.pos1() - aa.seg.pos1();
                
                if ( modulo )
                    modulo->fold(vab);
                
                const real len = aa.radius + bb.radius;
                if ( vab.normSqr() < len*len  &&  dot(vab, bb.seg.diff()) >= 0 )
                    meca.addLongLink(aa.seg.exact1(), bb.seg.exact1(), len, pam.stiff_push);
            }
        }
        else
        {
            /*
             Check the projection to the segment located before 'aa',
             and interact if 'bb.point1()' falls on the right side of it
             */
            Vector vab = bb.seg.pos1() - aa.seg.pos1();
            
            if ( modulo )
                modulo->fold(vab);
            
            if ( dot(vab, aa.seg.fiber()->diffPoints(aa.seg.point()-1)) >= 0 )
            {
                const real d = vab.normSqr();
                if ( d < ran*ran )
                {
                    const real len = aa.radius + bb.radius;
                    if ( d > len*len )
                        meca.addLongLink(aa.seg.exact1(), bb.seg.exact1(), len, pam.stiff_pull);
                    else
                        meca.addLongLink(aa.seg.exact1(), bb.seg.exact1(), len, pam.stiff_push);
                }
            }
        }
    }
}


/**
 This is used to check a segment of a fiber against another segment of fiber,
 the non-terminal vertex of a fiber.
 
 The interaction is applied only if the vertex projects 'inside' the segment.
 */
void PointGrid::checkLL2(Meca& meca, PointGridParam const& pam,
                         FatSegment const& aa, FatSegment const& bb) const
{
    //std::clog << "   LL2 " << aa.seg << " " << bb.point2() << std::endl;
    const real ran = aa.range + bb.radius;
    
    // get position of bb.point2() with respect to segment 'aa'
    real dis2 = INFINITY;
    real abs = aa.seg.projectPoint0(bb.seg.pos2(), dis2);
    
    if ( dis2 < ran*ran )
    {
        /*
         bb.point2() projects inside segment 'aa'
         */
        assert_true( 0 <= abs  &&  abs <= aa.seg.len() );
        const real len = aa.radius + bb.radius;
        Interpolation ai(aa.seg, abs);
        if ( dis2 > len*len )
            meca.addSideSlidingLink(ai, bb.seg.exact2(), len, pam.stiff_pull);
        else
            meca.addSideSlidingLink(ai, bb.seg.exact2(), len, pam.stiff_push);
    }
    else if ( abs < 0 )
    {
        /*
         Check the projection to the segment located before 'aa',
         and interact if 'bb.point1()' falls on the right side of it
         */
        Vector vab = bb.seg.pos2() - aa.seg.pos1();
        
        if ( modulo )
            modulo->fold(vab);
        
        if ( aa.isFirst() )
        {
            assert_true(bb.isLast());
            const real len = aa.radius + bb.radius;
            if ( vab.normSqr() < len*len  && dot(vab, bb.seg.diff()) <= 0 )
                meca.addLongLink(aa.seg.exact1(), bb.seg.exact2(), len, pam.stiff_push);
        }
        else
        {
            if ( dot(vab, aa.seg.fiber()->diffPoints(aa.seg.point()-1)) >= 0 )
            {
                const real d = vab.normSqr();
                if ( d < ran*ran )
                {
                    const real len = aa.radius + bb.radius;
                    if ( d > len*len )
                        meca.addLongLink(aa.seg.exact1(), bb.seg.exact2(), len, pam.stiff_pull);
                    else
                        meca.addLongLink(aa.seg.exact1(), bb.seg.exact2(), len, pam.stiff_push);
                }
            }
        }
    }
    else if ( &bb < &aa  &&  aa.isLast()  &&  abs > aa.seg.len() )
    {
        /*
         Check the projection of aa.point2(),
         on the segment represented by 'bb'
         */
        assert_true(bb.isLast());
        
        Vector vab = bb.seg.pos2() - aa.seg.pos2();
        
        if ( modulo )
            modulo->fold(vab);
        
        const real len = aa.radius + bb.radius;
        if ( vab.normSqr() < len*len  &&  dot(vab, bb.seg.diff()) <= 0 )
            meca.addLongLink(aa.seg.exact2(), bb.seg.exact2(), len, pam.stiff_push);
    }
}


/**
 This is used to check two FiberSegment, that each represent a segment of a Fiber.
 The segments are tested for intersection in 3D.
 */
void PointGrid::checkLL(Meca& meca, PointGridParam const& pam,
                        FatSegment const& aa, FatSegment const& bb) const
{
    //std::clog << "LL " << aa.seg << " " << bb.seg << std::endl;
    checkLL1(meca, pam, aa, bb);
    
    if ( aa.isLast() )
        checkLL2(meca, pam, bb, aa);
        
    checkLL1(meca, pam, bb, aa);
      
    if ( bb.isLast() )
        checkLL2(meca, pam, aa, bb);
  
#if ( DIM == 3 )
    
    const real ran = std::max(aa.range+bb.radius, aa.radius+bb.range);

    /* in 3D, we use shortestDistance() to calculate the closest distance
     between two segments, and use the result to build an interaction */
    real a, b;
    real d = aa.seg.shortestDistance(bb.seg, a, b);
    if ( d < ran*ran && ( aa.seg.within(a) & bb.seg.within(b) ) )
    {
        const real len = aa.radius + bb.radius;
        
        Interpolation ai(aa.seg, a);
        Interpolation bi(bb.seg, b);

        //std::clog << "steric distance " << d << "  " << ai << " " << bi <<"\n";
     
        if ( d > len*len )
            meca.addSideSlidingLink(ai, bi, len, pam.stiff_pull);
        else
            meca.addSideSlidingLink(ai, bi, len, pam.stiff_push);
    }
    
#endif
}


//------------------------------------------------------------------------------
#pragma mark - Selections of pairs excluded from Sterics


/// excluding two spheres when they are from the same Solid
inline bool adjacent(FatPoint const* a, FatPoint const* b)
{
    return a->pnt.mecable() == b->pnt.mecable();
}


/// excluding Fiber and Solid from the same Aster
inline bool adjacent(FatPoint const* a, FatSegment const* b)
{
    //a->pnt.mecable()->Buddy::print(std::clog);
    //b->seg.fiber()->Buddy::print(std::clog);
    return b->seg.fiber()->buddy() == a->pnt.mecable()->buddy();
}


/// excluding segments that are adjacent on the same fiber
inline bool adjacent(FatSegment const* a, FatSegment const* b)
{
    return (( a->seg.fiber() == b->seg.fiber() )
            & ( a->seg.point() < 2 + b->seg.point() ) & ( b->seg.point() < 2 + a->seg.point() ));
}

//------------------------------------------------------------------------------
#pragma mark - Check all possible object pairs from two Cells

/**
 This will consider once all pairs of objects from the given lists
 */
void PointGrid::setInteractions(Meca& meca, PointGridParam const& stiff,
                                FatPointList & pots, FatSegmentList & locs) const
{
    for ( FatPoint* ii = pots.begin(); ii < pots.end(); ++ii )
    {
        for ( FatPoint* jj = ii+1; jj < pots.end(); ++jj )
            if ( !adjacent(ii, jj) )
                checkPP(meca, stiff, *ii, *jj);
        
        for ( FatSegment* kk = locs.begin(); kk < locs.end(); ++kk )
            if ( !adjacent(ii, kk) )
                checkPL(meca, stiff, *ii, *kk);
    }
    
    for ( FatSegment* ii = locs.begin(); ii < locs.end(); ++ii )
    {
        for ( FatSegment* jj = ii+1; jj < locs.end(); ++jj )
            if ( !adjacent(ii, jj) )
                checkLL(meca, stiff, *ii, *jj);
    }
}


/**
 This will consider once all pairs of objects from the given lists,
 assuming that the list are different and no object is repeated
 */
void PointGrid::setInteractions(Meca& meca, PointGridParam const& pam,
                                FatPointList & pots1, FatSegmentList & locs1,
                                FatPointList & pots2, FatSegmentList & locs2) const
{
    assert_true( &pots1 != &pots2 );
    assert_true( &locs1 != &locs2 );
    
    for ( FatPoint* ii = pots1.begin(); ii < pots1.end(); ++ii )
    {
        for ( FatPoint* jj = pots2.begin(); jj < pots2.end(); ++jj )
            if ( !adjacent(ii, jj) )
                checkPP(meca, pam, *ii, *jj);
        
        for ( FatSegment* kk = locs2.begin(); kk < locs2.end(); ++kk )
            if ( !adjacent(ii, kk) )
                checkPL(meca, pam, *ii, *kk);
    }
    
    for ( FatSegment* ii = locs1.begin(); ii < locs1.end(); ++ii )
    {
        for ( FatPoint* jj = pots2.begin(); jj < pots2.end(); ++jj )
            if ( !adjacent(jj, ii) )
                checkPL(meca, pam, *jj, *ii);
        
        for ( FatSegment* kk = locs2.begin(); kk < locs2.end(); ++kk )
        {
            if ( !adjacent(ii, kk)  )
                checkLL(meca, pam, *ii, *kk);
        }
    }
}


#if ( NB_STERIC_PANES == 1 )

/**
 Check interactions between objects contained in the grid.
 */
void  PointGrid::setInteractions(Meca& meca, PointGridParam const& pam) const
{
    assert_true(pam.stiff_push >= 0);
    assert_true(pam.stiff_pull >= 0);
    //std::clog << "----" << std::endl;

    // scan all cells to examine each pair of particles:
    for ( unsigned inx = 0; inx < pGrid.nbCells(); ++inx )
    {
        int * region;
        int nr = pGrid.getRegion(region, inx);
        assert_true(region[0] == 0);
        
        // We consider each pair of objects (ii, jj) only once:
        
        FatPointList & baseP = point_list(inx);
        FatSegmentList & baseL = segment_list(inx);
        
        setInteractions(meca, pam, baseP, baseL);
        
        for ( int reg = 1; reg < nr; ++reg )
        {
            FatPointList & sideP = point_list(inx+region[reg]);
            FatSegmentList & sideL = segment_list(inx+region[reg]);
            
            setInteractions(meca, pam, baseP, baseL, sideP, sideL);
        }
    }
}


#else

/**
 Check interactions between the FatPoints contained in Pane `pan`.
 */
void  PointGrid::setInteractions(Meca& meca, PointGridParam const& pam,
                                 const unsigned pan) const
{
    assert_true(pam.stiff_push >= 0);
    assert_true(pam.stiff_pull >= 0);
    
    // scan all cells to examine each pair of particles:
    for ( unsigned inx = 0; inx < pGrid.nbCells(); ++inx )
    {
        int * region;
        int nr = pGrid.getRegion(region, inx);
        assert_true(region[0] == 0);
        
        // We consider each pair of objects (ii, jj) only once:
        
        FatPointList & baseP = point_list(inx, pan);
        FatSegmentList & baseL = segment_list(inx, pan);
        
        setInteractions(meca, pam, baseP, baseL);

        for ( int reg = 1; reg < nr; ++reg )
        {
            FatPointList & sideP = point_list(inx+region[reg], pan);
            FatSegmentList & sideL = segment_list(inx+region[reg], pan);
            
            setInteractions(meca, pam, baseP, baseL, sideP, sideL);
        }
    }
}


/**
 Check interactions between the FatPoints contained in Panes `pan1` and `pan2`,
 where ( pan1 != pan2 )
 */
void  PointGrid::setInteractions(Meca& meca, PointGridParam const& pam,
                                 const unsigned pan1, const unsigned pan2) const
{
    assert_true(pam.stiff_push >= 0);
    assert_true(pam.stiff_pull >= 0);
    assert_true(pan1 != pan2);
    
    // scan all cells to examine each pair of particles:
    for ( unsigned inx = 0; inx < pGrid.nbCells(); ++inx )
    {
        int * region;
        int nr = pGrid.getRegion(region, inx);
        assert_true(region[0] == 0);

        // We consider each pair of objects (ii, jj) only once:
        
        FatPointList & baseP = point_list(inx, pan1);
        FatSegmentList & baseL = segment_list(inx, pan1);

        for ( int reg = 0; reg < nr; ++reg )
        {
            FatPointList & sideP = point_list(inx+region[reg], pan2);
            FatSegmentList & sideL = segment_list(inx+region[reg], pan2);

            setInteractions(meca, pam, baseP, baseL, sideP, sideL);
        }
        
        FatPointList & baseP2 = point_list(inx, pan2);
        FatSegmentList & baseL2 = segment_list(inx, pan2);
        
        for ( int reg = 1; reg < nr; ++reg )
        {
            FatPointList & sideP = point_list(inx+region[reg], pan1);
            FatSegmentList & sideL = segment_list(inx+region[reg], pan1);
            
            setInteractions(meca, pam, baseP2, baseL2, sideP, sideL);
        }
    }
}


#endif


//------------------------------------------------------------------------------
#pragma mark - Display

#ifdef DISPLAY

#  include "grid_display.h"

void PointGrid::draw() const
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1, 0, 1);
    glLineWidth(0.5);
    drawEdges(pGrid);
    glPopAttrib();
}
#endif

