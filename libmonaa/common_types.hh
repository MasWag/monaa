#pragma once

#include <cstdint>
#include <memory>
#include <vector>

typedef char Alphabet;
typedef uint8_t ClockVariables;

/*!
  @brief An automaton
 */
template <class State> struct Automaton {
  struct TATransition;

  //! @brief The states of this automaton.
  std::vector<std::shared_ptr<State>> states;
  //! @brief The initial states of this automaton.
  std::vector<std::shared_ptr<State>> initialStates;

  //! @brief Returns the number of the states.
  inline std::size_t stateSize() const { return states.size(); }

  inline bool operator==(const Automaton<State> A) const {
    return initialStates == A.initialStates && states == A.states;
  }
};
