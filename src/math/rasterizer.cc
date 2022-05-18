// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "assert_macro.h"
#include "rasterizer.h"
#include "vector2.h"
#include "vector3.h"
#include <cmath>

/**
 DISPLAY enables some code useful for visual debugging,
 and should be defined for compiling test_rasterizer.cc
 */
#ifdef DISPLAY
#  include "opengl.h"
bool rasterizer_draws = false;
#endif

//==============================================================================
//                             1D
//==============================================================================
#pragma mark - 1D

void Rasterizer::paintFatLine1D(void (*paint)(int, int, int, int, void*), void * arg,
                                const Vector1& P, const Vector1& Q,
                                const real radius, const Vector1& offset, const Vector1& delta)
{
    int inf, sup;
    
    if ( P.XX > Q.XX )
    {
        inf = (int) ceil( ( Q.XX - radius - offset.XX ) * delta.XX );
        sup = (int)floor( ( P.XX + radius - offset.XX ) * delta.XX );
    }
    else
    {
        inf = (int) ceil( ( P.XX - radius - offset.XX ) * delta.XX );
        sup = (int)floor( ( Q.XX + radius - offset.XX ) * delta.XX );
    }
    
    paint(inf, sup, 0, 0, arg);
}


//==============================================================================
//                               2D
//==============================================================================
#pragma mark - 2D

void Rasterizer::paintPolygon2D(void (*paint)(int, int, int, int, void*), void * arg,
                                const size_t n_pts, const Vector2 pts[],
                                const int zz)
{
#ifdef DISPLAY
    if ( rasterizer_draws )
    {
        glLineWidth(1);
        glColor3f(0.0, 0.0, 1.0);
        glBegin(GL_LINE_LOOP);
        for ( size_t n = 0; n < n_pts; ++n )
            glVertex3d(pts[n].XX, pts[n].YY, zz);
        glEnd();

        glPointSize(7);
        glColor3f(1.0, 0.0, 1.0);
        glBegin(GL_POINTS);
        glVertex3d(pts[0].XX, pts[0].YY, zz);
        glEnd();
    }
#endif
    
    size_t iR = 0;
    size_t iL = n_pts;
    
    Vector2 R = pts[0];
    Vector2 L = pts[0];
    
    real xxR, yyR, dxR;
    real xxL, yyL, dxL;
    
    // start on the line just above the bottom point
    int yy = (int)ceil(R.YY);
    
    while ( true )
    {
        //std::clog << "section at Y = " << yy << std::endl;
        
        // find next point on right side:
        if ( R.YY <= yy )
        {
            do {
                if ( ++iR > iL )
                    return;
                xxR = R.XX;
                yyR = R.YY;
                R = pts[iR];
            } while ( R.YY <= yy );
            
            dxR = ( R.XX - xxR ) / ( R.YY - yyR );
            xxR += dxR * ( yy - yyR );
        }
        
        // find next point on left side:
        if ( L.YY <= yy )
        {
            do {
                if ( --iL < iR )
                    return;
                xxL = L.XX;
                yyL = L.YY;
                L = pts[iL];
            } while ( L.YY <= yy );
            
            dxL = ( L.XX - xxL ) / ( L.YY - yyL );
            xxL += dxL * ( yy - yyL );
        }
        
        // index of the last line without changing edges:
        int yym = (int)floor(std::min(L.YY, R.YY));
        
        for ( ; yy <= yym; ++yy )
        {
            int inf = (int) ceil(xxL);
            int sup = (int)floor(xxR);
            if ( inf <= sup )
            {
                // draw the horizontal line:
                paint(inf, sup, yy, zz, arg);
            }
            xxL += dxL;
            xxR += dxR;
        }
    }
}


/*
 pts[] is an anti-clockwise polygon, starting with the point of lowest Y.
 */
