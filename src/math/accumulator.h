// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


/// Helper class to calculate moments of a cloud of points
/**
This calculated the mean and the variance of a set of given Vector
*/
class Accumulator
{
public:
    real sum;
    real avg[3];
    real var[9];
    
    /// set all accumulators to zero
    void reset()
    {
        sum = 0;
        for ( int i = 0; i < 3; ++i ) avg[i] = 0;
        for ( int i = 0; i < 9; ++i ) var[i] = 0;
    }
    
    Accumulator()
    {
        reset();
    }

    /// add `p` with weight 'w'
    void add(real w, Vector const& p)
    {
        sum += w;
        avg[0] += w * p.XX;
        var[0] += w * p.XX * p.XX;
#if ( DIM > 1 )
        avg[1] += w * p.YY;
        var[1] += w * p.YY * p.XX;
        var[4] += w * p.YY * p.YY;
#endif
#if ( DIM > 2 )
        avg[2] += w * p.ZZ;
        var[2] += w * p.ZZ * p.XX;
        var[5] += w * p.ZZ * p.YY;
        var[8] += w * p.ZZ * p.ZZ;
#endif
    }
    
    /// add `p` with weight 1.0
    void add(Vector const& p)
    {
        sum += 1;
        avg[0] += p.XX;
        var[0] += p.XX * p.XX;
#if ( DIM > 1 )
        avg[1] += p.YY;
        var[1] += p.YY * p.XX;
        var[4] += p.YY * p.YY;
#endif
#if ( DIM > 2 )
        avg[2] += p.ZZ;
        var[2] += p.ZZ * p.XX;
        var[5] += p.ZZ * p.YY;
        var[8] += p.ZZ * p.ZZ;
#endif
    }
    
    /// transform the second-order acumulators into variance
    void subtract_mean()
    {
        //Remove the mean:
        avg[0] /= sum;
        var[0] = var[0]/sum - avg[0] * avg[0];
#if ( DIM > 1 )
        avg[1] /= sum;
        var[1] = var[1]/sum - avg[1] * avg[0];
        var[4] = var[4]/sum - avg[1] * avg[1];
#endif
#if ( DIM > 2 )
        avg[2] /= sum;
        var[2] = var[2]/sum - avg[2] * avg[0];
        var[5] = var[5]/sum - avg[2] * avg[1];
        var[8] = var[8]/sum - avg[2] * avg[2];
#endif
    }
    
    real total_length() const
    {
        return sum;
    }
    
    real total_variance() const
    {
        return var[0] + var[4] + var[8];
    }

    void print_doc(std::ostream& out) const
    {
        out << COM << "cnt";
        out << SEP << "avgX" << SEP << "avgY" << SEP << "avgZ";
        out << SEP << "varX" << SEP << "varY" << SEP << "varZ";
        out << SEP << "var_sum";
    }
    
    void print(std::ostream& out, bool mode)
    {
        if ( mode )
            out << LIN << (int) sum;
        else
            out << SEP << sum;
        out << SEP << avg[0];
        out << SEP << avg[1];
        out << SEP << avg[2];
        out << SEP << var[0];
        out << SEP << var[4];
        out << SEP << var[8];
        out << SEP << var[0] + var[4] + var[8];
    }
};

