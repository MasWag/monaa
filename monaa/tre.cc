#include <numeric>
#include <ostream>

#include "tre.hh"
#include "intermediate_tre.hh"

std::ostream &
operator<<(std::ostream &stream, const TRE &expr) {
  switch (expr.tag) {
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
  std::vector<ClockVariables> rightClocks(right.clockSize());
  std::iota(rightClocks.begin(), rightClocks.end(), 0);
  for (auto &s: left.states) {
    for (auto &edges: s->next) {
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

  // add right initial states to overall initial states if left has accepting and initial state
  if (std::any_of(left.initialStates.begin(), left.initialStates.end(), [](auto state) {
    return state->isMatch;
  })) {
    left.initialStates.insert(left.initialStates.end(), right.initialStates.begin(), right.initialStates.end());
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

void removeStuckStates(TimedAutomaton &out) {
  std::vector<TAState *> stuckStates;
  for (auto &s: out.states) {
    if (!s->isMatch && s->next.empty()) {
      stuckStates.push_back(s.get());
    }
  }
  std::sort(stuckStates.begin(), stuckStates.end());
  const auto isStuckState = [&stuckStates](const std::shared_ptr<TAState> s) {
    return std::binary_search(stuckStates.begin(), stuckStates.end(), s.get());
  };
  const auto isStuckTransition = [&stuckStates](const TATransition &transition) {
    return std::binary_search(stuckStates.begin(), stuckStates.end(), transition.target);
  };
  for (auto &source: out.states) {
    for (auto &transitionVectorPair: source->next) {
      transitionVectorPair.second.erase(
              std::remove_if(transitionVectorPair.second.begin(), transitionVectorPair.second.end(), isStuckTransition),
              transitionVectorPair.second.end());
    }
    for (auto it = source->next.begin(); it != source->next.end();) {
      if (it->second.empty()) {
        it = source->next.erase(it);
      } else {
        ++it;
      }
    }
  }
  out.states.erase(std::remove_if(out.states.begin(), out.states.end(), isStuckState), out.states.end());
  out.initialStates.erase(std::remove_if(out.initialStates.begin(), out.initialStates.end(), isStuckState),
                          out.initialStates.end());
}

void removeUnreachableStates(TimedAutomaton &out) {
  std::vector<TAState *> reachableStates;
  std::vector<TAState *> newReachableStates;
  std::transform(out.initialStates.begin(), out.initialStates.end(), newReachableStates.begin(),
                 std::mem_fn(&std::shared_ptr<TAState>::get));
  std::sort(newReachableStates.begin(), newReachableStates.end());
  reachableStates = newReachableStates;

  while (!newReachableStates.empty()) {
    {
      std::vector<TAState *> c;
      std::set_union(std::make_move_iterator(reachableStates.begin()),
                     std::make_move_iterator(reachableStates.end()),
                     newReachableStates.begin(), newReachableStates.end(),
                     std::inserter(c, c.begin()));
      reachableStates.swap(c);
    }
    std::vector<TAState *> nextReachableStates;
    for (TAState *source: newReachableStates) {
      for (auto &transitionVectorPair: source->next) {
        for (auto &transition: transitionVectorPair.second) {
          nextReachableStates.push_back(transition.target);
        }
      }
    }
    std::sort(nextReachableStates.begin(), nextReachableStates.end());
    std::unique(nextReachableStates.begin(), nextReachableStates.end());
    {
      std::vector<TAState *> c;
      std::set_difference(std::make_move_iterator(nextReachableStates.begin()),
                          std::make_move_iterator(nextReachableStates.end()),
                          reachableStates.begin(), reachableStates.end(),
                          std::inserter(c, c.begin()));
      nextReachableStates.swap(c);
    }
    newReachableStates = std::move(nextReachableStates);
  }

  out.states.erase(
          std::remove_if(out.states.begin(), out.states.end(), [&reachableStates](const std::shared_ptr<TAState> s) {
            return std::binary_search(reachableStates.begin(), reachableStates.end(), s.get());
          }), out.states.end());
}

void reduceStates(TimedAutomaton &out) {
  for (;;) {
    const auto size = out.states.size();
    removeStuckStates(out);
    //removeUnreachableStates(out);
    if (size == out.states.size()) {
      return;
    }
  }
}

void TRE::toEventTA(TimedAutomaton &out) const {
  switch (tag) {
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
          for (auto edge: edges.second) {
            TAState *target = edge.target;
            if (target && target->isMatch) {
              edges.second.reserve(edges.second.size() + out.initialStates.size());
              for (auto initState: out.initialStates) {
                TATransition transition = edge;
                transition.target = initState.get();
                for (int clock = 0; clock < out.clockSize(); clock++) {
                  transition.resetVars.push_back(clock);
                }
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
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      regExprPair.first->toEventTA(tmpTA);
      regExprPair.second->toEventTA(another);
      intersectionTA(tmpTA, another, out, toIState);
      break;
    }
    case op::within: {
      regExprWithin.first->toEventTA(out);

      // add dummy accepting state
      bool isDummyUsed = false;
      std::shared_ptr<TAState> dummyAcceptingState = std::make_shared<TAState>(true);
      for (auto state: out.states) {
        for (auto &edges: state->next) {
          for (auto &edge: edges.second) {
            auto target = edge.target;
            if (target && target->isMatch) {
              // we use dummyAcceptingState if there is a transition from the accepting state
              if (target->next.empty()) {
                edge.guard.reserve(edge.guard.size() + 2);
                // upper bound
                if (regExprWithin.second->upperBound.second) {
                  edge.guard.emplace_back(
                                          TimedAutomaton::X(out.clockSize()) <= regExprWithin.second->upperBound.first);
                } else {
                  edge.guard.emplace_back(
                                          TimedAutomaton::X(out.clockSize()) < regExprWithin.second->upperBound.first);
                }
                // lower bound
                if (regExprWithin.second->lowerBound.second) {
                  edge.guard.emplace_back(
                                          TimedAutomaton::X(out.clockSize()) >= regExprWithin.second->lowerBound.first);
                } else {
                  edge.guard.emplace_back(
                                          TimedAutomaton::X(out.clockSize()) > regExprWithin.second->lowerBound.first);
                }
              } else {
                isDummyUsed = true;
                TATransition transition = edge;
                transition.target = dummyAcceptingState.get();

                transition.guard.reserve(transition.guard.size() + 2);
                // upper bound
                if (regExprWithin.second->upperBound.second) {
                  transition.guard.emplace_back(
                                                TimedAutomaton::X(out.clockSize()) <= regExprWithin.second->upperBound.first);
                } else {
                  transition.guard.emplace_back(
                                                TimedAutomaton::X(out.clockSize()) < regExprWithin.second->upperBound.first);
                }
                // lower bound
                if (regExprWithin.second->lowerBound.second) {
                  transition.guard.emplace_back(
                                                TimedAutomaton::X(out.clockSize()) >= regExprWithin.second->lowerBound.first);
                } else {
                  transition.guard.emplace_back(
                                                TimedAutomaton::X(out.clockSize()) > regExprWithin.second->lowerBound.first);
                }
                edges.second.emplace_back(std::move(transition));
              }
            }
          }
        }
      }
      for (auto state: out.states) {
        if (!state->next.empty()) {
          state->isMatch = false;
        }
      }
      if (isDummyUsed) {
        out.states.emplace_back(std::move(dummyAcceptingState));
      }
      out.maxConstraints.emplace_back(regExprWithin.second->upperBound.first);
      break;
    }
  }
  // Merge accepting states w/o outgoing transitions
  std::vector<std::shared_ptr<TAState>> terminalAcceptingStates;
  for (auto &s: out.states) {
    if (s->isMatch && s->next.empty()) {
      terminalAcceptingStates.push_back(s);
    }
  }
  std::sort(terminalAcceptingStates.begin(), terminalAcceptingStates.end());
  for (auto &source: out.states) {
    for (auto &transitionVectorPair: source->next) {
      for (auto &transition: transitionVectorPair.second) {
        if (std::find_if(terminalAcceptingStates.begin(), terminalAcceptingStates.end(),
                         [&](std::shared_ptr<TAState> sp) {
                           return sp.get() == transition.target;
                         }) != terminalAcceptingStates.end()) {
          transition.target = terminalAcceptingStates.front().get();
        }
      }
    }
  }
  if (terminalAcceptingStates.size() > 1) {
    out.states.erase(std::remove_if(out.states.begin(), out.states.end(),
                                    [&terminalAcceptingStates](const std::shared_ptr<TAState> s) {
                                      return std::binary_search(std::next(terminalAcceptingStates.begin()),
                                                                terminalAcceptingStates.end(), s);
                                    }), out.states.end());
    std::sort(out.initialStates.begin(), out.initialStates.end());
    if (!std::binary_search(out.initialStates.begin(), out.initialStates.end(), terminalAcceptingStates.front()) &&
        std::find_if(out.initialStates.begin(), out.initialStates.end(),
                     [&terminalAcceptingStates](const std::shared_ptr<TAState> s) {
                       return std::binary_search(std::next(terminalAcceptingStates.begin()),
                                                 terminalAcceptingStates.end(), s);
                     }) != out.initialStates.end()) {
      out.initialStates.push_back(terminalAcceptingStates.front());
    }
    out.initialStates.erase(std::remove_if(out.initialStates.begin(), out.initialStates.end(),
                                           [&terminalAcceptingStates](const std::shared_ptr<TAState> s) {
                                             return std::binary_search(std::next(terminalAcceptingStates.begin()),
                                                                       terminalAcceptingStates.end(), s);
                                           }), out.initialStates.end());
  }
  reduceStates(out);
}

//! rename to epsilon transitions
void renameToEpsilonTransitions(TimedAutomaton &out) {
  std::unordered_map<TAState *, std::shared_ptr<TAState>> toAccepting;
  const auto haveToChangeToEpsilon = [](const TAState *s, const Alphabet c) {
    static std::unordered_map<const TAState *, Alphabet> output;
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

void toSignalTA(std::shared_ptr<const TRE> tre, TimedAutomaton &out) {
  DNFTRE dnftre(tre);

  dnftre.toNormalForm();
  dnftre.toSignalTA(out);

  renameToEpsilonTransitions(out);
}
