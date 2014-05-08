#include "recipe_extractor.h"

#define LG(X) LOG(cyclus::LEV_##X, "RecXtr")

using cyclus::MatQuery;
using cyclus::Material;
using cyclus::Composition;
using cyclus::ResCast;

RecipeExtractor::RecipeExtractor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      inbuf_size_(0),
      outbuf_size_(0),
      wastebuf_size_(0),
      inpref_(0),
      throughput_(0),
      inpolicy_(this),
      outpolicy_(this),
      wastepolicy_(this) {}

void RecipeExtractor::EnterNotify() {
  cyclus::Facility::EnterNotify();

  outpolicy_.Init(&outbuf_, outcommod_);
  wastepolicy_.Init(&wastebuf_, wastecommod_);
  inpolicy_.Init(&inbuf_, incommod_, context()->GetRecipe(inrecipe_), inpref_);

  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&wastepolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void RecipeExtractor::Decommission() {
  context()->UnregisterTrader(&outpolicy_);
  context()->UnregisterTrader(&wastepolicy_);
  context()->UnregisterTrader(&inpolicy_);
  cyclus::Facility::Decommission();
}

void RecipeExtractor::Tick(int time) {
  if (inbuf_.count() == 0) {
    return;
  }

  LG(INFO3) << "RecipeExtractor id=" << id() << " is ticking";
  LG(INFO4) << "inbuf quantity = " << inbuf_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();
  LG(INFO4) << "wastebuf quantity = " << wastebuf_.quantity();

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

extern "C" cyclus::Agent* ConstructRecipeExtractor(cyclus::Context* ctx) {
  return new RecipeExtractor(ctx);
}
