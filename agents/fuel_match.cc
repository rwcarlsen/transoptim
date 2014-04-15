
#include "fuel_match.h"
#include "data.h"

using pyne::simple_xs;

double CosiWeight(cyclus::Composition::Ptr c) {
  cyclus::CompMap cm = c->mass();
  cyclus::compmath::Normalize(&cm);

  double fiss_u238 = simple_xs("u238", "fission", "thermal");
  double absorb_u238 = simple_xs("u238", "absorption", "thermal");
  double nu_u238 = 0;
  double p_u238 = nu_u238 * fiss_u238 - absorb_u238;

  double fiss_pu239 = simple_xs("Pu239", "fission", "thermal");
  double absorb_pu239 = simple_xs("Pu239", "absorption", "thermal");
  double nu_pu239 = 2.85;
  double p_pu239 = nu_pu239 * fiss_pu239 - absorb_pu239;

  cyclus::CompMap::iterator it;
  double w = 0;
  for (it = cm.begin(); it != cm.end(); ++it) {
    cyclus::Nuc nuc = it->first;
    double nu = 0;
    if (nuc == 922350000) {
      nu = 2.4;
    } else if (nuc == 922330000) {
      nu = 2.5;
    } else if (nuc == 942390000) {
      nu = 2.85;
    }
    double fiss = simple_xs(nuc, "fission", "thermal");
    double absorb = simple_xs(nuc, "absorption", "thermal");
    double p = nu * fiss - absorb;
    w += it->second * (p - p_u238) / (p_pu239 - p_u238);
  }
  return w;
}

double CosiFissileFrac(cyclus::Composition::Ptr target,
                       cyclus::Composition::Ptr filler,
                       cyclus::Composition::Ptr fissile) {
  double w_fill = CosiWeight(filler);
  double w_fiss = CosiWeight(fissile);
  double w_tgt = CosiWeight(target);
  return (w_tgt - w_fill) / (w_fiss - w_fill);
}

