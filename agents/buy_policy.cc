#include "buy_policy.h"

using cyclus::CapacityConstraint;
using cyclus::Material;
using cyclus::RequestPortfolio;
using cyclus::Trade;

void BuyPolicy::Init(cyclus::ResourceBuff* buf, std::string commod,
                     cyclus::Composition::Ptr c, double pref) {
  buf_ = buf;
  comp_ = c;
  commod_ = commod;
  pref_ = pref;
}

std::set<RequestPortfolio<Material>::Ptr>
BuyPolicy::GetMatlRequests() {
  std::set<RequestPortfolio<Material>::Ptr> ports;
  if (buf_->space() < cyclus::eps()) {
    return ports;
  }

  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  double amt = buf_->space();
  Material::Ptr m = Material::CreateUntracked(amt, comp_);
  port->AddRequest(m, this, commod_, pref_);

  ports.insert(port);
  CapacityConstraint<Material> cc(amt);
  port->AddConstraint(cc);

  return ports;
}

void BuyPolicy::AcceptMatlTrades(
  const std::vector< std::pair<Trade<Material>,
  Material::Ptr> >& resps) {
  std::vector< std::pair<Trade<Material>, Material::Ptr> >::const_iterator it;
  for (it = resps.begin(); it != resps.end(); ++it) {
    buf_->Push(it->second);
  }
}

