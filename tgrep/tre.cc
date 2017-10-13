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

    out.states[0]->next[c].push_back({out.states[1], {}, {}});

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
        for (auto &edge: edges) {
          std::shared_ptr<TAState> target = edge.target.lock();
          if (target && target->isMatch) {
            edges.reserve(edges.size() + out.initialStates.size());
            for (auto initState: out.initialStates) {
              TATransition transition = edge;
              edge.target = target;
              edges.emplace_back(std::move(transition));
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
    // make a transition to an accepting state of out to a transition to an initial state of another
    for (auto &s: out.states) {
      for(auto &edges: s->next) {
        for (auto &edge: edges) {
          std::shared_ptr<TAState> target = edge.target.lock();
          if (target && target->isMatch) {
            edges.reserve(edges.size() + another.initialStates.size());
            for (auto initState: another.initialStates) {
              TATransition transition = edge;
              transition.target = initState;
              edges.emplace_back(std::move(transition));
            }
          }
        }
      }
    }

    // make accepting states of out non-accepting
    for (auto &s: out.states) {
      s->isMatch = false;
    }

    out.states.insert(out.states.end(), another.states.begin(), another.states.end());

    // we can reuse variables since we have no overwrapping constraints
    out.maxConstraints.resize(std::max(out.maxConstraints.size(), another.maxConstraints.size()));
    another.maxConstraints.resize(std::max(out.maxConstraints.size(), another.maxConstraints.size()));
    for (int i = 0; i < out.maxConstraints.size(); ++i) {
      out.maxConstraints[i] = std::max(out.maxConstraints[i], another.maxConstraints[i]);
    }
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
    for (int i = 0; i < out.maxConstraints.size(); ++i) {
      out.maxConstraints[i] = std::max(out.maxConstraints[i], another.maxConstraints[i]);
    }
    break;
  }
  case op::conjunction: {
    TimedAutomaton tmpTA;
    TimedAutomaton another;
    boost::unordered_map<std::pair<std::shared_ptr<TAState>,std::shared_ptr<TAState>>, std::shared_ptr<TAState>> toIState;
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
      for (Alphabet c = 0; c < CHAR_MAX; c++) {
        dummyInitialState->next[c].reserve(dummyInitialState->next[c].size() + initialState->next[c].size());
        for (auto& edge: initialState->next[c]) {
          TATransition transition = edge;
          transition.resetVars.push_back(out.clockSize());
          dummyInitialState->next[c].emplace_back(std::move(transition));
        }
      }
    }
    // add dummy accepting state
    std::shared_ptr<TAState> dummyAcceptingState = std::make_shared<TAState>();
    dummyAcceptingState->isMatch = true;
    for (auto state: out.states) {
      for (auto& edges: state->next) {
        for (auto& edge: edges) {
          auto target = edge.target.lock();
          if (target && target->isMatch) {
            TATransition transition = edge;
            transition.target = dummyAcceptingState;
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
            edges.emplace_back(std::move(transition));
          }
        }
      }
    }
    for (auto state: out.states) {
      state->isMatch = false;
    }
    out.initialStates = {dummyInitialState};
    out.states.reserve(out.stateSize() + 2);
    out.states.emplace_back(std::move(dummyInitialState));
    out.states.emplace_back(std::move(dummyAcceptingState));
    out.maxConstraints.emplace_back(regExprWithin.second->upperBound.first);
    break;
  }
  }
}

void TRE::toSignalTA(TimedAutomaton& out) const {
  DNFTRE dnftre(std::shared_ptr<const TRE>(this));

  dnftre.toSignalTA(out);

  //! @todo rename to epsilon transitions
  std::unordered_map<std::shared_ptr<TAState>, std::shared_ptr<TAState>> toAccepting;
  auto haveToChangeToEpsilon = [](std::shared_ptr<TAState> s, char c) {
    return c && !s->next[c].empty();
  };
  for (auto s: out.states) {
    for (char c = 1; c < CHAR_MAX; c++) {
      for (auto it = s->next[c].begin(); it != s->next[c].end(); it++) {
        std::shared_ptr<TAState> target = it->target.lock();
        if (target) {
          if (haveToChangeToEpsilon(target, c)) {
            s->next[0].push_back(*it);
            if (target->isMatch) {
              auto ret = toAccepting.find(target);
              if (ret == toAccepting.end()) {
                toAccepting[target] = std::make_shared<TAState>(true);
              }
              it->target = toAccepting[target];
            } else {
              it = s->next[c].erase(it);
            }
          }
        } else {
          it = s->next[c].erase(it);
        }
      }
    }
  }

  for (const auto &pair: toAccepting) {
    pair.first->isMatch = false;
    out.states.emplace_back(std::move(pair.second));
  }
}
