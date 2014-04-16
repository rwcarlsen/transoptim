#ifndef FOREST_H_
#define FOREST_H_

#include <string>
#include "cyclus.h"
#include "sell_policy_prod.h"

class Forest : public cyclus::Facility {
 public:
  Forest(cyclus::Context* ctx);
  virtual ~Forest() {};
  virtual std::string str() {return "";};

  virtual void DoRegistration();

  #pragma cyclus

  virtual void Tick(int time);
  virtual void Tock(int time) {};

 private:
  #pragma cyclus var {'default': 100000}
  double bufsize_;
  #pragma cyclus var {'default': 'food'}
  std::string quality_;

  #pragma cyclus var {}
  std::string commod_;
  cyclus::ResourceBuff buf_;

  SellPolicyProd outpolicy_;
};

#endif  // FOREST_H_