void Rasterizer::paintPolygon2D(void (*paint)(int, int, int, int, void*), void * arg,
                                const size_t n_pts, const Vertex2 pts[],
                                const int zz)
{
#ifdef DISPLAY
    if ( rasterizer_draws )
    {
        glLineWidth(1);
        glColor3f(0.0, 0.0, 1.0);
        glBegin(GL_LINE_LOOP);
        for ( size_t n = 0; n < n_pts; ++n )
            glVertex3d(pts[n].XX, pts[n].YY, zz);
        glEnd();
        
        glPointSize(7);
        glColor3f(1.0, 0.0, 1.0);
        glBegin(GL_POINTS);
        glVertex3d(pts[0].XX, pts[0].YY, zz);
        glEnd();
    }
#endif

#if ( 0 )
    // print polygon:
    std::clog << std::endl << zz << " ";
    for ( size_t n = 0; n < n_pts; ++n )
        pts[n].print(std::clog);
#endif
    
    size_t iR = 0;
    size_t iL = n_pts;

    Vertex2 R = pts[0];
    Vertex2 L = pts[0];
  
    real xxR, yyR, dxR;
    real xxL, yyL, dxL;

    // start on the line just above the bottom point
    int yy = (int)ceil(pts[0].YY);
    
    while ( true )
    {
        // find next point on right side:
        if ( R.YY <= yy )
        {
            do {
                if ( ++iR > iL )
                    return;
                xxR = R.XX;
                yyR = R.YY;
                R = pts[iR];
            } while ( R.YY <= yy );
            
            dxR = ( R.XX - xxR ) / ( R.YY - yyR );
            xxR += dxR * ( yy - yyR );
        }
        
        // find next point on left side:
        if ( L.YY <= yy )
        {
            do {
                if ( --iL < iR )
                    return;
                xxL = L.XX;
                yyL = L.YY;
                L = pts[iL];
            } while ( L.YY <= yy );
            
            dxL = ( L.XX - xxL ) / ( L.YY - yyL );
            xxL += dxL * ( yy - yyL );
        }
        
        // index of the last line without changing edges:
        int yym = (int)floor(std::min(L.YY, R.YY));
        
        for ( ; yy <= yym; ++yy )
        {
            int inf = (int) ceil(xxL);
            int sup = (int)floor(xxR);
            if ( inf <= sup )
            {
                // draw the horizontal line:
                paint(inf, sup, yy, zz, arg);
            }
            xxL += dxL;
            xxR += dxR;
        }
    }
}


void Rasterizer::paintFatLine2D(void (*paint)(int, int, int, int, void*), void * arg,
                                const Vector2& P, const Vector2& Q, const real iPQ,
                                const real radius)
{
    Vector2 PQ = ( radius * iPQ ) * ( Q - P );
    
    Vector2 A = PQ + Vector2(PQ.YY, -PQ.XX);
    Vector2 B = PQ + Vector2(-PQ.YY, PQ.XX);
    
    Vector2 pts[4];
    
    // put lowest point at index 0, and build anti-clockwise polygon
    if ( P.YY < Q.YY )
    {
        if ( P.XX < Q.XX )
        {
            pts[0] = P - B;
            pts[1] = Q + A;
            pts[2] = Q + B;
            pts[3] = P - A;
        }
        else
        {
            pts[0] = P - A;
            pts[1] = P - B;
            pts[2] = Q + A;
            pts[3] = Q + B;
        }
    }
    else
    {
        if ( P.XX < Q.XX )
        {
            pts[0] = Q + A;
            pts[1] = Q + B;
            pts[2] = P - A;
            pts[3] = P - B;
        }
        else
        {
            pts[0] = Q + B;
            pts[1] = P - A;
            pts[2] = P - B;
            pts[3] = Q + A;
        }
    }
    
    paintPolygon2D(paint, arg, 4, pts, 0);
}


