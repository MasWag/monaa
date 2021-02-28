#pragma once
/*!
  @file interval.hh
  @author Masaki Waga
  @brief The implementation of the class Interval and related functions
 */

#include <cmath>

#include "zone.hh"

/*!
  @brief Class for an interval
 */
struct Interval {
  //! the lower bound of the interval
  Bounds lowerBound; 
  //! the upper bound of the interval
  Bounds upperBound; 

  /*!
    @brief Defail constructor
    
    By defalt the it returns the interval [0,∞)
   */
  Interval() {
    upperBound = Bounds{std::numeric_limits<double>::infinity(), false};
    lowerBound = {0, true};
  }

  /*!
    @brief Constructor returning the interval (lower, ∞)

    @param [in] lower the lower bound
   */
  Interval(int lower) {
    upperBound = Bounds{std::numeric_limits<double>::infinity(), false};
    lowerBound = {lower, false};
  }

  /*!
    @brief Constructor returning the interval (lower, upper)

    @param [in] lower the lower bound
    @param [in] upper the upper bound
   */
  Interval(int lower, int upper) {
    upperBound = {upper, false};
    lowerBound = {lower, false};
  }

  Interval(Bounds lower, Bounds upper) : lowerBound(lower), upperBound(upper){}

  /*!
    @brief The Kleene plus operator on intervals in [Dima'00]
    
    For an interval \f$I\f$, the @em Kleen @em plus @em closure \f$I^+\f$ of \f$I\f$ is a set of intervals satisfying the following.

    \f[
    \bigcup_{i \in N} I^i = \bigcup_{I' \in I^+} I'
    \f]

    - [Dima'00]: Dima C. (2000) Real-Time Automata and the Kleene Algebra of Sets of Real Numbers. In: Reichel H., Tison S. (eds) STACS 2000. STACS 2000. Lecture Notes in Computer Science, vol 1770. Springer, Berlin, Heidelberg

    @param [out] plusIntervals the result
   */
  inline void plus(std::vector<std::shared_ptr<Interval>> &plusIntervals) {
    plusIntervals.clear();
    if (std::isinf(upperBound.first)) {
      plusIntervals = {std::make_shared<Interval>(lowerBound, upperBound)};
      return;
    }
    const int m = ceil(double(lowerBound.first) / double(upperBound.first - lowerBound.first));
    plusIntervals.reserve(m + 1);
    for (int i = 1; i < m; i++) {
      plusIntervals.emplace_back(std::make_shared<Interval>(Bounds{lowerBound.first * i, lowerBound.second},
                                                            Bounds{upperBound.first * i, upperBound.second}));
    }
    plusIntervals.emplace_back(std::make_shared<Interval>(Bounds{lowerBound.first * m, lowerBound.second},
                                                          Bounds{std::numeric_limits<double>::infinity(), false}));
  }

  inline bool contain(double value) const {
    return (lowerBound.second ? lowerBound.first <= value : lowerBound.first < value) and
      (upperBound.second ? value <= upperBound.first : value < upperBound.first);
  }
};

//! @brief The intersection of two intervals
inline static Interval
operator&&( const Interval &left, const Interval &right) {
  Interval ret = Interval{std::max(left.lowerBound, right.lowerBound),
                          std::min(left.upperBound, right.upperBound)};
  return ret; 
}

/*!
  @brief The intersection of a set of intervals and an interval.
  @note This function is destructive.

  @param [in,out] left The set of intervals. We take intersection for each element of left and overwrite.
  @param [in] right The interval.

  We have the following relation between the input and the overwritten value of \f$\texttt{left}\f$.

  \f[t \in \bigcup_{I \in \texttt{left}_{\mathrm{post}}} I \iff t \in \bigcup_{I \in \texttt{left}_{\mathrm{pre}}} (I \cap \texttt{right})\f]
 */
inline static void
land( std::vector<std::shared_ptr<Interval>> &left, const Interval &right) {
  for (auto interval: left) {
    (*interval) = (*interval) && right;
  }
  left.erase(std::remove_if(left.begin(), left.end(), [](std::shared_ptr<Interval> interval) {
        return interval->lowerBound > interval->upperBound;
      }), left.end());
}

