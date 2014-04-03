#ifndef SRC_POLICY_H_
#define SRC_POLICY_H_

#include <string>
#include "cyclus.h"

namespace cyc = cyclus;

using cyc::Bid;
using cyc::BidPortfolio;
using cyc::CapacityConstraint;
using cyc::Material;
using cyc::Request;
using cyc::Trade;

class SrcPolicy : public cyc::Trader {
 public:
  SrcPolicy(cyc::Agent* manager) : cyc::Trader(manager) {};

  virtual ~SrcPolicy() {};

  void Init(double cap, cyc::Composition::Ptr c,
            std::string commod) {
    cap_ = cap;
    comp_ = c;
    commod_ = commod;
  }

  virtual std::set<cyc::BidPortfolio<cyc::Material>::Ptr>
  GetMatlBids(const cyc::CommodMap<cyc::Material>::type&
              commod_requests) {
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
      double qty = std::min(req->target()->quantity(), cap_);
      Material::Ptr offer = Material::CreateUntracked(qty, comp_);
      port->AddBid(req, offer, this);
    }
    CapacityConstraint<Material> cc(cap_);
    port->AddConstraint(cc);
    ports.insert(port);
    return ports;
  }

  virtual void GetMatlTrades(
    const std::vector< cyc::Trade<cyc::Material> >& trades,
    std::vector<std::pair<cyc::Trade<cyc::Material>,
    cyc::Material::Ptr> >& responses) {
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
    if (provided > cap_) {
      std::stringstream ss;
      ss << "source facility is being asked to provide " << provided
         << " but its capacity is " << cap_ << ".";
      throw cyclus::ValueError(ss.str());
    }
  }

 private:
  double cap_;
  cyc::Composition::Ptr comp_;
  std::string commod_;
};

#endif

