#ifndef ANIMAL_H_
#define ANIMAL_H_

#include <string>
#include "cyclus.h"
#include "buy_policy_prod.h"
#include "sell_policy_prod.h"

class Animal : public cyclus::Facility {
 public:
  Animal(cyclus::Context* ctx);
  virtual ~Animal() {};
  virtual std::string str() {return "";};

  virtual void DoRegistration();

  #pragma cyclus

  virtual void Tick(int time) {};
  virtual void Tock(int time);

 private:
  #pragma cyclus var {'default': 1}
  double bufsize_;
  #pragma cyclus var {'default': 3}
  double full_grown_;
  #pragma cyclus var {'default': 4}
  double lifespan_;
  #pragma cyclus var {'default': 'food'}
  std::string quality_;
  #pragma cyclus var {'default': 0}
  int for_sale_;

  #pragma cyclus var {}
  std::string incommod_;
  #pragma cyclus var {'capacity': 'bufsize_'}
  cyclus::ResourceBuff inbuf_;

  #pragma cyclus var {}
  std::string outcommod_;
  #pragma cyclus var {'capacity': 'bufsize_'}
  cyclus::ResourceBuff outbuf_;


  SellPolicyProd outpolicy_;
  BuyPolicyProd inpolicy_;
};

#endif  // ANIMAL_H_
