#ifndef RECIPE_MIXER_H_
#define RECIPE_MIXER_H_

#include <string>
#include "cyclus.h"
#include "kitlus/sell_policy.h"
#include "kitlus/buy_policy.h"

using kitlus::BuyPolicy;
using kitlus::SellPolicy;

class RecipeMixer : public cyclus::Facility {
 public:
  RecipeMixer(cyclus::Context* ctx);
  virtual ~RecipeMixer() {};
  virtual std::string str() {return "";};

  virtual void EnterNotify();
  virtual void Decommission();

  #pragma cyclus

  virtual void Tick();
  virtual void Tock() {};

  double Weight(cyclus::Composition::Ptr c);

 private:
  #pragma cyclus var {}
  std::string incommod1_;
  #pragma cyclus var {}
  std::string inrecipe1_;
  #pragma cyclus var {}
  double inbuf1_size_;
  #pragma cyclus var {'capacity': 'inbuf1_size_'}
  cyclus::toolkit::ResourceBuff inbuf1_;

  #pragma cyclus var {}
  std::string incommod2_;
  #pragma cyclus var {}
  std::string inrecipe2_;
  #pragma cyclus var {}
  double inbuf2_size_;
  #pragma cyclus var {'capacity': 'inbuf2_size_'}
  cyclus::toolkit::ResourceBuff inbuf2_;

  #pragma cyclus var {}
  std::string outcommod_;
  #pragma cyclus var {}
  std::string outrecipe_;
  #pragma cyclus var {}
  double outbuf_size_;
  #pragma cyclus var {'capacity': 'outbuf_size_'}
  cyclus::toolkit::ResourceBuff outbuf_;

  #pragma cyclus var {}
  double throughput_;

  SellPolicy outpolicy_;
  BuyPolicy inpolicy1_;
  BuyPolicy inpolicy2_;
};

#endif  // RECIPE_MIXER_H_
