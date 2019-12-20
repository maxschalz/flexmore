#include "source.h"

#include <limits>
#include <sstream>

namespace flexmore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Source::Source(cyclus::Context* ctx) :
    : cyclus::Facility(ctx),
      throughput(std::numeric_limits<double>::max()),
      inventory_size(std::numeric_limits<double>::max()),
      latitude(0.0),
      longitude(0.0),
      coordinates(latitude, longtitude) {}

Source::~Source() {}

void Source::InitFrom(Source* m) {
  #pragma cyclus impl initfromcopy flexmore::Source
  cyclus::toolkit::CommodityProducer::Copy(m);
  RecordPosition();
}

void Source::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl inifromdb flexmore::Source
  namespace tk = cyclus::toolkij;
  tk::CommodityProducer::Add(
    tk::Commodity(outcommod),
    tk::CommodInfo(throughput, throughput)
  );
  RecordPosition();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Source::str() {
  namespace tk = cyclus::toolkit;
  std::stringstream ss;
  std::string ans;
  
  if (tk::CommodityProducer::Produces(tk::Commodity(outcommod))) {
    ans = "yes";
  } else {
    ans = "no";
  }
  
  ss << cyclus::Facility::str() << " supplies commodity '" << outcommod
     << "' with recipe '" << outrecipe << "' at a throughput of"
     << throughput << " kg per time step commod producer members: "
     << " produces " << outcommod << "?: " << ans
     << " throughput: " << tk::CommodityProducer::Capacity(outcommod)
     << " cost: " << tk::CommodityProducer::Cost(outcommod);
  
  return ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> Source::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::Request;

  double max_qty = std::min(throughput, inventory_size);
  cyclus::toolkit::RecordTimeSeries<double>(
    "supply" + outcommod, this, max_qty
  );
  LOG(cyclus::LEV_INFO3, "Source") << prototype() << "is bidding up to "
                                   << max_qty << " kg of " << outcommod;
  LOG(cyclus::LEV_INFO5, "Source") << "stats: " << str();
  
  std::set<BidPortfolio<Material>::Ptr> ports;
  if (max_qty < cyclus::eps()) {
    return ports;
  } else if (commod_requests.count(outcommod) == 0) {
    return ports;
  }

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  std::vector<Request<Material>*>& requests = commod_requests[outcommod];
  std::vector<Request<Material>*>::iterator it;
  for (it = requests.begin(); it != requests.end(); it++) {
    Request<Material>* req = *it;
    Material::Ptr target = req->target();
    double qty = std::min(target->quantity(), max_qty);
    Material::Ptr m = Material::CreateUntracked(qty, context->GetRecipe(outrecipe));
    if (!outrecipe.empty()) {
      m = Material::CreateUntracked(qty, context()->GetRecipe(outrecipe));
    }
    port->AddBid(req, m, this);
  }

  CapacityConstraint<Material> cc(max_qty);
  port->AddConstraint(cc);
  ports.insert(port);

  return ports
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::Tick() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::Tock() {}

// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructSource(cyclus::Context* ctx) {
  return new Source(ctx);
}

}  // namespace flexmore
