// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 return the maximum segmentation of all existing FiberProp,
 multiplied by 0.5
 */
real Simul::estimateFiberGridStep() const
{
    real res = 0.0;
    
    for ( Property * i : properties.find_all("fiber") )
    {
        FiberProp const* fp = static_cast<FiberProp*>(i);
        res = std::max(res, fp->segmentation);
    }
    
    return res * 0.5;
}


/**
 The FiberGrid is used to quickly find the fibers that are close to any point.
 Procedure:
 1. if binding_grid_step is not set, attempt to find a suitable value for it,
 2. if the number of cells is superior to 1e5, double the step size,
 2. initialize the grid with this calculated step size.
 */
void Simul::setFiberGrid(Space const* spc) const
{
    assert_true(spc);
    real step = prop->binding_grid_step;
    
    // try to find cell size from the filaments characteristics
    if ( step <= 0 )
        step = estimateFiberGridStep();

    /// otherwise, try to get it from the space
    if ( step <= 0 )
        step = spc->max_extension() / 128;
    
    assert_true( step > 0 );

    // increase the cell size until we get acceptable memory requirements:
    const size_t sup = 1 << 18;
    while ( fiberGrid.setGrid(spc, step) > sup )
    {
        //std::clog << "increasing simul:binding_grid_step\n";
        step *= 2;
    }
    prop->binding_grid_step = step;
    //std::clog << "simul:binding_grid_step = " << prop->binding_grid_step << "\n";

    // create the grid cells:
    fiberGrid.createCells();

    //Cytosim::log("simul:binding_grid_step %.3f\n", prop->binding_grid_step);
    Cytosim::log(" BindingGrid has %i cells of size %.3f um\n", fiberGrid.nbCells(), step);
    //std::clog << " BindingGrid with " << fiberGrid.nbCells() << " cells of size " << step << '\n';
}


/**
 Will pepare the simulation engine to make it ready to make a step():
 - set FiberGrid used for attachment of Hands,
 - set StericGrid
 - call complete() for all registered Property
 .
 The simulated objects should not be changed.
 
 */
void Simul::prepare()
{
    if ( !spaces.master() )
        throw InvalidSyntax("A space must be defined first!");

    // make sure properties are ready for simulations:
    sReady = true;
    prop->complete(*this);
    
    // prepare grid for attachments:
    setFiberGrid(spaces.master());
    
    // this is necessary for diffusion in Field:
    fields.prepare();
    
    // this prepares for 'fast_diffusion':
    singles.prepare(properties);
    couples.prepare(properties);
}


/**
 This is the master Monte-Carlo step function.
 
 Lists are mixed such that objects are considered in a different
 and random order at each step, to avoid biais in the simulation

 step() is called for every list, i.e. for every Object
 */
void Simul::step()
{
    // increment time:
    prop->time += prop->time_step;
    //printf("\n------ time is %8.3f\n", prop->time);

    // mix object lists
    events.shuffle();
    organizers.shuffle();
    beads.shuffle();
    solids.shuffle();
    fibers.shuffle();
    spheres.shuffle();
    couples.shuffle();
    singles.shuffle();
    spaces.shuffle();
    
    // Monte-Carlo step for all objects
    events.step();
    organizers.step();
    fields.step();
    spaces.step();
    spheres.step();
    beads.step();
    solids.step();
    fibers.step();
    
    // calculate grid range from Hand's binding range:
    real range = 0.0;
    for ( Property * i : properties.find_all("hand") )
        range = std::max(range, static_cast<HandProp const*>(i)->binding_range);

    // distribute Fibers over a grid for binding of Hands:
    fiberGrid.paintGrid(fibers.first(), nullptr, range);
    
#if ( 0 )
    
    // This code continuously tests the binding algorithm.
    
    if ( fiberGrid.hasGrid() )
    {
        HandProp hp("test_binding");
        hp.binding_rate  = INFINITY;
        hp.binding_range = RNG.preal() * range;
        hp.bind_also_end = BOTH_ENDS;
        hp.complete(*this);
        
        Space const* spc = spaces.master();
        for ( unsigned i = 0; i < 16; ++i )
        {
            Vector pos = spc->randomPlace();
            fiberGrid.testAttach(stdout, pos, fibers, &hp);
        }
    }
    
#endif
    
    // step Hand-containing objects, giving them a possibility to attach Fibers:
    couples.step();
    singles.step();
    
    //printf("     ::attach   %16llu\n", (__rdtsc()-rdtsc)>>3);
}


void Simul::relax()
{
    singles.relax();
    couples.relax();
    sReady = false;
}


void Simul::drawLinks() const
{
    sMeca.drawLinks = true;
    if ( !sReady )
        prop->complete(*this);
    sMeca.prepare(this);
    setAllInteractions(sMeca);
    sMeca.drawLinks = false;
}
