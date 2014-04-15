#include "recipe_mixer.h"
#include "fuel_match.h"

#define LG(X) LOG(cyclus::LEV_##X, "RecMix")

using cyclus::Material;
using cyclus::Composition;
using cyclus::ResCast;

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
  LG(INFO3) << "RecipeMixer id=" << id() << " is ticking";
  LG(INFO4) << "inbuf1 quantity = " << inbuf1_.quantity();
  LG(INFO4) << "inbuf2 quantity = " << inbuf2_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();
  double qty = std::min(throughput_, outbuf_.space());
  if (inbuf1_.count() == 0 || inbuf2_.count() == 0 || qty < cyclus::eps()) {
    return;
  }

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
  Composition::Ptr tgt = context()->GetRecipe(outrecipe_);
  double frac2 = CosiFissileFrac(tgt, m1->comp(), m2->comp());
  double frac1 = 1 - frac2;

  LG(INFO4) << "filler frac = " << frac1;
  LG(INFO4) << "fissile frac = " << frac2;

  // deal with stream quantity and outbuf space constraints
  double ratio1 = frac1 * qty / m1->quantity();
  double ratio2 = frac2 * qty / m2->quantity();
  if (ratio1 <= 1 && ratio2 <= 1) {
    // not constrained by inbuf quantities
  } else if (ratio1 > ratio2) {
    // constrained by inbuf1
    LG(INFO5) << "Constrained by incommod '" << incommod1_
              << "' - reducing qty from " << qty
              << " to " << m1->quantity() / frac1;
    qty = m1->quantity() / frac1;
  } else {
    // constrained by inbuf2
    LG(INFO5) << "Constrained by incommod '" << incommod2_
              << "' - reducing qty from " << qty
              << " to " << m2->quantity() / frac2;
    qty = m2->quantity() / frac2;
  }

  Material::Ptr mix = m1->ExtractQty(frac1 * qty);
  mix->Absorb(m2->ExtractQty(frac2 * qty));

  cyclus::MatQuery mq(mix);
  LG(INFO4) << "Mixed " << mix->quantity() << " kg to recipe";
  LG(INFO5) << " u238 = " << mq.mass_frac(922380000);
  LG(INFO5) << " u235 = " << mq.mass_frac(922350000);
  LG(INFO5) << "Pu239 = " << mq.mass_frac(942390000);

  outbuf_.Push(mix);
  if (m1->quantity() > 0) {
    inbuf1_.Push(m1);
  }
  if (m2->quantity() > 0) {
    inbuf2_.Push(m2);
  }
}

extern "C" cyclus::Agent* ConstructRecipeMixer(cyclus::Context* ctx) {
  return new RecipeMixer(ctx);
}



