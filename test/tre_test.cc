#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../tgrep/tre_driver.hh"
#include "../tgrep/tre.hh"

BOOST_AUTO_TEST_SUITE(treTest)
struct SimpleUnTimedExpression {
  SimpleUnTimedExpression() {
    stream << "ab";
  }

  std::stringstream stream;
};

struct SimpleTimedExpression {
  SimpleTimedExpression() {
    stream << "((aba)%(1,2))*";
  }

  std::stringstream stream;
};

BOOST_AUTO_TEST_SUITE(treTest)

BOOST_FIXTURE_TEST_CASE(toEventTAUntimed, SimpleUnTimedExpression)
{
  TREDriver driver;
  TimedAutomaton TA;
  driver.parse(stream);
  driver.getResult()->toEventTA(TA);

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 2.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTATimed, SimpleTimedExpression)
{
  TREDriver driver;
  TimedAutomaton TA;
  driver.parse(stream);
  driver.getResult()->toEventTA(TA);

  BOOST_TEST(TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
