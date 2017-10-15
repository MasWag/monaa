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
  WordVector<std::pair<Alphabet,double> > w(file, false);
  AnsVec<Zone> ans;
  timedFranekJenningsSmyth(w, TA, ans);
  BOOST_CHECK_EQUAL(ans.size(), 2);
}

BOOST_AUTO_TEST_CASE(timedFJSa0_1b0_1) {
  TimedAutomaton TA;
  TA.states.resize(3);
  for (auto &state: TA.states) {
    state = std::make_shared<TAState>();
  }

  TA.initialStates = {TA.states[0]};

  TA.states[0]->isMatch = false;
  TA.states[1]->isMatch = false;
  TA.states[2]->isMatch = true;

  // Transitions
  TA.states[0]->next['a'].push_back({TA.states[1], {0}, {{TimedAutomaton::X(0) < 1}}});
  TA.states[1]->next['b'].push_back({TA.states[2], {}, {{TimedAutomaton::X(0) < 1}}});

  TA.maxConstraints = {1};

  FILE* file(fopen("../test/ascii_test.txt", "r"));
  WordVector<std::pair<Alphabet,double> > w(file, false);
  AnsVec<Zone> ans;
  timedFranekJenningsSmyth(w, TA, ans);
  BOOST_CHECK_EQUAL(ans.size(), 1);
  const auto ansZone = ans.begin()->value;
  BOOST_CHECK_CLOSE(ansZone(0, 1).first, -0.1, 1e-6);
  BOOST_CHECK_EQUAL(ansZone(1 ,0).first, 1.1);
  BOOST_CHECK_EQUAL(ansZone(0, 2).first, -1.1);
  BOOST_CHECK_EQUAL(ansZone(2 ,0).first, 2.1);
}

BOOST_AUTO_TEST_SUITE_END()
