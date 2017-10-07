#pragma once

#include <boost/test/unit_test.hpp>

#include "../tgrep/timed_automaton.hh"
#include "../tgrep/sunday_skip_value.hh"

BOOST_AUTO_TEST_SUITE(sundaySkipValueTests)

class TAFixture {
public:
  TimedAutomaton TA;
  TAFixture() {
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
    TA.states[0]->next['a'].push_back({TA.states[1], {1}, {}});
    TA.states[1]->next['b'].push_back({TA.states[2], {}, {{{TimedAutomaton::X(1) >= 1}, {TimedAutomaton::X(1) <= 1}}}});
    TA.states[1]->next['c'].push_back({TA.states[3], {}, {{TimedAutomaton::X(0) < 1}}});
    TA.states[2]->next['c'].push_back({TA.states[3], {}, {{TimedAutomaton::X(0) < 1}}});
    TA.states[3]->next['a'].push_back({TA.states[1], {1}, {{TimedAutomaton::X(1) < 1}}});
    TA.states[3]->next['d'].push_back({TA.states[3], {}, {{{TimedAutomaton::X(0) > 1}}}});
  
    TA.maxConstraints = {1,1}; 
  }
};

BOOST_FIXTURE_TEST_CASE( sundaySkipValueTest, TAFixture )
{
  // Run
  SundaySkipValue beta(TA);

  // Comparison
  BOOST_CHECK_EQUAL(beta.getM(), 2);
  std::unordered_set<char> endChars;
  beta.getEndChars(endChars);
  BOOST_TEST(bool(endChars == std::unordered_set<char>{'c'}));
  BOOST_CHECK_EQUAL(beta['a'], 2);
  BOOST_CHECK_EQUAL(beta['b'], 3);
  BOOST_CHECK_EQUAL(beta['c'], 1);
  BOOST_CHECK_EQUAL(beta['d'], 3);
}

BOOST_AUTO_TEST_SUITE_END()
