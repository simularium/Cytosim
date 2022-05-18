// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIELD_H
#define FIELD_H

#include "dim.h"
#include "real.h"
#include "grid.h"
#include "space.h"
#include "object.h"
#include "iowrapper.h"
#include "messages.h"
#include "exceptions.h"
#include "matsparsesym1.h"
#include "matsparsesym2.h"
#include "field_prop.h"
#include "field_values.h"

class FiberSet;

#ifdef DISPLAY
#  include "gle.h"
#  include "grid_display.h"
#endif


/// the type of Grid contained in a Field
typedef Grid<FieldScalar, DIM> FieldGrid;


/// value of type VAL defined as a function of position over the simulation Space
/**
 A field represents a value that is varying with position over space.
 It does so by storing a different value for each cell in a regular grid.
 
 Each cell holds the amount of molecules.
 The local concentration can be obtained by dividing by the cell volume:

     concentration = mGrid.cell(position) / mGrid.cellVolume();
 
 Note that the field is build on a Grid with square cells, because diffusion/reaction are then
 easier to implement. The Grid does not necessarily match the edges of the Space exactly,
 but instead extends outside, such as to cover the 'inside' region entirely.
 
 */
class Field : public Object
{
public:

    /// forward the type of value
    typedef FieldGrid::value_type value_type;
    
    /// Grid data object
    FieldGrid  mGrid;
    
    /// property
    FieldProp const* prop;
    
private:
    
    /// disabled default constructor
    Field();
    
    /// duplicate field
    real*    fiTMP;
    
    /// allocated size of fiTMP
    unsigned fiTMPSize;
    
    /// matrix for diffusion
    MatrixSparseSymmetric1 fiDiffusionMatrix;
    
    /// initialize to cover the given Space with squares of size 'step'
    void setGrid(Vector inf, Vector sup, real step, bool tight)
    {
        assert_true( step > REAL_EPSILON );
        // we add a safety border (in micro-meters)
        const real extra = tight ? 0 : 1;
        
        int size[3] = { 0, 0, 0 };
        // we use square cells:
        for ( int d = 0; d < DIM; ++d )
        {
            size[d] = (int)ceil( (sup[d]-inf[d]+extra) / step );
            real mid = 0.5 * ( inf[d] + sup[d] );
            inf[d] = mid - 0.5 * step * size[d];
            sup[d] = mid + 0.5 * step * size[d];
        }
        
        mGrid.setDimensions(inf, sup, size);
        
        //verify the cell size:
        for ( int d = 0; d < DIM; ++d )
        {
            real dif = fabs( step - mGrid.cellWidth(d) );
            if ( fabs(dif) > 1e-3 )
            {
                Cytosim::warn << "Field:step[" << d << "] is not as expected:\n";
                Cytosim::warn << "  field: " << mGrid.cellWidth(d) << "  prop: " << step << "\n";
            }
        }
    }
    
    /// allocate memory for the scalar field (setGrid() must be called before)
    void createCells()
    {
        assert_true( mGrid.hasDimensions() );
        // delete preexisting grid if necessary:
        mGrid.destroy();
        // create the grid using the calculated dimensions:
        mGrid.createCells();
        // set all values to zero (already done in the constructor of FieldValue)
        // mGrid.clear();
        // report dimensions:
        //mGrid.printSummary(std::clog, "Field");
    }
    
public:
#pragma mark -
    
    /// constructor
    Field(FieldProp const* p)
    {
        prop      = p;
        fiTMP     = nullptr;
        fiTMPSize = 0;
    }
    
    /// destructor
    ~Field()
    {
        free_real(fiTMP);
    }
    
    /// initialize with squares of size 'step'
    void setField()
    {
        assert_true( prop );
        
        if ( ! mGrid.hasCells() )
        {
            if ( !prop->confine_space_ptr )
                throw InvalidParameter("A space must be defined to set a field");
            
            Vector inf, sup;
            prop->confine_space_ptr->boundaries(inf, sup);
            
            if ( prop->periodic )
            {
                for ( int d = 0; d < DIM; ++d )
                    mGrid.setPeriodic(d, true);
                setGrid(inf, sup, prop->step, true);
            }
            else
            {
                setGrid(inf, sup, prop->step, false);
            }
            createCells();
            //std::clog << "Field step "<<prop->step<<" nCells "<<mGrid.nbCells()<<std::endl;
            Cytosim::log("Field %lx set with %i cells of size %.3f um\n", this, mGrid.nbCells(), prop->step);
        }
    }
    
    
    /// true if field was set
    size_t hasField() const { return mGrid.hasCells(); }
    
    /// size of cell
    real cellWidth() const { return mGrid.cellWidth(0); }
    
    /// volume of cell
    real cellVolume() const { return mGrid.cellVolume(); }
    
    /// access to data
    value_type& cell(const real w[]) const { return mGrid.cell(w); }
    
    /// access to data
    index_t nbCells() const { return mGrid.nbCells(); }

    /// info
    void infoValues(value_type& s, value_type& n, value_type& x) const { return mGrid.infoValues(s, n, x); }
    
    //------------------------------ simulation --------------------------------
#pragma mark -
    
