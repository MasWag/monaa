#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include <fstream>
#include <iostream>

#include "timed_automaton.hh"

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

std::ostream& operator<<(std::ostream& os, const Constraint& p)
{
  os << "x" << int(p.x) << " " << p.odr << " " << p.c;
  return os;
}

template<class T>
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

std::istream& operator>>(std::istream& is, std::vector<Constraint>& guard)
{
  guard.clear();
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
    Constraint g;
    is >> g;
    guard.emplace_back(std::move(g));
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
  std::istream& operator>>(std::istream& is, boost::optional<T>& x)
  {
    T result;
    if (is >> result) {
      x = result;
    }
    return is;
  }
}

struct BoostTATransition : public TATransition{
  Alphabet c;
  std::string resetVars;
};

using BoostTimedAutomaton = 
                          boost::adjacency_list<
  boost::listS, boost::vecS, boost::directedS,
  TAState, BoostTATransition>;

// using BoostTimedAutomaton = 
//                           boost::adjacency_list<
//   boost::listS, boost::vecS, boost::directedS,
//   TAState, boost::no_property>;

void parseTA(std::istream &file, TimedAutomaton &)
{
  BoostTimedAutomaton BoostTA;

  boost::dynamic_properties dp(boost::ignore_other_properties);
  // dp.property("match", boost::get(boost::vertex_match, BoostTA));
  dp.property("match", boost::get(&TAState::isMatch, BoostTA));
  dp.property("label", boost::get(&BoostTATransition::c, BoostTA));
  dp.property("reset", boost::get(&BoostTATransition::resetVars, BoostTA));
  dp.property("guard", boost::get(&BoostTATransition::guard, BoostTA));

  boost::read_graphviz(file, BoostTA, dp, "id");


  std::cout << "isMatch: " << BoostTA[0].isMatch << std::endl;
  std::cout << "isMatch: " << BoostTA[1].isMatch << std::endl;
  std::cout << "label: " << boost::get(&BoostTATransition::c, BoostTA, boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first) << std::endl;
  std::cout << "reset: " << boost::get(&BoostTATransition::resetVars, BoostTA, boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first).size() << std::endl;
  std::cout << "reset0: " << int(boost::get(&BoostTATransition::resetVars, BoostTA, boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first)[0]) << std::endl;
  std::cout << "guard: " << boost::get(&BoostTATransition::guard, BoostTA, boost::edge(boost::vertex(0, BoostTA), boost::vertex(1, BoostTA), BoostTA).first).size() << std::endl;

  // std::cout << "isMatch: " << boost::get(boost::vertex_match, BoostTA, 0) << std::endl;
  // std::cout << "isMatch: " << boost::get(boost::vertex_match, BoostTA, 1) << std::endl;
}
