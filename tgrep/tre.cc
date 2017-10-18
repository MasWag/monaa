#include <numeric>
#include <ostream>

#include "tre.hh"
#include "intermediate_tre.hh"

std::ostream& 
operator<<( std::ostream &stream, const TRE& expr)
{
  switch(expr.tag) {
  case TRE::op::atom:
    stream << expr.c;
    break;
  case TRE::op::epsilon:
    stream << '@';
    break;
  case TRE::op::plus:
    stream << "(" << *(expr.regExpr) << "+)";
    break;
  case TRE::op::concat:
    stream << "(" << *(expr.regExprPair.first) << *(expr.regExprPair.second) << ")";
    break;
  case TRE::op::disjunction:
    stream << "(" << *(expr.regExprPair.first) << "|" << *(expr.regExprPair.second) << ")";
    break;
  case TRE::op::conjunction:
    stream << "(" << *(expr.regExprPair.first) << "&" << *(expr.regExprPair.second) << ")";
    break;
  case TRE::op::within:
    stream << "(" << *(expr.regExprWithin.first) << "%" << *(expr.regExprWithin.second) << ")";
    break;
  }

  return stream;
}

const std::shared_ptr<TRE> TRE::epsilon = std::make_shared<TRE>(op::epsilon);

void concat2(TimedAutomaton &left, const TimedAutomaton &right) {
  // make a transition to an accepting state of left to a transition to an initial state of right
  std::vector<ClockVariables> rightClocks (right.clockSize());
  std::iota(rightClocks.begin(), rightClocks.end(), 0);
  for (auto &s: left.states) {
    for(auto &edges: s->next) {
      std::vector<TATransition> newTransitions;
      for (auto &edge: edges.second) {
        TAState *target = edge.target;
        if (target && target->isMatch) {
          newTransitions.reserve(newTransitions.size() + right.initialStates.size());
          for (auto initState: right.initialStates) {
            TATransition transition = edge;
            transition.target = initState.get();
            transition.resetVars = rightClocks;
            newTransitions.emplace_back(std::move(transition));
          }
        }
      }
      if (!newTransitions.empty()) {
        edges.second.insert(edges.second.end(), newTransitions.begin(), newTransitions.end());
      }
    }
  }

  // make accepting states of left non-accepting
  for (auto &s: left.states) {
    s->isMatch = false;
  }

  left.states.insert(left.states.end(), right.states.begin(), right.states.end());

  // we can reuse variables since we have no overwrapping constraints
  left.maxConstraints.resize(std::max(left.maxConstraints.size(), right.maxConstraints.size()));
  std::vector<int> maxConstraints = right.maxConstraints;
  maxConstraints.resize(std::max(left.maxConstraints.size(), maxConstraints.size()));
  for (std::size_t i = 0; i < left.maxConstraints.size(); ++i) {
    left.maxConstraints[i] = std::max(left.maxConstraints[i], maxConstraints[i]);
  }
}

