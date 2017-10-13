#pragma once

#include <cassert>
#include <memory>
#include <iostream>
#include "interval.hh"
#include "timed_automaton.hh"
#include "intersection.hh"

class SingletonTRE;
class AtomicTRE;
class DNFTRE;

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
  void toEventTA(TimedAutomaton& out) const;

  //! @brief Construct a signal-TA
  void toSignalTA(TimedAutomaton& out) const;

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
  friend SingletonTRE;
  friend AtomicTRE;
  friend DNFTRE;
};
