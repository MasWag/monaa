#include <boost/test/unit_test.hpp>

#include "../tgrep/timedFJS.hh"

BOOST_AUTO_TEST_SUITE(timedFJSTest)

BOOST_AUTO_TEST_CASE(timedFJS) {
  TimedAutomaton TA;
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
  TA.states[0]->next['a'].push_back({TA.states[1], {0}, {}});
  TA.states[1]->next['b'].push_back({TA.states[2], {}, {{TimedAutomaton::X(0) < 1}}});
  TA.states[2]->next['a'].push_back({TA.states[3], {}, {}});

  TA.maxConstraints = {1};

  FILE* file(fopen("../test/timed_word.txt", "r"));
  WordVector<std::pair<Alphabet,double> > w(8, file, false);
  AnsVec<Zone> ans;
  timedFranekJenningsSmyth(w, TA, ans);
  BOOST_CHECK_EQUAL(ans.size(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
