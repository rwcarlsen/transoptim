#ifndef FULL_SELL_POLICY_H_
#define FULL_SELL_POLICY_H_

#include <string>
#include "cyclus/cyclus.h"

namespace cyc = cyclus;

class FullSellPolicy : public cyc::Trader {
 public:
  FullSellPolicy(cyc::Model* manager, cyc::ResourceBuff* buf, std::string commod);

  virtual ~FullSellPolicy() {};

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


