#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only
#include <boost/test/unit_test.hpp>

#include "../libmonaa/parametric_monaa.hh"
//#include "../monaa/timed_automaton_parser.hh"

BOOST_AUTO_TEST_SUITE(parametricTimedFJSTest)

BOOST_AUTO_TEST_CASE(timedFJS) {
  ParametricTimedAutomaton TA;
  TA.states.resize(4);
  for (auto &state: TA.states) {
    state = std::make_shared<PTAState>();
  }

  TA.initialStates = {TA.states[0]};

  TA.states[0]->isMatch = false;
  TA.states[1]->isMatch = false;
  TA.states[2]->isMatch = false;
  TA.states[3]->isMatch = true;

  // Transitions
  Parma_Polyhedra_Library::Variable beginningTimeVariable(0);
  Parma_Polyhedra_Library::Variable x(1);
  Parma_Polyhedra_Library::Variable endTimeVariable(1);
  TA.states[0]->next['a'].push_back({TA.states[1].get(), {0}, Parma_Polyhedra_Library::NNC_Polyhedron(2)});
  TA.states[1]->next['b'].push_back({TA.states[2].get(), {},  Parma_Polyhedra_Library::NNC_Polyhedron(Parma_Polyhedra_Library::Constraint_System(Parma_Polyhedra_Library::Constraint(x < 1)))});
  TA.states[2]->next['$'].push_back({TA.states[3].get(), {}, Parma_Polyhedra_Library::NNC_Polyhedron(2)});

  TA.clockDimensions = 1;
  TA.paramDimensions = 0;

  FILE* file(fopen("../test/timed_word.txt", "r"));
  WordVector<std::pair<Alphabet,double> > w(file, false);
  AnsVec<Parma_Polyhedra_Library::NNC_Polyhedron> ans;
  parametricMonaa(w, TA, ans);
  BOOST_CHECK_EQUAL(ans.size(), 3);

  auto ansPolyhedron = *(ans.begin());
  Parma_Polyhedra_Library::Coefficient n, d;
  bool f;
  ansPolyhedron.maximize(beginningTimeVariable, n, d, f);
  BOOST_CHECK_CLOSE(n.get_d() / d.get_d(), 2.4, 1e-6);
  ansPolyhedron.minimize(beginningTimeVariable, n, d, f);
  BOOST_CHECK_CLOSE(n.get_d() / d.get_d(), 0, 1e-6);

  ansPolyhedron.maximize(endTimeVariable, n, d, f);
  BOOST_CHECK_CLOSE(n.get_d() / d.get_d(), 3.4, 1e-6);
  ansPolyhedron.minimize(endTimeVariable, n, d, f);
  BOOST_CHECK_CLOSE(n.get_d() / d.get_d(), 2.9, 1e-6);

  ansPolyhedron.maximize(endTimeVariable - beginningTimeVariable, n, d, f);
  BOOST_CHECK_CLOSE(n.get_d() / d.get_d(), 3.4, 1e-6);
  ansPolyhedron.minimize(endTimeVariable - beginningTimeVariable, n, d, f);
  BOOST_CHECK_CLOSE(n.get_d() / d.get_d(), 0.5, 1e-6);
}

/*
BOOST_AUTO_TEST_CASE(timedFJSa0_1b0_1) {
  ParamerricTimedAutomaton TA;
  TA.states.resize(3);
  for (auto &state: TA.states) {
    state = std::make_shared<TAState>();
  }

  TA.initialStates = {TA.states[0]};

  TA.states[0]->isMatch = false;
  TA.states[1]->isMatch = false;
  TA.states[2]->isMatch = true;

  // Transitions
  TA.states[0]->next['a'].push_back({TA.states[1].get(), {0}, {{ParamerricTimedAutomaton::X(0) < 1}}});
  TA.states[1]->next['b'].push_back({TA.states[2].get(), {}, {{ParamerricTimedAutomaton::X(0) < 1}}});

  TA.maxConstraints = {1};

  FILE* file(fopen("../test/ascii_test.txt", "r"));
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
  std::ifstream taStream("../test/torque.dot");
  BoostTimedAutomaton BoostTA;
  ParamerricTimedAutomaton TA;
  parseBoostTA(taStream, BoostTA);
  convBoostTA(BoostTA, TA);

  FILE* file(fopen("../test/torque_short.txt", "r"));
  WordVector<std::pair<Alphabet,double> > w(file, false);

  AnsVec<Zone> ans;
  monaa(w, TA, ans);
  BOOST_TEST(ans.size() >  1);
  const auto ansZone = (++ans.begin())->value;
  BOOST_CHECK_CLOSE(ansZone(1, 2).first, -1, 1e-6);
  BOOST_CHECK_CLOSE(ansZone(2, 1).first, 1.00988, 1e-6);
}
*/

BOOST_AUTO_TEST_SUITE_END()
