// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SIMUL_PROP_H
#define SIMUL_PROP_H


#include "real.h"
#include "vector.h"
#include "property.h"

class SpaceProp;
class Simul;
class Space;


/**
 Enables capacity to simulate constant fluid flow that transports all objects
 Option normally OFF
 */
#define NEW_CYTOPLASMIC_FLOW 0

/**
 @defgroup Parameters All object parameters
 List of parameters for user-accessible objects.
 */


/// Property for Simul
/** 
 There is normally only one instantiation of this class.
 
 @ingroup Properties
 */
class SimulProp : public Property
{
    friend class Simul;
    
public:
    
    /**
     @defgroup Properties All Object Properties
     List of properties accessible to the user.
     */

    /**
     @defgroup SimulPar Parameters of Simul
     @ingroup Parameters
     @{
     */
    
    
    /// Current time in the simulated world
    real      time;

    /// A small interval of time
    /**
     The `time_step` is the amount of time between two consecutive simulation states.
     It controls the precision of the simulation, at the expense of computation.\n
     We expect that the numerical result will converge to the true mathematical solution
     of the equations when `time_step` becomes infinitely small, but we do not necessarily 
     know how fast this convergence will be. Thus it is not usually possible to 
     calculate the precision as a function of the `time_step`.\n
     To check that `time_step` is appropriate for a particular problem, one should 
     always run several simulations where `time_step` is varied systematically.
     
     Useful rules:
     - A smaller time step is always preferable, provided that the time to run the simulation
     remains acceptable,
     - You should always check that the results of two simulations with <em>time_step=h</em>\
     and <em>time_step=h/2</em> are identical. If this is not the case, then `h`\
     is most likely not an appropriate value for `time_step`,
     - If some reaction in the simulation occures with a rate R, then `time_step`\
     should be such that <em>time_step * R << 1</em>. In practice `time_step * R` should not be higher than 0.2.
     .
     */
    real      time_step;
    
    
    /// Ambient viscosity
    /**
     The viscosity of the medium should be given in units of pN.s/um^2 = Pa.s.
     <em>The default value is 1</em>
     
     Medium                | Viscosity | Reference                             |
     ----------------------|-----------|----------------------------------------
     Water                 |  0.001    | http://en.wikipedia.org/wiki/Viscosity
     C.elegans embryo      |  ~1       | Daniels et al. 10.1529/biophysj.105.080606  http://dx.doi.org/10.1529/biophysj.105.080606
     C.elegans embryo      |  ~0.5     | Garzon-Coral et al. 10.1126/science.aad9745  http://dx.doi.org/10.1126/science.aad9745
     S.pombe               |  ~1       | Tolic et al. PRL 93, 078102 (2004). http://dx.doi.org/10.1103/PhysRevLett.93.078102
     D.melanogaster embryo |  ~0.312   | Mechanical Aspects of Drosophila Gastrulation, Oleg Polyakov PhD Thesis. Princeton U., 9.2013
     Cleared egg cytoplasm |  ~0.02    | Valentine et al. Biophys J 88, 680–689 (2005). http://dx.doi.org/10.1529/biophysj.104.048025
     Cultivated cells      |  ~1       | Kole et al. Mol Bio Cell 15, 3475--84 (2004) http://dx.doi.org/10.1091/mbc.E04-03-0218
     
     Note that non-linear effects are not taken into account in Cytosim. Hydrodynamic effects are also neglected, such that the drag coefficient of a collection of objects is simply the sum of the individual drag coefficients. However, an `effective` viscosity can be set for each object class, and with this option, it is possible to adjust the drag coefficient of the collection to a realistic value.
     */
    real      viscosity;
    
#if NEW_CYTOPLASMIC_FLOW
    /// uniform and constant fluid flow
    Vector    flow;
#endif

