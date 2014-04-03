#ifndef SELL_POLICY_H_
#define SELL_POLICY_H_

#include <string>
#include "cyclus.h"

namespace cyc = cyclus;

class SellPolicy : public cyc::Trader {
 public:
  SellPolicy(cyc::Agent* manager);

  void Init(cyc::ResourceBuff* buf, std::string commod);

  virtual ~SellPolicy() {};

  virtual std::set<cyc::BidPortfolio<cyc::Material>::Ptr>
  GetMatlBids(const cyc::CommodMap<cyc::Material>::type&
              commod_requests);

  virtual void GetMatlTrades(
    const std::vector< cyc::Trade<cyc::Material> >& trades,
    std::vector<std::pair<cyc::Trade<cyc::Material>,
    cyc::Material::Ptr> >& responses);

 private:
  cyc::ResourceBuff* buf_;
  std::string commod_;
};

#endif
