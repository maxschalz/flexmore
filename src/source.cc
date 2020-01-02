#include "source.h"

#include <limits>
#include <sstream>

namespace flexmore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Source::Source(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      throughput(std::vector<double>(1, std::numeric_limits<double>::max())),
      inventory_size(std::numeric_limits<double>::max()),
      latitude(0.0),
      longitude(0.0),
      coordinates(0.0, 0.0) {}

Source::~Source() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::InitFrom(Source* m) {
  #pragma cyclus impl initfromcopy flexmore::Source
  cyclus::toolkit::CommodityProducer::Copy(m);
  RecordPosition();
}

void Source::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl initfromdb flexmore::Source
  namespace tk = cyclus::toolkit;
  tk::CommodityProducer::Add(
    tk::Commodity(outcommod),
    tk::CommodInfo(currentThroughput, currentThroughput)
  );
  RecordPosition();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::EnterNotify() {
  cyclus::Facility::EnterNotify();
   
  int ltime = lifetime() != -1 ?
      lifetime() : context()->sim_info().duration - enter_time();
  
  // if only one throughput is indicated, then expand this to all timesteps
  if (throughput.size() == 1) {
    throughput = std::vector<double>(ltime, throughput[0]);
  }

  // input consistency checks
  std::stringstream ss;
  if (throughput.size() != ltime) {
    ss << "Prototype '" << prototype() << "' has "
       << throughput.size() << " throughput vals, expected "
       << ltime << "\n";
  }
  for (int i = 0; i < throughput.size(); i++) {
    if (throughput[i] < 0 ||
        throughput[i] > std::numeric_limits<double>::max()) {
      ss << "Prototype '" << prototype() 
         << "' has invalid value " << throughput[i]
         << " in position " << i << " of throughput\n";
    }
  }
  
  if (ss.str().size() > 0) {
    throw cyclus::ValueError(ss.str());
  } 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
     << "' with recipe '" << outrecipe << "' at a current throughput of"
     << currentThroughput << " kg per time step commod producer members: "
     << " produces " << outcommod << "?: " << ans
     << " throughput: " << tk::CommodityProducer::Capacity(outcommod)
     << " cost: " << tk::CommodityProducer::Cost(outcommod);
  
  return ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::SetThroughput() {
  int t = context()->time() - enter_time();
  currentThroughput = throughput[t];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::Tick() {
  SetThroughput();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> Source::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::Request;

  double max_qty = std::min(currentThroughput, inventory_size);
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
    Material::Ptr m = Material::CreateUntracked(qty, context()->GetRecipe(outrecipe));
    if (!outrecipe.empty()) {
      m = Material::CreateUntracked(qty, context()->GetRecipe(outrecipe));
    }
    port->AddBid(req, m, this);
  }

  CapacityConstraint<Material> cc(max_qty);
  port->AddConstraint(cc);
  ports.insert(port);

  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::GetMatlTrades(
    const std::vector<cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  std::vector<cyclus::Trade<cyclus::Material> >::const_iterator it;
  for(it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    inventory_size -= qty;

    Material::Ptr response;
    if (!outrecipe.empty()) {
      response = Material::Create(this, qty, context()->GetRecipe(outrecipe));
    } else {
      response = Material::Create(this, qty, it->request->target()->comp());
    }
    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "Source") << prototype() << " sent an order"
                                     << " for " << qty << " of " << outcommod;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::RecordPosition() {
  std::string specification = this->spec();
  context()
    ->NewDatum("AgentPosition")
    ->AddVal("Spec", specification)
    ->AddVal("Prototype", this->prototype())
    ->AddVal("AgentId", id())
    ->AddVal("Latitude", latitude)
    ->AddVal("Longitude", longitude)
    ->Record();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSource(cyclus::Context* ctx) {
  return new Source(ctx);
}

}  // namespace flexmore
