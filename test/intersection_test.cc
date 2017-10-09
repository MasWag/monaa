
#include <boost/test/unit_test.hpp>

#include "../tgrep/timed_automaton.hh"
#include "../tgrep/intersection.hh"

BOOST_AUTO_TEST_SUITE(intersectionTests)

class IntersectionFixture {
public:
  std::array<TimedAutomaton, 2> TAs;  
  TimedAutomaton out;
  boost::unordered_map<std::pair<std::shared_ptr<TAState>, std::shared_ptr<TAState>>, std::shared_ptr<TAState>> toIState;
  IntersectionFixture() {
    // Input
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

    // Run
    intersectionTA (TAs[0], TAs[1], out, toIState);
  }
};

BOOST_FIXTURE_TEST_CASE( intersectionTest, IntersectionFixture )
{
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

BOOST_FIXTURE_TEST_CASE( updateInitAcceptingTest, IntersectionFixture)
{
  TAs[1].initialStates.clear();
  TAs[1].states[0]->isMatch = true;
  updateInitAccepting(TAs[0], TAs[1], out, toIState);
  BOOST_CHECK_EQUAL(out.initialStates.size(), 0);
  BOOST_CHECK_EQUAL(std::count_if(out.states.begin(), out.states.end(), [](std::shared_ptr<TAState> state) {
        return state->isMatch;
      }), 2);
}

BOOST_AUTO_TEST_SUITE_END()
