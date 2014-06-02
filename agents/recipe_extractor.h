#ifndef RECIPE_EXTRACTOR_H_
#define RECIPE_EXTRACTOR_H_

#include <string>
#include "cyclus.h"
#include "kitlus/sell_policy.h"
#include "kitlus/buy_policy.h"

using kitlus::BuyPolicy;
using kitlus::SellPolicy;

class RecipeExtractor : public cyclus::Facility {
 public:
  RecipeExtractor(cyclus::Context* ctx);
  virtual ~RecipeExtractor() {};
  virtual std::string str() {return "";};

  virtual void EnterNotify();
  virtual void Decommission();

  #pragma cyclus

  virtual void Tick();
  virtual void Tock() {};

 private:
  #pragma cyclus var {}
  std::string incommod_;
  #pragma cyclus var {}
  std::string inrecipe_;
  #pragma cyclus var {}
  double inpref_;
  #pragma cyclus var {}
  double inbuf_size_;
  #pragma cyclus var {'capacity': 'inbuf_size_'}
  cyclus::toolkit::ResourceBuff inbuf_;

  #pragma cyclus var {}
  std::string outcommod_;
  #pragma cyclus var {}
  std::string outrecipe_;
  #pragma cyclus var {}
  double outbuf_size_;
  #pragma cyclus var {'capacity': 'outbuf_size_'}
  cyclus::toolkit::ResourceBuff outbuf_;

  #pragma cyclus var {}
  std::string wastecommod_;
  #pragma cyclus var {}
  double wastebuf_size_;
  #pragma cyclus var {'capacity': 'wastebuf_size_'}
  cyclus::toolkit::ResourceBuff wastebuf_;

  #pragma cyclus var {}
  double throughput_;

  SellPolicy outpolicy_;
  SellPolicy wastepolicy_;
  BuyPolicy inpolicy_;
};

#endif  // RECIPE_EXTRACTOR_H_
