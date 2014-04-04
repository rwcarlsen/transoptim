#ifndef BUY_POLICY_H_
#define BUY_POLICY_H_

#include <string>
#include "cyclus.h"

class BuyPolicy : public cyclus::Trader {
 public:
  BuyPolicy(cyclus::Agent* manager) : cyclus::Trader(manager) {};

  virtual ~BuyPolicy() {};

  void Init(cyclus::ResourceBuff* buf, std::string commod,
            cyclus::Composition::Ptr c);

  std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
  GetMatlRequests();

  void AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& resps);

 private:
  cyclus::ResourceBuff* buf_;
  cyclus::Composition::Ptr comp_;
  std::string commod_;
};

#endif
