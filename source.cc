#include "source.h"

#include <limits>

Source::Source(cyclus::Context* ctx) : cyc::Model(ctx), policy_(this) {}

std::string Source::schema() {
  return
    "    <element name=\"commod\">         \n"
    "      <data type=\"string\"/>         \n"
    "    </element>                        \n"
    "    <optional>                        \n"
    "      <element name=\"capacity\">     \n"
    "        <data type=\"double\"/>       \n"
    "      </element>                      \n"
    "    </optional>                       \n"
    "    <element name=\"recipe\">         \n"
    "      <data type=\"string\"/>         \n"
    "    </element>                        \n";
}

void Source::InitFrom(cyclus::QueryEngine* qe) {
  cyclus::Model::InitFrom(qe);
  qe = qe->QueryElement("model/" + ModelImpl());

  using std::string;
  using std::numeric_limits;

  std::string recipe = qe->GetElementContent("recipe");
  cyc::Composition::Ptr c = context()->GetRecipe(recipe);

  std::string commod = qe->GetElementContent("commod");

  double cap = cyclus::GetOptionalQuery<double>(qe,
                                                "capacity",
                                                numeric_limits<double>::max());
  policy_.Init(cap, c, commod);
}

cyclus::Model* Source::Clone() {
  Source* m = new Source(context());
  m->InitFrom(this);
  return m;
}

void Source::InitFrom(Source* m) {
  Model::InitFrom(m);
  policy_.InitFrom(m->policy_);
}

void Source::Deploy(cyc::Model* parent) {
  Model::Deploy(parent);
  context()->RegisterTrader(&policy_);
}

extern "C" cyclus::Model* ConstructSource(cyclus::Context* ctx) {
  return new Source(ctx);
}

