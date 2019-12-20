#include <gtest/gtest.h>

#include "source.h"

#include "agent_tests.h"
#include "context.h"
#include "facility_tests.h"
#include "pyhooks.h"

using flexmore::Source;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SourceTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc;
  Source* facility;

  virtual void SetUp() {
    cyclus::PyStart();
    facility = new Source(tc.get());
  }

  virtual void TearDown() {
    delete facility;
    cyclus::PyStop();
  }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, Print) {
  EXPECT_NO_THROW(std::string s = facility->str());
  // Test Source specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, Tick) {
  ASSERT_NO_THROW(facility->Tick());
  // Test Source specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, Tock) {
  EXPECT_NO_THROW(facility->Tock());
  // Test Source specific behaviors of the Tock function here
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Do Not Touch! Below section required for connection with Cyclus
cyclus::Agent* SourceConstructor(cyclus::Context* ctx) {
  return new Source(ctx);
}
// Required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED
INSTANTIATE_TEST_CASE_P(Source, FacilityTests,
                        ::testing::Values(&SourceConstructor));
INSTANTIATE_TEST_CASE_P(Source, AgentTests,
                        ::testing::Values(&SourceConstructor));
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
