
class LongConstraintTAFixture {
public:
  TimedAutomaton TA;
  const int m = 3;
  LongConstraintTAFixture() {
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
    TA.states[1]->next['a'].push_back({TA.states[2].get(), {}, {{{TimedAutomaton::X(0) < 1}}}});
    TA.states[2]->next['a'].push_back({TA.states[3].get(), {}, {{TimedAutomaton::X(1) > 1}}});
  
    TA.maxConstraints = {1,1}; 
  }
};
