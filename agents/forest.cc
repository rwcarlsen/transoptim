#include "forest.h"

#define LG(X) LOG(cyclus::LEV_##X, "Forest")

using cyclus::ResCast;
using cyclus::Product;

Forest::Forest(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      bufsize_(0),
      outpolicy_(this) {}

void Forest::DoRegistration() {
  cyclus::Facility::DoRegistration();
  outpolicy_.Init(&buf_, commod_);
  context()->RegisterTrader(&outpolicy_);
}

void Forest::Tick(int time) {
  LG(INFO3) << "Forest id=" << id() << " is ticking";
  LG(INFO4) << "buf quantity = " << buf_.quantity();
  if (buf_.space() > 0) {
    buf_.Push(Product::Create(this, buf_.space(), quality_));
  }
}

extern "C" cyclus::Agent* ConstructForest(cyclus::Context* ctx) {
  return new Forest(ctx);
}
