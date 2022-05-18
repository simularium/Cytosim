// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DYNAMIC_FIBER_H
#define DYNAMIC_FIBER_H

#include "sim.h"
#include "vector.h"
#include "node_list.h"
#include "fiber.h"

class DynamicFiberProp;


/// A Fiber with discrete growth and dynamic instability at the PLUS_END
/**

 This implements the microtubule dynamic instability model proposed by
 Brun, Rupp et al. with a 'hard-coded' coupling parameter N=2.
 
 Assembly and disassembly follow discrete steps of size `prop->unit_length`.
 The model keeps track of the discrete state of the two terminal units of tubulin.
 This leads to 4 different states, which are mapped to [STATE_GREEN, STATE_RED].
 
 
 The growth speed is reduced under antagonistic force by an exponential factor:\n
 <em>
 <b>Measurement of the Force-Velocity Relation for Growing Microtubules</b>\n
 Marileen Dogterom and Bernard Yurke\n
 Science Vol 278 pp 856-860; 1997\n
 http://www.sciencemag.org/content/278/5339/856.abstract
 </em>
 
 ...and this increases the catastrophe rate:\n
 <em>
 <b>Dynamic instability of MTs is regulated by force</b>\n
 M.Janson, M. de Dood, M. Dogterom.\n
 Journal of Cell Biology Vol 161, Nb 6, 2003\n
 Figure 2 C\n
 </em>
 http://www.jcb.org/cgi/doi/10.1083/jcb.200301147
 
 
 If you use this model, please cite:\n
 <em>
 <b>A theory of microtubule catastrophes and their regulation</b>\n
 Brun L, Rupp B, Ward J, Nedelec F\n
 PNAS 106 (50) 21173-21178; 2009\n
 http://www.pnas.org/content/106/50/21173
 </em>

 The predicted mean time until catastrophe is approximately

     growing_rate = growing_speed / unit_length
     real ctime = growing_rate / ( 3 * hydrolysis_rate * hydrolysis_rate );
 
 The implemented model includes off-rate in the assembly state, as described in:\n
 <em>
 <b>Random Hydrolysis Controls the Dynamic Instability of Microtubules</b>\n
 Ranjith Padinhateeri, Anatoly B Kolomeisky, and David Lacoste\n
 Biophys J 102, 1274–1283 (2012)\n
 http://dx.doi.org/10.1016/j.bpj.2011.12.059
 </em>
 
 

 This is not implemented:
 - the MINUS_END is not dynamic,
 - rescues are not included.
 .
 
 See the @ref DynamicFiberPar.

 // @todo DynamicFiber detach_rate should depend on the state of the subunit
 // @todo DynamicFiber should keep the entire state vector of the subunits

 Note: A Gillespie simulation method is used.
 This class is not fully tested (17. Feb 2011).
 @ingroup FiberGroup
 */
class DynamicFiber : public Fiber
{
private:
    
    /// assembly during last time-step
    real       mGrowthP;
    real       mGrowthM;
    
    /// Gillespie countdown timers for PLUS_END:
    real       nextGrowthP;
    real       nextHydrolP;
    real       nextShrinkP;
    
    /// Gillespie countdown timers for MINUS_END:
    real       nextGrowthM;
    real       nextHydrolM;
    real       nextShrinkM;
    
    /// state of units near the PLUS_END: [0] is terminal, [1] is penultimate unit
    unsigned   unitP[3];
    
    /// dynamic state of PLUS_END
    state_t    mStateP;
    
    /// state of units near the MINUS_END
    unsigned   unitM[3];
    
    /// dynamic state of MINUS_END
    state_t    mStateM;

    /// calculate dynamic state from unit states near PLUS_END
    state_t    calculateStateP() const;
    
    /// calculate dynamic state from unit states near MINUS_END
    state_t    calculateStateM() const;
   
public:
    
    /// the Property of this object
    DynamicFiberProp const* prop;
  
    /// constructor
    DynamicFiber(DynamicFiberProp const*);

    /// destructor
    virtual ~DynamicFiber();
        
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of MINUS_END
    state_t     dynamicStateM() const;
    
    /// return assembly/disassembly state of PLUS_END
    state_t     dynamicStateP() const;
    
    /// change state of MINUS_END
    void        setDynamicStateM(state_t s);
    
    /// change state of PLUS_END
    void        setDynamicStateP(state_t s);
    
    /// length increment at MINUS_END during last time-step
    real        freshAssemblyM() const { return mGrowthM; }
    
    /// length increment at PLUS_END during last time-step
    real        freshAssemblyP() const { return mGrowthP; }

    //--------------------------------------------------------------------------
    
    /// simulate dynamic instability of PLUS_END
    int         stepPlusEnd();
    
    /// simulate dynamic instability of MINUS_END
    int         stepMinusEnd();
    
    /// monte-carlo step
    void        step();
    
    //--------------------------------------------------------------------------
    
    /// write to Outputter
    void        write(Outputter&) const;

    /// read from Inputter
    void        read(Inputter&, Simul&, ObjectTag);
    
};


#endif
