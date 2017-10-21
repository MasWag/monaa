#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include <fstream>
#include <iostream>

#include "../libmonaa/timed_automaton.hh"

namespace boost{
  enum vertex_match_t {vertex_match};
  enum edge_label_t {edge_label};
  enum edge_reset_t {edge_reset};
  enum edge_guard_t {edge_guard};
  BOOST_INSTALL_PROPERTY(vertex, match);
  BOOST_INSTALL_PROPERTY(edge, label);
  BOOST_INSTALL_PROPERTY(edge, reset);
  BOOST_INSTALL_PROPERTY(edge, guard);
}

static inline 
std::ostream& operator<<(std::ostream& os, const Constraint::Order& odr) {
  switch (odr) {
  case Constraint::Order::lt:
    os << "<";
    break;
  case Constraint::Order::le:
    os << "<=";
    break;
  case Constraint::Order::ge:
    os << ">=";
    break;
  case Constraint::Order::gt:
    os << ">";
    break;
  }
  return os;
}

static inline 
std::ostream& operator<<(std::ostream& os, const Constraint& p)
{
  os << "x" << int(p.x) << " " << p.odr << " " << p.c;
  return os;
}

template<class T>
static inline 
std::ostream& operator<<(std::ostream& os, const std::vector<T>& guard)
{
  bool first = true;
  os << "{";
  for (const auto &g: guard) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << g;
  }
  os << "}";
  return os;
}

static inline 
std::istream& operator>>(std::istream& is, Constraint& p)
{
  if (is.get() != 'x') {
    is.setstate(std::ios_base::failbit);
    return is;
  }
  int x;
  is >> x;
  p.x = x;
  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  char odr[2];
  is >> odr[0] >> odr[1];

  switch (odr[0]) {
  case '>':
    if (odr[1] == '=') {
      p.odr = Constraint::Order::ge;
      if (is.get() != ' ') {
        is.setstate(std::ios_base::failbit);
        return is;
      }
    } else if (odr[1] == ' ') {
      p.odr = Constraint::Order::gt;
    } else {
      is.setstate(std::ios_base::failbit);
      return is;
    }
    break;
  case '<':
    if (odr[1] == '=') {
      p.odr = Constraint::Order::le;
      if (is.get() != ' ') {
        is.setstate(std::ios_base::failbit);
        return is;
      }
    } else if (odr[1] == ' ') {
      p.odr = Constraint::Order::lt;
    } else {
      is.setstate(std::ios_base::failbit);
      return is;
    }
    break;
  default:
    is.setstate(std::ios_base::failbit);
    return is;
  }

  is >> p.c;
  return is;
}

static inline 
std::ostream& operator<<(std::ostream& os, const std::string& resetVars)
{
  bool first = true;
  os << "{";
  for (const auto &x: resetVars) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << int(x);
  }
  os << "}";
  return os;
}

