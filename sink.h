#ifndef SINK_H_
#define SINK_H_

#include "cyclus/cyclus.h"
#include "sink_policy.h"

namespace cyc = cyclus;

class Sink : public cyc::Model {
 public:
  Sink(cyc::Context* ctx);

  virtual ~Sink() {};

  virtual std::string schema();

  virtual void InitFrom(cyc::QueryEngine* qe);

  virtual cyc::Model* Clone();

  void InitFrom(Sink* m);

  void Deploy(cyc::Model* parent);

 private: 
  SinkPolicy policy_;
  cyc::ResourceBuff inv_;
};

#endif

