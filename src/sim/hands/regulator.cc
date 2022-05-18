// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "regulator.h"
#include "regulator_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


Regulator::Regulator(RegulatorProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
    throw InvalidParameter("the regulator class in unfinished");
}


    
void Regulator::attach(FiberSite const& s)
{
    Hand::attach(s);
    // freeze the PLUS_END:
    fiber()->setDynamicStateP(STATE_WHITE);
}


void Regulator::stepUnloaded()
{
    assert_true( attached() );
    
    testDetachment();
}


void Regulator::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    testKramersDetachment(force_norm);
}

