#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only
#include <filesystem>
#include <boost/test/unit_test.hpp>

#include "../libmonaa/monaa.hh"
#include "../monaa/timed_automaton_parser.hh"

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
  TA.states[0]->next['a'].push_back({TA.states[1].get(), {0}, {}});
  TA.states[1]->next['b'].push_back({TA.states[2].get(), {}, {{TimedAutomaton::X(0) < 1}}});
  TA.states[2]->next['a'].push_back({TA.states[3].get(), {}, {}});

  TA.maxConstraints = {1};

  std::filesystem::path inputPath = std::filesystem::path{PROJECT_ROOT_DIR}.append("test").append("timed_word.txt");
  FILE* file(fopen(inputPath.c_str(), "r"));
  WordVector<std::pair<Alphabet,double> > w(file, false);
  AnsVec<Zone> ans;
  monaa(w, TA, ans);
  BOOST_CHECK_EQUAL(ans.size(), 2);

  auto ansZone = ans.begin()->value;
  BOOST_TEST(bool(ansZone(0, 1) == Bounds{0, true}));
  BOOST_TEST(bool(ansZone(1, 0) == Bounds{2.4, false}));
  BOOST_TEST(bool(ansZone(0, 2) == Bounds{-2.9, false}));
  BOOST_TEST(bool(ansZone(2, 0) == Bounds{3.4, true}));
  BOOST_TEST(bool(ansZone(1, 2) == Bounds{-0.5, false}));
  BOOST_TEST(bool(ansZone(2, 1) == Bounds{3.4, true}));
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
  TA.states[0]->next['a'].push_back({TA.states[1].get(), {0}, {{TimedAutomaton::X(0) < 1}}});
  TA.states[1]->next['b'].push_back({TA.states[2].get(), {}, {{TimedAutomaton::X(0) < 1}}});

  TA.maxConstraints = {1};

  std::filesystem::path inputPath = std::filesystem::path{PROJECT_ROOT_DIR}.append("test").append("ascii_test.txt");
  FILE* file(fopen(inputPath.c_str(), "r"));
  WordVector<std::pair<Alphabet,double> > w(file, false);
  AnsVec<Zone> ans;
  monaa(w, TA, ans);
  BOOST_CHECK_EQUAL(ans.size(), 1);
  const auto ansZone = ans.begin()->value;
  BOOST_CHECK_CLOSE(ansZone(0, 1).first, -0.1, 1e-6);
  BOOST_CHECK_EQUAL(ansZone(1 ,0).first, 1.1);
  BOOST_CHECK_EQUAL(ansZone(0, 2).first, -1.1);
  BOOST_CHECK_EQUAL(ansZone(2 ,0).first, 2.1);
  BOOST_CHECK_EQUAL(ansZone(1, 2).first, 0);
  BOOST_CHECK_EQUAL(ansZone(2 ,1).first, 2);
}

BOOST_AUTO_TEST_CASE(timedFJSTorque) {
  std::filesystem::path inputPath = std::filesystem::path{PROJECT_ROOT_DIR}.append("test").append("torque.dot");
  std::ifstream taStream(inputPath);
  BoostTimedAutomaton BoostTA;
  TimedAutomaton TA;
  parseBoostTA(taStream, BoostTA);
  convBoostTA(BoostTA, TA);

  inputPath = std::filesystem::path{PROJECT_ROOT_DIR}.append("test").append("torque_short.txt");
  FILE* file(fopen(inputPath.c_str(), "r"));
  WordVector<std::pair<Alphabet,double> > w(file, false);

  AnsVec<Zone> ans;
  monaa(w, TA, ans);
  BOOST_TEST(ans.size() >  1);
  const auto ansZone = (++ans.begin())->value;
  BOOST_CHECK_CLOSE(ansZone(1, 2).first, -1, 1e-6);
  BOOST_CHECK_CLOSE(ansZone(2, 1).first, 1.00988, 1e-6);
}

BOOST_AUTO_TEST_SUITE_END()
