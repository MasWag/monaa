#pragma once

#include <boost/unordered_map.hpp>
#include "timed_automaton.hh"

/*
  Specifications
  ==============
  * (s,s') is encoded as s + |S| * s'
  * x = x1 or x2 + |C|
  * in1 and in2 can be same.
*/
void intersectionTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out, boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> &toIState);

void updateInitAccepting(const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out, boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> toIState);

void intersectionSignalTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out);

/*!
  @brief Construct product states and the mapping function.

  @param [in] in1 First set of states
  @param [in] in2 Second set of states
  @param [out] toIState Mapping from the pair (s1,s2) of the original automata to the state in the product automaton.
 */
template<class State>
void makeProductStates(const std::vector<std::shared_ptr<State>> &in1, const std::vector<std::shared_ptr<State>> &in2, 
                       boost::unordered_map<std::pair<State*, State*>, std::shared_ptr<State>> &toIState) {
  toIState.clear();
  toIState.reserve(in1.size() * in2.size());
  for (auto s1: in1) {
    for (auto s2: in2) {
      toIState[std::make_pair(s1.get(), s2.get())] = std::make_shared<State>(s1->isMatch && s2->isMatch);
    }
  }
}


/*!
  @brief Construct product states and the mapping function.

  @param [in] in1 First set of states
  @param [in] in2 Second set of states
  @param [in] toIState Mapping from the pair (s1,s2) of the original automata to the state in the product automaton.
  @param [out] out Set of product states
 */
template<class State>
void pushProductStates(const std::vector<std::shared_ptr<State>> &in1, const std::vector<std::shared_ptr<State>> &in2, 
                       boost::unordered_map<std::pair<State*, State*>, std::shared_ptr<State>> &toIState,
                       std::vector<std::shared_ptr<State>> &out) {
  out.clear();
  out.reserve(in1.size() * in2.size());
  for (auto s1: in1) {
    for (auto s2: in2) {
      out.push_back(toIState[std::make_pair(s1.get(), s2.get())]);
    }
  }
}

/*!
  @brief Add product transifions

  @param [in] in1 First set of states
  @param [in] in2 Second set of states
  @param [in] addProductTransition Function to add the product transition to the product automaton
 */
template<class State, class Transition>
class AddProductTransitions {
private:
  virtual void addProductTransition(State*, State*, State*, State*, const Transition&, const Transition&, Alphabet) = 0;
public:
  void operator()(const std::vector<std::shared_ptr<State>> &in1, const std::vector<std::shared_ptr<State>> &in2) {
    Transition emptyTransition;
    // make edges
    for (auto s1: in1) {
      for (auto s2: in2) {
        // Epsilon transitions
        for (const auto &e1: s1->next[0]) {
          auto nextS1 = e1.target;
          if (!nextS1) {
            continue;
          }
          addProductTransition(s1.get(), s2.get(), nextS1, s2.get(), e1, emptyTransition, 0);
        }
        for (const auto &e2: s2->next[0]) {
          auto nextS2 = e2.target;
          if (!nextS2) {
            continue;
          }
          addProductTransition(s1.get(), s2.get(), s1.get(), nextS2, emptyTransition, e2, 0);
        }

        // Observable transitions
        for (auto it1 = s1->next.begin(); it1 != s1->next.end(); it1++) {
          const Alphabet c = it1->first;
          for (const auto &e1: it1->second) {
            auto nextS1 = e1.target;
            if (!nextS1) {
              continue;
            }
            auto it2 = s2->next.find(c);
            if (it2 == s2->next.end()) {
              continue;
            }
            for (const auto &e2: it2->second) {
              auto nextS2 = e2.target;
              if (!nextS2) {
                continue;
              }
              addProductTransition(s1.get(), s2.get(), nextS1, nextS2, e1, e2, c);
            }
          }
        }
      }
    }
  }
};
