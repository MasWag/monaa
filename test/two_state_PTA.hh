#include <ppl.hh>
#include "../libmonaa/parametric_timed_automaton.hh"

class TwoStatesPTAFixture {
public:
  ParametricTimedAutomaton TA;
  TwoStatesPTAFixture() {
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
    TA.states[1]->next['a'].push_back({TA.states[2].get(), {0}, NNC_Polyhedron(3)});
    TA.states[1]->next['a'].back().guard.add_constraints(Constraint_System(x < 1));
    TA.states[2]->next['a'].push_back({TA.states[3].get(), {}, NNC_Polyhedron(3)});
    TA.states[2]->next['a'].back().guard.add_constraints(Constraint_System(x > p));
  
    TA.clockDimensions = 1;
    TA.paramDimensions = 1;
  }
};
