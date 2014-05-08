#ifndef ANIMAL_H_
#define ANIMAL_H_

#include <string>
#include "cyclus.h"
#include "kitlus/sell_policy.h"
#include "kitlus/buy_policy.h"

using kitlus::BuyPolicy;
using kitlus::SellPolicy;

class Animal : public cyclus::Facility {
 public:
  Animal(cyclus::Context* ctx);
  virtual ~Animal() {};
  virtual std::string str() {return "";};

  virtual void EnterNotify();
  virtual void Build(cyclus::Agent* parent = NULL);
  virtual void Decommission();

  #pragma cyclus

  virtual void Tick(int time) {};
  virtual void Tock(int time);

 private:
  static std::map<std::string, int> alive_;

  /// food eaten per timestep to live
  #pragma cyclus var {'default': 1}
  double bufsize_;
  /// number of timsteps between having a single child
  #pragma cyclus var {'default': 1}
  int birth_freq_;
  /// number of timsteps until natural death
  #pragma cyclus var {'default': 4}
  int lifespan_;
  /// probability of being captured when pred population equals this species'
  /// population.
  #pragma cyclus var {'default': 2.0}
  double capture_ratio_;

  #pragma cyclus var {'default': 0}
  int for_sale_;

  #pragma cyclus var {}
  std::string incommod_;
  #pragma cyclus var {}
  std::string inrecipe_;
  #pragma cyclus var {'capacity': 'bufsize_'}
  cyclus::ResourceBuff inbuf_;

  #pragma cyclus var {}
  std::string outcommod_;
  #pragma cyclus var {'capacity': 'bufsize_'}
  cyclus::ResourceBuff outbuf_;

  SellPolicy outpolicy_;
  BuyPolicy inpolicy_;
};

#endif  // ANIMAL_H_
