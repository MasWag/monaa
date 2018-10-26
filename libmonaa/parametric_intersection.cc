#include "parametric_intersection.hh"
#include "intersection.hh"

class Dimension2to1SwapFunction {
public:
  Dimension2to1SwapFunction(const Parma_Polyhedra_Library::dimension_type clockSize1,
                            const Parma_Polyhedra_Library::dimension_type clockSize2,
                            const Parma_Polyhedra_Library::dimension_type paramSize) : clockSize1(clockSize1), clockSize2(clockSize2), paramSize(paramSize) {}
  bool has_empty_codomain() const { return false; }
  Parma_Polyhedra_Library::dimension_type max_in_codomain() const { return clockSize1 + clockSize2 + paramSize; }
  bool maps(Parma_Polyhedra_Library::dimension_type i, Parma_Polyhedra_Library::dimension_type& j) const {
    if (i <= paramSize) {
      j = i;
      return true;
    } else if (i <= paramSize + clockSize2) {
      j = i + clockSize1;
      return true;
    } else if (i <= paramSize + clockSize1 + clockSize2) {
      j = i - clockSize2;
      return true;
    } else {
      return false;
    }
  }
private:
  const Parma_Polyhedra_Library::dimension_type clockSize1, clockSize2, paramSize;
};

class AddProductTransitionsPTA : public AddProductTransitions<PTAState, PTATransition> {
private:
  const std::size_t clockSize1;
  const std::size_t clockSize2;
  std::vector<std::shared_ptr<PTAState>> &states;
  boost::unordered_map<std::pair<PTAState*, PTAState*>, std::shared_ptr<PTAState>> & toIState;

  /*!
    @brief Add product transition

    @param [in] s1 source state in the first automaton
    @param [in] s2 source state in the second automaton
    @param [in] nextS1 target state in the first automaton
    @param [in] nextS2 target state in the second automaton
    @param [in] e1 transition in the first automaton
    @param [in] e2 transition in the second automaton
    @param [in] c character labelled in the constructed transition

    @note This function assumes that states is sorted
  */
  void addProductTransition(PTAState* s1, PTAState* s2, PTAState* nextS1, PTAState* nextS2, const PTATransition& e1, const PTATransition& e2, Alphabet c) {
    PTATransition transition;
    std::shared_ptr<PTAState> target =toIState[std::make_pair(nextS1, nextS2)];

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
    transition.guard = e1.guard;
    auto g2 = e2.guard;

    if (transition.guard.space_dimension()) {
      transition.guard.add_space_dimensions_and_embed(clockSize2);
    } else {
      transition.guard = Parma_Polyhedra_Library::NNC_Polyhedron(g2.space_dimension() + clockSize1);
    }
    if (g2.space_dimension()) {
      g2.add_space_dimensions_and_embed(clockSize1);
      auto dimensionSwapFunc = Dimension2to1SwapFunction(clockSize1, clockSize2, e1.guard.space_dimension() - clockSize1 - 1);
      g2.map_space_dimensions(dimensionSwapFunc);
      transition.guard.intersection_assign(g2);
    } 

    toIState[std::make_pair(s1, s2)]->next[c].push_back(std::move(transition));    
  }
public:
  /*!
    @param [in] clockSize1 Number of the clock variables in the first automaton
    @param [in] clockSize2 Number of the clock variables in the second automaton
    @param [out] states Set of states to write the product states
    @param [in] toIState Mapping from the pair (s1,s2) of the original automata to the state in the product automaton.
  */
  AddProductTransitionsPTA(const std::size_t clockSize1, const std::size_t clockSize2, std::vector<std::shared_ptr<PTAState>> &out,
                           boost::unordered_map<std::pair<PTAState*, PTAState*>, std::shared_ptr<PTAState>> & toIState) : clockSize1(clockSize1), clockSize2(clockSize2), states(out), toIState(toIState) {}
};

/*
  Specifications
  ==============
  * x = x1 or x2 + |C|
  * in1 and in2 can be same.
  */
void intersectionTA (const ParametricTimedAutomaton &in1, const ParametricTimedAutomaton &in2, ParametricTimedAutomaton &out,
                     boost::unordered_map<std::pair<PTAState*, PTAState*>, std::shared_ptr<PTAState>> &toIState)
{
  // toIState :: (in1.State, in2.State) -> out.State

  // make states
  makeProductStates(in1.states, in2.states, toIState);

  // make initial states
  pushProductStates(in1.initialStates, in2.initialStates, toIState, out.initialStates);
  out.states = out.initialStates;

  // make dimensions
#ifdef DEBUG
  assert(in1.paramDimensions == in2.paramDimensions);
#endif
  out.clockDimensions = in1.clockDimensions + in2.clockDimensions;
  out.paramDimensions = in1.paramDimensions;

  // make product transitions
  AddProductTransitionsPTA addProductTransitionsPTA(in1.clockDimensions, in2.clockDimensions, out.states, toIState);
  addProductTransitionsPTA(in1.states, in2.states);
}
