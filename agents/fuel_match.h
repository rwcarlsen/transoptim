
#include "cyclus.h"

double CosiWeight(cyclus::Composition::Ptr c);

// Returns the mass fraction of the fissile composition that should be mixed
// with "1 - frac" of the filler composition. In order to hit the target
// as close as possible.
double CosiFissileFrac(cyclus::Composition::Ptr target,
                       cyclus::Composition::Ptr filler,
                       cyclus::Composition::Ptr fissile);