    /// set all cells to value = volume * conc
    void setConcentration(FieldGrid::value_type conc)
    {
        mGrid.setValues( conc * mGrid.cellVolume() );
    }
    
    
    /// set cells that are inside `spc` to value = volume * conc
    void setConcentration(Space const* spc, FieldGrid::value_type in, FieldGrid::value_type ou)
    {
        real i = in * mGrid.cellVolume();
        real o = ou * mGrid.cellVolume();
        
        for ( FieldGrid::index_t c = 0; c < mGrid.nbCells(); ++c )
        {
            Vector w;
            mGrid.setPositionFromIndex(w, c, 0.5);
            if ( spc->inside(w) )
                mGrid.icell(c) = i;
            else
                mGrid.icell(c) = o;
        }
    }
    
    /// initialize Field
    void prepare();

    /// simulation step
    void step(FiberSet&);
    
    /// calculate second derivative of field
    void laplacian(const real*, real*) const;
    
    /// calculate second derivative of field
    void diffuseX(real*, real);
    
    /// set values of field on its edges
    void setEdgesX(real*, real);
    
    /// set values of field on its edges
    void setEdgesY(real*, real);
    
    /// set values of field on its edges
    void setEdgesZ(real*, real);
    
    /// initialize diffusion matrix (only for FieldScalar)
    void prepareDiffusion(real);
    
    /// initialize diffusion matrix (only for FieldScalar)
    void prepareDiffusion(real, unsigned char *);
    
    //------------------------------- object -----------------------------------
#pragma mark -
    
    /// a unique character identifying the class
    static const ObjectTag TAG = 'i';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return index of 'prop' in corresponding PropertyList
    Property const* property() const { return prop; }
    
    /// a static_cast<> of Node::next()
    Field* next()  const  { return static_cast<Field*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Field* prev()  const  { return static_cast<Field*>(nPrev); }
    
    //------------------------------ read/write --------------------------------
#pragma mark -
    
    /// print total, minimum and maximum value
    void   writeInfo(std::ostream& out) const
    {
        real vol = mGrid.cellVolume();
        FieldGrid::value_type sum, mn, mx;
        mGrid.infoValues(sum, mn, mx);
        out << prop->name() << " sum " << sum << " min " << mn/vol << " max " << mx/vol << std::endl;
    }
    
    /// write Field to file using VAL::write()
    /** Some of this should be moved to Grid */
    void   write(Outputter& out) const
    {
        if ( mGrid.hasCells() && prop->save )
        {
            out.writeUInt16(DIM);
            for ( int d = 0; d < DIM; ++d )
            {
                out.writeSoftSpace();
                out.writeUInt32(mGrid.breadth(d));
                out.writeFloat(mGrid.inf(d));
                out.writeFloat(mGrid.sup(d));
            }
            out.writeSoftSpace();
            out.writeUInt32(mGrid.nbCells());
            for ( FieldGrid::index_t c = 0; c < mGrid.nbCells(); ++c )
                mGrid.icell(c).write(out);
            out.writeSoftNewline();
        }
        
        if ( prop->positive )
        {
            if ( mGrid.hasNegativeValue() )
                throw Exception("Aborting because Field has negative values");
        }
    }
    
    
    /// read Field from file using VAL::read()
    void   readFieldData(Inputter& in, Simul&)
    {
        int  size[DIM] = { 0 };
        real minB[DIM] = { 0 }, maxB[DIM] = { 0 };
        
        unsigned int dim = in.readUInt16();
        if ( dim != DIM )
            throw InvalidIO("cannot read field due to dimensionality mismatch");
        
        for ( unsigned int d = 0; d < dim; ++d )
        {
            size[d] = in.readUInt32();
            minB[d] = in.readFloat();
            maxB[d] = in.readFloat();
        }
        
        mGrid.setDimensions(minB, maxB, size);
        createCells();
        
        FieldGrid::index_t nbc = in.readUInt32();
        if ( nbc != mGrid.nbCells() )
        {
            printf("file: %u field:%lu\n", nbc, mGrid.nbCells());
            throw InvalidIO("mismatch in Field::size");
        }
        //std::clog << "Field::read() nb_cells=" << nbc << std::endl;
        
        for ( FieldGrid::index_t c = 0; c < nbc; ++c )
            mGrid.icell(c).read(in);
    }
    
    /// read Field and checks that the Grid::step has not changed
    void   read(Inputter& in, Simul& sim, ObjectTag)
    {
        try
        {
            readFieldData(in, sim);
        }
        catch( Exception & e )
        {
            e << ", in Field::read()";
            throw;
        }
        
        if ( prop )
        {
            for ( unsigned int d = 0; d < DIM; ++d )
            {
                real dif = fabs( prop->step - mGrid.cellWidth(d) );
                if ( fabs(dif) > 1e-3 )
                {
                    Cytosim::warn << "Field:step["<<d<<"] has changed:\n";
                    Cytosim::warn << "  file: " << mGrid.cellWidth(d) << " prop: " << prop->step << "\n";
                }
            }
            
            /*
             we should extrapolate the data that were read to a grid with the
             resolution specified by prop->step
             */
        }
    }
    
    /// OpenGL display
    void draw() const;
    
    /// OpenGL display
    void draw(bool all, Vector3 const& dir, const real pos) const;
    
};


#endif
