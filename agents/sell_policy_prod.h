#ifndef SELL_POLICY_PROD_H_
#define SELL_POLICY_PROD_H_

#include <string>
#include "cyclus.h"

class SellPolicyProd : public cyclus::Trader {
 public:
  SellPolicyProd(cyclus::Agent* manager) : Trader(manager) {};

  void Init(cyclus::ResourceBuff* buf, std::string commod);

  virtual ~SellPolicyProd() {};

  virtual std::set<cyclus::BidPortfolio<cyclus::Product>::Ptr>
  GetProductBids(const cyclus::CommodMap<cyclus::Product>::type&
              commod_requests);

  virtual void GetProductTrades(
    const std::vector< cyclus::Trade<cyclus::Product> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Product>,
    cyclus::Product::Ptr> >& responses);

 private:
  cyclus::ResourceBuff* buf_;
  std::string commod_;
};

#endif
