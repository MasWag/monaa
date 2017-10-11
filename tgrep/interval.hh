#pragma once

#include "zone.hh"

struct Interval {
  Bounds upperBound;
  Bounds lowerBound;

  Interval(int lower, int upper) {
    upperBound = {upper, false};
    lowerBound = {lower, false};
  }
};

inline static std::ostream& 
operator<<( std::ostream &stream, const Interval& interval)
{
  stream << '(' << interval.lowerBound << ',' << interval.upperBound << ')';
  return stream;
}
