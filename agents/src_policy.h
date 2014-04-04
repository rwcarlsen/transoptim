#ifndef SRC_POLICY_H_
#define SRC_POLICY_H_

#include <string>
#include "cyclus.h"

class SrcPolicy : public cyclus::Trader {
 public:
  SrcPolicy(cyclus::Agent* manager) : cyclus::Trader(manager) {};

  virtual ~SrcPolicy() {};

  void Init(double cap, cyclus::Composition::Ptr c,
            std::string commod);

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
  GetMatlBids(const cyclus::CommodMap<cyclus::Material>::type&
              commod_requests);

  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

 private:
  double cap_;
  cyclus::Composition::Ptr comp_;
  std::string commod_;
};

#endif
