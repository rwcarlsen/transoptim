#include "sell_policy_prod.h"

using cyclus::BidPortfolio;
using cyclus::CapacityConstraint;
using cyclus::Product;
using cyclus::Request;
using cyclus::Trade;
using cyclus::Manifest;

void SellPolicyProd::Init(cyclus::ResourceBuff* buf, std::string commod) {
  buf_ = buf;
  commod_ = commod;
}

std::set<BidPortfolio<Product>::Ptr>
SellPolicyProd::GetProductBids(
    const cyclus::CommodMap<Product>::type& commod_requests) {

  std::set<BidPortfolio<Product>::Ptr> ports;
  if (buf_->empty()) {
    return ports;
  } else if (commod_requests.count(commod_) < 1) {
    return ports;
  }

  BidPortfolio<Product>::Ptr port(new BidPortfolio<Product>());

  const std::vector<Request<Product>::Ptr>& requests = commod_requests.at(
                                                          commod_);

  std::vector<Request<Product>::Ptr>::const_iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    const Request<Product>::Ptr req = *it;
    double qty = std::min(req->target()->quantity(), buf_->capacity());
    Product::Ptr m = buf_->Pop<Product>();
    buf_->Push(m);
    Product::Ptr offer = Product::CreateUntracked(qty, m->quality());
    port->AddBid(req, offer, this);
  }

  CapacityConstraint<Product> cc(buf_->quantity());
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

void SellPolicyProd::GetProductTrades(
    const std::vector< Trade<Product> >& trades,
    std::vector<std::pair<Trade<Product>,
    Product::Ptr> >& responses) {

  std::vector< Trade<Product> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    std::vector<Product::Ptr> man = cyclus::ResCast<Product>(buf_->PopQty(qty));
    for (int i = 1; i < man.size(); ++i) {
      man[0]->Absorb(man[i]);
    }
    responses.push_back(std::make_pair(*it, man[0]));
  }
}

