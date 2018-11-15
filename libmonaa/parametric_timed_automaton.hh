#pragma once

#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
#include <climits>
#include <valarray>
#include <ppl.hh>

#include "constraint.hh"
#include "common_types.hh"

struct PTATransition;

/*!
  @brief A state of a parametric timed automaton
 */
struct PTAState {
  //! @brief The value is true if and only if the state is an accepting state.
  bool isMatch;
  /*! 
    @brief An mapping of a character to the transitions.
    @note Because of non-determinism, the second element is a vector.
   */
  std::unordered_map<Alphabet, std::vector<PTATransition>> next;
  PTAState (bool isMatch = false) : isMatch(isMatch) {
    next.clear();
  }
  PTAState (bool isMatch, std::unordered_map<Alphabet, std::vector<PTATransition>> next) : isMatch(isMatch), next(std::move(next)) {}
};

/*!
  @brief A transition of a parametric timed automaton
 */
struct PTATransition {
  using Guard = Parma_Polyhedra_Library::NNC_Polyhedron;
  //! @brief The pointer to the target state.
  PTAState *target;
  //! @brief The clock variables reset after this transition.
  std::vector<ClockVariables> resetVars;
  //! @brief The guard for this transition.
  Parma_Polyhedra_Library::NNC_Polyhedron guard;
};

/*!
  @brief A parametric timed automaton
 */
struct ParametricTimedAutomaton : public Automaton<PTAState> {
  using X = ConstraintMaker;
  using State = ::PTAState;
  using Transition = PTATransition;

  /*!
    @brief make a deep copy of this parametric timed automaton.

    @param [out] dest The destination of the deep copy.
    @param [out] old2new The mapping from the original state to the corresponding new state.
   */
  void deepCopy(ParametricTimedAutomaton& dest, std::unordered_map<PTAState*, std::shared_ptr<PTAState>> &old2new) const {
    // copy states
    old2new.reserve(stateSize());
    dest.states.reserve(stateSize());
    for (auto oldState: states) {
      dest.states.emplace_back(std::make_shared<PTAState>(*oldState));
      old2new[oldState.get()] = dest.states.back();
    }
    // copy initial states
    dest.initialStates.reserve(initialStates.size());
    for (auto oldInitialState: initialStates) {
      dest.initialStates.emplace_back(old2new[oldInitialState.get()]);
    }
    // modify dest of transitions
    for (auto &state: dest.states) {
      for (auto &edges: state->next) {
        for (auto &edge: edges.second) {
          auto oldTarget = edge.target;
          edge.target = old2new[oldTarget].get();
        }
      }
    }
    dest.clockDimensions = clockDimensions;
    dest.paramDimensions = paramDimensions;
  }

  std::size_t clockDimensions;
  std::size_t paramDimensions;
};
