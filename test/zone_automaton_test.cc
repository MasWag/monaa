
#include <boost/test/unit_test.hpp>

#include "../tgrep/zone_automaton.hh"

BOOST_AUTO_TEST_SUITE(ZoneAutomatonTests)

BOOST_AUTO_TEST_CASE(removeDeadStatesTest)
{
  // Input
  ZoneAutomaton ZA;
  ZA.states.resize(3);
  for (auto &state: ZA.states) {
    state = std::make_shared<ZAState>();
  }
  ZA.initialStates.push_back(ZA.states[0]);

  ZA.states[0]->isMatch = false;
  ZA.states[1]->isMatch = false;
  ZA.states[2]->isMatch = true;

  // Transitions
  ZA.states[0]->next['a'].push_back(ZA.states[1]);
  ZA.states[0]->next['a'].push_back(ZA.states[2]);

  // Run
  ZA.removeDeadStates();

  // Comparison
  BOOST_TEST (ZA.stateSize() == 2);
  BOOST_TEST (ZA.initialStates.size() == 1);
}

BOOST_AUTO_TEST_CASE(epsilonClosureTest)
{
  // Input
  ZoneAutomaton ZA;
  ZA.states.resize(3);
  for (auto &state: ZA.states) {
    state = std::make_shared<ZAState>();
  }
  ZA.initialStates.push_back(ZA.states[0]);

  ZA.states[0]->isMatch = false;
  ZA.states[1]->isMatch = false;
  ZA.states[2]->isMatch = true;

  // Transitions
  ZA.states[0]->next[0].push_back(ZA.states[0]);
  ZA.states[0]->next[0].push_back(ZA.states[1]);
  ZA.states[0]->next['a'].push_back(ZA.states[2]);

  std::unordered_set<std::shared_ptr<ZAState>> closure;
  closure.insert(ZA.states[0]);

  // Run
  epsilonClosure(closure);

  // Comparison
  BOOST_TEST (closure.size() == 2);
}

BOOST_AUTO_TEST_SUITE_END()
