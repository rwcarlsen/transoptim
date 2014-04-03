#include "full_sell_policy.h"

using cyc::Bid;
using cyc::BidPortfolio;
using cyc::CapacityConstraint;
using cyc::Material;
using cyc::Request;
using cyc::Trade;
using cyc::ResourceBuff;

FullSellPolicy::FullSellPolicy(cyc::Model* manager, cyc::ResourceBuff* buf,
                               std::string commod)
  : cyc::Trader(manager), buf_(buf), commod_(commod) {}

std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
FullSellPolicy::GetMatlBids(
  const cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  std::set<BidPortfolio<Material>::Ptr> ports;
  if (buf_->empty()) {
    return ports;
  } else if (commod_requests.count(commod_) < 1) {
    return ports;
  }

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  const std::vector<Request<Material>::Ptr>& requests = commod_requests.at(
                                                          commod_);

  std::vector<Request<Material>::Ptr>::const_iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    const Request<Material>::Ptr req = *it;
    double qty = std::min(req->target()->quantity(), buf_->capacity());
    Material::Ptr m = buf_->Pop<Material>();
    buf_->Push(m);
    Material::Ptr offer = Material::CreateUntracked(qty, m->comp());
    port->AddBid(req, offer, this);
  }

  CapacityConstraint<Material> cc(buf_->capacity());
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

void FullSellPolicy::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {

  double provided = 0;
  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    provided += qty;
    cyc::Manifest man = buf_->PopQty(qty)
    std::vector<Material::Ptr> man = cyc::ResCast<Material>(buf_->PopQty(qty));
    Material::Ptr m = man.back();
    man.pop_back();
    for (int i = 0; i < man.size(); ++i) {
      m->Absorb(man[i]);
    }
    responses.push_back(std::make_pair(*it, m));
  }
  if (provided > capacity_) {
    std::stringstream ss;
    ss << "source facility is being asked to provide " << provided
       << " but its capacity is " << capacity_ << ".";
    throw cyclus::ValueError(ss.str());
  }
}




