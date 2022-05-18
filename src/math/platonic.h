// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PLATONIC_H
#define PLATONIC_H

#include "real.h"
#include <iostream>

namespace Platonic
{
    /// One of the corner of a Platonic solid
    class Corner
    {
    public:

        /// an index to identify this vertex
        unsigned inx_;
        
        /// Coordinates in space
        real     pos_[3];
        
        Corner()  { inx_=0; pos_[0]=0; pos_[1]=0; pos_[2]=0; }
        
        ~Corner() {}
    };
    
    
    /// A vertex is interpolated from 3 Corners
    class Vertex
    {
    public:
        
        /// pointers to the corners being interpolated
        Corner*   vertex_[3];
        
        /// Weights of the interpolation
        unsigned  weight_[3];
        
        void sort(unsigned ii, unsigned jj);
        
        void sort();
        
        bool equal(const Vertex &) const;
        
        /// export coordinates
        void store(float vec[3]) const;
        
        /// export coordinates
        void store(double vec[3]) const;
        
        Vertex() { vertex_[0]=nullptr; vertex_[1]=nullptr; vertex_[2]=nullptr; }
        
        Vertex(Corner *, unsigned, Corner *, unsigned, Corner *, unsigned);
        
        ~Vertex() {}
        
        int      weight(int x) const { return weight_[x]; }
        
        int      sum_weights() const { return weight_[0]+weight_[1]+weight_[2]; }
        
        void     print(unsigned, std::ostream&) const;
    };
    
    
    /// A refined polyhedra to serve as a tesselation
    /**
     Platonic solids made of triangles can be refined by subdividing the faces
     into smaller triangles. The faces are re-assembled together into the Solid
     without duplicating points.
     
     The level of refinement is set by an integer N > 0, corresponding to the 
     number of section in which each edge of the original Platonic solid is divided.
     
     */
    class Solid
    {
    public:
        
        ///regular polyhedra made of triangles
        enum   Polyhedra { TETRAHEDRON=0, OCTAHEDRON=1, ICOSAHEDRON=2 };
        
        /// Number of Vertices
        static unsigned nb_vertices(Polyhedra K);
        
        /// Number of Faces
        static unsigned nb_faces(Polyhedra K);
        
        /// Number of Edges
        static unsigned nb_edges(Polyhedra K);
        
        /// Number of Vertices after refinement of level 'r'
        static unsigned nb_vertices(Polyhedra K, unsigned r);
        
        /// Number of Faces after refinement of level 'r'
        static unsigned nb_faces(Polyhedra K, unsigned r);
        
        /// Number of Edges after refinement of level 'r'
        static unsigned nb_edges(Polyhedra K, unsigned r);
        
        /// an estimation of the length of the edge
        static real     length_edge(Polyhedra K, unsigned r);
        
        /// build as polyhedra `K` refined by order `div`
        Solid(Polyhedra K, unsigned div, bool make_edges = true);
        
        /// destructor
        ~Solid();
        
        /// set array of indices that define the edges
        void           setEdges();
        
        /// number of derived vertices
        unsigned int   nb_vertices()        const { return num_vertices_; }
        
        /// reference to derived vertex `ii`
        Vertex&        vertex(int ii)       const { return vertices_[ii]; }
        
        /// copy coordinates of points to given array
        void           put_vertices(float* vec) const;
        
        /// copy coordinates of points to given array
        void           put_vertices(double* vec) const;

        /// return pointer to array of coordinates of vertices
        const float*   vertex_data()        const { return coordinates_; }
        
        /// address of coordinates for vertex `v` ( `v < nb_vertices()` )
        const float*   vertex_data(int v)   const { return coordinates_ + 3 * v; }
        
        
        /// number of points in the edges = 2 * nb-of-edges
        unsigned int   nb_edges()           const { return num_edges_; }
        
        /// array of indices to the vertices in each edge (2 per edge)
        unsigned int*  edges_data()         const { return edges_; }
        
        /// return address of first vertex of edge `e`
        const float*   edge_data0(int e)    const { return coordinates_ + 3 * edges_[2*e]; }
        
        /// return address of second vertex of edge `e`
        const float*   edge_data1(int e)    const { return coordinates_ + 3 * edges_[2*e+1]; }
        
        
        /// return address of first vertex of edge `e`
        unsigned       edge_indx0(int e)    const { return edges_[2*e]; }
        
        /// return address of second vertex of edge `e`
        unsigned       edge_indx1(int e)    const { return edges_[2*e+1]; }
        
        
        /// number of faces (each face is a triangle of 3 vertices)
        unsigned int   nb_faces()           const { return num_faces_; }
        
        /// array of indices to the vertices in each face (3 vertices per face)
        unsigned int*  faces_data()         const { return faces_; }
        
        /// return address of first vertex of face `f`
        const float*   face_data0(int f)    const { return coordinates_ + 3 * faces_[3*f]; }
        /// return address of second vertex of face `f`
        const float*   face_data1(int f)    const { return coordinates_ + 3 * faces_[3*f+1]; }
        /// return address of third vertex of face `f`
        const float*   face_data2(int f)    const { return coordinates_ + 3 * faces_[3*f+2]; }
        
        
        
        /// return address of first vertex of face `f`
        unsigned       face_indx0(int f)    const { return faces_[3*f]; }
        /// return address of second vertex of face `f`
        unsigned       face_indx1(int f)    const { return faces_[3*f+1]; }
        /// return address of third vertex of face `f`
        unsigned       face_indx2(int f)    const { return faces_[3*f+2]; }
        
    private:
        
        /// C-array of vertices of the Platonic solid
        Corner    * corners_;
        /// C-array of derived vertices
        Vertex    * vertices_;
        
        unsigned    num_corners_;
        
        unsigned    num_vertices_, max_vertices_;
        
        unsigned    num_vertices_on_edges_;
        
        /// coordinates of all vertices
        float     * coordinates_;
        
        
        unsigned    num_faces_,  max_faces_;
        
        /// array of indices of the points making the faces
        unsigned  * faces_;
        
        
        unsigned    num_edges_,  max_edges_;
        
        /// array of indices of the points making the edges
        unsigned  * edges_;
        
        void        setCorner(unsigned, real x, real y, real z);
        unsigned    addVertex(const Vertex&);
        unsigned    getVertex(const Vertex&);
        
        void        init(unsigned, real* vdata[3], unsigned, unsigned* fdata[3], unsigned);
        void        initTetrahedron(unsigned div);
        void        initOctahedron(unsigned div);
        void        initIcosahedron(unsigned div);
        
        void        setVertices(Polyhedra K, unsigned div);
        void        addFace(unsigned, unsigned, unsigned);
        void        addEdge(unsigned, unsigned);
        void        refineEdge(unsigned a, unsigned b, unsigned div);
        void        refineFace(unsigned a, unsigned b, unsigned c, unsigned div);
    };
}

#endif
