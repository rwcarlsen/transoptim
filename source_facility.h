#ifndef SOURCE_H_
#define SOURCE_H_

#include <set>
#include <vector>

#include "cyclus/cyclus.h"
#include "src_policy.h"

namespace cyc = cyclus;

class Source : public cyc::Model {
 public:
  Source(cyc::Context* ctx);

  virtual ~Source() {};

  virtual std::string schema();

  virtual cyc::Model* Clone();

  void InitFrom(Source* m);

  virtual void InitFrom(cyc::QueryEngine* qe);

  void Deploy(cyc::Model* parent);

 private:
  SrcPolicy policy_;
};

#endif

