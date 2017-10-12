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
