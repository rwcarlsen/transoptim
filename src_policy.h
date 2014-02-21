#ifndef SRC_POLICY_H_
#define SRC_POLICY_H_

#include <string>
#include "cyclus/cyclus.h"

namespace cyc = cyclus;

class SrcPolicy : public cyc::Trader {
 public:
  SrcPolicy(cyc::Model* manager);

  virtual ~SrcPolicy() {};

  void Init(double cap, cyc::Composition* c,
            std::string commod);

  void InitFrom(const SrcPolicy& other);

  double capacity() {return cap_;}
  std::string commod() {return commod_;}
  cyc::Composition::Ptr comp() {return comp_;}

  virtual std::set<cyc::BidPortfolio<cyc::Material>::Ptr>
  GetMatlBids(const cyc::CommodMap<cyc::Material>::type&
              commod_requests);

  virtual void GetMatlTrades(
    const std::vector< cyc::Trade<cyc::Material> >& trades,
    std::vector<std::pair<cyc::Trade<cyc::Material>,
    cyc::Material::Ptr> >& responses);

 private:
  double cap_;
  cyc::Composition::Ptr comp_;
  std::string commod_;
};

#endif

