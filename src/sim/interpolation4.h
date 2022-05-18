// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INTERPOLATION4_H
#define INTERPOLATION4_H

#include "vector.h"
#include "iowrapper.h"
#include "mecapoint.h"

class Interpolation;
class Simul;
class Meca;

/// This represents an interpolation over up to 4 vertices of a Mecable
/** FJN 17.9.2018 */
class Interpolation4
{
private:
    
    /// Mecable from which points are interpolated
    Mecable const* mec_;

    /// index of first interpolated point
    unsigned       ref_;
    
    /// interpolation coefficients for points [ref, ref+1, ref+2, ref+3]
    /** The sum of these 4 coefficients is equal to one */
    real       coef_[4];
    
    /// number of interpolated points (order)
    unsigned       ord_;

public:
    
    /// constructor
    Interpolation4()
    {
        set(nullptr, 0);
        ord_ = 0;
    }
    
    /// set as pointing to vertex `p` of `mec`
    void set(Mecable const* mec, unsigned p);
    
    /// set as interpolated between vertices `p` and `q` of `mec`
    void set(Mecable const* mec, unsigned p, unsigned q, real coef);

    /// set as interpolated over 4 vertices, defined by position 'vec'
    void set(Mecable const*, unsigned, Vector const& vec);

    /// attachment mecable
    Mecable const* base() const { return mec_; }
    
    /// position in space
    Vector pos() const;
    
    /// number of points beeing interpolated
    size_t rank() const { return ord_; }

    /// first attachement point
    Mecapoint vertex0() const { return Mecapoint(mec_, ref_); }

    /// create addLink with given Interpolation
    void addLink(Meca&, Interpolation const&, real weight) const;
    
    /// create addLink with given Mecapoint
    void addLink(Meca&, Mecapoint const&, real weight) const;

    /// output
    void write(Outputter& out) const;
    
    /// input
    void read(Inputter& in, Simul&);
    
    /// printout
    void print(std::ostream& out) const;
};

#endif
