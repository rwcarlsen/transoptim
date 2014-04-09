#ifndef SEPARATOR_H_
#define SEPARATOR_H_

#include <string>
#include "cyclus.h"
#include "sell_policy.h"
#include "buy_policy.h"

class Separator : public cyclus::Facility {
 public:
  Separator(cyclus::Context* ctx);
  virtual ~Separator() {};
  virtual std::string str() {return "";};

  virtual void DoRegistration();

  #pragma cyclus

  virtual void Tick(int time);
  virtual void Tock(int time) {};

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
  cyclus::ResourceBuff inbuf_;

  #pragma cyclus var {}
  std::string outcommod_;
  #pragma cyclus var {}
  double outbuf_size_;
  #pragma cyclus var {'capacity': 'outbuf_size_'}
  cyclus::ResourceBuff outbuf_;

  #pragma cyclus var {}
  std::string wastecommod_;
  #pragma cyclus var {}
  double wastebuf_size_;
  #pragma cyclus var {'capacity': 'wastebuf_size_'}
  cyclus::ResourceBuff wastebuf_;

  #pragma cyclus var {}
  std::set<int> nucs_;
  #pragma cyclus var {}
  std::map<int, double> effs_;

  #pragma cyclus var {}
  double throughput_;

  SellPolicy outpolicy_;
  SellPolicy wastepolicy_;
  BuyPolicy inpolicy_;
};

#endif  // RECIPE_EXTRACTOR_H_
