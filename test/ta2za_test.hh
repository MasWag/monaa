#pragma once

#include <boost/test/unit_test.hpp>

#include "../tgrep/timed_automaton.hh"
#include "../tgrep/zone_automaton.hh"
#include "../tgrep/ta2za.hh"

BOOST_AUTO_TEST_SUITE(ta2zaTests)

class ZAFixture {
public:
  TimedAutomaton TA;
  ZoneAutomaton ZA;
  ZAFixture() {
    // Input
    TA.states.resize(4);
    for (auto &state: TA.states) {
      state = std::make_shared<TAState>();
    }

    TA.initialStates = {TA.states[0]};

    TA.states[0]->isMatch = false;
    TA.states[1]->isMatch = false;
    TA.states[2]->isMatch = false;
    TA.states[3]->isMatch = true;

    // Transitions
    TA.states[0]->next['a'].push_back({TA.states[0], {1}, {}});
    TA.states[0]->next['a'].push_back({TA.states[1], {}, {{{TimedAutomaton::X(0) >= 1}, {TimedAutomaton::X(0) <= 1}}}});
    TA.states[1]->next['a'].push_back({TA.states[1], {}, {}});
    TA.states[1]->next['a'].push_back({TA.states[2], {}, {{{TimedAutomaton::X(1) >= 1}, {TimedAutomaton::X(1) <= 1}}}});
    TA.states[2]->next['$'].push_back({TA.states[3], {}, {}});
  
    TA.maxConstraints = {1,1}; 

    // Run
    ta2za (TA, ZA);
  }
};

BOOST_FIXTURE_TEST_CASE( ta2zaTest, ZAFixture )
{
  // Expected results
  std::vector<std::size_t> expectedDegrees = {2,2,1,2,1,2,0,0};
  // std::vector<std::vector<NFA::Edge> > expectedEdges = {
  //   {{0,1,'a'},
  //    {0,2,'a'}},
  //   {{1,1,'a'},
  //    {1,3,'a'}},
  //   {{2,4,'a'}},
  //   {{3,5,'a'},
  //    {3,6,'a'}},
  //   {{4,4,'a'}},
  //   {{5,5,'a'},
  //    {5,6,'a'}},
  //   {{6,7,'$'}},
  //   {}
  // };

  // Comparison
  BOOST_TEST (ZA.stateSize() == 8);
  BOOST_TEST (ZA.initialStates.size() == 1);
  for (std::size_t i = 0; i < ZA.stateSize(); i++) {
    BOOST_TEST (ZA.states[i]->next['a'].size() == expectedDegrees[i]);
  }
  BOOST_CHECK_EQUAL(std::count_if(ZA.states.begin(), ZA.states.end(), [](std::shared_ptr<ZAState> state) {
        return state->isMatch;
      }), 1);
}

BOOST_FIXTURE_TEST_CASE( updateAcceptingTest, ZAFixture ) {
  TA.states[0]->isMatch = true;
  ZA.updateAccepting();
  BOOST_CHECK_EQUAL(std::count_if(ZA.states.begin(), ZA.states.end(), [](std::shared_ptr<ZAState> state) {
        return state->isMatch;
      }), 2);
}

BOOST_AUTO_TEST_SUITE_END()
