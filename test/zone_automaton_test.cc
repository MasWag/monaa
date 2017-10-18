
#include <boost/test/unit_test.hpp>

#include "../tgrep/zone_automaton.hh"
#include "../tgrep/ta2za.hh"

BOOST_AUTO_TEST_SUITE(ZoneAutomatonTests)

class TAFixture {
public:
  TimedAutomaton TA;
  const int m = 2;
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
    TA.states[0]->next['a'].push_back({TA.states[1].get(), {1}, {}});
    TA.states[1]->next['b'].push_back({TA.states[2].get(), {}, {{{TimedAutomaton::X(1) >= 1}, {TimedAutomaton::X(1) <= 1}}}});
    TA.states[1]->next['c'].push_back({TA.states[3].get(), {}, {{TimedAutomaton::X(0) < 1}}});
    TA.states[2]->next['c'].push_back({TA.states[3].get(), {}, {{TimedAutomaton::X(0) < 1}}});
    TA.states[3]->next['a'].push_back({TA.states[1].get(), {1}, {{TimedAutomaton::X(1) < 1}}});
    TA.states[3]->next['d'].push_back({TA.states[3].get(), {}, {{{TimedAutomaton::X(0) > 1}}}});
  
    TA.maxConstraints = {1,1}; 
  }
};

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

BOOST_FIXTURE_TEST_CASE( emptyTestNonEmpty, TAFixture )
{
  ZoneAutomaton ZA;
  ta2za(TA, ZA);
  BOOST_TEST(!ZA.empty());
}

BOOST_FIXTURE_TEST_CASE( emptyTestEmpty, TAFixture )
{
  TA.states[1]->next['c'].clear();
  ZoneAutomaton ZA;
  ta2za(TA, ZA);
  BOOST_TEST(ZA.empty());
}
BOOST_AUTO_TEST_SUITE_END()
