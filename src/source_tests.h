#ifndef FLEXMORE_SRC_SOURCE_TESTS_H_
#define FLEXMORE_SRC_SOURCE_TESTS_H_
#include "source.h"

#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "agent_tests.h"
#include "context.h"
#include "exchange_context.h"
#include "facility_tests.h"
#include "material.h"

namespace flexmore {

class SourceTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  TestFacility* trader;
  flexmore::Source* src_facility;
  std::string commod, recipe_name;
  double capacity;
  cyclus::Composition::Ptr recipe;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSource();

  std::string outrecipe(flexmore::Source* s) { return s->outrecipe; }
  std::string outcommod(flexmore::Source* s) { return s->outcommod; }
  std::vector<double> throughput(flexmore::Source* s) {
    return s->throughput;
  }

  void outrecipe(flexmore::Source* s, std::string recipe) {
    s->outrecipe = recipe;
  }
  void outcommod(flexmore::Source* s, std::string commod) {
    s->outcommod = commod;
  }
  void throughput(flexmore::Source* s, double val) {
    s->throughput = std::vector<double>(1, val);
  }
  void throughput(flexmore::Source* s, std::vector<double> val) {
    s->throughput = val;
  }

  boost::shared_ptr<cyclus::ExchangeContext<cyclus::Material> > GetContext(
      int nreqs, std::string commodity);
};

} // namespace flexmore

#endif  // FLEXMORE_SRC_SOURCE_TESTS_H_