void Rasterizer::paintFatLine2D(void (*paint)(int, int, int, int, void*), void * arg,
                                const Vector2& P, const Vector2& Q, const real iPQ,
                                const real radius, const Vector2& offset, const Vector2& delta )
{
    Vector2 PQ = ( radius * iPQ ) * ( Q - P );
    
    Vector2 A = PQ + Vector2(PQ.YY, -PQ.XX);
    Vector2 B = PQ + Vector2(-PQ.YY, PQ.XX);

    Vector2 oP = P - offset;
    Vector2 oQ = Q - offset;

    Vector2 pts[4];
    
    // put lowest point at index 0, and build anti-clockwise polygon
    if ( P.YY < Q.YY )
    {
        if ( P.XX < Q.XX )
        {
            pts[0] = ( oP - B ).e_mul(delta);
            pts[1] = ( oQ + A ).e_mul(delta);
            pts[2] = ( oQ + B ).e_mul(delta);
            pts[3] = ( oP - A ).e_mul(delta);
        }
        else
        {
            pts[0] = ( oP - A ).e_mul(delta);
            pts[1] = ( oP - B ).e_mul(delta);
            pts[2] = ( oQ + A ).e_mul(delta);
            pts[3] = ( oQ + B ).e_mul(delta);
        }
    }
    else
    {
        if ( P.XX < Q.XX )
        {
            pts[0] = ( oQ + A ).e_mul(delta);
            pts[1] = ( oQ + B ).e_mul(delta);
            pts[2] = ( oP - A ).e_mul(delta);
            pts[3] = ( oP - B ).e_mul(delta);
        }
        else
        {
            pts[0] = ( oQ + B ).e_mul(delta);
            pts[1] = ( oP - A ).e_mul(delta);
            pts[2] = ( oP - B ).e_mul(delta);
            pts[3] = ( oQ + A ).e_mul(delta);
        }
    }
    
    paintPolygon2D(paint, arg, 4, pts, 0);
}


