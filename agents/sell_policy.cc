#include "sell_policy.h"

using cyc::BidPortfolio;
using cyc::CapacityConstraint;
using cyc::Material;
using cyc::Request;
using cyc::Trade;
using cyc::Manifest;

SellPolicy::SellPolicy(cyc::Agent* manager)
  : Trader(manager) {}

void SellPolicy::Init(cyc::ResourceBuff* buf, std::string commod) {
  buf_ = buf;
  commod_ = commod;
}

std::set<BidPortfolio<Material>::Ptr>
SellPolicy::GetMatlBids(
    const cyc::CommodMap<Material>::type& commod_requests) {

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

  CapacityConstraint<Material> cc(buf_->quantity());
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

void SellPolicy::GetMatlTrades(
    const std::vector< Trade<Material> >& trades,
    std::vector<std::pair<Trade<Material>,
    Material::Ptr> >& responses) {

  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    std::vector<Material::Ptr> man = cyclus::ResCast<Material>(buf_->PopQty(qty));
    for (int i = 1; i < man.size(); ++i) {
      man[0]->Absorb(man[i]);
    }
    responses.push_back(std::make_pair(*it, man[0]));
  }
}


