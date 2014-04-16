#include "animal.h"
#include <time.h>

#define LG(X) LOG(cyclus::LEV_##X, "Animal")

using cyclus::ResCast;
using cyclus::Product;

Animal::Animal(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      bufsize_(0),
      full_grown_(0),
      inpolicy_(this),
      for_sale_(0),
      outpolicy_(this) {}

void Animal::DoRegistration() {
  cyclus::Facility::DoRegistration();
  outpolicy_.Init(&outbuf_, outcommod_);
  inpolicy_.Init(&inbuf_, incommod_, quality_, 0);
  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void Animal::Tock(int t) {
  LG(INFO3) << "Animal id=" << id() << " is tocking";
  LG(INFO4) << "inbuf quantity = " << inbuf_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();
  int age = context()->time() - enter_time();
  if (inbuf_.space() > cyclus::eps()) {
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
  } else if (age == full_grown_) {
    LG(INFO3) << "Animal id=" << id() << " is reproducing";
    context()->SchedBuild(this, prototype());
    context()->SchedBuild(this, prototype());
  }
  
  outbuf_.PopN(outbuf_.count());
  for_sale_ = 0;

  srand(time(NULL));
  if (rand() % 2 == 0) {
    outbuf_.PushAll(inbuf_.PopN(inbuf_.count()));
    for_sale_ = 1;
  } else {
    inbuf_.PopN(inbuf_.count());
  }
}

extern "C" cyclus::Agent* ConstructAnimal(cyclus::Context* ctx) {
  return new Animal(ctx);
}
