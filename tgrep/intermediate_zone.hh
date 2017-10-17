#pragma once

#include <iostream>
#include <boost/variant.hpp>

#include "interval.hh"
#include "zone.hh"

class IntermediateZone : public Zone {
private:
  std::vector<bool> isAllocated;
  static constexpr ClockVariables initialClock = 1;
public:
  ClockVariables newestClock;
  // intervals]0]: t
  // intervals]1]: t'
  // intervals]2]: t' - t
  // This is not an intervals but cells of DBMs
  std::vector<Interval> intervals;
  bool useInterval;
  /*!
    @brief construct an intermediate zone from an interval

   */
  IntermediateZone(const Interval& interval) : intervals({interval}) {
    useInterval = true;
    intervals[0].lowerBound.first *= -1;
    newestClock = 1;
  }

  /*!
    @brief construct an intermediate zone from a zone

    x0 is the special constant variable (x0 == 0) as usual.
    When x1, the trimming starts and the last variable is the newest one.
   */
  IntermediateZone(Zone zone, std::size_t currentNewestClock = 0) : Zone(std::move(zone)) {
    useInterval = false;
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
    ClockVariables newClock;
    if (useInterval && intervals.size() == 1) {
      // use intervals
      intervals.resize(3);
      intervals[1].upperBound = upperBound;
      intervals[1].lowerBound = lowerBound;
      intervals[1].lowerBound.first *= -1;
      // value(2, 1) <= value(2, 0) + value(0, 1)
      intervals[2].upperBound = intervals[1].upperBound + intervals[0].lowerBound;
      // value(1, 2) <= value(1, 0) + value(0, 2)
      intervals[2].lowerBound = std::min(intervals[0].upperBound + intervals[1].lowerBound, Bounds{0, true});
      return newestClock = 2;
    } else if (useInterval) {
      // convert intervals to a zone
      useInterval = false;
      isAllocated.resize(4, true);
      value.resize(4, 4);
      value.fill(Bounds(infinity));
      for (int i = 0; i < 4; i++)  {
        value(i, i) = Bounds{0, true};
      }
      value(1, 0) = intervals[0].upperBound;
      value(0, 1) = intervals[0].lowerBound;
      value(2, 0) = intervals[1].upperBound;
      value(0, 2) = intervals[1].lowerBound;
      value(2, 1) = intervals[2].upperBound;
      value(1, 2) = intervals[2].lowerBound;
      value(2, 3) = Bounds{0, true};
      value(3, 0) = upperBound;
      value(0, 3) = lowerBound;
      value(0, 3).first *= -1;
      intervals.clear();
      newClock = 3;
    } else {
      const auto newPos = std::find(isAllocated.begin(), isAllocated.end(), false);
#ifdef DEBUG
      assert(value.cols() == value.rows());
#endif

      if (newPos != isAllocated.end()) {
        // we do not have to alloc
        newClock = newPos - isAllocated.begin();
      } else {
        // we have to alloc
        //! @note this allocation is expected to be avoided by preallocation
        // throw "monaa: Unexpected Variable Allocation";
        //! @note Eigen's resize changes the value.
        value.conservativeResize(value.cols() + 1, value.cols() + 1);
        newClock = value.cols() - 1;
        isAllocated.resize(value.cols());
        value.row(newClock).fill(infinity);
        value.col(newClock).fill(infinity);
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
    }
    for (int i = 0; i < value.rows(); i++) {
      // for (int x = 0; x < value.cols(); x++) {
      //   value(i, newClock) = std::min(value(i, newClock), value(i, x) + value(x, newClock));
      // }
      value(i, newClock) = std::min(value(i, newClock), (value.row(i) + value.col(newClock).transpose()).minCoeff());
    }
    for (int x = 0; x < value.rows(); x++) {
      for (int j = 0; j < value.cols(); j++) {
        value(newClock, j) = std::min(value(newClock, j), value(newClock, x) + value(x, j));
      }
    }
    return newestClock = newClock;
  }

  void update(const std::vector<boost::variant<double, ClockVariables>> &resetTime) {
    if (useInterval) {
      return;
    }
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
    if (useInterval) {
      if (intervals.size() < 3) {
        return;
      }
      ansZone.value(0, 1) = intervals[0].lowerBound;
      ansZone.value(1, 0) = intervals[0].upperBound;
      ansZone.value(0, 2) = intervals[1].lowerBound;
      ansZone.value(2, 0) = intervals[1].upperBound;
      ansZone.value(1, 2) = intervals[2].lowerBound;
      ansZone.value(2, 1) = intervals[2].upperBound;
    } else {
      ansZone.value(0, 1) = value(0, 1);
      ansZone.value(1, 0) = value(1, 0);
      ansZone.value(0, 2) = value(0, newestClock);
      ansZone.value(2, 0) = value(newestClock, 0);
      ansZone.value(1, 2) = value(1, newestClock);
      ansZone.value(2, 1) = value(newestClock, 1);
    }
  }

  /*!
    @brief add the constraint x - y \le (c,s)
    @note This is different from Zone::tighten because we have to handle x0, too. That is unnecessary and harmful in zone construction.
   */
  void tighten(const ClockVariables x, const ClockVariables y, Bounds c) {
    if (useInterval) {
      if (x == 0 && y == 1) {
        intervals[0].lowerBound = std::min(intervals[0].lowerBound, c);
        if (intervals.size() > 1) {
          // value(0, 2) <= value(0, 1) + value(1, 2)
          intervals[1].lowerBound = std::min(intervals[1].lowerBound, 
                                             intervals[0].lowerBound + intervals[2].lowerBound);
          // value(2, 1) <= value(2, 0) + value(0, 1)
          intervals[2].upperBound = std::min(intervals[2].upperBound, 
                                             intervals[1].upperBound + intervals[0].lowerBound);
        }
      } else if (x == 1 && y == 0) {
        intervals[0].upperBound = std::min(intervals[0].upperBound, c);
        if (intervals.size() > 1) {
          // value(2, 0) <= value(2, 1) + value(1, 0)
          intervals[1].upperBound = std::min(intervals[1].upperBound, 
                                             intervals[2].upperBound + intervals[0].upperBound);
          // value(1, 2) <= value(1, 0) + value(0, 2)
          intervals[2].lowerBound = std::min(intervals[2].lowerBound, 
                                             intervals[0].upperBound + intervals[1].lowerBound);
        }
      } else if (x == 0 && y == 2) {
        intervals[1].lowerBound = std::min(intervals[1].lowerBound, c);
        intervals[0].lowerBound = std::min(intervals[0].lowerBound, 
                                           intervals[1].lowerBound + intervals[2].upperBound);
        intervals[2].lowerBound = std::min(intervals[2].lowerBound, 
                                           intervals[1].lowerBound + intervals[0].upperBound);
      } else if (x == 2 && y == 0) {
        intervals[1].upperBound = std::min(intervals[1].upperBound, c);
        intervals[0].upperBound = std::min(intervals[0].upperBound, 
                                           intervals[1].upperBound + intervals[2].lowerBound);
        intervals[2].upperBound = std::min(intervals[2].upperBound, 
                                           intervals[1].upperBound + intervals[0].lowerBound);
      } else if (x == 1 && y == 2) {
        intervals[2].lowerBound = std::min(intervals[2].lowerBound, c);
        intervals[0].upperBound = std::min(intervals[0].upperBound, 
                                           intervals[1].upperBound + intervals[2].lowerBound);
        intervals[1].lowerBound = std::min(intervals[1].lowerBound, 
                                           intervals[2].lowerBound + intervals[0].lowerBound);
      } else if (x == 2 && y == 1) {
        intervals[2].upperBound = std::min(intervals[2].upperBound, c);
        intervals[1].upperBound = std::min(intervals[1].upperBound, 
                                           intervals[2].upperBound + intervals[0].upperBound);
        intervals[0].lowerBound = std::min(intervals[0].lowerBound, 
                                           intervals[1].lowerBound + intervals[2].upperBound);
      }
    } else {
      value(x,y) = std::min(value(x, y), c);
      if (value(x,y) == c && newestClock != initialClock) {
        close1(x);
        close1(y);
      }
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
          if (useInterval) {
            static constexpr Bounds negInfinity = Bounds(-std::numeric_limits<double>::infinity(), false);
            intervals[0].lowerBound = negInfinity;
          } else {
            makeUnsat();
          }
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
    if (useInterval) {
      return std::all_of(intervals.begin(), intervals.end(), [](const Interval &interval) {
          return interval.upperBound + interval.lowerBound >= Bounds(0.0, true);
        });
    } else {
      return (value + value.transpose()).minCoeff() >= Bounds(0.0, true);
    }
  }

  void deallocate(const ClockVariables x) {
    static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
    static constexpr Bounds zero = Bounds(0, true);
    value.col(x).fill(infinity);
    value.row(x).fill(infinity);
    value(x, x) = zero;
  }
};
