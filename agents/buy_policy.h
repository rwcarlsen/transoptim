#ifndef BUY_POLICY_H_
#define BUY_POLICY_H_

#include <string>
#include "cyclus.h"

namespace cyc = cyclus;

using cyc::CapacityConstraint;
using cyc::Material;
using cyc::Request;
using cyclus::RequestPortfolio;
using cyc::Trade;

class BuyPolicy : public cyc::Trader {
 public:
  BuyPolicy(cyc::Model* manager) : cyc::Trader(manager) {};

  virtual ~BuyPolicy() {};

  void Init(cyc::ResourceBuff* buf, std::string commod, double cap,
            cyc::Composition::Ptr c) {
    buf_ = buf;
    cap_ = cap;
    comp_ = c;
    commod_ = commod;
  }

  std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
  GetMatlRequests() {
    std::set<RequestPortfolio<Material>::Ptr> ports;
    if (buf_->space() < cyclus::eps()) {
      return ports;
    }

    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
    double amt = std::min(cap_, buf_->space());
    Material::Ptr m = Material::CreateUntracked(amt, comp_);
    port->AddRequest(m, this, commod_);

    ports.insert(port);
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);

    return ports;
  }

  void AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& resps) {
    std::vector< std::pair<Trade<Material>, Material::Ptr> >::const_iterator it;
    for (it = resps.begin(); it != resps.end(); ++it) {
      buf_->Push(it->second);
    }
  }

 private:
  double cap_;
  cyc::ResourceBuff* buf_;
  cyc::Composition::Ptr comp_;
  std::string commod_;
};

#endif

