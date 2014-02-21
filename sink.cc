
#include "sink.h"

Sink::Sink(cyc::Context* ctx) : cyc::Model(ctx), policy_(this, &inv_) {}

std::string Sink::schema() {
  return
    "    <element name=\"commod\">         \n"
    "      <data type=\"string\"/>         \n"
    "    </element>                        \n"
    "    <optional>                        \n"
    "      <element name=\"capacity\">     \n"
    "        <data type=\"double\"/>       \n"
    "      </element>                      \n"
    "    </optional>                       \n"
    "    <element name=\"throughput\">     \n"
    "      <data type=\"double\"/>         \n"
    "    </element>                        \n"
    "    <element name=\"recipe\">         \n"
    "      <data type=\"string\"/>         \n"
    "    </element>                        \n";
}

void Sink::InitFrom(cyc::QueryEngine* qe) {
  cyclus::Model::InitFrom(qe);
  qe = qe->QueryElement("model/" + ModelImpl());

  using std::numeric_limits;

  std::string recipe = qe->GetElementContent("recipe");
  cyc::Composition::Ptr c = context()->GetRecipe(recipe);

  std::string commod = qe->GetElementContent("commod");

  double cap = cyclus::GetOptionalQuery<double>(qe,
                                                "capacity",
                                                numeric_limits<double>::max());
  double through = cyclus::GetOptionalQuery<double>(qe,
                                                    "throughput",
                                                    0);
  inv_.set_capacity(cap);
  policy_.Init(through, c, commod);
}

cyc::Model* Sink::Clone() {
  Sink* m = new Sink(context());
  m->InitFrom(this);
  return m;
}

void Sink::InitFrom(Sink* m) {
  Model::InitFrom(m);
  policy_.InitFrom(m->policy_);
  inv_.set_capacity(m->inv_.capacity());
}

void Sink::Deploy(cyc::Model* parent) {
  Model::Deploy(parent);
  context()->RegisterTrader(&policy_);
}

extern "C" cyc::Model* ConstructSink(cyc::Context* ctx) {
  return new Sink(ctx);
}

