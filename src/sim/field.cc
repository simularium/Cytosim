// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 18/12/07.

#include "field.h"
#include "fiber_site.h"
#include "fiber_set.h"
#include "cblas.h"
#include "sim.h"


/**
 Initialize the diffusion matrix using periodic boundary conditions
 if the underlying space is peridic
 */
void Field::prepareDiffusion(real theta)
{
    const FieldGrid::index_t nbc = mGrid.nbCells();
    
    fiDiffusionMatrix.resize(nbc);
    fiDiffusionMatrix.reset();
    
    for ( FieldGrid::index_t c = 0; c < nbc; ++c )
    {
        for ( int d = 0; d < DIM; ++d )
        {
            FieldGrid::index_t n = mGrid.next(c, d);
                
            if ( n != c )
            {
                fiDiffusionMatrix(c, n) += theta;
                fiDiffusionMatrix(c, c) -= theta;
                fiDiffusionMatrix(n, n) -= theta;
            }
        }
    }
    fiDiffusionMatrix.prepareForMultiply(1);
    
    /*
    std::clog << "tight Field has diffusion matrix with ";
    std::clog << fiDiffusionMatrix.nbElements() << " elements" << std::endl;
     */
}


/**
 Initialize the diffusion matrix.
 Diffusion is allowed between neighboring cells that are in the same domain:

     ( domain[c] > 0 ) && ( domain[c] == domain[n] )

 */
void Field::prepareDiffusion(real theta, unsigned char * domain)
{
    const FieldGrid::index_t nbc = mGrid.nbCells();
    
    fiDiffusionMatrix.resize(nbc);
    fiDiffusionMatrix.reset();
    
    for ( FieldGrid::index_t c = 0; c < nbc; ++c )
    {
        if ( domain[c] )
        {
            for ( int d = 0; d < DIM; ++d )
            {
                FieldGrid::index_t n = c + mGrid.stride(d);
                
                if ( n < nbc  &&  domain[c] == domain[n] )
                {
                    fiDiffusionMatrix(c, n) += theta;
                    fiDiffusionMatrix(c, c) -= theta;
                    fiDiffusionMatrix(n, n) -= theta;
                }
            }
        }
    }
    fiDiffusionMatrix.prepareForMultiply(1);
    
    /*
     std::clog << "Field has diffusion matrix with ";
     std::clog << fiDiffusionMatrix.nbElements() << " elements" << std::endl;
     */
}


/**
 Initialize Field to be ready for step()
 */
void Field::prepare()
{
    Space const* spc = prop->confine_space_ptr;

    if ( !spc )
        throw InvalidParameter("A Space must be created before the field");

    const FieldGrid::index_t nbc = mGrid.nbCells();
    assert_true( nbc > 0 );
    
    free_real(fiTMP);
    fiTMP = new_real(nbc);
    fiTMPSize = nbc;

    if ( prop->diffusion > 0 )
    {
        real theta = prop->diffusion * prop->time_step / ( prop->step * prop->step );

        if ( DIM == 1 || prop->periodic )
            prepareDiffusion(theta);
        else
        {
            unsigned char * domain = new unsigned char[nbc];
            
            // determine which cell is inside the space:
#if ( 1 )
            for ( FieldGrid::index_t c = 0; c < nbc; ++c )
            {
                Vector pos;
                mGrid.setPositionFromIndex(pos, c, 0.5);
                domain[c] = spc->inside(pos);
            }
#else
            // extended covered area:
            const real range = 2 * cellWidth();
            for ( FieldGrid::index_t c = 0; c < nbc; ++c )
            {
                Vector pos;
                mGrid.setPositionFromIndex(pos, c, 0.5);
                domain[c] = ! spc->allOutside(pos, range);
            }
#endif
            
            prepareDiffusion(theta, domain);
            
            delete[] domain;
        }
    }
}


//------------------------------------------------------------------------------
#pragma mark - Diffusion


