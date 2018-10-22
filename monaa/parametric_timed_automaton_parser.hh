#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include <fstream>
#include <iostream>

#include "../libmonaa/parametric_timed_automaton.hh"

namespace boost{
  enum vertex_match_t {vertex_match};
  enum edge_label_t {edge_label};
  enum edge_reset_t {edge_reset};
  enum edge_guard_t {edge_guard};
  enum graph_clock_dimensions_t {graph_clock_dimensions};
  enum graph_param_dimensions_t {graph_param_dimensions};

  BOOST_INSTALL_PROPERTY(vertex, match);
  BOOST_INSTALL_PROPERTY(edge, label);
  BOOST_INSTALL_PROPERTY(edge, reset);
  BOOST_INSTALL_PROPERTY(edge, guard);
  BOOST_INSTALL_PROPERTY(graph, clock_dimensions);
  BOOST_INSTALL_PROPERTY(graph, param_dimensions);
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
std::istream& operator>>(std::istream& is, Parma_Polyhedra_Library::Linear_Expression& p)
{
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

struct BoostPTAState {
  bool isInit;
  bool isMatch;
};

struct BoostPTATransition {
  Alphabet c;
  //! @note this structure is necessary because of some problem in boost graph
  ResetVars resetVars;
  Parma_Polyhedra_Library::Constraint_System guard;
};

using BoostParametricTimedAutomaton = 
        boost::adjacency_list<
  boost::listS, boost::vecS, boost::directedS,
  BoostPTAState, BoostPTATransition,boost::property<boost::graph_clock_dimensions_t, std::size_t,
                                                    boost::property<boost::graph_param_dimensions_t, std::size_t>>>;

static inline 
void parseBoostTA(std::istream &file, BoostParametricTimedAutomaton &BoostPTA)
{

  boost::dynamic_properties dp(boost::ignore_other_properties);
  dp.property("match", boost::get(&BoostPTAState::isMatch, BoostPTA));
  dp.property("init",  boost::get(&BoostPTAState::isInit, BoostPTA));
  dp.property("label", boost::get(&BoostPTATransition::c, BoostPTA));
  dp.property("reset", boost::get(&BoostPTATransition::resetVars, BoostPTA));
  dp.property("guard", boost::get(&BoostPTATransition::guard, BoostPTA));

  boost::read_graphviz(file, BoostPTA, dp, "id");
}

static inline 
void convBoostTA(const BoostParametricTimedAutomaton &BoostPTA, ParametricTimedAutomaton &PTA)
{
  PTA.states.clear();
  PTA.initialStates.clear();
  auto vertex_range = boost::vertices(BoostPTA);
  std::unordered_map<BoostParametricTimedAutomaton::vertex_descriptor, std::shared_ptr<PTAState>> stateConvMap;
  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    BoostParametricTimedAutomaton::vertex_descriptor v = *first;
    stateConvMap[v] = std::make_shared<PTAState>(boost::get(&BoostPTAState::isMatch, BoostPTA, v));
    PTA.states.emplace_back(stateConvMap[v]);
    if (boost::get(&BoostPTAState::isInit, BoostPTA, v)) {
      PTA.initialStates.emplace_back(stateConvMap[v]);
    }
  }

  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    auto edge_range = boost::out_edges(*first, BoostPTA);
    for (auto firstEdge = edge_range.first, lastEdge = edge_range.second; firstEdge != lastEdge; ++firstEdge) {    
      PTATransition transition;
      transition.target = stateConvMap[boost::target(*firstEdge, BoostPTA)].get();
      transition.guard = Parma_Polyhedra_Library::NNC_Polyhedron(boost::get(&BoostPTATransition::guard, BoostPTA, *firstEdge));
      transition.resetVars = boost::get(&BoostPTATransition::resetVars, BoostPTA, *firstEdge).resetVars;
      stateConvMap[*first]->next[boost::get(&BoostPTATransition::c, BoostPTA, *firstEdge)].emplace_back(std::move(transition));
    }
  }
}
