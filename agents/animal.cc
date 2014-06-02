#include "animal.h"
#include <time.h>
#include <algorithm>

#define LG(X) LOG(cyclus::LEV_##X, "Animal")

using cyclus::ResCast;
using cyclus::Material;

std::map<std::string, int> Animal::alive_;

Animal::Animal(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    bufsize_(0),
    birth_freq_(0),
    lifespan_(0),
    capture_ratio_(0),
    for_sale_(0),
    inpolicy_(this),
    outpolicy_(this) { }

void Animal::EnterNotify() {
  cyclus::Facility::EnterNotify();
  outpolicy_.Init(&outbuf_, outcommod_);
  inpolicy_.Init(&inbuf_, incommod_, context()->GetRecipe(inrecipe_), 0);
  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void Animal::Build(cyclus::Agent* parent) {
  cyclus::Facility::Build(parent);
  alive_["in_" + incommod_] += 1;
  alive_["out_" + outcommod_] += 1;
}

void Animal::Decommission() {
  context()->UnregisterTrader(&outpolicy_);
  context()->UnregisterTrader(&inpolicy_);
  alive_["in_" + incommod_] -= 1;
  alive_["out_" + outcommod_] -= 1;

  if (alive_["in_" + incommod_] == 0) {
    context()->KillSim();
  } else if (alive_["in_" + incommod_] > 5000) {
    context()->KillSim();
  }

  cyclus::Facility::Decommission();
}

#define LABEL "Ainimal (id=" << id() << ", proto=" << prototype() << ") "

void Animal::Tock() {
  LG(INFO3) << LABEL << "is tocking";
  LG(INFO4) << "inbuf quantity = " << inbuf_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();
  int age = context()->time() - enter_time();
  if (inbuf_.quantity() < cyclus::eps()) {
    LG(INFO3) << LABEL << "is dying of starvation";
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Time", context()->time())
      ->AddVal("Stat", "starved")
      ->Record();
    context()->SchedDecom(this);
    return;
  } else if (age >= lifespan_) {
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Time", context()->time())
      ->AddVal("Stat", "died naturally")
      ->Record();
    LG(INFO3) << LABEL << "is dying of old age";
    context()->SchedDecom(this);
    return;
  } else if (outbuf_.quantity() < cyclus::eps() && age > 0 && for_sale_ != 0) {
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Time", context()->time())
      ->AddVal("Stat", "eaten")
      ->Record();
    LG(INFO3) << LABEL << "got eaten";
    context()->SchedDecom(this);
    return;
  }
  
  if (age > 0 && age % birth_freq_ == 0) {
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Time", context()->time())
      ->AddVal("Stat", "reproduced")
      ->Record();
    LG(INFO3) << LABEL << "is having 1 child";
    context()->SchedBuild(this, prototype());
  }

  double nself = alive_["in_" + incommod_];
  double npred = alive_["in_" + outcommod_];
  double nprey = alive_["out_" + incommod_];

  for_sale_ = 0;
  outbuf_.PopN(outbuf_.count());
  cyclus::toolkit::Manifest mats = inbuf_.PopN(inbuf_.count());
  double r = ((double)(rand() % 100000000)) / (double)100000000; // between 0 and 1
  if (r < std::exp(nself / npred - capture_ratio_)) {
    for_sale_ = 1;
    outbuf_.PushAll(mats);
  }
}

extern "C" cyclus::Agent* ConstructAnimal(cyclus::Context* ctx) {
  return new Animal(ctx);
}

