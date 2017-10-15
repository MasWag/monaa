#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "common_types.hh"

//! @brief The return values of comparison of two values. Similar to strcmp.
enum class Order {
  LT, EQ, GT
};

inline bool toBool (Order odr) {
  return odr == Order::EQ;
}

//! @brief A constraint in a guard of transitions
struct Constraint {
  enum class Order {
    lt,le,ge,gt
  };

  ClockVariables x;
  Order odr;
  int c;

  bool satisfy (double d) const {
    switch (odr) {
    case Order::lt:
      return d < c;
    case Order::le:
      return d <= c;
    case Order::gt:
      return d > c;
    case Order::ge:
      return d >= c;
    }
    return false;
  }
  using Interpretation = std::vector<double>;
  ::Order operator() (Interpretation val) const {
    if (satisfy (val.at (x))) {
      return ::Order::EQ;
    } else if (odr == Order::lt || odr == Order::le) {
      return ::Order::GT;
    } else {
      return ::Order::LT;
    }
  }
};

// An interface to write an inequality constrait easily
class ConstraintMaker {
  ClockVariables x;
public:
  ConstraintMaker(ClockVariables x) : x(x) {}
  Constraint operator<(int c) {
    return Constraint {x, Constraint::Order::lt, c};
  }
  Constraint operator<=(int c) {
    return Constraint {x, Constraint::Order::le, c};
  }
  Constraint operator>(int c) {
    return Constraint {x, Constraint::Order::gt, c};
  }
  Constraint operator>=(int c) {
    return Constraint {x, Constraint::Order::ge, c};
  }
};

/*!
  @brief remove any inequality x > c or x >= c
 */
static inline void widen(std::vector<Constraint> &guard) {
  guard.erase(std::remove_if(guard.begin(), guard.end(), [](Constraint g) {
        return g.odr == Constraint::Order::ge || g.odr == Constraint::Order::gt;
      }), guard.end());
}
