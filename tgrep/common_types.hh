#pragma once

#include <cstdint>
#include <vector>

typedef char Alphabet;
typedef uint8_t ClockVariables;


/*!
  @brief Automaton
 */
template<class State>
struct Automaton {
  struct TATransition;

  std::vector<std::shared_ptr<State>> states;
  std::vector<std::shared_ptr<State>> initialStates;

  inline std::size_t stateSize() const {return states.size ();}

  inline bool operator == (const Automaton<State> A) const {
    return initialStates == A.initialStates &&
      states == A.states;
  }
};
