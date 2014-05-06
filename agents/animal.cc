#include "animal.h"
#include <time.h>

#define LG(X) LOG(cyclus::LEV_##X, "Animal")

using cyclus::ResCast;
using cyclus::Material;

std::map<std::string, int> Animal::alive_;

Animal::Animal(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    bufsize_(0),
    birth_freq_(0),
    lifespan_(0),
    capture_prob_(0),
    for_sale_(0),
    inpolicy_(this),
    outpolicy_(this) { }

void Animal::DoRegistration() {
  cyclus::Facility::DoRegistration();
  outpolicy_.Init(&outbuf_, outcommod_);
  inpolicy_.Init(&inbuf_, incommod_, context()->GetRecipe(inrecipe_), 0);
  context()->RegisterTrader(&outpolicy_);
  context()->RegisterTrader(&inpolicy_);
}

void Animal::Build(cyclus::Agent* parent) {
  cyclus::Facility::Build();
  Material::Ptr m;
  m = Material::Create(this, bufsize_, context()->GetRecipe(inrecipe_));
  inbuf_.Push(m);
  alive_[prototype()] += 1;
}

void Animal::Decommission() {
  context()->UnregisterTrader(&outpolicy_);
  context()->UnregisterTrader(&inpolicy_);
  alive_[prototype()] -= 1;
  cyclus::Facility::Decommission();
}

#define LABEL "Ainimal (id=" << id() << ", proto=" << prototype() << ") "

void Animal::Tock(int t) {
  LG(INFO3) << LABEL << "is tocking";
  LG(INFO4) << "inbuf quantity = " << inbuf_.quantity();
  LG(INFO4) << "outbuf quantity = " << outbuf_.quantity();
  int age = context()->time() - enter_time();
  if (inbuf_.space() > cyclus::eps()) {
    LG(INFO3) << LABEL << "is dying of starvation";
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Stat", "starved")
      ->Record();
    context()->SchedDecom(this);
    return;
  } else if (age >= lifespan_) {
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Stat", "died")
      ->Record();
    LG(INFO3) << LABEL << "is dying of old age";
    context()->SchedDecom(this);
    return;
  } else if (outbuf_.quantity() < cyclus::eps() && age > 0 && for_sale_ != 0) {
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Stat", "eaten")
      ->Record();
    LG(INFO3) << LABEL << "got eaten";
    context()->SchedDecom(this);
    return;
  }
  
  if (age > 0 && age % birth_freq_ == 0) {
    context()->NewDatum("AnimalEvents")
      ->AddVal("AgentId", id())
      ->AddVal("Stat", "reproduced")
      ->Record();
    LG(INFO3) << LABEL << "is having 1 child";
    context()->SchedBuild(this, prototype());
  }

  for_sale_ = 0;
  outbuf_.PopN(outbuf_.count());
  cyclus::Manifest mats = inbuf_.PopN(inbuf_.count());
  double r = ((double)(rand() % 1000000)) / 1000000; // between 0 and 1
  int nself = alive_[prototype()];
  int npred = alive_[prototype()];
  int prob = capture_prob * (npred - nself) / npred*nself;
  if (nself >= npred && r < capture_prob_) {
    for_sale_ = 1;
    outbuf_.PushAll(mats);
  } else if (r < prob) {
    for_sale_ = 1;
    outbuf_.PushAll(mats);
  }
}

extern "C" cyclus::Agent* ConstructAnimal(cyclus::Context* ctx) {
  return new Animal(ctx);
}

