// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "platonic.h"
#include <cmath>


//------------------------------------------------------------------------------
#pragma mark -

namespace Platonic
{
    
    unsigned Solid::nb_vertices(Polyhedra K)
    {
        static const unsigned V[] = { 4, 6,  12 };
        return V[K];
    }
    
    unsigned Solid::nb_faces(Polyhedra K)
    {
        static const unsigned F[] = { 4, 8,  20 };
        return F[K];
    }
    
    unsigned Solid::nb_edges(Polyhedra K)
    {
        static const unsigned E[] = { 6, 12, 30 };
        return E[K];
    }
    
    unsigned Solid::nb_vertices(Polyhedra K, unsigned N)
    {
        if ( N > 0 )
            return nb_vertices(K) + nb_edges(K)*(N-1) + nb_faces(K)*((N-1)*(N-2))/2;
        return 0;
    }
    
    unsigned Solid::nb_faces(Polyhedra K, unsigned N)
    {
        return nb_faces(K)*N*N;
    }
    
    unsigned Solid::nb_edges(Polyhedra K, unsigned N)
    {
        return nb_edges(K)*N*N;
    }
    
    /**
     We estimate L from:
     - the area of the sphere is 4*PI
     - the number of triangles on it is nb_faces()
     - the area of an equilateral triangle of side L is sqrt(3)*(L/2)^2
     */
    real Solid::length_edge(Polyhedra K, unsigned N)
    {
        return 4 * sqrt( M_PI / ( sqrt(3) * nb_faces(K,N) ) );
    }
    
