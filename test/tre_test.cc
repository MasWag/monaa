#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../tgrep/tre_driver.hh"
#include "../tgrep/tre.hh"

BOOST_AUTO_TEST_SUITE(treTest)
class ParseTRE {
public:
  TREDriver driver;
  ParseTRE() {}
  void parse(const char* tre) {
    stream << tre;
    driver.parse(stream);
  }
private:
  std::stringstream stream;
};

class ConstructTA : private ParseTRE {
public:
  ConstructTA() {}
  TimedAutomaton TA;
  void constructEventTA(const char* tre) {
    parse(tre);
    driver.getResult()->toEventTA(TA);
  }
};

BOOST_AUTO_TEST_SUITE(treTest)

BOOST_FIXTURE_TEST_CASE(toEventTAUntimed, ConstructTA)
{
  constructEventTA("ab");

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 2.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTATimed, ConstructTA)
{
  constructEventTA("((aba)%(1,2))*");

  BOOST_TEST(TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTATimedSigleton, ConstructTA)
{
  constructEventTA("(a)%(0,1)");

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 0.2}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
