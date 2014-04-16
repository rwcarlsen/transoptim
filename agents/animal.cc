#include "animal.h"
#include <time.h>

#define LG(X) LOG(cyclus::LEV_##X, "Animal")

using cyclus::ResCast;
using cyclus::Material;

Animal::Animal(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    bufsize_(0),
    burnrate_(0),
    full_grown_(0),
    inpolicy_(this),
    for_sale_(0),
    outpolicy_(this) {}

void Animal::DoRegistration() {
  cyclus::Facility::DoRegistration();
  outpolicy_.Init(&outbuf_, outcommod_);
  inpolicy_.Init(&inbuf_, incommod_, context()->GetRecipe(inrecipe_), 0);
  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void Animal::Build() {
  cyclus::Facility::Build();
  Material::Ptr m;
  m = Material::Create(this, bufsize_, context()->GetRecipe(inrecipe_));
  inbuf_.Push(m);
}

void Animal::Decommission() {
  context()->UnregisterTrader(&outpolicy_);
  context()->UnregisterTrader(&inpolicy_);
  cyclus::Facility::Decommission();
}

void Animal::Tock(int t) {
  LG(INFO3) << "Animal id=" << id() << " is tocking";
  LG(INFO4) << "inbuf quantity = " << inbuf_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();
  int age = context()->time() - enter_time();
  if (inbuf_.quantity() < burnrate_) {
    LG(INFO3) << "Animal id=" << id() << " is dying of starvation";
    context()->SchedDecom(this);
    return;
  } else if (age >= lifespan_) {
    LG(INFO3) << "Animal id=" << id() << " is dying of old age";
    context()->SchedDecom(this);
    return;
  } else if (outbuf_.empty() && for_sale_ != 0) {
    LG(INFO3) << "Animal id=" << id() << " got eaten";
    context()->SchedDecom(this);
    return;
  }
  
  if (age == full_grown_) {
    LG(INFO3) << "Animal id=" << id() << " is reproducing";
    context()->SchedBuild(this, prototype());
    context()->SchedBuild(this, prototype());
  }

  outbuf_.PopQty(outbuf_.count());
  for_sale_ = 0;

  srand(time(NULL));
  cyclus::Manifest mats = inbuf_.PopQty(burnrate_);
  if (rand() % 2 == 0) {
    outbuf_.PushAll(mats);
  }
}

extern "C" cyclus::Agent* ConstructAnimal(cyclus::Context* ctx) {
  return new Animal(ctx);
}

