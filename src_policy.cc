#include "src_policyh"

using cyc::Bid;
using cyc::BidPortfolio;
using cyc::CapacityConstraint;
using cyc::Material;
using cyc::Request;
using cyc::Trade;
using cyc::ResourceBuff;

SrcPolicy::SrcPolicy(cyc::Model* manager) : cyc::Trader(manager) {}

void SrcPolicy::Init(double cap, cyc::Composition::Ptr c,
                     std::string commod) {
  cap_ = cap;
  comp_ = c;
  commod_ = commod;
}

void SrcPolicy::InitFrom(const SrcPolicy& other) {
  cap_ = other.cap_;
  commod_ = other.commod_;
  comp_ = other.comp_;
}

std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
SrcPolicy::GetMatlBids(
  const cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  std::set<BidPortfolio<Material>::Ptr> ports;
  if (commod_requests.count(commod_) < 1) {
    return ports;
  }

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  const std::vector<Request<Material>::Ptr>& requests = commod_requests.at(
                                                          commod_);

  std::vector<Request<Material>::Ptr>::const_iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    const Request<Material>::Ptr req = *it;
    double qty = std::min(req->target()->quantity(), cap);
    Material::Ptr offer = Material::CreateUntracked(qty, comp_);
    port->AddBid(req, offer, this);
  }
  CapacityConstraint<Material> cc(cap_);
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

void SrcPolicy::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {

  double provided = 0;
  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    provided += qty;
    Material::Ptr response = Material::Create(manager(),
                                              qty,
                                              comp_);
    responses.push_back(std::make_pair(*it, response));
  }
  if (provided > capacity_) {
    std::stringstream ss;
    ss << "source facility is being asked to provide " << provided
       << " but its capacity is " << capacity_ << ".";
    throw cyclus::ValueError(ss.str());
  }
}

