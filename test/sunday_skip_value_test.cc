
#include <boost/test/unit_test.hpp>
#include <ppl.hh>

#include "../libmonaa/timed_automaton.hh"
#include "../libmonaa/pta2pza.hh"
#include "../libmonaa/sunday_skip_value.hh"
#include "../libmonaa/parametric_timed_automaton.hh"

BOOST_AUTO_TEST_SUITE(sundaySkipValueTests)

class TAFixture {
public:
  TimedAutomaton TA;
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

BOOST_FIXTURE_TEST_CASE( sundaySkipValueTest, TAFixture )
{
  // Run
  SundaySkipValue beta(TA);

  // Comparison
  BOOST_CHECK_EQUAL(beta.getM(), 2);
  std::unordered_set<char> endChars;
  beta.getEndChars(endChars);
  BOOST_TEST(bool(endChars == std::unordered_set<char>{'c'}));
  BOOST_CHECK_EQUAL(beta['a'], 2);
  BOOST_CHECK_EQUAL(beta['b'], 3);
  BOOST_CHECK_EQUAL(beta['c'], 1);
  BOOST_CHECK_EQUAL(beta['d'], 3);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ParametricSundaySkipValueTests)

using PSundaySkipValue = SundaySkipValueTemplate<ParametricTimedAutomaton, typename Parma_Polyhedra_Library::NNC_Polyhedron>;

class PTAFixture {
public:
  ParametricTimedAutomaton TA;
  PTAFixture() {
    using namespace Parma_Polyhedra_Library;
    // Input
    TA.states.resize(4);
    for (auto &state: TA.states) {
      state = std::make_shared<PTAState>();
    }

    TA.initialStates = {TA.states[0]};

    TA.states[0]->isMatch = false;
    TA.states[1]->isMatch = false;
    TA.states[2]->isMatch = false;
    TA.states[3]->isMatch = true;

    Variable p(1), x(2);
    // Transitions
    TA.states[0]->next['a'].push_back({TA.states[1].get(), {0}, NNC_Polyhedron(3)});
    TA.states[1]->next['b'].push_back({TA.states[2].get(), {}, NNC_Polyhedron(3)});
    TA.states[1]->next['b'].back().guard.add_constraints(Constraint_System(x == p));
    TA.states[1]->next['c'].push_back({TA.states[3].get(), {}, NNC_Polyhedron(3)});
    TA.states[1]->next['c'].back().guard.add_constraints(Constraint_System(x < 1));
    TA.states[2]->next['c'].push_back({TA.states[3].get(), {}, NNC_Polyhedron(3)});
    TA.states[2]->next['c'].back().guard.add_constraints(Constraint_System(x < p));
    TA.states[3]->next['a'].push_back({TA.states[1].get(), {0}, NNC_Polyhedron(3)});
    TA.states[3]->next['a'].back().guard.add_constraints(Constraint_System(x < 1));
    TA.states[3]->next['d'].push_back({TA.states[3].get(), {}, NNC_Polyhedron(3)});
    TA.states[3]->next['d'].back().guard.add_constraints(Constraint_System(x > 1));
  
    TA.clockDimensions = 1;
    TA.paramDimensions = 1;
  }
};

BOOST_FIXTURE_TEST_CASE( ParametricSundaySkipValueTest, PTAFixture )
{
  // Run
  PSundaySkipValue beta(TA);

  // Comparison
  BOOST_CHECK_EQUAL(beta.getM(), 2);
  std::unordered_set<char> endChars;
  beta.getEndChars(endChars);
  BOOST_TEST(bool(endChars == std::unordered_set<char>{'c'}));
  BOOST_CHECK_EQUAL(beta['a'], 2);
  BOOST_CHECK_EQUAL(beta['b'], 3);
  BOOST_CHECK_EQUAL(beta['c'], 1);
  BOOST_CHECK_EQUAL(beta['d'], 3);
}

BOOST_AUTO_TEST_SUITE_END()
