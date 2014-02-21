#ifndef BUILDER_H_
#define BUILDER_H_

#include "cyclus/cyclus.h"

namespace cyc = cyclus;

class Builder : public cyc::TimeListener, public cyc::Model {
 public:
  /// a list of prototypes to build on a given timestep
  typedef std::vector<std::string> Queue;

  Builder(cyc::Context* ctx);

  virtual ~Builder() { };

  virtual std::string schema();

  virtual void InitFrom(cyc::QueryEngine* qe);

  virtual cyc::Model* Clone();

  void InitFrom(Builder* m);

  void Deploy(cyc::Model* parent);

  virtual void Tick(int time) {};
  virtual void Tock(int time);

  void Schedule(std::string prototype, int build_time);

 private:
  std::map<int, Queue> schedule_;
};
#endif

