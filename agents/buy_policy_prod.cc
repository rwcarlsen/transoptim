#include "buy_policy_prod.h"

using cyclus::CapacityConstraint;
using cyclus::Product;
using cyclus::RequestPortfolio;
using cyclus::Trade;
using cyclus::Bid;
using cyclus::Request;
using cyclus::PrefMap;

void BuyPolicyProd::Init(cyclus::ResourceBuff* buf, std::string commod,
                         std::string quality, double pref) {
  buf_ = buf;
  quality_ = quality;
  commod_ = commod;
  pref_ = pref;
}

std::set<RequestPortfolio<Product>::Ptr>
BuyPolicyProd::GetProductRequests() {
  std::set<RequestPortfolio<Product>::Ptr> ports;
  double amt = buf_->space();
  if (amt < cyclus::eps()) {
    return ports;
  }

  RequestPortfolio<Product>::Ptr port(new RequestPortfolio<Product>());
  Product::Ptr m = Product::CreateUntracked(amt, quality_);
  port->AddRequest(m, this, commod_);

  ports.insert(port);
  CapacityConstraint<Product> cc(amt);
  port->AddConstraint(cc);

  return ports;
}

void BuyPolicyProd::AcceptProductTrades(
  const std::vector< std::pair<Trade<Product>,
  Product::Ptr> >& resps) {
  std::vector< std::pair<Trade<Product>, Product::Ptr> >::const_iterator it;
  for (it = resps.begin(); it != resps.end(); ++it) {
    buf_->Push(it->second);
  }
}

void BuyPolicyProd::AdjustProductPrefs(PrefMap<Product>::type& prefs) {
  PrefMap<Product>::type::iterator it;
  for (it = prefs.begin(); it != prefs.end(); ++it) {
    Request<Product>::Ptr r = it->first;
    std::map<Bid<Product>::Ptr, double>::iterator it2;
    std::map<Bid<Product>::Ptr, double> bids = it->second;
    for (it2 = bids.begin(); it2 != bids.end(); ++it2) {
      Bid<Product>::Ptr b = it2->first;
      prefs[r][b] = pref_;
    }
  }
}

