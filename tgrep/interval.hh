#pragma once

#include <cmath>

#include "zone.hh"

struct Interval {
  Bounds lowerBound;
  Bounds upperBound;

  Interval() {
    upperBound = Bounds{std::numeric_limits<double>::infinity(), false};
    lowerBound = {0, true};
  }

  Interval(int lower, int upper) {
    upperBound = {upper, false};
    lowerBound = {lower, false};
  }

  Interval(Bounds lower, Bounds upper) : lowerBound(lower), upperBound(upper){}
  inline void plus(std::vector<std::shared_ptr<Interval>> &plusIntervals) {
    const int m = ceil(double(lowerBound.first) / double(upperBound.first - lowerBound.first));
    plusIntervals.clear();
    plusIntervals.reserve(m + 1);
    for (int i = 1; i < m; i++) {
      plusIntervals.emplace_back(std::make_shared<Interval>(Bounds{lowerBound.first * i, lowerBound.second},
                                                            Bounds{upperBound.first * i, upperBound.second}));
    }
    plusIntervals.emplace_back(std::make_shared<Interval>(Bounds{lowerBound.first * m, lowerBound.second},
                                                          Bounds{std::numeric_limits<double>::infinity(), false}));
  }
};

inline static Interval
operator&&( const Interval &left, const Interval &right) {
  Interval ret = Interval{std::max(left.lowerBound, right.lowerBound),
                          std::min(left.upperBound, right.upperBound)};
  return ret; 
}

inline static void
land( std::vector<std::shared_ptr<Interval>> &left, const Interval &right) {
  for (auto interval: left) {
    (*interval) = (*interval) && right;
  }
  left.erase(std::remove_if(left.begin(), left.end(), [](std::shared_ptr<Interval> interval) {
        return interval->lowerBound > interval->upperBound;
      }), left.end());
}

inline static void
land( std::vector<std::shared_ptr<Interval>> &left, const std::vector<std::shared_ptr<Interval>> &right) {
  std::vector<std::shared_ptr<Interval>> tmp = left;
  for (auto interval: right) {
    std::vector<std::shared_ptr<Interval>> tmpL = left;
    land(tmpL, *interval);
    tmp.insert(tmp.end(), tmpL.begin(), tmpL.end());
  }

  left = tmp;
}

inline static Interval
operator+( Interval left, const Interval &right) {
  left.lowerBound += right.lowerBound;
  left.upperBound += right.upperBound;
  return left;
}

inline static void
operator+=( std::vector<std::shared_ptr<Interval>> &left, const std::vector<std::shared_ptr<Interval>> &right) {
  std::vector<std::shared_ptr<Interval>> ans;
  ans.reserve(left.size() * right.size());
  for (auto intervalLeft: left) {
    for (auto intervalRight: right) {
      ans.emplace_back(std::make_shared<Interval>(*intervalLeft + *intervalRight));
    }
  }
}

//! @todo Optimization: We can use * - 0 instead of cup of +
inline void
plus( std::vector<std::shared_ptr<Interval>> &intervals) {
  if (intervals.size() >= 32) {
    throw "too large expression";
  }
  std::vector<std::vector<std::shared_ptr<Interval>>> plusIntervals;
  plusIntervals.resize(intervals.size());
  for (int i = 0; i < intervals.size(); i++) {
    intervals[i]->plus(plusIntervals[i]);
  }
  std::vector<std::shared_ptr<Interval>> ansIntervals;
  const uint32_t subsetSize = 1 << intervals.size();
  for (uint32_t i = 0; i < subsetSize; i++) {
    std::vector<std::vector<std::shared_ptr<Interval>>> subSetVec;
    subSetVec.reserve(intervals.size());
    for (int j = 0; j < intervals.size(); j++) {
      if( (1<<j) & i){
        subSetVec.push_back(plusIntervals[i]);
      }
    }
    std::vector<std::shared_ptr<Interval>> tmpIntervals = {std::make_shared<Interval>(Bounds{0, true}, Bounds{0, true})};
    for (const auto &intervals : subSetVec) {
      tmpIntervals += intervals;
    }
    ansIntervals.insert(ansIntervals.end(), tmpIntervals.begin(), tmpIntervals.end());
  }
  intervals = std::move(ansIntervals);
}

// inline static std::vector<shared_ptr<Interval>>
// operator&&( const Interval &&left, const Interval &&right) {
//   Interval ret = Interval{std::max(left.lowerBound, right.lowerBound),
//                           std::min(left.upperBound, right.upperBound)};
//   return ret; 
// }

inline static std::ostream& 
operator<<( std::ostream &stream, const Interval& interval)
{
  stream << '(' << interval.lowerBound << ',' << interval.upperBound << ')';
  return stream;
}
