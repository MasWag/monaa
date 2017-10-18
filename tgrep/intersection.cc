#include "intersection.hh"
/*
  Specifications
  ==============
  * (s,s') is encoded as s + |S| * s'
  * x = x1 or x2 + |C|
  * in1 and in2 can be same.
*/
void intersectionTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out, boost::unordered_map<std::pair<std::shared_ptr<TAState>, std::shared_ptr<TAState>>, std::shared_ptr<TAState>> &toIState)
{
  // toIState :: (in1.State, in2.State) -> out.State

  // make states
  toIState.reserve(in1.stateSize() * in2.stateSize());
  out.states.reserve(in1.stateSize() * in2.stateSize());
  for (auto s1: in1.states) {
    for (auto s2: in2.states) {
      toIState[std::make_pair(s1,s2)] = std::make_shared<TAState>(s1->isMatch && s2->isMatch);      
      out.states.push_back(toIState[std::make_pair(s1,s2)]);
    }
  }

  // make initial states
  out.initialStates.clear ();
  out.initialStates.reserve (in1.initialStates.size() * in2.initialStates.size());
  for (auto s1: in1.initialStates) {
    for (auto s2: in2.initialStates) {
      out.initialStates.push_back(toIState[std::make_pair(s1,s2)]);
    }
  }

  // make max constraints
  out.maxConstraints.reserve( in1.maxConstraints.size() + in2.maxConstraints.size() ); // preallocate memory
  out.maxConstraints.insert( out.maxConstraints.end(), in1.maxConstraints.begin(), in1.maxConstraints.end() );
  out.maxConstraints.insert( out.maxConstraints.end(), in2.maxConstraints.begin(), in2.maxConstraints.end() );

  const auto addProductTransition = [&] (std::shared_ptr<TAState> s1, std::shared_ptr<TAState> s2, std::shared_ptr<TAState> nextS1, std::shared_ptr<TAState> nextS2, const TATransition &e1, const TATransition &e2, char c) {
    TATransition transition;
    transition.target = toIState[std::make_pair(nextS1, nextS2)];

    //concat resetVars
    transition.resetVars.reserve (e1.resetVars.size() + e2.resetVars.size());
    transition.resetVars.insert (transition.resetVars.end(),e1.resetVars.begin(), e1.resetVars.end());
    transition.resetVars.insert (transition.resetVars.end(),e2.resetVars.begin(), e2.resetVars.end());
    std::for_each (transition.resetVars.begin () + e1.resetVars.size(),
                   transition.resetVars.end(),
                   [&] (ClockVariables &v) { v += in1.clockSize();});

    //concat constraints
    transition.guard.reserve (e1.guard.size() + e2.guard.size());
    transition.guard.insert (transition.guard.end(),e1.guard.begin(), e1.guard.end());
    transition.guard.insert (transition.guard.end(),e2.guard.begin(), e2.guard.end());
    std::for_each (transition.guard.begin () + e1.guard.size(),
                   transition.guard.end(),
                   [&] (Constraint &guard) { guard.x += in1.clockSize();});

    toIState[std::make_pair(s1, s2)]->next[c].push_back(std::move(transition));
  };

  TATransition emptyTransition;
  // make edges
  for (auto s1: in1.states) {
    for (auto s2: in2.states) {
      // Epsilon transitions
      for (const auto &e1: s1->next[0]) {
        auto nextS1 = e1.target.lock();
        if (!nextS1) {
          continue;
        }
        addProductTransition(s1, s2, nextS1, s2, e1, emptyTransition, 0);
      }
      for (const auto &e2: s2->next[0]) {
        auto nextS2 = e2.target.lock();
        if (!nextS2) {
          continue;
        }
        addProductTransition(s1, s2, s1, nextS2, emptyTransition, e2, 0);
      }

      // Observable transitions
      for (auto it1 = s1->next.begin(); it1 != s1->next.end(); it1++) {
        const Alphabet c = it1->first;
        for (const auto &e1: it1->second) {
          auto nextS1 = e1.target.lock();
          if (!nextS1) {
              continue;
          }
          auto it2 = s2->next.find(c);
          if (it2 == s2->next.end()) {
            continue;
          }
          for (const auto &e2: it2->second) {
            auto nextS2 = e2.target.lock();
            if (!nextS2) {
              continue;
            }
            addProductTransition(s1, s2, nextS1, nextS2, e1, e2, c);
          }
        }
      }
    }
  }
}

void updateInitAccepting(const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out, boost::unordered_map<std::pair<std::shared_ptr<TAState>, std::shared_ptr<TAState>>, std::shared_ptr<TAState>> toIState) {
  // update initial states
  out.initialStates.clear();
  out.initialStates.reserve(in1.initialStates.size() * in2.initialStates.size());
  for (auto init1: in1.initialStates) {
    for (auto init2: in2.initialStates) {
      out.initialStates.push_back(toIState[std::make_pair(init1, init2)]);
    }
  }

  // update accepting states
  for (auto it = toIState.begin(); it != toIState.end(); ) {
    if (it->first.first && it->first.second && it->second) {
     it->second->isMatch = it->first.first->isMatch && it->first.second->isMatch;    
     it++;
    } else {
      it = toIState.erase(it);
    }
  }
}


void intersectionSignalTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out)
{
  // toIState :: (in1.State, in2.State) -> out.State
  boost::unordered_map<std::pair<std::shared_ptr<TAState>, std::shared_ptr<TAState>>, std::shared_ptr<TAState>> toIState;

  // make states
  toIState.reserve(in1.stateSize() * in2.stateSize());
  out.states.reserve(in1.stateSize() * in2.stateSize());
  for (auto s1: in1.states) {
    for (auto s2: in2.states) {
      toIState[std::make_pair(s1,s2)] = std::make_shared<TAState>(s1->isMatch && s2->isMatch);      
      out.states.push_back(toIState[std::make_pair(s1,s2)]);
    }
  }

  // make initial states
  out.initialStates.clear ();
  out.initialStates.reserve (in1.initialStates.size() * in2.initialStates.size());
  for (auto s1: in1.initialStates) {
    for (auto s2: in2.initialStates) {
      out.initialStates.push_back(toIState[std::make_pair(s1,s2)]);
    }
  }

  // make max constraints
  out.maxConstraints.reserve( in1.maxConstraints.size() + in2.maxConstraints.size() ); // preallocate memory
  out.maxConstraints.insert( out.maxConstraints.end(), in1.maxConstraints.begin(), in1.maxConstraints.end() );
  out.maxConstraints.insert( out.maxConstraints.end(), in2.maxConstraints.begin(), in2.maxConstraints.end() );

  const auto addProductTransition = [&] (std::shared_ptr<TAState> s1, std::shared_ptr<TAState> s2, std::shared_ptr<TAState> nextS1, std::shared_ptr<TAState> nextS2, const TATransition &e1, const TATransition &e2, char c) {
    TATransition transition;
    transition.target = toIState[std::make_pair(nextS1, nextS2)];

    //concat resetVars
    transition.resetVars.reserve (e1.resetVars.size() + e2.resetVars.size());
    transition.resetVars.insert (transition.resetVars.end(),e1.resetVars.begin(), e1.resetVars.end());
    transition.resetVars.insert (transition.resetVars.end(),e2.resetVars.begin(), e2.resetVars.end());
    std::for_each (transition.resetVars.begin () + e1.resetVars.size(),
                   transition.resetVars.end(),
                   [&] (ClockVariables &v) { v += in1.clockSize();});

    //concat constraints
    transition.guard.reserve (e1.guard.size() + e2.guard.size());
    transition.guard.insert (transition.guard.end(),e1.guard.begin(), e1.guard.end());
    transition.guard.insert (transition.guard.end(),e2.guard.begin(), e2.guard.end());
    std::for_each (transition.guard.begin () + e1.guard.size(),
                   transition.guard.end(),
                   [&] (Constraint &guard) { guard.x += in1.clockSize();});

    toIState[std::make_pair(s1, s2)]->next[c].push_back(std::move(transition));
  };

  TATransition emptyTransition;
  // make edges
  for (auto s1: in1.states) {
    for (auto s2: in2.states) {
      // Epsilon transitions
      for (const auto &e1: s1->next[0]) {
        auto nextS1 = e1.target.lock();
        if (!nextS1) {
          continue;
        }
        addProductTransition(s1, s2, nextS1, s2, e1, emptyTransition, 0);
      }
      for (const auto &e2: s2->next[0]) {
        auto nextS2 = e2.target.lock();
        if (!nextS2) {
          continue;
        }
        addProductTransition(s1, s2, s1, nextS2, emptyTransition, e2, 0);
      }

      // Observable transitions
      for (auto it1 = s1->next.begin(); it1 != s1->next.end(); it1++) {
        const Alphabet c = it1->first;
        // Syncronous transition
        for (const auto &e1: it1->second) {
          auto nextS1 = e1.target.lock();
          if (!nextS1) {
            continue;
          }
          auto it2 = s2->next.find(c);
          if (it2 == s2->next.end()) {
            continue;
          }
          for (const auto &e2: it2->second) {
            auto nextS2 = e2.target.lock();
            if (!nextS2) {
              continue;
            }
            addProductTransition(s1, s2, nextS1, nextS2, e1, e2, c);
          }
        }

        // Asyncronous transition
        if (s2->next.find(c) != s2->next.end()) {
          for (const auto &e1: it1->second) {
            auto nextS1 = e1.target.lock();
            if (!nextS1) {
              continue;
            }
            addProductTransition(s1, s2, nextS1, s2, e1, emptyTransition, 0);
          }
        }

        auto it2 = s2->next.find(c);
        if (it2 == s2->next.end()) {
          continue;
        }
        for (const auto &e2: it2->second) {
          auto nextS2 = e2.target.lock();
          if (!nextS2) {
            continue;
          }
          addProductTransition(s1, s2, s1, nextS2, emptyTransition, e2, 0);
        }
      }
    }
  }
}
