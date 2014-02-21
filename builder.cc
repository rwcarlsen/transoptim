#include "builder.h"
#include "boost/lexical_cast.hpp"

Builder::Builder(cyc::Context* ctx) : cyc::Model(ctx) { }

std::string Builder::schema() {
  return
    "  <element name=\"schedule\">     \n"
    "    <oneOrMore>                       \n"
    "      <element name=\"entry\">     \n"
    "        <element name=\"prototype\">     \n"
    "          <data type=\"string\"/>       \n"
    "        </element>                      \n"
    "        <element name=\"time\">     \n"
    "          <data type=\"integer\"/>       \n"
    "        </element>                      \n"
    "      </element>                      \n"
    "    </oneOrMore>                        \n"
    "  </element>                        \n";
}

void Builder::InitFrom(cyc::QueryEngine* qe) {
  cyc::Model::InitFrom(qe);
  qe = qe->QueryElement("model/" + ModelImpl());

  using std::numeric_limits;

  int n = qe->NElementsMatchingQuery("entry");
  for (int i = 0; i < n; ++i) {
    std::string proto = qe->GetElementContent("entry/prototype", i);
    int t = boost::lexical_cast<int>(qe->GetElementContent("entry/prototype", i));
    Schedule(proto, t);
  }
}

cyc::Model* Builder::Clone() {
  Builder* m = new Builder(context());
  m->InitFrom(this);
  return m;
}

void Builder::InitFrom(Builder* m) {
  Model::InitFrom(m);
  schedule_ = m->schedule_;
}

void Builder::Deploy(cyc::Model* parent) {
  Model::Deploy(parent);
  context()->RegisterTimeListener(this);
}

void Builder::Tock(int time) {
  Queue protos = schedule_[time];
  for (int i = 0; i < protos.size(); ++i) {
    Model* m = context()->CreateModel<Model>(protos[i]);
    m->Deploy(this);
  }
}

void Builder::Schedule(std::string prototype, int build_time) {
  schedule_[build_time].push_back(prototype);
}

extern "C" cyc::Model* ConstructBuilder(cyc::Context* ctx) {
  return new Builder(ctx);
}
