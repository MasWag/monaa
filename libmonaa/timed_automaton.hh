#pragma once

#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
#include <climits>
#include <valarray>

#include "constraint.hh"
#include "common_types.hh"

struct TATransition;

/*!
  @brief A state of timed automata
 */
struct TAState {
  //! @brief The value is true if and only if the state is an accepting state.
  bool isMatch;
  /*! 
    @brief An mapping of a character to the transitions.
    @note Because of non-determinism, the second element is a vector.
   */
  std::unordered_map<Alphabet, std::vector<TATransition>> next;
  TAState (bool isMatch = false) : isMatch(isMatch) {
    next.clear();
  }
  TAState (bool isMatch, std::unordered_map<Alphabet, std::vector<TATransition>> next) : isMatch(isMatch), next(std::move(next)) {}
};

/*!
  @brief A state of timed automata
 */
struct TATransition {
  using Guard = std::vector<Constraint>;
  //! @brief The pointer to the target state.
  TAState *target;
  //! @brief The clock variables reset after this transition.
  std::vector<ClockVariables> resetVars;
  //! @brief The guard for this transition.
  std::vector<Constraint> guard;
};

/*!
  @brief A timed automaton
 */
struct TimedAutomaton : public Automaton<TAState> {
  using X = ConstraintMaker;
  using TATransition = TATransition;
  using State = ::TAState;

  //! @brief The maximum constraints for each clock variables.
  std::vector<int> maxConstraints;
  /*!
    @brief make a deep copy of this timed automaton.

    @param [out] dest The destination of the deep copy.
    @param [out] old2new The mapping from the original state to the corresponding new state.
   */
  void deepCopy(TimedAutomaton& dest, std::unordered_map<TAState*, std::shared_ptr<TAState>> &old2new) const {
    // copy states
    old2new.reserve(stateSize());
    dest.states.reserve(stateSize());
    for (auto oldState: states) {
      dest.states.emplace_back(std::make_shared<TAState>(*oldState));
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
    dest.maxConstraints = maxConstraints;
  }
  //! @brief Returns the number of clock variables used in the timed automaton.
  inline size_t clockSize() const {return maxConstraints.size ();}

  /*!
    @brief solve membership problem for observable timed automaton
    @note This is only for testing.
    @note If there are epsilon transitions, this does not work.
   */
  bool isMember(const std::vector<std::pair<Alphabet, double>> &w) const {
    std::vector<std::pair<TAState*, std::valarray<double>>> CStates;
    CStates.reserve(initialStates.size());
    for (const auto& s: initialStates) {
      CStates.emplace_back(s.get(), std::valarray<double>(0.0, clockSize()));
    }
    for (std::size_t i = 0; i < w.size(); i++) {
      std::vector<std::pair<TAState*, std::valarray<double>>> NextStates;
      for (std::pair<TAState*, std::valarray<double>> &config: CStates) {
        if (i > 0) {
          config.second += w[i].second - w[i-1].second;
        } else {
          config.second += w[i].second;
        }
        auto it = config.first->next.find(w[i].first);
        if (it == config.first->next.end()) {
          continue;
        }
        for (const auto &edge: it->second) {
          if (std::all_of(edge.guard.begin(), edge.guard.end(), [&](const Constraint &g) {
                return g.satisfy(config.second[g.x]);
              })) {
            auto tmpConfig = config;
            tmpConfig.first = edge.target;
            if (tmpConfig.first) {
              for (ClockVariables x: edge.resetVars) {
                tmpConfig.second[x] = 0; 
              }
              NextStates.emplace_back(std::move(tmpConfig));
            }
          }
        }
      }
      CStates = std::move(NextStates);
    }
    return std::any_of(CStates.begin(), CStates.end(), [](std::pair<const TAState*, std::valarray<double>> p) {
        return p.first->isMatch;
      });
  }
};
