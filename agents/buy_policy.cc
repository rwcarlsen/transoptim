#include "buy_policy.h"

using cyclus::CapacityConstraint;
using cyclus::Material;
using cyclus::RequestPortfolio;
using cyclus::Trade;
using cyclus::Bid;
using cyclus::Request;
using cyclus::PrefMap;

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
  double amt = buf_->space();
  if (amt < cyclus::eps()) {
    return ports;
  }

  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
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

void BuyPolicy::AdjustMatlPrefs(PrefMap<Material>::type& prefs) {
  PrefMap<Material>::type::iterator it;
  for (it = prefs.begin(); it != prefs.end(); ++it) {
    Request<Material>::Ptr r = it->first;
    std::map<Bid<Material>::Ptr, double>::iterator it2;
    std::map<Bid<Material>::Ptr, double> bids = it->second;
    for (it2 = it->second.begin(); it != prefs.end(); ++it) {
      Bid<Material>::Ptr b = it2->first;
      prefs[r][b] = pref_;
    }
  }
}
