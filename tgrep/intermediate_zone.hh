#pragma once

#include <iostream>
#include <boost/variant.hpp>

#include "zone.hh"

class IntermediateZone : public Zone {
private:
  std::vector<bool> isAllocated;
  ClockVariables newestClock;
  static constexpr ClockVariables initialClock = 1;
public:
  /*!
    @brief construct an intermediate zone from a zone

    x0 is the special constant variable (x0 == 0) as usual.
    When x1, the trimming starts and the last variable is the newest one.
   */
  IntermediateZone(Zone zone, std::size_t currentNewestClock = 0) : Zone(std::move(zone)) {
    // The zone must have at least one variable
#ifdef DEBUG
    assert(value.cols() == value.rows());
    assert(value.cols() > 1);
#endif
    isAllocated.resize(value.cols(), false);
    if (currentNewestClock) {
      newestClock = currentNewestClock;
      for (ClockVariables x = currentNewestClock + 1; x < value.cols(); x++) {
        deallocate(x);
      }
    } else {
      newestClock = value.cols() - 1;
    }
    isAllocated[0] = true;
    isAllocated[initialClock] = true;
    isAllocated[newestClock] = true;
  }

  ClockVariables alloc(const std::pair<double,bool> &upperBound, std::pair<double,bool> lowerBound = {0, true}) {
    static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
    const auto newPos = std::find(isAllocated.begin(), isAllocated.end(), false);
#ifdef DEBUG
    assert(value.cols() == value.rows());
#endif

    ClockVariables newClock;
    if (newPos != isAllocated.end()) {
      // we do not have to alloc
      newClock = newPos - isAllocated.begin();
    } else {
      // we have to alloc
      //! @note this allocation is expected to be avoided by preallocation
      throw "monaa: Unexpected Variable Allocation";
      //! @note Eigen's resize changes the value.
      value.conservativeResize(value.cols() + 1, value.cols() + 1);
      newClock = value.cols() - 1;
      isAllocated.resize(value.cols());
      value(newClock, newClock) = Bounds(0, true);
    }
    isAllocated[newClock] = true;
    value(newClock, 0) = upperBound;
    lowerBound.first = -lowerBound.first;
    value(0, newClock) = lowerBound;
    value(newClock, newestClock) = Bounds(infinity);
    value(newestClock, newClock) = {0, true};
#ifdef DEBUG
    assert(value.cols() == value.rows());
#endif
    return newestClock = newClock;
  }

  void update(const std::vector<boost::variant<double, ClockVariables>> &resetTime) {
#ifdef DEBUG
    assert(value.cols() == value.rows());
#endif
    std::fill(isAllocated.begin(), isAllocated.end(), false);
    isAllocated[0] = true;
    isAllocated[initialClock] = true;
    isAllocated[newestClock] = true;
    for (auto rtime: resetTime) {
      const ClockVariables* p_x = boost::get<ClockVariables>(&rtime);
      if (p_x) {
        isAllocated[*p_x] = true;
      }
    }
    for (ClockVariables x = 1; x < value.cols(); x++) {
      if (!isAllocated[x]) {
        deallocate(x);
      }
    }
  }

  void toAns(Zone &ansZone) const {
    ansZone = Zone::zero(3);
    ansZone.value(0, 1) = value(0, 1);
    ansZone.value(1, 0) = value(1, 0);
    ansZone.value(0, 2) = value(0, newestClock);
    ansZone.value(2, 0) = value(newestClock, 0);
    ansZone.value(1, 2) = value(1, newestClock);
    ansZone.value(2, 1) = value(newestClock, 1);
  }

  /*!
    @brief add the constraint x - y \le (c,s)
    @note This is different from Zone::tighten because we have to handle x0, too. That is unnecessary and harmful in zone construction.
   */
  void tighten(const ClockVariables x, const ClockVariables y, const Bounds &c) {
    value(x,y) = std::min(value(x, y), c);
    if (value(x,y) == c && newestClock != initialClock) {
      close1(x);
      close1(y);
    }
  }

  void tighten(const ClockVariables x, const Constraint &c, const ClockVariables reset = 0) {
    switch (c.odr) {
    case Constraint::Order::lt:
      tighten(x, reset, Bounds{c.c, false});
      break;
    case Constraint::Order::le:
      tighten(x, reset, Bounds{c.c, true});
      break;
    case Constraint::Order::gt:
      tighten(reset, x, Bounds{-c.c, false});
      break;
    case Constraint::Order::ge:
      tighten(reset, x, Bounds{-c.c, true});
      break;
    }
  }

  void tighten(const ClockVariables x, const Constraint &c, const double t) {
    switch (c.odr) {
    case Constraint::Order::lt:
      tighten(x, 0, Bounds{c.c + t, false});
      break;
    case Constraint::Order::le:
      tighten(x, 0, Bounds{c.c + t, true});
      break;
    case Constraint::Order::gt:
      tighten(0, x, Bounds{-c.c - t, false});
      break;
    case Constraint::Order::ge:
      tighten(0, x, Bounds{-c.c - t, true});
      break;
    }
  }

  void tighten(const std::vector<Constraint> &constraints, const std::vector<boost::variant<double, ClockVariables>> &resetTime) {
    for (const Constraint &guard: constraints) {
      const double* p_resetDouble = boost::get<double>(&resetTime[guard.x]);
      const ClockVariables* p_resetClock = boost::get<ClockVariables>(&resetTime[guard.x]);
      if (p_resetDouble) {
        tighten(newestClock, guard, *p_resetDouble);
      } else {
        tighten(newestClock, guard, *p_resetClock);
      }
    }
  }

  void tighten(const std::vector<Constraint> &constraints, const std::vector<boost::variant<double, ClockVariables>> &resetTime, const double t) {
    for (const Constraint &guard: constraints) {
      const double* p_resetDouble = boost::get<double>(&resetTime[guard.x]);
      const ClockVariables* p_resetClock = boost::get<ClockVariables>(&resetTime[guard.x]);
      if (p_resetDouble) {
        // This constraint is unsatisfyable
        if (!guard.satisfy(t - *p_resetDouble)) {
          makeUnsat();
        }
      } else {
        switch (guard.odr) {
        case Constraint::Order::lt:
          tighten(0, *p_resetClock, Bounds{guard.c - t, false});
          break;
        case Constraint::Order::le:
          tighten(0, *p_resetClock, Bounds{guard.c - t, true});
          break;
        case Constraint::Order::gt:
          tighten(*p_resetClock, 0, Bounds{t - guard.c, false});
          break;
        case Constraint::Order::ge:
          tighten(*p_resetClock, 0, Bounds{t - guard.c, true});
          break;
        }
      }
    } 
  }

  bool isSatisfiableCanonized() {
    return (value + value.transpose()).minCoeff() >= Bounds(0.0, true);
  }

  void deallocate(const ClockVariables x) {
    static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
    static constexpr Bounds zero = Bounds(0, true);
    value.col(x).fill(infinity);
    value.row(x).fill(infinity);
    value(x, x) = zero;
  }
};