void Rasterizer::paintBox2D(void (*paint)(int, int, int, int, void*), void * arg,
                            const Vector2& P, const Vector2& Q, const real radius,
                            const Vector2& offset, const Vector2& delta )
{
    int inf[2], sup[2];
    
    for ( int d = 0; d < 2; ++d )
    {
        if ( P[d] > Q[d] )
        {
            inf[d] = (int) ceil( ( Q[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( P[d] + radius - offset[d] ) * delta[d] );
        }
        else
        {
            inf[d] = (int) ceil( ( P[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( Q[d] + radius - offset[d] ) * delta[d] );
        }
    }
    
    for ( int yy = inf[1]; yy <= sup[1]; ++yy )
        paint(inf[0], sup[0], yy, 0, arg);
}


//==============================================================================
//                               3D
//==============================================================================
#pragma mark - 3D


/// function for qsort: compares the Z component of two points
int Rasterizer::compareVertex3(const void * a, const void * b)
{
    real az = ((Vertex3 const*)(a))->ZZ;
    real bz = ((Vertex3 const*)(b))->ZZ;
    
    return ( az > bz ) - ( bz > az );
}


void Rasterizer::paintPolygon3D(void (*paint)(int, int, int, int, void*), void * arg,
                                const size_t n_pts, Vertex3 pts[])
{
    assert_true( n_pts > 1 );
    
#ifdef DISPLAY
    if ( rasterizer_draws )
    {
        //draw the vertex of the volume:
        glPointSize(6);
        glBegin(GL_POINTS);
        glColor3f(1.0, 0.0, 0.0);
        for ( size_t n = 0; n < n_pts; ++n )
            glVertex3d( pts[n].XX, pts[n].YY, pts[n].ZZ );
        glEnd();
        
        //draw the edges of the volume:
        glLineWidth(0.5);
        glBegin(GL_LINES);
        glColor3f(0.0, 1.0, 1.0);
        for ( size_t n = 0;   n < n_pts; ++n )
        for ( size_t m = n+1; m < n_pts; ++m )
            if ( pts[n].UU  &  pts[m].UU )
            {
                glVertex3d( pts[n].XX, pts[n].YY, pts[n].ZZ );
                glVertex3d( pts[m].XX, pts[m].YY, pts[m].ZZ );
            }
        glEnd();
    }
#endif
    
    //order the points in increasing Z:
    qsort(pts, n_pts, sizeof(Vertex3), &compareVertex3);
    
    //we can normally only cross four sides of a parallelogram in 3D
    //but in some degenerate cases, it can be more
    const size_t max = 16;
    Vertex2 xy[max];
    
    size_t above = 0;
    int zz  = (int) ceil( pts[0].ZZ );
    
    while ( ++above < n_pts )
    {
        //printf("restart at zz %4i\n", zz );
        
        //find the first point strictly above the plane Z = zz:
        //the index of this point is (above-1)
        while ( pts[above].ZZ <= zz )
        {
            if ( ++above >= n_pts )
                return;
        }
        
        //the next time we have to recalculate the lines
        //is when pts[above] will be below the plane Z = zzn:
        int zzn = (int)ceil( pts[above].ZZ );
        
        //number of edges crossing the plane at Z=zz;
        size_t nbl = 0;
        //set-up all the lines, which join any point below the plane
        //to any point above the plane, being a edge of the solid polygon:
        for ( size_t ii = 0;     ii < above; ++ii )
        for ( size_t jj = above; jj < n_pts; ++jj )
        {
            //test if [ii, jj] are joined:
            if ( pts[ii].UU  &  pts[jj].UU )
            {
                real dzz = pts[jj].ZZ - pts[ii].ZZ;
                
                if ( dzz > 0 )
                {
                    real dxz = ( pts[jj].XX - pts[ii].XX ) / dzz;
                    real dyz = ( pts[jj].YY - pts[ii].YY ) / dzz;
                    real dz  = zz - pts[ii].ZZ;
                    xy[nbl].set(pts[ii].XX + dxz * dz,
                                pts[ii].YY + dyz * dz, dxz, dyz);
                    ++nbl;
                    assert_true( nbl < max );
                }
            }
        }
        
        // the edges of the convex solid polygon should not intersect,
        // so we can take the convex hull only once here:
        bool need_hull = true;
        size_t nbp; //number of points in the hull.
        
        for ( ; zz < zzn; ++zz )
        {
            if ( need_hull )
            {
                //make the convex hull of the points from xy[]:
                nbp = convexHull2D(nbl, xy);
                //printf("zz %3i : nbp = %i\n", zz, nbp);
                
                //in the particular case where some points overlap, we might
                //loose them, in which case we need to redo the hull later
                need_hull = ( nbp != nbl );
            }
            
            paintPolygon2D(paint, arg, nbp, xy, zz);
            
            //update the coordinates according to the slopes, for the next zz:
            for ( size_t ii = 0; ii < nbl; ++ii )
                xy[ii].move();
        }
    }
}


void Rasterizer::paintFatLine3D(void (*paint)(int, int, int, int, void*), void * arg,
                                const Vector3& P, const Vector3& Q, const real iPQ,
                                const real radius, const Vector3& offset, const Vector3& delta )
{
    real radius2 = radius * M_SQRT2;
    Vector3 PQ = ( Q - P ) * iPQ;
    Vector3 A, B;
    
    // make an orthogonal basis with norm = radius:
#if ( 1 )
    //std::clog << std::scientific << PQ.normSqr() << '\n';
    PQ.orthonormal(A, B);
    A *= radius2;
    B *= radius2;
#else
    A = PQ.orthogonal(radius2);
    B = cross(PQ, A);
#endif
    
    PQ *= radius;
    A = A.e_mul(delta);
    B = B.e_mul(delta);
    
    /*
     Extend segment PQ by `radius` on each side,
     and convert to the grid's coordinates:
     grid coordinates = ( coordinates - min ) * delta
     */
    Vector3 endP = ( P - PQ - offset ).e_mul(delta);
    Vector3 endQ = ( Q + PQ - offset ).e_mul(delta);
    
    // set the vertex of a generalized cylinder aligned along PQ
    Vertex3 pts[8];
    
    /*
     Below, the last arguments is a bitfield defining which point
     are connected to form the edges of the polygonal volume.
     */
    pts[0].set(endP + A, 0b000000010011);
    pts[1].set(endP + B, 0b000000100110);
    pts[2].set(endP - A, 0b000001001100);
    pts[3].set(endP - B, 0b000010001001);
    pts[4].set(endQ + A, 0b001100010000);
    pts[5].set(endQ + B, 0b011000100000);
    pts[6].set(endQ - A, 0b110001000000);
    pts[7].set(endQ - B, 0b100110000000);
    
    //paint the volume:
    paintPolygon3D(paint, arg, 8, pts);
}


/**
 Paint a cylinder of Hexagonal base.
 The hexagon covering the unit disc has vertices:
     A (  0, -2*a )
     B (  b,   -a )
     C (  b,    a )
     D (  0,  2*a ) = -A
     E ( -b,    a ) = -B
     F ( -b,   -a ) = -C
 with b = sqrt(3) * a
     b = 1
     a = 1 / sqrt(3)
 */
void Rasterizer::paintHexLine3D(void (*paint)(int, int, int, int, void*), void * arg,
                                const Vector3& P, const Vector3& Q, const real iPQ,
                                const real radius, const Vector3& offset, const Vector3& delta)
{
    Vector3 A, B, C;
    Vector3 PQ = ( Q - P ) * iPQ;
    
    PQ.orthonormal(A, C);
    
    // normalize vectors to norm = radius:
    constexpr real alpha = 1.1547005383792517; // = 2.0 / sqrt(3);
    
    PQ *= radius;

    // build the vertices of the Hexagon
    A *= radius * alpha;
    B  = C * radius + A * 0.5;
    C  = B - A;
    
    A = A.e_mul(delta);
    B = B.e_mul(delta);
    C = C.e_mul(delta);
    
    /*
     Extend segment PQ by `radius` on each side,
     and convert to the grid's coordinates:
     grid coordinates = ( coordinates - min ) * delta
    */
    Vector3 endP = ( P - PQ - offset ).e_mul(delta);
    Vector3 endQ = ( Q + PQ - offset ).e_mul(delta);
    
    // set the vertex of a generalized cylinder aligned along PQ
    Vertex3 pts[12];
    
    /*
     Below, the last arguments is a bitfield defining which point
     are connected to form the edges of the polygonal volume.
    */
    pts[ 0].set(endP + A, 0b000000000001000011);
    pts[ 1].set(endP + B, 0b000000000010000110);
    pts[ 2].set(endP + C, 0b000000000100001100);
    pts[ 3].set(endP - A, 0b000000001000011000);
    pts[ 4].set(endP - B, 0b000000010000110000);
    pts[ 5].set(endP - C, 0b000000100000100001);
    pts[ 6].set(endQ + A, 0b000011000001000000);
    pts[ 7].set(endQ + B, 0b000110000010000000);
    pts[ 8].set(endQ + C, 0b001100000100000000);
    pts[ 9].set(endQ - A, 0b011000001000000000);
    pts[10].set(endQ - B, 0b110000010000000000);
    pts[11].set(endQ - C, 0b100001100000000000);
    
    paintPolygon3D(paint, arg, 12, pts);
}


void Rasterizer::paintBox3D(void (*paint)(int, int, int, int, void*), void * arg,
                            const Vector3& P, const Vector3& Q, const real radius,
                            const Vector3& offset, const Vector3& delta )
{
    int inf[3], sup[3];
    
    for ( int d = 0; d < 3; ++d )
    {
        if ( P[d] > Q[d] )
        {
            inf[d] = (int) ceil( ( Q[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( P[d] + radius - offset[d] ) * delta[d] );
        }
        else
        {
            inf[d] = (int) ceil( ( P[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( Q[d] + radius - offset[d] ) * delta[d] );
        }
    }
    
    for ( int zz = inf[2]; zz <= sup[2]; ++zz )
    for ( int yy = inf[1]; yy <= sup[1]; ++yy )
        paint(inf[0], sup[0], yy, zz, arg);
}

