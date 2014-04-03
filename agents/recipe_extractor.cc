#include "recipe_extractor.h"

using cyclus::MatQuery;
using cyclus::Material;
using cyclus::Composition;
using cyclus::ResCast;

RecipeExtractor::RecipeExtractor(cyc::Context* ctx)
    : cyc::Facility(ctx),
      inbuf_size_(0),
      outbuf_size_(0),
      wastebuf_size_(0),
      throughput_(0),
      inpolicy_(this),
      outpolicy_(this),
      wastepolicy_(this) {}

void RecipeExtractor::DoRegistration() {
  outpolicy_.Init(&outbuf_, outcommod_);
  wastepolicy_.Init(&wastebuf_, wastecommod_);
  inpolicy_.Init(&inbuf_, incommod_, context()->GetRecipe(inrecipe_));

  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&wastepolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void RecipeExtractor::Tick(int time) {
  if (inbuf_.count() == 0) {
    return;
  }

  std::vector<Material::Ptr> mats;
  mats = ResCast<Material>(inbuf_.PopN(inbuf_.count()));
  for (int i = 1; i < mats.size(); ++i) {
    mats[0]->Absorb(mats[i]);
  }
  Material::Ptr m = mats[0];
  MatQuery mq(m);

  Composition::Ptr recipe = context()->GetRecipe(outrecipe_);
  double qty = mq.Amount(recipe);
  Material::Ptr keep = m->ExtractComp(qty, recipe);

  if (m->quantity() < wastebuf_.space()) {
    wastebuf_.Push(m);
  } else {
    wastebuf_.Push(m->ExtractQty(wastebuf_.space()));
    inbuf_.Push(m);
  }

  qty = keep->quantity();
  if (qty < throughput_ && qty < outbuf_.space()) {
    outbuf_.Push(keep);
  } else {
    qty = std::min(throughput_, outbuf_.space());
    outbuf_.Push(keep->ExtractQty(qty));
    inbuf_.Push(keep);
  }
}

extern "C" cyc::Agent* ConstructRecipeExtractor(cyc::Context* ctx) {
  return new RecipeExtractor(ctx);
}
