#pragma once

#include <cassert>
#include <memory>
#include <iostream>
#include "interval.hh"

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
    @brief construct an NFA without unnecessary states (every states are reachable from an initial state and reachable to an accepting state)
  */
  // void toNFA(tmpNFA& out) const {
  //   switch(tag) {
  //   case op::atom:
  //     out = tmpNFA(c);
  //     break;
  //   case op::epsilon:
  //     out = tmpNFA();
  //     break;
  //   case op::plus: {
  //     regExpr->toNFA(out);
  //     for (auto &s: out.states) {
  //       for(auto &ns: s->next) {
  //         // if we can go to an accepting state, we add transitions to initial states.
  //         if(std::any_of(ns.begin(), ns.end(), [](std::weak_ptr<NFAState> ps) {
  //               return ps.lock()->isMatch;
  //             })) {
  //           ns.insert(ns.end(), out.initStates.begin(), out.initStates.end());
  //         }
  //       }
  //     }
  //     // append an immidiate accepting state
  //     out.states.push_back(std::make_shared<NFAState>(true));
  //     out.initStates.push_back(out.states.back());
  //     break;
  //   }
  //   case op::concat: {
  //     regExprPair.first->toNFA(out);
  //     tmpNFA another;
  //     regExprPair.second->toNFA(another);
  //     for (auto &s: out.states) {
  //       for(auto &ns: s->next) {
  //         if(std::any_of(ns.begin(), ns.end(), [](std::weak_ptr<NFAState> ps) {
  //               return ps.lock()->isMatch;
  //             })) {
  //           // append the transitions to the another's initial states
  //           ns.insert(ns.end(), another.initStates.begin(), another.initStates.end());
  //         }
  //       }
  //     }
  //     // remove accepting states in out they are not used anymore
  //     for (auto &s: out.states) {
  //       s->isMatch = false;
  //     }
  //     // remove any accepting states of out that is unreachable to an initial state of another ( remove states without any transitions to other states)
  //     size_t statesNum = out.states.size();
  //     do {
  //       statesNum = out.states.size();
  //       out.states.remove_if([] (std::shared_ptr<NFAState> &s) {
  //           return std::all_of(s->next.begin(), s->next.end(), [] (std::vector<std::weak_ptr<NFAState>> states) {
  //               return states.empty();
  //             });
  //         });
  //     } while (statesNum != out.states.size());

  //     out.states.splice(out.states.end(), another.states);
  //     for (auto &s: out.states) {
  //       for (std::vector<std::weak_ptr<NFAState>> &states: s->next) {
  //         states.erase(std::remove_if(states.begin(), states.end(), [&out](std::weak_ptr<NFAState> pstate) {
  //               return std::find_if(out.states.begin(), out.states.end(), [&pstate](std::shared_ptr<NFAState> &s) {
  //                   return s == pstate.lock();
  //                 }) == out.states.end();
  //             }), states.end());
  //       }
  //     }
  //     out.initStates.remove_if([&out](std::shared_ptr<NFAState> pstate) {
  //         return std::find_if(out.states.begin(), out.states.end(), [&pstate](std::shared_ptr<NFAState> s) {
  //             return s == pstate;
  //           }) == out.states.end();
  //       });
  //     break;
  //   }
  //   case op::disjunction: {
  //     regExprPair.first->toNFA(out);
  //     tmpNFA another;
  //     regExprPair.second->toNFA(another);
  //     out.states.splice(out.states.end(), another.states);
  //     out.initStates.splice(out.initStates.end(), another.initStates);
  //     break;
  //   }
  //   case op::conjunction: {
  //     regExprPair.first->toNFA(out);
  //     tmpNFA another;
  //     regExprPair.second->toNFA(another);
  //     out.states.splice(out.states.end(), another.states);
  //     out.initStates.splice(out.initStates.end(), another.initStates);
  //     break;
  //   }
  //   case op::within: {
  //     regExprPair.first->toNFA(out);
  //     tmpNFA another;
  //     regExprPair.second->toNFA(another);
  //     out.states.splice(out.states.end(), another.states);
  //     out.initStates.splice(out.initStates.end(), another.initStates);
  //     break;
  //   }
  //   }
  // }

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

