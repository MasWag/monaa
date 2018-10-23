#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../monaa/parametric_timed_automaton_parser.hh"

std::ostream& operator<<(std::ostream& os, const std::vector<ClockVariables> &resetVars);

BOOST_AUTO_TEST_SUITE(ParametricTimedAutomatonParserTests)
BOOST_AUTO_TEST_CASE(ParseParametricSmall)
{
  BoostParametricTimedAutomaton BoostTA;
  std::ifstream file("../test/p_small.dot");
  parseBoostTA(file, BoostTA);

  BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_clock_dimensions), 1);
  BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_param_dimensions), 2);

  BOOST_REQUIRE_EQUAL(boost::num_vertices(BoostTA), 4);
  BOOST_TEST(!BoostTA[0].isMatch);
  BOOST_TEST(!BoostTA[1].isMatch);
  BOOST_TEST(!BoostTA[2].isMatch);
  BOOST_TEST( BoostTA[3].isMatch);

  BOOST_TEST( BoostTA[0].isInit);
  BOOST_TEST(!BoostTA[1].isInit);
  BOOST_TEST(!BoostTA[2].isInit);
  BOOST_TEST(!BoostTA[3].isInit);

  Parma_Polyhedra_Library::Variable p(1), q(2), x(3);
  Parma_Polyhedra_Library::Constraint_System expected;

  {// transition 1
    auto transition = boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first;
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transition), 'l');
    BOOST_REQUIRE_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars.size(), 1);
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars[0], 0);

    auto cs = boost::get(&BoostPTATransition::guard, BoostTA, transition);
    BOOST_TEST(!cs.empty());
    BOOST_CHECK_EQUAL(cs.space_dimension(), 4);

    expected = Parma_Polyhedra_Library::Constraint_System(x < p);
    BOOST_CHECK_EQUAL(cs, expected);
  }

  {// transition 2
    auto transition = boost::edge(boost::vertex(1, BoostTA), boost::vertex(2, BoostTA), BoostTA).first;
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transition), 'h');
    BOOST_REQUIRE_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars.size(), 0);

    auto cs = boost::get(&BoostPTATransition::guard, BoostTA, transition);
    BOOST_TEST(!cs.empty());
    BOOST_CHECK_EQUAL(cs.space_dimension(), 4);

    expected = Parma_Polyhedra_Library::Constraint_System(x < q);
    expected.insert(x < 1);
    BOOST_CHECK_EQUAL(cs, expected);
  }

  {// transition 3
    auto transition = boost::edge(boost::vertex(2, BoostTA), boost::vertex(1, BoostTA), BoostTA).first;
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transition), 'l');
    BOOST_REQUIRE_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars.size(), 0);

    auto cs = boost::get(&BoostPTATransition::guard, BoostTA, transition);
    BOOST_TEST(!cs.empty());
    BOOST_CHECK_EQUAL(cs.space_dimension(), 4);

    expected = Parma_Polyhedra_Library::Constraint_System(x < 1);
    BOOST_CHECK_EQUAL(cs, expected);
  }

  {// transition 4
    auto transition = boost::edge(boost::vertex(2, BoostTA), boost::vertex(3, BoostTA), BoostTA).first;
    BOOST_CHECK_EQUAL(boost::get(&BoostPTATransition::c, BoostTA, transition), '$');
    BOOST_REQUIRE_EQUAL(boost::get(&BoostPTATransition::resetVars, BoostTA, transition).resetVars.size(), 0);

    auto cs = boost::get(&BoostPTATransition::guard, BoostTA, transition);
    BOOST_TEST(cs.empty());
    BOOST_CHECK_EQUAL(cs.space_dimension(), 0);
    expected = Parma_Polyhedra_Library::Constraint_System();
    BOOST_CHECK_EQUAL(cs, expected);
  }
}

#if 0

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
#endif

