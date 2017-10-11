#pragma once

#include <cassert>
#include <memory>
#include <iostream>
#include "interval.hh"
#include "timed_automaton.hh"
#include "intersection.hh"

class TRE {
public:
  enum class op {
    atom,
    epsilon,
    plus,
    concat,
    disjunction,
    conjunction,
    within
  };

  static const std::shared_ptr<TRE> epsilon;

  // Atom
  TRE(op tag) : tag(tag) {
    assert(tag == op::epsilon);
  }

  // Atom
  TRE(op tag, char c) : tag(tag), c(c) {
    assert(tag == op::atom);
  }

  // Kleene Plus
  TRE(op tag, const std::shared_ptr<TRE> expr) : tag(tag), regExpr(expr) {
    assert(tag == op::plus);
  }

  // Concat, Disjunction, and Conjunction
  TRE(op tag, const std::shared_ptr<TRE> expr0, const std::shared_ptr<TRE> expr1) : tag(tag), regExprPair(std::pair<std::shared_ptr<TRE>, std::shared_ptr<TRE>>(expr0, expr1)) {
    assert(tag == op::disjunction || tag == op::conjunction || tag == op::concat);
  }

  // Within
  TRE(op tag, const std::shared_ptr<TRE> expr, const std::shared_ptr<Interval> interval) : tag(tag), regExprWithin(std::pair<std::shared_ptr<TRE>, std::shared_ptr<Interval>>(expr, interval)) {
    assert(tag == op::within);
  }

  /*!
    @brief construct an event TA without unnecessary states (every states are reachable from an initial state and reachable to an accepting state)
  */
  void toEventTA(TimedAutomaton& out) const {
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
      out.states.resize(2);
      for (auto &state: out.states) {
        state = std::make_shared<TAState>();
      }
      out.initialStates = {out.states[0]};

      out.states[0]->isMatch = false;
      out.states[1]->isMatch = true;

      out.states[0]->next[0].push_back({out.states[1], {}, {}});
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
      for (auto &s: out.states) {
        for(auto &edges: s->next) {
          for (auto &edge: edges) {
            std::shared_ptr<TAState> target = edge.target.lock();
            if (target && target->isMatch) {
              edges.reserve(edges.size() + another.initialStates.size());
              for (auto initState: another.initialStates) {
                TATransition transition = edge;
                edge.target = target;
                edges.emplace_back(std::move(transition));
              }
            }
          }
        }
        // remove accepting states in out since they are not used anymore
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
      std::vector<std::shared_ptr<TAState>> newStates(out.initialStates.size());
      for (int i = 0; i < out.initialStates.size(); ++i) {
        newStates[i] = std::make_shared<TAState>();
        newStates[i]->next = out.initialStates[i]->next;
        for (auto& edges: newStates[i]->next) {
          for (auto& edge: edges) {
            edge.resetVars.push_back(out.clockSize());
          }
        }
      }
      std::shared_ptr<TAState> dummyAcceptingState = std::make_shared<TAState>();
      dummyAcceptingState->isMatch = true;
      for (auto state: out.states) {
        for (auto& edges: state->next) {
          for (auto& edge: edges) {
            auto target = edge.target.lock();
            if (target && target->isMatch) {
              TATransition transition = edge;
              transition.target = dummyAcceptingState;
              edges.emplace_back(std::move(transition));
            }
          }
        }
      }
      for (auto state: out.states) {
        state->isMatch = false;
      }
      out.initialStates = std::move(newStates);
      out.states.resize(out.stateSize() + out.initialStates.size() + 1);
      out.states.insert(out.states.end(), out.initialStates.begin(), out.initialStates.end());
      out.states.push_back(dummyAcceptingState);
      break;
    }
    }
  }

  ~TRE() {
    switch(tag) {
    case op::atom:
    case op::epsilon:
      break;
    case op::plus:
      regExpr.reset();
      break;
    case op::concat:
    case op::disjunction:
    case op::conjunction:
      regExprPair.first.reset();
      regExprPair.second.reset();
      break;
    case op::within:
      regExprWithin.first.reset();
      regExprWithin.second.reset();
      break;
    }
  }

private:

  const op tag;

  const union {
    char c;
    std::shared_ptr<TRE> regExpr;
    std::pair<std::shared_ptr<TRE>, std::shared_ptr<TRE>> regExprPair;
    std::pair<std::shared_ptr<TRE>, std::shared_ptr<Interval>>  regExprWithin;
  };

  friend std::ostream& operator<<(std::ostream& os, const TRE&);
};

