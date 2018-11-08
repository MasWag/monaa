#include "parametric_intersection.hh"
#include "intersection.hh"

class DimensionFrom2SwapFunction {
public:
  DimensionFrom2SwapFunction(const Parma_Polyhedra_Library::dimension_type clockSize1,
                             const Parma_Polyhedra_Library::dimension_type clockSize2,
                             const Parma_Polyhedra_Library::dimension_type paramSize1,
                             const Parma_Polyhedra_Library::dimension_type paramSize2) : clockSize1(clockSize1), clockSize2(clockSize2), paramSize1(paramSize1), paramSize2(paramSize2){}
  bool has_empty_codomain() const { return false; }
  Parma_Polyhedra_Library::dimension_type max_in_codomain() const { return clockSize1 + clockSize2 + paramSize1 + paramSize2; }
  bool maps(Parma_Polyhedra_Library::dimension_type i, Parma_Polyhedra_Library::dimension_type& j) const {
    if (i <= paramSize2) {
      j = i + paramSize1;
      return true;
    } else if (i <= paramSize2 + clockSize2) {
      j = i + paramSize1 + clockSize1;
      return true;
    } else if (i <= paramSize2 + clockSize2 + paramSize1) {
      j = i - paramSize2 - clockSize2;
      return true;
    } else if (i <= paramSize2 + clockSize2 + paramSize1 + clockSize1) {
      j = i - clockSize2;
      return true;
    } else {
      return false;
    }
  }
private:
  const Parma_Polyhedra_Library::dimension_type clockSize1, clockSize2, paramSize1, paramSize2;
};

class DimensionFrom1SwapFunction {
public:
  DimensionFrom1SwapFunction(const Parma_Polyhedra_Library::dimension_type clockSize1,
                             const Parma_Polyhedra_Library::dimension_type clockSize2,
                             const Parma_Polyhedra_Library::dimension_type paramSize1,
                             const Parma_Polyhedra_Library::dimension_type paramSize2) : clockSize1(clockSize1), clockSize2(clockSize2), paramSize1(paramSize1), paramSize2(paramSize2){}
  bool has_empty_codomain() const { return false; }
  Parma_Polyhedra_Library::dimension_type max_in_codomain() const { return clockSize1 + clockSize2 + paramSize1 + paramSize2; }
  bool maps(Parma_Polyhedra_Library::dimension_type i, Parma_Polyhedra_Library::dimension_type& j) const {
    if (i <= paramSize1) {
      j = i;
      return true;
    } else if (i <= paramSize1 + clockSize1) {
      j = i + paramSize2;
      return true;
    } else if (i <= paramSize1 + clockSize1 + paramSize2) {
      j = i - paramSize2;
      return true;
    } else if (i <= paramSize1 + clockSize1 + paramSize2 + clockSize2) {
      j = i;
      return true;
    } else {
      return false;
    }
  }
private:
  const Parma_Polyhedra_Library::dimension_type clockSize1, clockSize2, paramSize1, paramSize2;
};

class AddProductTransitionsPTA : public AddProductTransitions<PTAState, PTATransition> {
private:
  const std::size_t clockSize1;
  const std::size_t clockSize2;
  const std::size_t paramSize1;
  const std::size_t paramSize2;
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
      transition.guard.add_space_dimensions_and_embed(paramSize2 + clockSize2);
      auto dimensionSwapFunc = DimensionFrom1SwapFunction(clockSize1, clockSize2, paramSize1, paramSize2);
      transition.guard.map_space_dimensions(dimensionSwapFunc);
    } else {
      transition.guard = Parma_Polyhedra_Library::NNC_Polyhedron(1 + paramSize1 + clockSize1 + paramSize2 + clockSize2);
    }
    if (g2.space_dimension() && !g2.is_universe()) {
      g2.add_space_dimensions_and_embed(paramSize1 + clockSize1);
      auto dimensionSwapFunc = DimensionFrom2SwapFunction(clockSize1, clockSize2, paramSize1, paramSize2);
      using namespace Parma_Polyhedra_Library::IO_Operators;
      g2.map_space_dimensions(dimensionSwapFunc);
      transition.guard.intersection_assign(g2);
    } 

    toIState[std::make_pair(s1, s2)]->next[c].push_back(std::move(transition));    
  }
public:
  /*!
    @param [in] clockSize1 Number of the clock variables in the first automaton
    @param [in] clockSize2 Number of the clock variables in the second automaton
    @param [in] paramSize1 Number of the parameter variables in the first automaton
    @param [in] paramSize2 Number of the parameter variables in the second automaton
    @param [out] states Set of states to write the product states
    @param [in] toIState Mapping from the pair (s1,s2) of the original automata to the state in the product automaton.
  */
  AddProductTransitionsPTA(const std::size_t clockSize1, const std::size_t clockSize2, const std::size_t paramSize1, const std::size_t paramSize2, std::vector<std::shared_ptr<PTAState>> &out,
                           boost::unordered_map<std::pair<PTAState*, PTAState*>, std::shared_ptr<PTAState>> & toIState) : clockSize1(clockSize1), clockSize2(clockSize2), paramSize1(paramSize1), paramSize2(paramSize2), states(out), toIState(toIState) {}
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
  out.clockDimensions = in1.clockDimensions + in2.clockDimensions;
  out.paramDimensions = in1.paramDimensions + in2.paramDimensions;

  // make product transitions
  AddProductTransitionsPTA addProductTransitionsPTA(in1.clockDimensions, in2.clockDimensions, in1.paramDimensions, in2.paramDimensions, out.states, toIState);
  addProductTransitionsPTA(in1.states, in2.states);
}
