#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <boost/unordered_map.hpp>

#include "common_types.hh"
#include "constraint.hh"

using Bounds = std::pair<double, bool>;
static inline Bounds
operator+ (const Bounds &a,const Bounds &b) {
  return Bounds(a.first + b.first, a.second && b.second);
}
static inline Bounds
operator- (const Bounds &a,const Bounds &b) {
  return Bounds(a.first - b.first, a.second && b.second);
}
static inline void
operator+= (Bounds &a, const Bounds b) {
  a.first += b.first;
  a.second = a.second && b.second;
}
static inline std::ostream& operator << (std::ostream& os, const Bounds& b) {
  os << "(" << b.first << ", " << b.second << ")";
  return os;
}

#include <eigen3/Eigen/Core>
//! @TODO configure include directory for eigen

struct Zone {
  using Tuple = std::tuple<std::vector<Bounds>,Bounds>;
  Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic> value;
  Bounds M;

  inline std::size_t getNumOfVar() const {
    return value.cols() - 1;
  }

  inline void cutVars (std::shared_ptr<Zone> &out,std::size_t from,std::size_t to) {
    out = std::make_shared<Zone>();
    out->value.resize(to - from + 2, to - from + 2);
    out->value.block(0,0,1,1) << Bounds(0,true);
    out->value.block(1, 1, to - from + 1, to - from + 1) = value.block(from + 1, from + 1, to - from + 1,to - from + 1);
    out->value.block(1, 0, to - from + 1, 1) = value.block(from + 1, 0, to - from + 1, 1);
    out->value.block(0, 1, 1, to - from + 1) = value.block(0, from + 1, 1, to - from + 1);
    out->M = M;
  }
  
  static Zone zero(int size) {
    static Zone zeroZone;
    if (zeroZone.value.cols() == size) {
      return zeroZone;
    }
    zeroZone.value.resize(size, size);
    zeroZone.value.fill(Bounds(0, true));
    return zeroZone;
  }

  std::tuple<std::vector<Bounds>,Bounds> toTuple() const {
    // omit (0,0)
    return std::tuple<std::vector<Bounds>,Bounds>(std::vector<Bounds>(value.data() + 1, value.data() + value.size()),M);
  }

  //! @brief add the constraint x - y \le (c,s)
  void tighten(ClockVariables x, ClockVariables y, Bounds c) {
    x++;
    y++;
    value(x,y) = std::min(value(x, y), c);
    close1(x);
    close1(y);
  }

  void close1(ClockVariables x) {
    for (int i = 0; i < value.rows(); i++) {
      value.row(i) = value.row(i).array().min(value.row(x).array() + value(i, x));
      //      for (int j = 0; j < value.cols(); j++) {
      //        value(i, j) = std::min(value(i, j), value(i, x) + value(x, j));
      //      }
    }
  }
  
  // The reset value is always (0, \le)
  void reset(ClockVariables x) {
    // 0 is the special varibale here
    x++;
    value(0,x) = Bounds(0, true);
    value(x,0) = Bounds(0, true);
    value.col(x).tail(value.rows() - 1) = value.col(0).tail(value.rows() - 1);
    value.row(x).tail(value.cols() - 1) = value.row(0).tail(value.cols() - 1);
  }
  
  void elapse() {
    static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
    value.col(0).fill(Bounds(infinity));
    for (int i = 0; i < value.row(0).size(); ++i) {
      value.row(0)[i].second = false;
    }
  }

  void canonize() {
    for (int k = 0; k < value.cols(); k++) {
      close1(k);
    }
  }

  bool isSatisfiable() {
    canonize();
    return (value + value.transpose()).minCoeff() >= Bounds(0.0, true);
  }

  void abstractize() {
    static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
    for (auto it = value.data(); it < value.data() + value.size(); it++) {
      if (*it >= M) {
        *it = Bounds(infinity);
      }
    }
  }

  void makeUnsat() {
    value(0, 0) = Bounds(-std::numeric_limits<double>::infinity(), false);
  }

  bool operator== (Zone z) const {
    z.value(0,0) = value(0,0);
    return value == z.value;
  }
};

// struct ZoneAutomaton : public AbstractionAutomaton<Zone> {
//   struct TAEdge {
//     State source;
//     State target;
//     Alphabet c;
//     std::vector<Alphabet> resetVars;
//     std::vector<Constraint> guard;
//   };

//   boost::unordered_map<std::tuple<State, State, Alphabet>, TAEdge> edgeMap;
//   boost::unordered_map<std::pair<TAState, typename Zone::Tuple>, RAState> zones_in_za;
//   int numOfVariables;
// };

// static inline std::ostream& operator << (std::ostream& os, const Zone& z) {
//   for (int i = 0; i < z.value.rows();i++) {
//     for (int j = 0; j < z.value.cols();j++) {
//       os << z.value(i,j);
//     }
//     os << "\n";
//   }
//   os << std::endl;
//   return os;
// }


