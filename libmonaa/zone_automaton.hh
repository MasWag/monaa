#pragma once

#include "abstract_nfa.hh"
#include "zone.hh"
#include <stack>
#include <unordered_set>
#include <utility>
#include "timed_automaton.hh"

template<class TAState, class Zone>
struct AbstractZAState : public AbstractNFAState<AbstractZAState<TAState, Zone>> {
  TAState *taState;
  Zone zone;
  AbstractZAState (TAState *taState, Zone zone) : AbstractNFAState<AbstractZAState<TAState, Zone>>(taState->isMatch, {}), taState(taState), zone(std::move(zone)) {}
  AbstractZAState (bool isMatch = false, std::array<std::vector<std::weak_ptr<AbstractZAState<TAState, Zone>>>, CHAR_MAX> next = {}) : AbstractNFAState<AbstractZAState<TAState, Zone>>(isMatch, next) {}
  bool operator==(std::pair<TAState*, Zone> pair) {
    return pair.first == taState && pair.second == zone;
  }
};

using ZAState = AbstractZAState<TAState, Zone>;

struct NoEpsilonZAState {
  bool isMatch;
  std::array<std::vector<std::weak_ptr<NoEpsilonZAState>>, CHAR_MAX> next;    
  std::unordered_set<std::shared_ptr<ZAState>> zaStates;
  bool operator==(const std::unordered_set<std::shared_ptr<ZAState>> &zas) {
    return zas == zaStates;
  }
};

//! @brief returns the set of states that is reachable from a state in the state by unobservable transitions
template<class TAState, class Zone>
void epsilonClosure(std::unordered_set<std::shared_ptr<AbstractZAState<TAState, Zone>>> &closure) {
  auto waiting = std::deque<std::shared_ptr<AbstractZAState<TAState, Zone>>>(closure.begin(), closure.end());
  while (!waiting.empty()) {
    for(auto wstate: waiting.front()->next[0]) {
      auto state = wstate.lock();
      if ( state && closure.find(state) == closure.end()) {
        closure.insert(state);
        waiting.push_back(state);
      }
    }
    waiting.pop_front();
  }
}



template<class TAState, class Zone>
struct AbstractZoneAutomaton : public AbstractNFA<AbstractZAState<TAState, Zone>> {
  using State = AbstractZAState<TAState, Zone>;
  /*!
    @brief Propagate accepting states from the original timed automaton
    @note taInitSates must be sorted
  */
  void updateInitAccepting(const std::vector<std::shared_ptr<TAState>> taInitialStates) {
    // update initial states
    this->initialStates.clear();
    for (std::shared_ptr<ZAState> s: this->states) {
      if (std::find_if(taInitialStates.begin(), taInitialStates.end(), [&](std::shared_ptr<TAState> taS) {
            return taS.get() == s->taState;}) != taInitialStates.end()) {
        this->initialStates.push_back(s);
      }
    }

    // update accepting states
    for (auto &state: this->states) {
      state->isMatch = state->taState->isMatch;
    }
  }
};

using ZoneAutomaton = AbstractZoneAutomaton<TAState, Zone>;