/*!
  @brief The intersection of two sets of intervals.
  @note This function is destructive.

  @param [in,out] left The set of intervals. We take intersection for each element of left and overwrite.
  @param [in] right A set of interval. This is not overwritten.

  We have the following relation between the input and the overwritten value of \f$\texttt{left}\f$.

  \f[t \in \bigcup_{I \in \texttt{left}_{\mathrm{post}}} I \iff t \in \bigcup_{I \in \texttt{left}_{\mathrm{pre}}, I' \in \texttt{right}} (I \cap I')\f]
 */
inline static void
land( std::vector<std::shared_ptr<Interval>> &left, const std::vector<std::shared_ptr<Interval>> &right) {
  std::vector<std::shared_ptr<Interval>> tmp;
  for (auto interval: right) {
    std::vector<std::shared_ptr<Interval>> tmpL = left;
    for (auto &ptr: tmpL) {
      ptr = std::make_shared<Interval>(*ptr);
    }
    land(tmpL, *interval);
    tmp.insert(tmp.end(), tmpL.begin(), tmpL.end());
  }

#ifdef DEBUG
  assert(left.size() * right.size() == tmp.size());
#endif
  left = tmp;
}

/*!
  @brief The sum of two intervals. The formal definition is as follows.
  
  \f[I + I' = \{t + t' \mid \exists t \in I, t' \in I'\}\f]
*/
inline static Interval
operator+( Interval left, const Interval &right) {
  left.lowerBound += right.lowerBound;
  left.upperBound += right.upperBound;
  return left;
}

/*!
  @brief The sum of two sets of intervals.
  @note This function is destructive.

  @param [in,out] left The set of intervals. We take sum for each element of left and overwrite.
  @param [in] right A set of interval. This is not overwritten.

  We have the following for any \f$t \in \mathbb{R}\f$.

  \f[t \in \bigcup_{I \in \texttt{left}_{\mathrm{post}}} I \iff \exists t' \in \bigcup_{I \in \texttt{left}_{\mathrm{pre}}}, \exists t'' \in \bigcup_{I' \in \texttt{right}}. t = t' + t'' \f]
 */
inline static void
operator+=( std::vector<std::shared_ptr<Interval>> &left, const std::vector<std::shared_ptr<Interval>> &right) {
  std::vector<std::shared_ptr<Interval>> ans;
  ans.reserve(left.size() * right.size());
  for (auto intervalLeft: left) {
    for (auto intervalRight: right) {
      ans.emplace_back(std::make_shared<Interval>(*intervalLeft + *intervalRight));
    }
  }
  left = std::move(ans);
}

//! @todo Optimization: We can use * - 0 instead of cup of +
inline void
plus( std::vector<std::shared_ptr<Interval>> &intervals) {
  if (intervals.size() >= 32) {
    throw "too large expression";
  }
  std::vector<std::vector<std::shared_ptr<Interval>>> plusIntervals;
  plusIntervals.resize(intervals.size());
  for (std::size_t i = 0; i < intervals.size(); i++) {
    if (std::isinf(intervals[i]->upperBound.first)) {
      plusIntervals[i] = {intervals[i]};
    } else {
      intervals[i]->plus(plusIntervals[i]);
    }
  }
  if (plusIntervals.size() == 1) {
    intervals = std::move(plusIntervals[0]);
    return;
  }

  std::vector<std::shared_ptr<Interval>> ansIntervals;
  const uint32_t subsetSize = 1 << intervals.size();
  for (uint32_t i = 1; i < subsetSize; i++) {
    std::vector<const std::vector<std::shared_ptr<Interval>>*> subSetVec;
    subSetVec.reserve(intervals.size());
    for (std::size_t j = 0; j < intervals.size(); j++) {
      if( (1<<j) & i){
        subSetVec.emplace_back(&(plusIntervals[j]));
      }
    }
    std::vector<std::shared_ptr<Interval>> tmpIntervals = {std::make_shared<Interval>(Bounds{0, true}, Bounds{0, true})};
    for (const auto &intervals : subSetVec) {
      tmpIntervals += *intervals;
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
