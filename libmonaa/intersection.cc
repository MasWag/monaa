#include "intersection.hh"


/*!
  @brief Add product transition

  @param [in] s1 source state in the first automaton
  @param [in] s2 source state in the second automaton
  @param [in] nextS1 target state in the first automaton
  @param [in] nextS2 target state in the second automaton
  @param [in] e1 transition in the first automaton
  @param [in] e2 transition in the second automaton
  @param [in] c character labelled in the constructed transition
  @param [in] clockSize1 Number of the clock variables in the first automaton
  @param [in] toIState Mapping from the pair (s1,s2) of the original automata to the state in the product automaton.

  @note This function assumes that states is sorted
 */
void addProductTransitionTA(TAState *s1, TAState *s2,
                            TAState *nextS1, TAState *nextS2,
                            const TATransition &e1, const TATransition &e2, char c, std::size_t clockSize1, std::vector<std::shared_ptr<TAState>> &states,
                            boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> &toIState) {
  TATransition transition;
  std::shared_ptr<TAState> target =toIState[std::make_pair(nextS1, nextS2)];

  if (!std::binary_search(states.begin(), states.end(), target)) {
    states.push_back(target);
    std::sort(states.begin(), states.end());
  }

  transition.target = target.get();

  //concat resetVars
  transition.resetVars.reserve (e1.resetVars.size() + e2.resetVars.size());
  transition.resetVars.insert (transition.resetVars.end(),e1.resetVars.begin(), e1.resetVars.end());
  transition.resetVars.insert (transition.resetVars.end(),e2.resetVars.begin(), e2.resetVars.end());
  std::for_each (transition.resetVars.begin () + e1.resetVars.size(),
                 transition.resetVars.end(),
                 [&] (ClockVariables &v) { v += clockSize1;});

  //concat constraints
  transition.guard.reserve (e1.guard.size() + e2.guard.size());
  transition.guard.insert (transition.guard.end(),e1.guard.begin(), e1.guard.end());
  transition.guard.insert (transition.guard.end(),e2.guard.begin(), e2.guard.end());
  std::for_each (transition.guard.begin () + e1.guard.size(),
                 transition.guard.end(),
                 [&] (Constraint &guard) { guard.x += clockSize1;});

  toIState[std::make_pair(s1, s2)]->next[c].push_back(std::move(transition));
}

class AddProductTransitionsTA : public AddProductTransitions<TAState, TATransition> {
private:
  const std::size_t clockSize1;
  std::vector<std::shared_ptr<TAState>> &out;
  boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> & toIState;

  void addProductTransition(TAState* s1, TAState* s2, TAState* nextS1, TAState* nextS2, const TATransition& e1, const TATransition& e2, Alphabet c) {
    addProductTransitionTA(s1, s2, nextS1, nextS2, e1, e2, c, clockSize1, out, toIState);
  }
public:
  AddProductTransitionsTA(const std::size_t clockSize1, std::vector<std::shared_ptr<TAState>> &out,
                          boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> & toIState) : clockSize1(clockSize1), out(out), toIState(toIState) {}
};

/*
  Specifications
  ==============
  * (s,s') is encoded as s + |S| * s'
  * x = x1 or x2 + |C|
  * in1 and in2 can be same.
*/
void intersectionTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out,
                     boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> &toIState)
{
  // toIState :: (in1.State, in2.State) -> out.State

  // make states
  makeProductStates(in1.states, in2.states, toIState);

  // make initial states
  pushProductStates(in1.initialStates, in2.initialStates, toIState, out.initialStates);
  out.states = out.initialStates;

  // make max constraints
  out.maxConstraints.reserve( in1.maxConstraints.size() + in2.maxConstraints.size() ); // preallocate memory
  out.maxConstraints.insert( out.maxConstraints.end(), in1.maxConstraints.begin(), in1.maxConstraints.end() );
  out.maxConstraints.insert( out.maxConstraints.end(), in2.maxConstraints.begin(), in2.maxConstraints.end() );

  // make product transitions
  AddProductTransitionsTA addProductTransitionsTA(in1.clockSize(), out.states, toIState);
  addProductTransitionsTA(in1.states, in2.states);
}

void intersectionSignalTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out)
{
  // toIState :: (in1.State, in2.State) -> out.State
  boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> toIState;

  // make states
  makeProductStates(in1.states, in2.states, toIState);

  // make initial states
  pushProductStates(in1.initialStates, in2.initialStates, toIState, out.initialStates);
  out.states = out.initialStates;

  // make max constraints
  out.maxConstraints.reserve( in1.maxConstraints.size() + in2.maxConstraints.size() ); // preallocate memory
  out.maxConstraints.insert( out.maxConstraints.end(), in1.maxConstraints.begin(), in1.maxConstraints.end() );
  out.maxConstraints.insert( out.maxConstraints.end(), in2.maxConstraints.begin(), in2.maxConstraints.end() );

  TATransition emptyTransition;
  // make edges
  for (auto s1: in1.states) {
    for (auto s2: in2.states) {
      // Epsilon transitions
      for (const auto &e1: s1->next[0]) {
        auto nextS1 = e1.target;
        if (!nextS1) {
          continue;
        }
        addProductTransitionTA(s1.get(), s2.get(), nextS1, s2.get(), e1, emptyTransition, 0, in1.clockSize(), out.states, toIState);
      }
      for (const auto &e2: s2->next[0]) {
        auto nextS2 = e2.target;
        if (!nextS2) {
          continue;
        }
        addProductTransitionTA(s1.get(), s2.get(), s1.get(), nextS2, emptyTransition, e2, 0, in1.clockSize(), out.states, toIState);
      }

      // Observable transitions
      for (auto it1 = s1->next.begin(); it1 != s1->next.end(); it1++) {
        const Alphabet c = it1->first;
        // Syncronous transition
        for (const auto &e1: it1->second) {
          auto nextS1 = e1.target;
          if (!nextS1) {
            continue;
          }
          auto it2 = s2->next.find(c);
          if (it2 == s2->next.end()) {
            continue;
          }
          for (const auto &e2: it2->second) {
            auto nextS2 = e2.target;
            if (!nextS2) {
              continue;
            }
            addProductTransitionTA(s1.get(), s2.get(), nextS1, nextS2, e1, e2, c, in1.clockSize(), out.states, toIState);
          }
        }

        // Asyncronous transition
        if (s2->next.find(c) != s2->next.end()) {
          for (const auto &e1: it1->second) {
            auto nextS1 = e1.target;
            if (!nextS1) {
              continue;
            }
            addProductTransitionTA(s1.get(), s2.get(), nextS1, s2.get(), e1, emptyTransition, 0, in1.clockSize(), out.states, toIState);
          }
        }

        auto it2 = s2->next.find(c);
        if (it2 == s2->next.end()) {
          continue;
        }
        for (const auto &e2: it2->second) {
          auto nextS2 = e2.target;
          if (!nextS2) {
            continue;
          }
          addProductTransitionTA(s1.get(), s2.get(), s1.get(), nextS2, emptyTransition, e2, 0, in1.clockSize(), out.states, toIState);
        }
      }
    }
  }
}
