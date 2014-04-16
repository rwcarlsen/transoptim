#ifndef BUY_POLICY_PROD_H_
#define BUY_POLICY_PROD_H_

#include <string>
#include "cyclus.h"

class BuyPolicyProd : public cyclus::Trader {
 public:
  BuyPolicyProd(cyclus::Agent* manager) : cyclus::Trader(manager) {};

  virtual ~BuyPolicyProd() {};

  void Init(cyclus::ResourceBuff* buf, std::string commod,
            std::string quality, double pref = 0.0);

  std::set<cyclus::RequestPortfolio<cyclus::Product>::Ptr>
  GetProductRequests();

  void AcceptProductTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Product>,
    cyclus::Product::Ptr> >& resps);

  virtual void AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs);

 private:
  cyclus::ResourceBuff* buf_;
  std::string quality_;
  std::string commod_;
  double pref_;
};

#endif
