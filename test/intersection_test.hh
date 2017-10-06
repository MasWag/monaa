#pragma once

#include <boost/test/unit_test.hpp>

#include "../tgrep/timed_automaton.hh"
#include "../tgrep/intersection.hh"

BOOST_AUTO_TEST_SUITE(intersectionTests)

BOOST_AUTO_TEST_CASE( intersectionTest )
{
  // Input
  std::array<TimedAutomaton, 2> TAs;
  for (auto& A: TAs) {
    A.states.resize(2);
    for (auto &state: A.states) {
      state = std::make_shared<TAState>();
    }
    A.initialStates = {A.states[0]};
    A.states[0]->isMatch = false;
    A.states[1]->isMatch = true;
  }

  // Transitions
  TAs[0].states[0]->next['a'].push_back({TAs[0].states[1], {}, {{TimedAutomaton::X(0) >= 2}}});
  TAs[1].states[0]->next[0].push_back({TAs[1].states[0], {0}, {}});
  TAs[1].states[0]->next['a'].push_back({TAs[1].states[1], {}, {{TimedAutomaton::X(0) < 1}}});  
  
  TAs[0].maxConstraints = {2}; 
  TAs[1].maxConstraints = {1}; 

  // Output
  TimedAutomaton out;
  boost::unordered_map<std::pair<std::shared_ptr<TAState>, std::shared_ptr<TAState>>, std::shared_ptr<TAState>> toIState;

  // Run
  intersectionTA (TAs[0], TAs[1], out, toIState);

  // Expected Results
  auto initState = toIState[std::make_pair(TAs[0].states[0], TAs[1].states[0])];
  auto acceptState = toIState[std::make_pair(TAs[0].states[1], TAs[1].states[1])];

  // Comparison
  BOOST_TEST (out.initialStates.size() == 1);
  BOOST_REQUIRE_EQUAL (out.initialStates[0], initState);
  BOOST_CHECK_EQUAL (initState->next[0].size(), 1);
  BOOST_CHECK_EQUAL (initState->next['a'].size(), 1);
  BOOST_TEST (initState->next[0].front().target.lock() == initState);
  BOOST_TEST (initState->next['a'].front().target.lock() == acceptState);
  BOOST_TEST (initState->isMatch == false);
  BOOST_TEST (acceptState->isMatch == true);
}

BOOST_AUTO_TEST_SUITE_END()
