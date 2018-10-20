#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../monaa/parametric_timed_automaton_parser.hh"

std::ostream& operator<<(std::ostream& os, const std::vector<ClockVariables> &resetVars);
std::ostream & operator<< (std::ostream &s, const Parma_Polyhedra_Library::Constraint_System &cs);

BOOST_AUTO_TEST_SUITE(ParametricTimedAutomatonParserTests)
BOOST_AUTO_TEST_CASE(parseBoostPhi7TATest)
{
  Parma_Polyhedra_Library::Constraint_System cs;
  Parma_Polyhedra_Library::Constraint c;
  Parma_Polyhedra_Library::Variable x(0);
  Parma_Polyhedra_Library::Variable y(1);


  c = Parma_Polyhedra_Library::Constraint(x < 3);
  c.ascii_dump();

  std::stringstream sstr;
  sstr << "{x < 3}";
  cs.ascii_load(sstr);
  cs.ascii_dump();

  BoostParametricTimedAutomaton BoostTA;
  std::ifstream file("../test/p_phi7.dot");
  parseBoostTA(file, BoostTA);

  BOOST_REQUIRE_EQUAL(boost::num_vertices(BoostTA), 3);

  BOOST_TEST(!BoostTA[0].isMatch);
  BOOST_TEST(!BoostTA[1].isMatch);
  BOOST_TEST( BoostTA[2].isMatch);
  BOOST_TEST( BoostTA[0].isInit);
  BOOST_TEST(!BoostTA[1].isInit);
  BOOST_TEST(!BoostTA[2].isInit);
  auto transition = boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first;
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transition), 'A');
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars.size(), 1);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars[0], 0);

  //  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition).size(), 0);
}

#if 0
BOOST_AUTO_TEST_CASE(parseBoostTATest)
{
  BoostParametricTimedAutomaton BoostTA;
  std::ifstream file("../test/timed_automaton.dot");
  parseBoostTA(file, BoostTA);

  BOOST_TEST(!BoostTA[0].isMatch);
  BOOST_TEST( BoostTA[1].isMatch);
  BOOST_TEST( BoostTA[0].isInit);
  BOOST_TEST(!BoostTA[1].isInit);
  auto transition = boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first;
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transition), 'a');
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars.size(), 2);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars[0], 0);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars[1], 1);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition).size(), 2);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition)[0].x, 0);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition)[0].c, 1);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition)[0].odr, Constraint::Order::gt);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition)[1].x, 2);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition)[1].c, 10);
  BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transition)[1].odr, Constraint::Order::lt);
}

BOOST_AUTO_TEST_CASE(parseBoostTASimpleTest)
{
  BoostParametricTimedAutomaton BoostTA;
  std::ifstream file("../test/small.dot");
  parseBoostTA(file, BoostTA);

  std::array<bool, 4> initResult  = {{true, false, false, false}};
  std::array<bool, 4> matchResult = {{false, false, false, true}};

  for (int i = 0; i < 4; i++) {
    BOOST_CHECK_EQUAL(BoostTA[i].isInit,  initResult[i]);
    BOOST_CHECK_EQUAL(BoostTA[i].isMatch, matchResult[i]);
  }

  std::array<BoostParametricTimedAutomaton::edge_descriptor, 4> transitions = {{
      boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first,
      boost::edge(boost::vertex(1, BoostTA), boost::vertex(2, BoostTA), BoostTA).first,
      boost::edge(boost::vertex(2, BoostTA), boost::vertex(1, BoostTA), BoostTA).first,
      boost::edge(boost::vertex(2, BoostTA), boost::vertex(3, BoostTA), BoostTA).first
    }};

  std::array<Alphabet, 4> labelResult  = {{'l', 'h', 'l', '$'}};
  std::array<std::size_t, 4> resetVarNumResult  = {{0, 0, 0, 0}};
  std::array<std::size_t, 4> guardNumResult  = {{1, 1, 1, 1}};
  std::array<int, 4> guardXResult  = {{0, 0, 0, 0}};
  std::array<int, 4> guardCResult  = {{1, 1, 1, 1}};
  std::array<Constraint::Order, 4> guardOdrResult  = {{Constraint::Order::lt,
                                                       Constraint::Order::lt, 
                                                       Constraint::Order::lt, 
                                                       Constraint::Order::lt}};

  for (int i = 0; i < 4; i++) {
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transitions[i]), labelResult[i]);
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transitions[i]).resetVars.size(), resetVarNumResult[i]);
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transitions[i]).size(), guardNumResult[i]);
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transitions[i])[0].x, guardXResult[i]);
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transitions[i])[0].c, guardCResult[i]);
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::guard, BoostTA, transitions[i])[0].odr, guardOdrResult[i]);
  }
}

BOOST_AUTO_TEST_CASE(convBoostTATest)
{
  BoostParametricTimedAutomaton BoostTA;
  TimedAutomaton TA;
  std::ifstream file("../test/timed_automaton.dot");
  parseBoostTA(file, BoostTA);
  convBoostTA(BoostTA, TA);

  BOOST_CHECK_EQUAL(TA.stateSize(), 2);
  BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
  BOOST_CHECK_EQUAL(TA.clockSize(), 3);
  auto initialState = TA.initialStates[0];
  BOOST_TEST(!initialState->isMatch);
  BOOST_CHECK_EQUAL(initialState->next['a'].size(), 1);
  auto toAcceptingState = initialState->next['a'][0];
  BOOST_CHECK_EQUAL(toAcceptingState.resetVars.size(), 2);
  BOOST_CHECK_EQUAL(toAcceptingState.resetVars[0], 0);
  BOOST_CHECK_EQUAL(toAcceptingState.resetVars[1], 1);
  BOOST_CHECK_EQUAL(toAcceptingState.guard.size(), 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].x, 0);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].c, 1);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].odr, Constraint::Order::gt);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].x, 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].c, 10);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].odr, Constraint::Order::lt);
  auto acceptingState = toAcceptingState.target;
  BOOST_TEST(acceptingState->isMatch);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
