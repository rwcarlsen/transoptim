#ifndef ANIMAL_H_
#define ANIMAL_H_

#include <string>
#include "cyclus.h"
#include "buy_policy.h"
#include "sell_policy.h"

class Animal : public cyclus::Facility {
 public:
  Animal(cyclus::Context* ctx);
  virtual ~Animal() {};
  virtual std::string str() {return "";};

  virtual void DoRegistration();
  virtual void Build(cyclus::Agent* parent = NULL);
  virtual void Decommission();

  #pragma cyclus

  virtual void Tick(int time) {};
  virtual void Tock(int time);

 private:
  #pragma cyclus var {'default': 1}
  double bufsize_;
  #pragma cyclus var {'default': 2}
  int num_kids_;
  #pragma cyclus var {'default': 0.5}
  double burnrate_;
  #pragma cyclus var {'default': 2}
  double full_grown_;
  #pragma cyclus var {'default': 4}
  double lifespan_;
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
