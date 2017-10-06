#pragma once

#include <array>
#include <memory>
#include <vector>
#include <unordered_map>

#include "constraint.hh"
#include "common_types.hh"

namespace {
  struct TATransition;

  struct TAState {
    bool isMatch;
    std::array<std::vector<TATransition>, CHAR_MAX> next;
    TAState () : isMatch(false), next({}) {}
    TAState (bool isMatch) : isMatch(isMatch), next({}) {}
    TAState (bool isMatch, std::array<std::vector<TATransition>, CHAR_MAX> next) : isMatch(isMatch), next(std::move(next)) {}
  };

  struct TATransition {
    std::weak_ptr<TAState> target;
    std::vector<Alphabet> resetVars;
    std::vector<Constraint> guard;
  };
}

/*!
  @brief Timed Automaton
 */
struct TimedAutomaton : public Automaton<TAState> {
  using X = ConstraintMaker;
  using TATransition = TATransition;
  using TAState = TAState;

  std::vector<int> maxConstraints;
  /*!
    @brief make a deep copy of this timed automaton.
   */
  void deepCopy(TimedAutomaton& dest, std::unordered_map<std::shared_ptr<TAState>, std::shared_ptr<TAState>> &old2new) const {
    // copy states
    old2new.reserve(stateSize());
    dest.states.reserve(stateSize());
    for (auto oldState: states) {
      dest.states.emplace_back(std::make_shared<TAState>(*oldState));
      old2new[oldState] = dest.states.back();
    }
    // copy initial states
    dest.initialStates.reserve(initialStates.size());
    for (auto oldInitialState: initialStates) {
      dest.initialStates.emplace_back(old2new[oldInitialState]);
    }
    // modify dest of transitions
    for (auto &state: dest.states) {
      for (auto &edges: state->next) {
        for (auto &edge: edges) {
          auto oldTarget = edge.target.lock();
          edge.target = old2new[oldTarget];
        }
      }

    }
    dest.maxConstraints = maxConstraints;
  }
  inline size_t clockSize() const {return maxConstraints.size ();}
};
