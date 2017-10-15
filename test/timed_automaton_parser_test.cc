#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include <boost/test/unit_test.hpp>
#include "../tgrep/timed_automaton_parser.hh"

std::ostream& operator<<(std::ostream& os, const std::vector<ClockVariables> &resetVars);

BOOST_AUTO_TEST_SUITE(timedAutomatonParserTests)
BOOST_AUTO_TEST_CASE(parseDot)
{
  TimedAutomaton TA;
  std::ifstream file("../test/timed_automaton.dot");
  parseTA(file, TA);
}

BOOST_AUTO_TEST_SUITE_END()