void TRE::toEventTA(TimedAutomaton& out) const {
  switch(tag) {
  case op::atom: {
    out.states.resize(2);
    for (auto &state: out.states) {
      state = std::make_shared<TAState>();
    }
    out.initialStates = {out.states[0]};

    out.states[0]->isMatch = false;
    out.states[1]->isMatch = true;

    out.states[0]->next[c].push_back({out.states[1].get(), {}, {}});

    out.maxConstraints.clear();
    break;
  }
  case op::epsilon: {
    out.states.resize(1);
    out.states[0] = std::make_shared<TAState>();
    out.initialStates = {out.states[0]};
    out.states[0]->isMatch = true;
    out.maxConstraints.clear();
    break;
  }
  case op::plus: {
    regExpr->toEventTA(out);
    for (auto &s: out.states) {
      for (auto &edges: s->next) {
        for (auto &edge: edges.second) {
          TAState *target = edge.target;
          if (target && target->isMatch) {
            edges.second.reserve(edges.second.size() + out.initialStates.size());
            for (auto initState: out.initialStates) {
              TATransition transition = edge;
              transition.target = initState.get();
              edges.second.emplace_back(std::move(transition));
            }
          }
        }
      }
    }
    break;
  }
  case op::concat: {
    regExprPair.first->toEventTA(out);
    TimedAutomaton another;
    regExprPair.second->toEventTA(another);
    concat2(out, another);

    break;
  }
  case op::disjunction: {
    regExprPair.first->toEventTA(out);
    TimedAutomaton another;
    regExprPair.second->toEventTA(another);
    out.states.insert(out.states.end(), another.states.begin(), another.states.end());
    out.initialStates.insert(out.initialStates.end(), another.initialStates.begin(), another.initialStates.end());

    // we can reuse variables since we have no overwrapping constraints
    out.maxConstraints.resize(std::max(out.maxConstraints.size(), another.maxConstraints.size()));
    another.maxConstraints.resize(std::max(out.maxConstraints.size(), another.maxConstraints.size()));
    for (std::size_t i = 0; i < out.maxConstraints.size(); ++i) {
      out.maxConstraints[i] = std::max(out.maxConstraints[i], another.maxConstraints[i]);
    }
    break;
  }
  case op::conjunction: {
    TimedAutomaton tmpTA;
    TimedAutomaton another;
    boost::unordered_map<std::pair<TAState*,TAState*>, std::shared_ptr<TAState>> toIState;
    regExprPair.first->toEventTA(tmpTA);
    regExprPair.second->toEventTA(another);
    intersectionTA(tmpTA, another, out, toIState);
    break;
  }
  case op::within: {
    regExprWithin.first->toEventTA(out);
    // add dummy initial state
    std::shared_ptr<TAState> dummyInitialState = std::make_shared<TAState>();
    for (auto initialState: out.initialStates) {
      for (const auto &initTransitionsPair: initialState->next) {
        const Alphabet c = initTransitionsPair.first;
        dummyInitialState->next[c].reserve(dummyInitialState->next[c].size() + initTransitionsPair.second.size());
        for (auto& edge: initTransitionsPair.second) {
          TATransition transition = edge;
          transition.resetVars.push_back(out.clockSize());
          dummyInitialState->next[c].emplace_back(std::move(transition));
        }
      }
    }
    out.initialStates = {dummyInitialState};
    out.states.emplace_back(std::move(dummyInitialState));

    // add dummy accepting state
    std::shared_ptr<TAState> dummyAcceptingState = std::make_shared<TAState>(true);
    for (auto state: out.states) {
      for (auto& edges: state->next) {
        for (auto& edge: edges.second) {
          auto target = edge.target;
          if (target && target->isMatch) {
            TATransition transition = edge;
            transition.target = dummyAcceptingState.get();
            transition.guard.reserve(transition.guard.size() + 2);
            // upper bound
            if (regExprWithin.second->upperBound.second) {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) <= regExprWithin.second->upperBound.first);
            } else {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) < regExprWithin.second->upperBound.first);
            }
            // lower bound
            if (regExprWithin.second->lowerBound.second) {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) >= regExprWithin.second->lowerBound.first);
            } else {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) > regExprWithin.second->lowerBound.first);
            }
            edges.second.emplace_back(std::move(transition));
          }
        }
      }
    }
    for (auto state: out.states) {
      state->isMatch = false;
    }
    out.states.emplace_back(std::move(dummyAcceptingState));
    out.maxConstraints.emplace_back(regExprWithin.second->upperBound.first);
    break;
  }
  }
}

//! rename to epsilon transitions
void renameToEpsilonTransitions(TimedAutomaton& out) {
  std::unordered_map<TAState*, std::shared_ptr<TAState>> toAccepting;
  const auto haveToChangeToEpsilon = [](const TAState *s, const Alphabet c) {
    static std::unordered_map<const TAState*, Alphabet> output;
    if (c != 0) {
      if (s->next.find(c) != s->next.end()) {
        output[s] = c;
        return true;
      } else {
        return output[s] == c;
      }
    } 
    return false;
  };
  for (auto s: out.states) {
    for (auto &transitionsPair: s->next) {
      const Alphabet c = transitionsPair.first;
      for (auto it = transitionsPair.second.begin(); it != transitionsPair.second.end();) {
        TAState *target = it->target;
        if (target) {
          if (haveToChangeToEpsilon(target, c)) {
            s->next[0].push_back(*it);
            if (target->isMatch) {
              auto ret = toAccepting.find(target);
              if (ret == toAccepting.end()) {
                toAccepting[target] = std::make_shared<TAState>(true);
              }
              it->target = toAccepting[target].get();
            } else {
              it = transitionsPair.second.erase(it);
            }
          } else {
            it++;
          }
        } else {
          it = transitionsPair.second.erase(it);
        }
      }
    }
  }

  for (const auto &pair: toAccepting) {
    pair.first->isMatch = false;
    out.states.emplace_back(std::move(pair.second));
  }
}

void toSignalTA(std::shared_ptr<const TRE> tre, TimedAutomaton& out) {
  DNFTRE dnftre(tre);

  dnftre.toNormalForm();
  dnftre.toSignalTA(out);

  renameToEpsilonTransitions(out);
}
