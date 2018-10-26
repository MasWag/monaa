#pragma once

#include "common_types.hh"

/*!
  @brief States of automata with nondeterministic branches

  @tparam Final The final child class
 */
template<class Final>
struct AbstractNFAState {
  bool isMatch;
  // An epsilon transition is denoted by the null character (\0)
  std::array<std::vector<std::weak_ptr<Final>>, CHAR_MAX> next;
  AbstractNFAState(bool isMatch, const std::array<std::vector<std::weak_ptr<Final>>, CHAR_MAX> &next) : isMatch(isMatch), next(next) {}
};

/*!
  @brief Abstract NFA
 */
template<class State>
struct AbstractNFA : public Automaton<State> {
  static_assert(std::is_base_of<AbstractNFAState<State>, State>::value, "State must be a child of AbstractNFAState");

  //! @brief remove states unreachable to any accepting states
  void removeDeadStates() {
    // Find states unreachable to any accepting states
    using Config = std::pair<std::shared_ptr<State>, std::unordered_set<std::shared_ptr<State>>>;
    std::stack<Config> States;
    for (auto state: this->initialStates) {
      States.push(Config(state, {}));
    }
    std::unordered_set<std::shared_ptr<State>> reachable;
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
    for (auto it = this->states.begin(); it != this->states.end(); ) {
      if(reachable.find(*it) == reachable.end()) {
        it = this->states.erase(it);
      } else {
        it++;
      }
    }
    for (auto it = this->initialStates.begin(); it != this->initialStates.end(); ) {
      if(reachable.find(*it) == reachable.end()) {
        it = this->initialStates.erase(it);
      } else {
        it++;
      }
    }
  }

  //! @brief emptiness check of the language
  bool empty() const {
    std::vector<std::shared_ptr<State>> currentStates = this->initialStates;
    std::unordered_set<std::shared_ptr<State>> visited = {this->initialStates.begin(), this->initialStates.end()};
    while (!currentStates.empty()) {
      std::vector<std::shared_ptr<State>> nextStates;
      for (auto state: currentStates) {
        if (state->isMatch) {
          return false;
        }
        for (const auto &edges: state->next) {
          for (const auto edge: edges) {
            auto target = edge.lock();
            if (target && visited.find(target) == visited.end()) {
              // We have not visited the state
              nextStates.push_back(target);
              visited.insert(target);
            }
          }
        }
        
      }
      currentStates = std::move(nextStates);
    }
    return true;
  }
};
