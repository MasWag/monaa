#pragma once

#include "zone.hh"

struct Interval {
  Bounds lowerBound;
  Bounds upperBound;

  Interval(int lower, int upper) {
    upperBound = {upper, false};
    lowerBound = {lower, false};
  }

  Interval(Bounds lower, Bounds upper) : lowerBound(lower), upperBound(upper){}
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
      }));
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
