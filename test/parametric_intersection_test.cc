
#include <boost/test/unit_test.hpp>

#include "../libmonaa/parametric_intersection.hh"

BOOST_AUTO_TEST_SUITE(ParametricIntersectionTests)

class IntersectionFixture {
public:
  std::array<ParametricTimedAutomaton, 2> TAs;  
  ParametricTimedAutomaton out;
  boost::unordered_map<std::pair<PTAState*, PTAState*>, std::shared_ptr<PTAState>> toIState;
  IntersectionFixture() {
    // Input
    for (auto& A: TAs) {
      A.states.resize(2);
      for (auto &state: A.states) {
        state = std::make_shared<PTAState>();
      }
      A.initialStates = {A.states[0]};
      A.states[0]->isMatch = false;
      A.states[1]->isMatch = true;
      A.paramDimensions = 1;
    }

    using namespace Parma_Polyhedra_Library;

    Variable t(0);
    Variable p(1);
    Variable x(2);
    Variable y(3);
    // Transitions
    TAs[0].states[0]->next['a'].push_back({TAs[0].states[1].get(), {}, NNC_Polyhedron(3)});
    TAs[0].states[0]->next['a'][0].guard.add_constraints(Constraint_System(Parma_Polyhedra_Library::Constraint( x >= p )));
    TAs[1].states[0]->next[0].push_back({TAs[1].states[0].get(), {0}, NNC_Polyhedron(4)});
    TAs[1].states[0]->next[0][0].guard.add_constraints(Constraint_System(Parma_Polyhedra_Library::Constraint( x < p )));
    TAs[1].states[0]->next['a'].push_back({TAs[1].states[1].get(), {}, NNC_Polyhedron(4)});
    TAs[1].states[0]->next['a'][0].guard.add_constraints(Constraint_System(Parma_Polyhedra_Library::Constraint( y > 10 )));

    TAs[0].clockDimensions = 1;
    TAs[1].clockDimensions = 2;
    // Run
    intersectionTA (TAs[0], TAs[1], out, toIState);
  }
};

BOOST_FIXTURE_TEST_CASE( intersectionTest, IntersectionFixture )
{
  using namespace Parma_Polyhedra_Library::IO_Operators;

  // Expected Results
  auto initState = toIState[std::make_pair(TAs[0].states[0].get(), TAs[1].states[0].get())];
  auto acceptState = toIState[std::make_pair(TAs[0].states[1].get(), TAs[1].states[1].get())];

  using namespace Parma_Polyhedra_Library;

  Variable  t(0);
  Variable p1(1);
  Variable p2(2);
  Variable x1(3);
  Variable x2(4);
  Variable y2(5);

  // Comparison
  BOOST_TEST (out.initialStates.size() == 1);
  BOOST_CHECK_EQUAL(out.states.size(), 3);
  BOOST_REQUIRE_EQUAL (out.initialStates[0], initState);
  BOOST_REQUIRE_EQUAL (initState->next[0].size(), 1);
  {
    auto guard = initState->next[0].front().guard;
    BOOST_CHECK_EQUAL(guard.space_dimension(), 6);
    BOOST_TEST(!guard.is_empty());
    BOOST_TEST(!guard.is_universe());
    auto expected = NNC_Polyhedron(6, UNIVERSE);
    expected.add_constraint(x2 < p2);
    BOOST_TEST((guard == expected));
    // std::cout << "guard: " << guard << std::endl;
    // std::cout << "expected: " << expected << std::endl;
  }
  BOOST_CHECK_EQUAL (initState->next['a'].size(), 1);
  {
    auto guard = initState->next['a'].front().guard;
    BOOST_CHECK_EQUAL(guard.space_dimension(), 6);
    BOOST_TEST(!guard.is_empty());
    BOOST_TEST(!guard.is_universe());
    auto expected = NNC_Polyhedron(6, UNIVERSE);
    expected.add_constraint(x1 >= p1);
    expected.add_constraint(y2 > 10);
    BOOST_TEST((guard == expected));
    // std::cout << "guard: " << guard << std::endl;
    // std::cout << "expected: " << expected << std::endl;
  }
  BOOST_TEST (initState->next[0].front().target == initState.get());
  BOOST_TEST (initState->next['a'].front().target == acceptState.get());
  BOOST_TEST (initState->isMatch == false);
  BOOST_TEST (acceptState->isMatch == true);
}

BOOST_AUTO_TEST_SUITE_END()