    //------------------------------------------------------------------------------
    Solid::Solid(Polyhedra K, unsigned div, bool make_edges)
    {
        num_corners_  = nb_vertices(K);
        corners_      = new Corner[num_corners_];
        
        max_vertices_ = nb_vertices(K, div);
        num_vertices_ = 0;
        vertices_     = new Vertex[max_vertices_];
        
        num_vertices_on_edges_ = nb_vertices(K) + (div-1) * nb_edges(K);

        max_faces_    = nb_faces(K, div);
        num_faces_    = 0;
        faces_        = new unsigned[3*max_faces_];
        
        max_edges_    = nb_edges(K, div);
        num_edges_    = 0;
        edges_        = nullptr;
        
        coordinates_  = nullptr;
        
        if ( div > 0 )
        {
            setVertices(K, div);
        
            if ( make_edges )
                setEdges();
        }
    }
    
    
    Solid::~Solid()
    {
        delete[] corners_;
        delete[] vertices_;
        delete[] edges_;
        delete[] faces_;
    }
    
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    void Solid::setCorner(unsigned inx, real x, real y, real z)
    {
        assert_true(inx < num_corners_);
        corners_[inx].inx_ = inx;
        corners_[inx].pos_[0] = x;
        corners_[inx].pos_[1] = y;
        corners_[inx].pos_[2] = z;
        
        // also create the mirror 'derived' vertex for this point
        addVertex(Vertex(&corners_[inx], 1, nullptr, 0, nullptr, 0));
    }

    
    /**
     register a new derived-vertex:
     */
    unsigned Solid::addVertex(Vertex const& X)
    {
        assert_true( num_vertices_ < max_vertices_ );
        unsigned inx = num_vertices_++;
        vertices_[inx] = X;
        return inx;
    }
    
    
    /**
     return the Vertex identical to `X` if it exists, or store it otherwise.
     
     We only check existence if X is on an edge, since only then may it
     has been created previously while processing another face.
     */
    unsigned Solid::getVertex(Vertex const& X)
    {
        // check if Vertex belong to an edge of the original Solid:
        if ( X.weight_[2] == 0 )
        {
            /**
             We limit the search to the vertices that are on the edges of
             the original PlatonicSolid, since they are the only one to be duplicated
             Yet, we have a linear search that may be limiting for very high orders
             */
            for ( unsigned ii = 0; ii < num_vertices_on_edges_; ++ii )
            {
                if ( X.equal(vertices_[ii]) )
                    return ii;
            }
        }
        return addVertex(X);
    }
    
    
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    
    void Solid::refineEdge(unsigned a, unsigned b, unsigned div)
    {
        Corner *A = &corners_[a];
        Corner *B = &corners_[b];
        
        for ( unsigned ii = 1; ii < div; ++ii )
            addVertex(Vertex(A, div-ii, B, ii, nullptr, 0));
    }

    
    void Solid::addFace(unsigned a, unsigned b, unsigned c)
    {
        //printf("face %i %i %i\n", a, b, c);
        assert_true( a!=b && b!=c && a!=c );
        faces_[3*num_faces_  ] = a;
        faces_[3*num_faces_+1] = b;
        faces_[3*num_faces_+2] = c;
        ++num_faces_;
    }

    
    void Solid::refineFace(unsigned a, unsigned b, unsigned c, unsigned div)
    {
        Corner *A = &corners_[a];
        Corner *B = &corners_[b];
        Corner *C = &corners_[c];

        unsigned* line = new unsigned[div+1];
        
        for ( unsigned jj = 0;  jj <= div;  ++jj )
            line[jj] = getVertex(Vertex(A, div-jj, B, jj, nullptr, 0));
        
        for ( unsigned ii = 1;  ii <= div;  ++ii )
        {
            unsigned N = getVertex(Vertex(A, div-ii, C, ii, nullptr, 0));
            
            addFace(line[0], line[1], N);
            line[0] = N;
            
            for ( unsigned jj = 1;  ii+jj <= div;  ++jj )
            {
                N = getVertex(Vertex(A, div-ii-jj, B, jj, C, ii));
                
                addFace(line[jj], line[jj+1], N);
                addFace(line[jj-1], line[jj], N);
                
                line[jj] = N;
            }
        }
        delete[] line;
    }
    
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    void Solid::initTetrahedron(unsigned div)
    {
        real a = 1/3.0;
        real b = sqrt(2)/3.0;
        real c = sqrt(2/3.0);
        
        
        // Four vertices on unit sphere
        real vex[4][3] = {
            { 0, 2*b, -a},
            {-c,  -b, -a},
            { c,  -b, -a},
            { 0,  0,   1},
        };
        
        // Faces are ordered for OpenGL's default rule:
        // Counter-Clockwise = facing out
        unsigned fac[4][3] = {
            {0, 2, 1},
            {1, 3, 0},
            {0, 3, 2},
            {1, 2, 3}
        };
        
        for ( int v = 0; v < 4; ++v )
            setCorner(v, vex[v][0], vex[v][1], vex[v][2]);
        
        for ( int f = 0; f < 4; ++f )
        {
            unsigned i = fac[f][0];
            unsigned j = fac[f][1];
            unsigned k = fac[f][2];
            
            if ( i < j ) refineEdge(i, j, div);
            if ( j < k ) refineEdge(j, k, div);
            if ( k < i ) refineEdge(k, i, div);
        }
        
        assert_true( num_vertices_ == num_vertices_on_edges_ );
        
        for ( int f = 0; f < 4; ++f )
            refineFace(fac[f][0], fac[f][1], fac[f][2], div);
    }
    
    
    void Solid::initOctahedron(unsigned div)
    {
        // Eight vertices on unit sphere
        real vex[6][3] = {
            { 0,  0,  1},
            { 0,  0, -1},
            { 1,  0,  0},
            {-1,  0,  0},
            { 0, -1,  0},
            { 0,  1,  0},
        };
        
        // Faces are ordered for OpenGL's default rule:
        // Counter-Clockwise = facing out
        unsigned fac[8][3] = {
            {2, 0, 4},
            {1, 3, 5},
            {0, 3, 4},
            {1, 5, 2},
            {3, 0, 5},
            {1, 2, 4},
            {0, 2, 5},
            {1, 4, 3}
        };

        
        for ( int v = 0; v < 6; ++v )
            setCorner(v, vex[v][0], vex[v][1], vex[v][2]);
        
        for ( int f = 0; f < 8; ++f )
        {
            unsigned a = fac[f][0];
            unsigned b = fac[f][1];
            unsigned c = fac[f][2];
            
            if ( a < b ) refineEdge(a, b, div);
            if ( b < c ) refineEdge(b, c, div);
            if ( c < a ) refineEdge(c, a, div);
        }
        
        assert_true( num_vertices_ == num_vertices_on_edges_ );
        
        for ( int f = 0; f < 8; ++f )
            refineFace(fac[f][0], fac[f][1], fac[f][2], div);
    }
    
    
    void Solid::initIcosahedron(unsigned div)
    {
#if ( 0 )
        const real x = (1+std::sqrt(5.0))*0.5;
        const real T = x/sqrt(1+x*x);
        const real Z = 1/sqrt(1+x*x);
#else
        constexpr real T = 0.850650808352039932;
        constexpr real Z = 0.525731112119133606;
#endif
        
        // Twelve vertices of icosahedron on unit sphere
        real vex[12][3] = {
            { T,  Z,  0},
            {-T, -Z,  0},
            {-T,  Z,  0},
            { T, -Z,  0},
            { Z,  0,  T},
            {-Z,  0, -T},
            { Z,  0, -T},
            {-Z,  0,  T},
            { 0,  T,  Z},
            { 0, -T, -Z},
            { 0, -T,  Z},
            { 0,  T, -Z}
        };
        
        // Faces are ordered for OpenGL's default rule:
        // Counter-Clockwise = facing out
        unsigned fac[20][3] = {
            {0,  3,  6},
            {1,  7,  2},
            {0,  4,  3},
            {1,  2,  5},
            {0,  8,  4},
            {1,  5,  9},
            {0, 11,  8},
            {1,  9, 10},
            {0,  6, 11},
            {1, 10,  7},
            {4,  8,  7},
            {5,  6,  9},
            {2,  7,  8},
            {6,  3,  9},
            {2,  8, 11},
            {9,  3, 10},
            {5,  2, 11},
            {3,  4, 10},
            {6,  5, 11},
            {4,  7, 10},
        };
        
        for ( int v = 0; v < 12; ++v )
            setCorner(v, vex[v][0], vex[v][1], vex[v][2]);

        for ( int f = 0; f < 20; ++f )
        {
            unsigned a = fac[f][0];
            unsigned b = fac[f][1];
            unsigned c = fac[f][2];
            
            if ( a < b ) refineEdge(a, b, div);
            if ( b < c ) refineEdge(b, c, div);
            if ( c < a ) refineEdge(c, a, div);
        }
        
        assert_true( num_vertices_ == num_vertices_on_edges_ );

        for ( int f = 0; f < 20; ++f )
            refineFace(fac[f][0], fac[f][1], fac[f][2], div);
    }
    
    
    void Solid::setVertices(Polyhedra kind, unsigned div)
    {
        assert_true(div > 0);
        
        switch( kind )
        {
            case TETRAHEDRON:
                initTetrahedron(div);
                break;
                
            case OCTAHEDRON:
                initOctahedron(div);
                break;
                
            case ICOSAHEDRON:
                initIcosahedron(div);
                break;
        }
        
        assert_true( num_vertices_ == max_vertices_ );
        assert_true( num_faces_ == max_faces_ );
        
        delete[] coordinates_;
        coordinates_ = new float[3*max_vertices_];

        for ( unsigned n = 0; n < num_vertices_; ++n )
            vertices_[n].store(coordinates_+3*n);
    }
    
    
    void Solid::put_vertices(float * vec) const
    {
        for ( unsigned n = 0; n < num_vertices_; ++n )
            vertices_[n].store(vec+3*n);
    }

    
    void Solid::put_vertices(double * vec) const
    {
        for ( unsigned n = 0; n < num_vertices_; ++n )
            vertices_[n].store(vec+3*n);
    }

    
    void Solid::addEdge(unsigned a, unsigned b)
    {
        edges_[2*num_edges_  ] = a;
        edges_[2*num_edges_+1] = b;
        ++num_edges_;
    }
    
    
    void Solid::setEdges()
    {
        delete[] edges_;
        edges_ = new unsigned[2*max_edges_];
        num_edges_ = 0;

        //build edges from the faces:
        for ( unsigned f=0; f < max_faces_; ++f )
        {
            unsigned a = faces_[3*f  ];
            unsigned b = faces_[3*f+1];
            unsigned c = faces_[3*f+2];

            if ( a < b ) addEdge(a, b);
            if ( b < c ) addEdge(b, c);
            if ( c < a ) addEdge(c, a);
        }
        
        assert_true( num_edges_ == max_edges_ );
    }
    
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    
    Vertex::Vertex(Corner * v0, unsigned w0,
                   Corner * v1, unsigned w1,
                   Corner * v2, unsigned w2 )
    {
        vertex_[0] = v0;
        weight_[0] = w0;
        assert_true( w0 == 0 || v0 );
        
        vertex_[1] = v1;
        weight_[1] = w1;
        assert_true( w1 == 0 || v1 );
        
        vertex_[2] = v2;
        weight_[2] = w2;
        assert_true( w2 == 0 || v2 );
        
        sort();
    }
    
    
    void Vertex::sort(unsigned ii, unsigned jj)
    {
        if ( weight_[ii] < weight_[jj] || ( weight_[ii] == weight_[jj] && vertex_[ii] < vertex_[jj]) )
        {
            Corner  * p = vertex_[jj];
            int w = weight_[jj];
            vertex_[jj] = vertex_[ii];
            weight_[jj] = weight_[ii];
            vertex_[ii] = p;
            weight_[ii] = w;
        }
    }
    
    
    /**
     order vertices in decreasing weights:
     */
    void Vertex::sort()
    {
        sort(0,1);
        sort(1,2);
        sort(0,1);
    }
    
    
    /**
     The indices should be sorted for this to work
     */
    bool Vertex::equal(const Vertex & X) const
    {
        int s =   sum_weights();
        int t = X.sum_weights();
        return (   ((vertex_[0]==X.vertex_[0] && t*weight_[0]==s*X.weight_[0]) || (weight_[0]==0 && X.weight_[0]==0))
                && ((vertex_[1]==X.vertex_[1] && t*weight_[1]==s*X.weight_[1]) || (weight_[1]==0 && X.weight_[1]==0))
                && ((vertex_[2]==X.vertex_[2] && t*weight_[2]==s*X.weight_[2]) || (weight_[2]==0 && X.weight_[2]==0)));
    }
        
    
    void Vertex::store(real C[3]) const
    {
        C[0] = 0;
        C[1] = 0;
        C[2] = 0;
        
        for ( int i = 0; i < 3; ++i )
        {
            Corner const* v = vertex_[i];
            if ( v )
            {
                real w = weight_[i];
                C[0] += w * v->pos_[0];
                C[1] += w * v->pos_[1];
                C[2] += w * v->pos_[2];
            }
        }
        
        //normalize:
        real n = C[0]*C[0] + C[1]*C[1] + C[2]*C[2];
        if ( n > 0 )
        {
            n = sqrt(n);
            for ( int d = 0; d < 3; ++d )
                C[d] /= n;
        }
    }
    
#if REAL_IS_DOUBLE
    void Vertex::store(float C[3]) const
    {
        double tmp[3];
        store(tmp);
        C[0] = (float)tmp[0];
        C[1] = (float)tmp[1];
        C[2] = (float)tmp[2];
    }
#else
    void Vertex::store(double C[3]) const
    {
        float tmp[3];
        store(tmp);
        C[0] = (double)tmp[0];
        C[1] = (double)tmp[1];
        C[2] = (double)tmp[2];
    }
#endif

    void Vertex::print(unsigned inx, std::ostream& out) const
    {
        out << "P" << inx << " = ( ";
        if ( weight_[2] == 0 && weight_[1] == 0 )
        {
            out << vertex_[0]->inx_ << " " << weight_[0] ;
        }
        else if ( weight_[2] == 0 )
        {
            out << vertex_[0]->inx_ << " " << weight_[0] << ", ";
            out << vertex_[1]->inx_ << " " << weight_[1];
        }
        else
        {
            out << vertex_[0]->inx_ << " " << weight_[0] << ", ";
            out << vertex_[1]->inx_ << " " << weight_[1] << ", ";
            out << vertex_[2]->inx_ << " " << weight_[2];
        }
        out << " )" << std::endl;
    }
    
}
