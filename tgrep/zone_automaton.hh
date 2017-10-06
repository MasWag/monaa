#pragma once

#include "timed_automaton.hh"
#include "zone.hh"
#include <stack>
#include <unordered_set>
#include <utility>

namespace 
{
  struct ZAState {
    bool isMatch;
    // An epsilon transition is denoted by the null character (\0)
    std::array<std::vector<std::weak_ptr<ZAState>>, CHAR_MAX> next;
    std::shared_ptr<TAState> taState;
    Zone zone;
    ZAState () : isMatch(false), next({}) {}
    ZAState (std::shared_ptr<TAState> taState, Zone zone) : isMatch(taState->isMatch), next({}), taState(taState), zone(std::move(zone)) {}
    ZAState (bool isMatch) : isMatch(isMatch), next({}) {}
    ZAState (bool isMatch, std::array<std::vector<std::weak_ptr<ZAState>>, CHAR_MAX> next) : isMatch(isMatch), next(next) {}
    bool operator==(std::pair<std::shared_ptr<TAState>, Zone> pair) {
      return pair.first == taState && pair.second == zone;
    }
  };

  struct NoEpsilonZAState {
    bool isMatch;
    std::array<std::vector<std::weak_ptr<NoEpsilonZAState>>, CHAR_MAX> next;    
    std::unordered_set<std::shared_ptr<ZAState>> zaStates;
    bool operator==(const std::unordered_set<std::shared_ptr<ZAState>> &zas) {
      return zas == zaStates;
    }
  };
}

//! @brief returns the set of states that is reachable from a state in the state by unobservable transitions
void epsilonClosure(std::unordered_set<std::shared_ptr<ZAState>> &closure) {
  auto waiting = std::deque<std::shared_ptr<ZAState>>(closure.begin(), closure.end());
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


struct ZoneAutomaton : public Automaton<ZAState> {
  using ZAState = ZAState;

  //! @brief remove states unreachable to any accepting states
  void removeDeadStates() {
    // Find states unreachable to any accepting states
    using Config = std::pair<std::shared_ptr<ZAState>, std::unordered_set<std::shared_ptr<ZAState>>>;
    std::stack<Config> States;
    for (auto state: initialStates) {
      States.push(Config(state, {}));
    }
    std::unordered_set<std::shared_ptr<ZAState>> reachable;
    while (!States.empty()) {
      Config conf = States.top();
      States.pop();
      if (conf.first->isMatch) {
        reachable.insert(conf.first);
        reachable.insert(conf.second.begin(), conf.second.end());
      }
      for (const auto &edges: conf.first->next) {
        for (const auto &edge: edges) {
          if (conf.second.find(conf.first) != conf.second.end()) {
            // We are in a loop
            continue;
          }
          if(reachable.find(edge.lock()) != reachable.end()) {
            // The next state is reachable
            reachable.insert(conf.first);
            reachable.insert(conf.second.begin(), conf.second.end());
          } else {
            // The next state may be unreachable
            auto parents = conf.second;
            parents.insert(conf.first);
            States.push(Config(edge.lock(), parents));
          }
        }
      }
    }

    // Remove unreachable states 
    for (auto it = states.begin(); it != states.end(); ) {
      if(reachable.find(*it) == reachable.end()) {
        it = states.erase(it);
      } else {
        it++;
      }
    }
    for (auto it = initialStates.begin(); it != initialStates.end(); ) {
      if(reachable.find(*it) == reachable.end()) {
        it = initialStates.erase(it);
      } else {
        it++;
      }
    }
  }

  //! @brief Propagate accepting states from the original timed automaton
  void updateAccepting() {
    for (auto &state: states) {
      state->isMatch = state->taState->isMatch;
    }
  }
  
  //! @brief emptiness check of the language
  bool empty() const {
    std::unordered_set<std::shared_ptr<ZAState>> visited;
    std::vector<std::shared_ptr<ZAState>> currentStates = initialStates;
    while (!currentStates.empty()) {
      std::vector<std::shared_ptr<ZAState>> nextStates;
      for (auto state: currentStates) {
        if (state->isMatch) {
          return false;
        }
        for (const auto &edges: state->next) {
          for (const auto &edge: edges) {
            auto target = edge.lock();
            if (!target || visited.find(target) != visited.end()) {
              // We have visited the state
              continue;
            }
            nextStates.push_back(target);
          }
        }
        
      }
      currentStates = std::move(nextStates);
    }
    return true;
  }
};