void Field::diffuseX(real * field, real c)
{
    const auto nbc = mGrid.nbCells();

    FieldGrid::index_t nx = mGrid.breadth(0);
    FieldGrid::index_t nyz = nbc / nx;
    FieldGrid::index_t ide = mGrid.stride(1);
    
    real * a = new_real(nyz);
    real * b = new_real(nyz);
    
    // diffusion in X-direction:
    zero_real(nyz, a);
    for ( FieldGrid::index_t x = 1; x < nx; ++x )
    {
        real * h = field + x - 1;
        real * n = field + x;
        // b = n - h
        blas::xcopy(nyz,  n, ide, b, 1);
        blas::xaxpy(nyz, -1, h, ide, b, 1);
        // a = a - b
        blas::xaxpy(nyz, -1, b, 1, a, 1);
        // h = h - c * a
        blas::xaxpy(nyz, -c, a, 1, h, ide);
        // swap a and b
        real * t = a;
        a = b;
        b = t;
    }
    real * h = field + nx - 1;
    blas::xaxpy(nyz, -c, a, 1, h, ide);
    
    if ( prop->periodic )
    {
        real * n = field;
        blas::xcopy(nyz,  n, ide, b, 1);
        blas::xaxpy(nyz, -1, h, ide, b, 1);
        blas::xaxpy(nyz,  c, b, 1, h, ide);
        blas::xaxpy(nyz, -c, b, 1, n, ide);
    }
    
    free_real(a);
    free_real(b);
}


void Field::laplacian(const real* field, real * mat) const
{
    const FieldGrid::index_t nbc = mGrid.nbCells();
    const real sc = 2 * DIM;

    for ( FieldGrid::index_t c = 0; c < nbc; ++c )
        mat[c] = sc * field[c];
    
    const FieldGrid::index_t nx = mGrid.breadth(0);
#if ( 1 )
    // derivative in the X-direction:
    const FieldGrid::index_t nyz = nbc / nx;
    for ( FieldGrid::index_t xx = 1; xx < nx; ++xx )
    {
        blas::xaxpy(nyz, -1, field+xx-1, nx, mat+xx  , nx);
        blas::xaxpy(nyz, -1, field+xx  , nx, mat+xx-1, nx);
    }
    // index of last valid X index:
    int xx = mGrid.breadth(0) - 1;
    
    if ( prop->periodic )
    {
        blas::xaxpy(nyz, -1, field+xx, nx, mat   , nx);
        blas::xaxpy(nyz, -1, field   , nx, mat+xx, nx);
    }
    else
    {
        blas::xaxpy(nyz, -1, field   , nx, mat   , nx);
        blas::xaxpy(nyz, -1, field+xx, nx, mat+xx, nx);
    }
#endif
    
#if ( DIM == 2 )
    // derivative in the Y-direction:
    blas::xaxpy(nbc-nx, -1, field,    1, mat+nx, 1);
    blas::xaxpy(nbc-nx, -1, field+nx, 1, mat,    1);
    
    FieldGrid::index_t yy = mGrid.breadth(1) - 1;
    if ( prop->periodic )
    {
        blas::xaxpy(nx, -1, field+nx*yy, 1, mat      , 1);
        blas::xaxpy(nx, -1, field      , 1, mat+nx*yy, 1);
    }
    else
    {
        blas::xaxpy(nx, -1, field      , 1, mat      , 1);
        blas::xaxpy(nx, -1, field+nx*yy, 1, mat+nx*yy, 1);
    }
#endif

#if ( DIM == 3 )
    // derivative in the Y-direction:
    const FieldGrid::index_t ss = mGrid.stride(2);
    for ( FieldGrid::index_t yy = 1; yy < mGrid.breadth(1); ++yy )
    for ( FieldGrid::index_t zz = 0; zz < mGrid.breadth(2); ++zz )
    {
        blas::xaxpy(nx, -1, field+nx*(yy-1)+ss*zz, 1, mat+nx*(yy  )+ss*zz, 1);
        blas::xaxpy(nx, -1, field+nx*(yy  )+ss*zz, 1, mat+nx*(yy-1)+ss*zz, 1);
    }
    int yy = mGrid.breadth(1) - 1;
    
    if ( prop->periodic )
    {
        for ( FieldGrid::index_t zz = 0; zz < mGrid.breadth(2); ++zz )
        {
            blas::xaxpy(nx, -1, field+nx*yy+ss*zz, 1, mat      +ss*zz, 1);
            blas::xaxpy(nx, -1, field      +ss*zz, 1, mat+nx*yy+ss*zz, 1);
        }
    }
    else
    {
        for ( FieldGrid::index_t zz = 0; zz < mGrid.breadth(2); ++zz )
        {
            blas::xaxpy(nx, -1, field      +ss*zz, 1, mat      +ss*zz, 1);
            blas::xaxpy(nx, -1, field+nx*yy+ss*zz, 1, mat+nx*yy+ss*zz, 1);
        }
    }
#endif

#if ( DIM == 3 )
    // derivative in the Z-direction:
    const FieldGrid::index_t nxy = nbc / mGrid.breadth(2);
    assert_true( nxy == ss );
    blas::xaxpy(nbc-nxy, -1, field,     1, mat+nxy, 1);
    blas::xaxpy(nbc-nxy, -1, field+nxy, 1, mat,     1);
    FieldGrid::index_t zz = mGrid.breadth(2) - 1;
    
    if ( prop->periodic )
    {
        blas::xaxpy(nxy, -1, field+ss*zz, 1, mat      , 1);
        blas::xaxpy(nxy, -1, field      , 1, mat+ss*zz, 1);
    }
    else
    {
        blas::xaxpy(nxy, -1, field      , 1, mat      , 1);
        blas::xaxpy(nxy, -1, field+ss*zz, 1, mat+ss*zz, 1);
    }
#endif
}


