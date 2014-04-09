#include "recipe_mixer.h"

#define LG(X) LOG(cyclus::LEV_##X, "RecMix")

using cyclus::MatQuery;
using cyclus::Material;
using cyclus::CompMap;
using cyclus::Composition;
using cyclus::ResCast;
using cyclus::compmath::Normalize;

RecipeMixer::RecipeMixer(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    inbuf1_size_(0),
    inbuf2_size_(0),
    outbuf_size_(0),
    throughput_(0),
    inpolicy1_(this),
    inpolicy2_(this),
    outpolicy_(this) {}

void RecipeMixer::DoRegistration() {
  cyclus::Facility::DoRegistration();

  outpolicy_.Init(&outbuf_, outcommod_);
  inpolicy1_.Init(&inbuf1_, incommod1_, context()->GetRecipe(inrecipe1_));
  inpolicy2_.Init(&inbuf2_, incommod2_, context()->GetRecipe(inrecipe2_));

  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&inpolicy1_);
  context()->RegisterTrader(&inpolicy2_);
}

void RecipeMixer::Tick(int time) {
  double qty = std::min(throughput_, outbuf_.space());
  if (inbuf1_.count() == 0 || inbuf2_.count() == 0 || qty < cyclus::eps()) {
    return;
  }

  LG(INFO3) << "RecipeMixer id=" << id() << " is ticking";
  LG(INFO4) << "inbuf1 quantity = " << inbuf1_.quantity();
  LG(INFO4) << "inbuf2 quantity = " << inbuf2_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();

  // combine inbuf resources to single mats for querying
  std::vector<Material::Ptr> mats;
  mats = ResCast<Material>(inbuf1_.PopN(inbuf1_.count()));
  for (int i = 1; i < mats.size(); ++i) {
    mats[0]->Absorb(mats[i]);
  }
  Material::Ptr m1 = mats[0];

  mats = ResCast<Material>(inbuf2_.PopN(inbuf2_.count()));
  for (int i = 1; i < mats.size(); ++i) {
    mats[0]->Absorb(mats[i]);
  }
  Material::Ptr m2 = mats[0];

  // determine frac needed from each input stream
  double w1 = Weight(m1->comp());
  double w2 = Weight(m2->comp());
  double wtgt = Weight(context()->GetRecipe(outrecipe_));
  double frac2 = (wtgt - w1) / (w2 - w1);
  double frac1 = 1 - frac2;

  // deal with stream quantity and outbuf space constraints
  double ratio1 = frac1 * qty / m1->quantity();
  double ratio2 = frac2 * qty / m2->quantity();
  if (ratio1 <= 1 && ratio2 <= 1) {
    // not constrained by inbuf quantities
  } else if (ratio1 > ratio2) {
    // constrained by inbuf1
    qty = m1->quantity() / frac1;
  } else {
    // constrained by inbuf2
    qty = m2->quantity() / frac2;
  }

  Material::Ptr mix = m1->ExtractQty(frac1 * qty);
  mix->Absorb(m2->ExtractQty(frac2 * qty));

  outbuf_.Push(mix);
  inbuf1_.Push(m1);
  inbuf2_.Push(m2);
}

double RecipeMixer::Weight(Composition::Ptr c) {
  CompMap cm = c->mass();
  Normalize(&cm);

  double fiss_u238; // = xs_fiss(u238)...
  double absorb_u238; // = xs_absorb(u238)...
  double nu_u238; // = nu(u238)...
  double p_u238 = nu_u238 * fiss_u238 - absorb_u238;

  double fiss_pu239; // = xs_fiss(u238)...
  double absorb_pu239; // = xs_absorb(u238)...
  double nu_pu239; // = nu(u238)...
  double p_pu239 = nu_u238 * fiss_u238 - absorb_u238;

  CompMap::iterator it;
  double w = 0;
  for (it = cm.begin(); it != cm.end(); ++it) {
    double fiss; // = xs_fiss(cm->first)...
    double absorb; // = xs_absorb(cm->first)...
    double nu; // = nu(cm->first)...
    double p = nu * fiss - absorb;
    w += (p - p_u238) / (p_pu239 - p_u238);
  }
  return w;
}

extern "C" cyclus::Agent* ConstructRecipeMixer(cyclus::Context* ctx) {
  return new RecipeMixer(ctx);
}

