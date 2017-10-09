#pragma once

#include <unordered_map>
#include <numeric>

#include "timed_automaton.hh"
#include "zone_automaton.hh"
#include "intersection.hh"
#include "ta2za.hh"

class KMPSkipValue {
private:
  std::unordered_map<std::shared_ptr<TAState>, int> beta;
public:
  KMPSkipValue(const TimedAutomaton &TA, int m) {
    ZoneAutomaton ZA2;
    TimedAutomaton A2;
    boost::unordered_map<std::pair<std::shared_ptr<TAState>, std::shared_ptr<TAState>>, std::shared_ptr<TAState>> toIState;
    //! @brief A0 is the automaton in A_{+n}^* in the paper. What we do is: 1) construct a dummy accepting state, and 2) construct m-dummy states to the original intial states
    TimedAutomaton A0;
    std::unordered_map<std::shared_ptr<TAState>, std::shared_ptr<TAState>> old2new0;
    TA.deepCopy(A0, old2new0);
    // Vector of extended initial states. if the accepting state is extendedInitialStates[n], the TA reads n-additional events.
    std::vector<std::shared_ptr<TAState>> extendedInitialStates(m+1);
    for (auto &es: extendedInitialStates) {
      es = std::make_shared<TAState>();
    }

    for (auto initialState: A0.initialStates) {
      for (char c = 0; c < CHAR_MAX; ++c) {
        for (TATransition edge: initialState->next[c]) {
          // We can modify edge because it is copied
          widen(edge.guard);
          extendedInitialStates[0]->next[c].push_back(std::move(edge));
        }
      }
    }
    
    std::vector<ClockVariables> allClocks(TA.clockSize());
    std::iota(allClocks.begin(), allClocks.end(), 0);
    for (int i = 1; i <= m; ++i) {
      for (char c = 1; c < CHAR_MAX; ++c) {
        extendedInitialStates[i]->next[c].push_back({extendedInitialStates[i-1], allClocks, {}});
      }      
    }
    auto dummyAcceptingState = std::make_shared<TAState>(true);
    // add self loop
    for (char c = 1; c < CHAR_MAX; ++c) {
      dummyAcceptingState->next[c].push_back({dummyAcceptingState, {}, {}});
    }
    for (auto &state: A0.states) {
      for (char c = 0; c < CHAR_MAX; ++c) {
        for (TATransition edge: state->next[c]) {
          // We can modify edge because it is copied
          if (edge.target.lock()->isMatch) {
            edge.target = dummyAcceptingState;
            widen(edge.guard);
            state->next[c].emplace_back(std::move(edge));
          }
        }
      }      
    }
    A0.states.reserve(A0.states.size() + 1 + (m + 1));
    A0.states.push_back(dummyAcceptingState);
    A0.states.insert(A0.states.end(), extendedInitialStates.begin(), extendedInitialStates.end());
    
    //! @brief As is the automaton in A_{s}^* in the paper. What we do is to construct a dummy state for each state s.
    TimedAutomaton As;
    std::unordered_map<std::shared_ptr<TAState>, std::shared_ptr<TAState>> old2newS;
    old2newS.reserve(TA.states.size());
    TA.deepCopy(As, old2newS);
    std::unordered_map<std::shared_ptr<TAState>, std::shared_ptr<TAState>> toDummyState;
    toDummyState.reserve(TA.states.size());
    for (auto state: As.states) {
      toDummyState[state] = std::make_shared<TAState>();
      state->next[0].push_back({toDummyState[state], {}, {}});
      // add self loop
      for (char c = 1; c < CHAR_MAX; ++c) {
        toDummyState[state]->next[c].push_back({toDummyState[state], {}, {}});
      }
    }
    As.states.reserve(As.states.size() * 2);
    for (auto dummyState: toDummyState) {
      As.states.push_back(dummyState.second);
    }
    
    intersectionTA (A0, As, A2, toIState);

    // Calculate KMP-type skip value
    for (auto origState: TA.states) {
      for (auto &stateAs: As.states) {
        stateAs->isMatch = stateAs == old2newS[origState] || stateAs == toDummyState[old2newS[origState]];
      }
      // Find the minumum n such that the intersection of the two languages is not empty.
      for (int n = 1; n <= m; n++) {
        A0.initialStates = {extendedInitialStates[n]};
        updateInitAccepting(A0, As, A2, toIState);
        std::sort(A2.initialStates.begin(), A2.initialStates.end());
        ta2za(A2, ZA2);
        ZA2.updateInitAccepting(A2.initialStates);
        if (!ZA2.empty()) {
          beta[origState] = n;
          break;
        }
      }
      // When the emptiness checking always failed, we set m
      if (beta.find(origState) == beta.end()) {
        beta[origState] = m;
      }
    }
  }

  int at(std::shared_ptr<TAState> s) const {
    return beta.at(s);
  }
  int operator[](std::shared_ptr<TAState> s) const {
    return beta.at(s);
  }
};