void Field::setEdgesX(real * field, real val)
{
    const FieldGrid::index_t nbc = mGrid.nbCells();
    
    // set X-edges:
    const FieldGrid::index_t nx = mGrid.breadth(0);
    
    real * lastf = field + nx - 1;
    for ( FieldGrid::index_t xx = 0; xx < nbc; xx += nx )
    {
        field[xx] = val;
        lastf[xx] = val;
    }
}


void Field::setEdgesY(real * field, real val)
{
#if ( DIM > 1 )
    const FieldGrid::index_t nbc = mGrid.nbCells();
    const FieldGrid::index_t nx = mGrid.breadth(0);
#endif
    
#if ( DIM == 2 )
    // set Y-edges:
    real * lastf = field + nbc - nx;
    for ( FieldGrid::index_t xx = 0; xx < nx; ++xx )
    {
        field[xx] = val;
        lastf[xx] = val;
    }
#endif
    
#if ( DIM == 3 )
    // set Y-edges:
    const FieldGrid::index_t nz = mGrid.breadth(2);
    const FieldGrid::index_t nxy = nbc / nz;
    
    real * lastf = field + nxy - nx;
    for ( FieldGrid::index_t zz = 0; zz < nz; ++zz )
    {
        for ( FieldGrid::index_t xx = 0; xx < nx; ++xx )
        {
            field[xx+zz*nxy] = val;
            lastf[xx+zz*nxy] = val;
        }
    }
#endif
}


void Field::setEdgesZ(real * field, real val)
{
#if ( DIM == 3 )
    const FieldGrid::index_t nbc = mGrid.nbCells();
    
    const FieldGrid::index_t nz = mGrid.breadth(2);
    const FieldGrid::index_t nxy = nbc / nz;
    
    real * lastf = field + nxy * ( nz - 1 );
    for ( FieldGrid::index_t xy = 0; xy < nxy; ++xy )
    {
        field[xy] = val;
        lastf[xy] = val;
    }
#endif
}


/**
 //\todo implement Crank-Nicholson for diffusion
 */
