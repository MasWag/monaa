#include <boost/test/unit_test.hpp>

#include "../libmonaa/parametric_kmp_skip_value.hh"

BOOST_AUTO_TEST_SUITE(ParametricKMPSkipValuePairTests)

#include "../test/two_state_PTA.hh"

BOOST_FIXTURE_TEST_CASE( ParametricKMPSkipValuePairTwoStatesTest, TwoStatesPTAFixture )
{
  // Run
  ParametricKMPSkipValue delta(TA, 2);

  Parma_Polyhedra_Library::Variable p(1);

  // Comparison
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 0);
    BOOST_CHECK_EQUAL(delta.at(TA.states[0].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 1000);
    BOOST_CHECK_EQUAL(delta.at(TA.states[0].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 0);
    BOOST_CHECK_EQUAL(delta.at(TA.states[1].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 1000);
    BOOST_CHECK_EQUAL(delta.at(TA.states[1].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 0);
    BOOST_CHECK_EQUAL(delta.at(TA.states[2].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 1000);
    BOOST_CHECK_EQUAL(delta.at(TA.states[2].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 0);
    BOOST_CHECK_EQUAL(delta.at(TA.states[3].get(), constraint), 1);
  }
  {
    Parma_Polyhedra_Library::NNC_Polyhedron constraint(2);
    constraint.add_constraint(p > 1000);
    BOOST_CHECK_EQUAL(delta.at(TA.states[3].get(), constraint), 2);
  }
}

BOOST_AUTO_TEST_SUITE_END()