static inline 
std::istream& operator>>(std::istream& is, std::vector<ClockVariables>& resetVars)
{
  resetVars.clear();
  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (is.get() != '{') {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  while (true) {
    int x;
    is >> x;
    resetVars.emplace_back(ClockVariables(x));
    if (!is) {
      is.setstate(std::ios_base::failbit);
      return is;
    }
    char c;
    is >> c;
    if (c == '}') {
      break;
    } else if (c == ',') {
      is >> c;
      if (c != ' ') {
        is.setstate(std::ios_base::failbit);
        return is;
      }
    } else {
      is.setstate(std::ios_base::failbit);
      return is;
    }
  }

  return is;
}

template <class T>
static inline 
std::istream& operator>>(std::istream& is, std::vector<T>& resetVars)
{
  resetVars.clear();
  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (is.get() != '{') {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  while (true) {
    T x;
    is >> x;
    resetVars.emplace_back(std::move(x));
    if (!is) {
      is.setstate(std::ios_base::failbit);
      return is;
    }
    char c;
    is >> c;
    if (c == '}') {
      break;
    } else if (c == ',') {
      is >> c;
      if (c != ' ') {
        is.setstate(std::ios_base::failbit);
        return is;
      }
    } else {
      is.setstate(std::ios_base::failbit);
      return is;
    }
  }

  return is;
}


namespace boost {
  template <class T>
  static inline 
  std::ostream& operator<<(std::ostream& os, const boost::optional<T>& x)
  {
    if (x) {
      os << x.get();
    }
    else {
      os << "";
    }
    return os;
  }

  template <class T>
  static inline 
  std::istream& operator>>(std::istream& is, boost::optional<T>& x)
  {
    T result;
    if (is >> result) {
      x = result;
    }
    return is;
  }
}

struct ResetVars {
  std::vector<ClockVariables> resetVars;
};

static inline 
std::istream& operator>>(std::istream& is, ResetVars& resetVars)
{
  is >> resetVars.resetVars;
  return is;
}

static inline 
std::ostream& operator<<(std::ostream& os, const ResetVars& resetVars)
{
  os << resetVars.resetVars;
  return os;
}

struct BoostTAState {
  bool isInit;
  bool isMatch;
};

struct BoostTATransition {
  Alphabet c;
  //! @note this structure is necessary because of some problem in boost graph
  ResetVars resetVars;
  std::vector<Constraint> guard;
};

using BoostTimedAutomaton = 
                          boost::adjacency_list<
  boost::listS, boost::vecS, boost::directedS,
  BoostTAState, BoostTATransition>;

static inline 
void parseBoostTA(std::istream &file, BoostTimedAutomaton &BoostTA)
{

  boost::dynamic_properties dp(boost::ignore_other_properties);
  dp.property("match", boost::get(&BoostTAState::isMatch, BoostTA));
  dp.property("init",  boost::get(&BoostTAState::isInit, BoostTA));
  dp.property("label", boost::get(&BoostTATransition::c, BoostTA));
  dp.property("reset", boost::get(&BoostTATransition::resetVars, BoostTA));
  dp.property("guard", boost::get(&BoostTATransition::guard, BoostTA));

  boost::read_graphviz(file, BoostTA, dp, "id");
}

static inline 
void convBoostTA(const BoostTimedAutomaton &BoostTA, TimedAutomaton &TA)
{
  TA.states.clear();
  TA.initialStates.clear();
  TA.maxConstraints.clear();
  auto vertex_range = boost::vertices(BoostTA);
  std::unordered_map<BoostTimedAutomaton::vertex_descriptor, std::shared_ptr<TAState>> stateConvMap;
  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    BoostTimedAutomaton::vertex_descriptor v = *first;
    stateConvMap[v] = std::make_shared<TAState>(boost::get(&BoostTAState::isMatch, BoostTA, v));
    TA.states.emplace_back(stateConvMap[v]);
    if (boost::get(&BoostTAState::isInit, BoostTA, v)) {
      TA.initialStates.emplace_back(stateConvMap[v]);
    }
  }

  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    auto edge_range = boost::out_edges(*first, BoostTA);
    for (auto firstEdge = edge_range.first, lastEdge = edge_range.second; firstEdge != lastEdge; ++firstEdge) {    
      TATransition transition;
      transition.target = stateConvMap[boost::target(*firstEdge, BoostTA)].get();
      transition.guard = boost::get(&BoostTATransition::guard, BoostTA, *firstEdge);
      transition.resetVars = boost::get(&BoostTATransition::resetVars, BoostTA, *firstEdge).resetVars;
      for (auto g: transition.guard) {
        TA.maxConstraints.resize(std::max<std::size_t>(TA.maxConstraints.size(), g.x + 1));
        TA.maxConstraints[g.x] = std::max(TA.maxConstraints[g.x], g.c);
      }
      stateConvMap[*first]->next[boost::get(&BoostTATransition::c, BoostTA, *firstEdge)].emplace_back(std::move(transition));
    }
  }
}