void Field::step(FiberSet& fibers)
{
    assert_true( prop );
    
    // we cast FieldScalar to floating-point type :
    assert_true( sizeof(FieldScalar) == sizeof(real) );
    real * field = reinterpret_cast<real*>(mGrid.data());
    const auto nbc = mGrid.nbCells();
    
    real * dup = fiTMP;

    // decay:
    if ( prop->decay_rate > 0 )
    {
        // field = field * exp( - decay_rate * dt ):
        blas::xscal(nbc, prop->decay_frac, field, 1);
    }

    // full grid diffusion:
    if ( prop->full_diffusion > 0 )
    {
        real c = prop->full_diffusion * prop->time_step / ( prop->step * prop->step );

#if ( DIM > 1 )
        laplacian(field, dup);
        blas::xaxpy(nbc, -c, dup, 1, field, 1);
#else
        diffuseX(field, c);
#endif
    }

    // diffusion:
    if ( prop->diffusion > 0 )
    {
        assert_true( fiTMP );
        assert_true( fiTMPSize == nbc );
        assert_true( fiDiffusionMatrix.size() == nbc );

        // dup = field:
        blas::xcopy(nbc, field, 1, dup, 1);
        
        // field = field + fiDiffusionMatrix * dup:
        fiDiffusionMatrix.vecMulAdd(dup, field);
    }

    if ( prop->boundary_condition & 1 )
        setEdgesX(field, prop->boundary_value * cellVolume());
    
#if ( DIM > 1 )
    if ( prop->boundary_condition & 2 )
        setEdgesY(field, prop->boundary_value * cellVolume());
#endif
    
#if ( DIM == 3 )
    if ( prop->boundary_condition & 4 )
        setEdgesZ(field, prop->boundary_value * cellVolume());
#endif
    
    
#if ( 0 ) // disabled features below

    Array<FiberSite> loc(1024);
    
    // instantaneous transport along Fibers
    if ( prop->transport_strength > 0 )
    {
        const real spread = 0.5 * cellWidth();
        const real rate = prop->transport_strength * spread / cellVolume();
        const real frac = -std::expm1( -rate * prop->time_step );
        
        if ( frac >= 0.5 )
            throw InvalidParameter("field:transport_strength is too high");
        
        fibers.uniFiberSites(loc, spread);
        for ( FiberSite & i : loc )
        {
            // abscissa for exit point of transport:
            real abs = i.abscissa() + RNG.exponential(prop->transport_length);

            // find index of cell:
            FieldGrid::value_type cell = mGrid.cell(i.pos());
            
            // amount to be transfered:
            real mass = cell * frac;
            
            // transport:
            cell -= mass;
            field[mGrid.index(i.fiber()->pos(abs))] += mass;
        }
    }
    
    // direct cutting of fiber
    // this is deprecated in favor of fiber:lattice_cut_fiber
    if ( prop->cut_fibers )
    {
        LOG_ONCE("!!!! Field severs fibers\n");
        const real spread = 0.5 / prop->time_step;
        const real fac = spread * prop->time_step / cellVolume();
        
        fibers.uniFiberSites(loc, spread);
        for ( FiberSite & i : loc )
        {
            real val = field[mGrid.index(i.pos())];
            if ( prop->cut_fibers == 2 )
                val = val * val / cellVolume();
            if ( RNG.test_not( exp(-fac*val) ) )
                i.fiber()->sever(i.abscissa(), STATE_RED, STATE_GREEN);
        }
    }
    
    if ( prop->chew_fibers )
    {
        LOG_ONCE("!!!! Field chews PLUS_END\n");
        const real fac = -prop->time_step / cellVolume();
        for ( Fiber * fib = fibers.first(); fib ; fib = fib->next() )
            fib->growP(fac*cell(fib->posEndP()));
    }
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Display

#ifdef DISPLAY
    
    class FieldDisplayParameters
    {
    public:
        FieldDisplayParameters()
        {
            amp = 0;
            spc = nullptr;
        }
        
        /// amplification for color
        real amp;
        
        /// Space for cropping
        Space const* spc;
    };
    
    
    static bool field_set_color(void* arg, FieldGrid::value_type const& val, Vector const& pos)
    {
        FieldDisplayParameters * fdp = static_cast<FieldDisplayParameters*>(arg);
        if ( fdp->spc && ! fdp->spc->inside(pos) )
            return false;
        val.setColor(fdp->amp);
        return true;
    }
    
    
    /// openGL display function
    void Field::draw() const
    {
        FieldDisplayParameters fdp;
        fdp.amp = 1.0 / ( prop->display_scale * mGrid.cellVolume() );
        fdp.spc = nullptr;
        
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        drawValues(mGrid, field_set_color, &fdp);
        if ( 0 )
        {
            glColor4f(1, 0, 1, 1);
            glLineWidth(0.5);
            drawEdges(mGrid);
        }
        glPopAttrib();
    }
    
    
    /// openGL display function
    /**
     display all cells that are inside field:confine_space
     */
    void Field::draw(bool all, Vector3 const& dir, const real pos) const
    {
        FieldDisplayParameters fdp;
        fdp.amp = 1.0 / ( prop->display_scale * mGrid.cellVolume() );
        if ( all )
            fdp.spc = nullptr;
        else
            fdp.spc = prop->confine_space_ptr;
        
        //glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glLineWidth(1);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#if ( DIM >= 3 )
        drawValues(mGrid, field_set_color, &fdp, dir, pos);
#else
        drawValues(mGrid, field_set_color, &fdp);
#endif
        glPopAttrib();
    }

#else

void Field::draw() const
{
    LOG_ONCE("no field:draw()\n");
}

void Field::draw(bool all, Vector3 const& dir, const real pos) const
{
    LOG_ONCE("no field:draw()\n");
}

#endif

