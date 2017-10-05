#pragma once

#include <array>
#include <memory>
#include <vector>

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

  inline size_t clockSize() const {return maxConstraints.size ();}
};
