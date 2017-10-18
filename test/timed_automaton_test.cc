
#include <boost/test/unit_test.hpp>

#include "../tgrep/timed_automaton.hh"

BOOST_AUTO_TEST_SUITE(timedAutomatonTests)

class DeepCopyFixture {
public:
  DeepCopyFixture() {
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
    TA.states[0]->next['a'].push_back({TA.states[0].get(), {1}, {}});
    TA.states[0]->next['a'].push_back({TA.states[1].get(), {}, {{{TimedAutomaton::X(0) >= 1}, {TimedAutomaton::X(0) <= 1}}}});
    TA.states[1]->next['a'].push_back({TA.states[1].get(), {}, {}});
    TA.states[1]->next['a'].push_back({TA.states[2].get(), {}, {{{TimedAutomaton::X(1) >= 1}, {TimedAutomaton::X(1) <= 1}}}});
    TA.states[2]->next['$'].push_back({TA.states[3].get(), {}, {}});
  
    TA.maxConstraints = {1,1}; 

    // Run    
    TA.deepCopy(TA_out, old2new);
  }
  TimedAutomaton TA;
  TimedAutomaton TA_out;
  std::unordered_map<TAState*, std::shared_ptr<TAState>> old2new;
};

BOOST_FIXTURE_TEST_CASE( deepCopyIdenticalTest, DeepCopyFixture )
{

  // Comparison
  BOOST_CHECK_EQUAL(TA.states.size(), TA_out.states.size());
  for (auto oldState: TA.states) {
    auto newState = old2new[oldState.get()];
    BOOST_CHECK_EQUAL(oldState->isMatch, newState->isMatch);
    for (char c = 0; c < CHAR_MAX; ++c) {
      BOOST_CHECK_EQUAL(oldState->next[c].size(), newState->next[c].size());
      for (std::size_t i = 0; i < oldState->next[c].size(); ++i) {
        BOOST_CHECK_EQUAL(old2new[oldState->next[c][i].target].get(), newState->next[c][i].target);
        BOOST_CHECK_EQUAL(oldState->next[c][i].resetVars.size(), newState->next[c][i].resetVars.size());
        BOOST_CHECK_EQUAL(oldState->next[c][i].guard.size(), newState->next[c][i].guard.size());
      }
    }
  }
  
  BOOST_CHECK_EQUAL(TA.initialStates.size(), TA_out.initialStates.size());
  for (auto oldState: TA.initialStates) {
    auto newState = old2new[oldState.get()];
    BOOST_TEST(bool(std::find(TA_out.initialStates.begin(), TA_out.initialStates.end(), newState) != TA_out.initialStates.end()), "An initial state changed");
  }
}

BOOST_FIXTURE_TEST_CASE( deepCopyImmutable, DeepCopyFixture )
{
  TA.states[0]->next['a'][0].resetVars = {};
  BOOST_CHECK_EQUAL(old2new[TA.states[0].get()]->next['a'][0].resetVars.size(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