BOOST_AUTO_TEST_CASE(ParseAndConvParametricSmall)
{
  BoostParametricTimedAutomaton BoostTA;
  ParametricTimedAutomaton TA;
  std::ifstream file("../test/p_small.dot");
  parseBoostTA(file, BoostTA);
  convBoostTA(BoostTA, TA);

  BOOST_CHECK_EQUAL(TA.clockDimensions, 1);
  BOOST_CHECK_EQUAL(TA.paramDimensions, 2);

  BOOST_REQUIRE_EQUAL(TA.states.size(), 4);
  BOOST_TEST(!TA.states[0]->isMatch);
  BOOST_TEST(!TA.states[1]->isMatch);
  BOOST_TEST(!TA.states[2]->isMatch);
  BOOST_TEST( TA.states[3]->isMatch);

  BOOST_REQUIRE_EQUAL(TA.initialStates.size(), 1);
  BOOST_CHECK_EQUAL(TA.states[0], TA.initialStates[0]);

  Parma_Polyhedra_Library::Variable p(1), q(2), x(3);
  Parma_Polyhedra_Library::Constraint_System expected;

  {// transition 1
    BOOST_REQUIRE_EQUAL(TA.states[0]->next['l'].size(), 1);
    auto transition = TA.states[0]->next['l'][0];
    BOOST_REQUIRE_EQUAL(transition.resetVars.size(), 1);
    BOOST_CHECK_EQUAL(transition.resetVars[0], 0);

    auto polynomial = transition.guard;
    BOOST_TEST(!polynomial.is_empty());
    BOOST_TEST(!polynomial.is_universe());
    BOOST_CHECK_EQUAL(polynomial.space_dimension(), 4);

    expected = Parma_Polyhedra_Library::Constraint_System(x < p);
    BOOST_CHECK_EQUAL(polynomial, Parma_Polyhedra_Library::NNC_Polyhedron(expected));
  }

  {// transition 2
    BOOST_REQUIRE_EQUAL(TA.states[1]->next['h'].size(), 1);
    auto transition = TA.states[1]->next['h'][0];
    BOOST_REQUIRE_EQUAL(transition.resetVars.size(), 0);

    auto polynomial = transition.guard;
    BOOST_TEST(!polynomial.is_empty());
    BOOST_TEST(!polynomial.is_universe());
    BOOST_CHECK_EQUAL(polynomial.space_dimension(), 4);

    expected = Parma_Polyhedra_Library::Constraint_System(x < q);
    expected.insert(x < 1);
    BOOST_CHECK_EQUAL(polynomial, Parma_Polyhedra_Library::NNC_Polyhedron(expected));
  }

  {// transition 3
    BOOST_REQUIRE_EQUAL(TA.states[2]->next['l'].size(), 1);
    auto transition = TA.states[2]->next['l'][0];
    BOOST_REQUIRE_EQUAL(transition.resetVars.size(), 0);

    auto polynomial = transition.guard;
    BOOST_TEST(!polynomial.is_empty());
    BOOST_TEST(!polynomial.is_universe());
    BOOST_CHECK_EQUAL(polynomial.space_dimension(), 4);

    expected = Parma_Polyhedra_Library::Constraint_System(x < 1);
    BOOST_CHECK_EQUAL(polynomial, Parma_Polyhedra_Library::NNC_Polyhedron(expected));
  }

  {// transition 4
    BOOST_REQUIRE_EQUAL(TA.states[2]->next['$'].size(), 1);
    auto transition = TA.states[2]->next['$'][0];
    BOOST_REQUIRE_EQUAL(transition.resetVars.size(), 0);

    auto polynomial = transition.guard;
    BOOST_TEST(!polynomial.is_empty());
    BOOST_TEST(polynomial.is_universe());
    BOOST_CHECK_EQUAL(polynomial.space_dimension(), 4);
  }
}

BOOST_AUTO_TEST_SUITE_END()
