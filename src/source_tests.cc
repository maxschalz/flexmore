#include "source_tests.h"

#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"

using pyne::nucname::id;

namespace flexmore {

Composition::Ptr genericRecipe() {
  cyclus::CompMap m;
  m[id("u235")] = 0.5;
  m[id("u238")] = 0.5;
  return cyclus::Composition::CreateFromMass(m);
}

void SourceTest::SetUp() {
  src_facility = new flexmore::Source(tc.get());
  trader = tc.trader();
  InitParameters();
  SetUpSource();
}

void SourceTest::TearDown() {
  delete src_facility;
}

void SourceTest::InitParameters() {
  commod = "commod";
  recipe_name = "recipe";
  capacity = 5;  // some magic number..

  recipe = cyclus::Composition::CreateFromAtom(cyclus::CompMap());
  tc.get()->AddRecipe(recipe_name, recipe);
}

void SourceTest::SetUpSource() {
  outcommod(src_facility, commod);
  outrecipe(src_facility, recipe_name);
  throughput(src_facility, capacity);
}

TEST_F(SourceTest, Clone) {
  cyclus::Context* ctx = tc.get();
  flexmore::Source* cloned_fac = dynamic_cast<flexmore::Source*>
                                         (src_facility->Clone());

  EXPECT_EQ(outcommod(src_facility),  outcommod(cloned_fac));
  EXPECT_EQ(throughput(src_facility), throughput(cloned_fac));
  EXPECT_EQ(outrecipe(src_facility),  outrecipe(cloned_fac));

  delete cloned_fac;
}

// Test that multiple throughput values indeed yield the correct result
TEST_F(SourceTest, MultiThroughputInput) {
  std::string config = 
      " <outcommod>commod</outcommod>  "
      " <outrecipe>genericRecipe</outrecipe>  "
      " <throughput> "
      "   <val>1</val> <val>2</val> <val>3</val> "
      " </throughput> ";
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec(":flexmore:Source"), config, simdur);
  sim.AddRecipe("genericRecipe", genericRecipe());
  sim.AddSink("commod").Finalize();
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("TimeSeriessupplycommod", NULL);
  EXPECT_EQ(1., qr.GetVal<double>("Value", 0));
  EXPECT_EQ(2., qr.GetVal<double>("Value", 1));
  EXPECT_EQ(3., qr.GetVal<double>("Value", 2));

}

// Test that a single throughput value gets indeed used every timestep
TEST_F(SourceTest, SingleThroughputInput) {
  std::string config = 
      " <outcommod>commod</outcommod>  "
      " <outrecipe>genericRecipe</outrecipe>  "
      " <throughput>  <val>1</val>        </throughput> ";
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec(":flexmore:Source"), config, simdur);
  sim.AddRecipe("genericRecipe", genericRecipe());
  sim.AddSink("commod").Finalize();
  
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("TimeSeriessupplycommod", NULL);
  EXPECT_EQ(1., qr.GetVal<double>("Value", 0));
  EXPECT_EQ(1., qr.GetVal<double>("Value", 1));
  EXPECT_EQ(1., qr.GetVal<double>("Value", 2));

}

TEST_F(SourceTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

TEST_F(SourceTest, AddBids) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::ExchangeContext;
  using cyclus::Material;

  int nreqs = 5;

  boost::shared_ptr< cyclus::ExchangeContext<Material> >
      ec = GetContext(nreqs, commod);

  std::set<BidPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlBids(ec.get()->commod_requests);

  ASSERT_TRUE(ports.size() > 0);
  EXPECT_EQ(ports.size(), 1);

  BidPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(port->bidder(), src_facility);
  EXPECT_EQ(port->bids().size(), nreqs);

  const std::set< CapacityConstraint<Material> >& constrs = port->constraints();
  ASSERT_TRUE(constrs.size() > 0);
  EXPECT_EQ(constrs.size(), 1);
  EXPECT_EQ(*constrs.begin(), CapacityConstraint<Material>(capacity));
}

TEST_F(SourceTest, Response) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::get_mat;

  std::vector< cyclus::Trade<cyclus::Material> > trades;
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                        cyclus::Material::Ptr> > responses;

  // Null response
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 0);

  double qty = capacity / 3;
  Request<Material>* request =
      Request<Material>::Create(get_mat(), trader, commod);
  Bid<Material>* bid =
      Bid<Material>::Create(request, get_mat(), src_facility);

  Trade<Material> trade(request, bid, qty);
  trades.push_back(trade);

  // 1 trade
  src_facility->GetMatlTrades(trades, responses);
  EXPECT_EQ(responses.size(), 1);
  EXPECT_EQ(responses[0].second->quantity(), qty);
  EXPECT_EQ(responses[0].second->comp(), recipe);

  // 2 trades, total qty = capacity
  trades.push_back(trade);
  responses.clear();
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 2);

  // reset!
  src_facility->Tick();

  delete request;
  delete bid;
}

TEST_F(SourceTest, PositionInitialize) {
  std::string config =
    "<outcommod>spent_fuel</outcommod>"
  ;
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":flexmore:Source"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);

}

TEST_F(SourceTest, Longitude) {
  std::string config =
    "<outcommod>spent_fuel</outcommod>"
    "<latitude>-0.01</latitude>"
    "<longitude>0.01</longitude>"
  ;
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":flexmore:Source"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), -0.01);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.01);

}

boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
SourceTest::GetContext(int nreqs, std::string commod) {
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::ExchangeContext;
  using test_helpers::get_mat;

  double qty = 3;
  boost::shared_ptr< ExchangeContext<Material> >
      ec(new ExchangeContext<Material>());
  for (int i = 0; i < nreqs; i++) {
    ec->AddRequest(Request<Material>::Create(get_mat(), trader, commod));
  }
  return ec;
}

} // namespace flexmore

cyclus::Agent* SourceConstructor(cyclus::Context* ctx) {
  return new flexmore::Source(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

INSTANTIATE_TEST_CASE_P(SourceFac, FacilityTests, Values(&SourceConstructor));
INSTANTIATE_TEST_CASE_P(SourceFac, AgentTests, Values(&SourceConstructor));