    /// Energy of Brownian motion in the system = Temperature * Boltzman constant
    /**
     <em>kT</em> is the product of the [Boltzmann constant](http://en.wikipedia.org/wiki/Boltzmann_constant) `k`
     by the [Thermodynamic temperature](http://en.wikipedia.org/wiki/Thermodynamic_temperature) in Kelvin:
     - k = 1.38065 x 10^-23 Joule/Kelvin = 13.8065 x 10^-6 pN.um / Kelvin
     - Kelvin = Celsius + 273.15
     .
     
     Celsius   | Kelvin   |  kT            |
     ----------|----------|-----------------
     ~10 C     |  283 K   |  0.0039  pN.um
     ~24 C     |  297 K   |  0.0041  pN.um
     ~31 C     |  303 K   |  0.0042  pN.um
     ~39 C     |  312 K   |  0.0043  pN.um

     <em>default value = 0.0042</em>
     */
    real      kT;
    
    
    /// 32-bits seed for random number generator
    /**
     The simulation uses SFMT, a fast Mersenne Twister to generate pseudo-random numbers
     http://en.wikipedia.org/wiki/Mersenne_twister
     
     The generator is initialized from `random_seed` specified in the config file,
     but if `random_seed == 0`, it is set automatically during initialization.
    
     <em>default value = 0</em>
     */
    unsigned int random_seed;
    
    
    /// Desired precision in the motion of the objects
    /**
     The motion of the objects is solved with a residual error that is lower than `tolerance * B`, 
     where `B` is the typical Brownian displacement of the objects in one time step.\n
     <em>Thus one should set 0 < tolerance < 0.1</em>\n
     Lowering tolerance increases precision at the expense of CPU time.
     In the special case where 'kT==0', the maximum residual is simply `tolerance`.
     
     <em>default value = 0.05</em>
    */
    real      tolerance;
    
    
    /// Precision threshold for stochastic events
    /** 
     A warning message is issued for a rate K if:
     
         K * time_step > acceptable_prob
     
     In most implementations, a stochastic event (binding/unbinding) may only occur once
     during a time_step, and this becomes inaccurate if ( K * time_step is not small compared to 1 ).
     
     A user may control the `rate overflow' by adjusting `acceptable_prob` and monitoring the
     warning messages.
     
     <em>default value = 0.5</em>
     */
    real      acceptable_prob;
    
    
    /// A flag to enable preconditionning when solving the system of equations
    /**
     This parameter can affect the performance greatly, and it is always a good idea
     to try the different accepted values of `precondition`:
     - 0 : do not use preconditionning
     - 1 : use a block preconditionner
     .
     
     With `precondition = 1`, Cytosim calculates a matrix (the preconditionner)
     that is approximately equal to the inverse of the matrix that characterize the
     dynamical system. Using this preconditionner can reduce the number of iterations
     needed to converge to a solution, resulting in a potential overall speedup.
     However, calculating the preconditionner itself is costly, and performing an
     iteration with preconditionning is also more expensive than without.
     Hence there is a complex tradeoff, and the performance will vary.
     In some cases, using `precondition=1` can degrade preformance, 
     in particular if some objects have many vertices.
     
     If there is only one filament in the system, `precondition=0` should perform best.
     With many filaments, trying `precondition = [0, 1]' is the recommended strategy.
     <em>default value = 0</em>
     */
    int       precondition;

    
    /// A flag to control the engine that implement steric interactions between objects
    int       steric;
    
    /// Stiffness for repulsive steric interaction
    real      steric_stiffness_push[2];
    
    /// Stiffness for attractive steric interaction
    real      steric_stiffness_pull[2];
    
    /// Lattice size used to determine steric interactions
    /**
     Cytosim uses a divide-and-conquer approach to find pairs of objects that are 
     close enough to interact, based on a dividing the Space with a rectangular grid (see PointGrid).
     
     `steric_max_range` defines the minimum size of the cells in the grid.
     A finer grid reduces false positives, but increases the amount of memory occupied by the grid,
     and the number calculations that are necessary to maintain and clear the grid.
     
     Thus optimal performance is usually obtained for an intermediate value of `steric_max_range`.
     However `steric_max_range` must remain greater than the maximum interaction distance,
     otherwise some interacting pairs will be missed. 
     Experimentation is usually necessary to find the best value.
     
     The maximum distance at which an object may interact with a sibling is its diameter.
     Generally, `steric_max_range` should be greater or equal to the sum of the radiuses,
     of any two object that may interact.
     In the case of fiber, the `interaction-radius` is a combination of the segmentation,
     and the radius: sqrt( (4/3*segmentation)^2 + 4*radius^2 )

     If the parameter is not set, cytosim attempts to calculate `steric_max_range` automatically.
     */
    real      steric_max_range;
    
    
    /// Lattice size used to determine the attachment of Hand to Fiber
    /**
     Cytosim uses a divide-and-conquer approach to detect which Fibers are near a given point,
     witout testing every Fiber. This is necessary to determine onto which Fiber a Hand may bind.
     The algorithm is based on partitionning Space with a rectangular grid
     with cells of size `binding_grid_step` (see FiberGrid).

     `binding_grid_step` affects the execution speed of the algorithm, but not its result.
     Smaller values of binding_grid_step reduce the number of false positives, 
     but requires more memory and housekeeping calculations. 
     Memory requirements also increase with the physical dimensions of the system, 
     to the power DIM (the dimensionality, set at compilation time).
     */
    real      binding_grid_step;
    
    /// level of verbosity
    int           verbose;

    /// Name of configuration file (<em>default = config.cym</em>)
    std::string   config_file;
    
    /// Name of output property file (<em>default = properties.cmo</em>)
    std::string   property_file;
    
    /// Name of output trajectory file (also known as `trajectory`, <em>default = objects.cmo</em>)
    std::string   trajectory_file;
    
    /// If `true`, any pre-existing trajectory_file will be erased (<em>default = true</em>)
    bool          clear_trajectory;
    
    /// If `true` free couples are not saved/read to/from file (<em>default = false</em>)
    bool          skip_free_couple;
    
    /// Display parameters (see @ref DisplayPar)
    std::string   display;

    /// @}

    /// this is set to true when 'display' is modified, and to 'false' when it is read
    bool          display_fresh;

public:
    
    /// constructor
    SimulProp(const std::string& n) : Property(n) { clear(); }
    
    /// destructor
    ~SimulProp()  { }
    
    /// identifies the property
    std::string category() const { return "simul"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(Simul const&);

    /// return a carbon copy of object
    Property* clone() const { return new SimulProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;

};

#endif

