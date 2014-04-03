#include "recipe_extractor.h"

RecipeExtractor::RecipeExtractor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      inbuf_size_(0),
      outbuf_size_(0),
      wastebuf_size_(0),
      throughput_(0),
      outpolicy_(this),
      wastepolicy_(this) {}

virtual void DoRegistration() {
  outpolicy_.Init(&outbuf_, outcommod_, outbuf_.capacity());
  wastepolicy_.Init(&wastebuf_, wastecommod_, wastebuf_.capacity());
  inpolicy_.Init(&inbuf_, incommod_, inbuf_.capacity());
  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&wastepolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void RecipeExtractor::Tick(int time) {
}

extern "C" cyclus::Agent* ConstructRecipeExtractor(cyclus::Context* ctx) {
  return new RecipeExtractor(ctx);
}
