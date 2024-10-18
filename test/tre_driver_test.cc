#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../monaa/tre_driver.hh"

BOOST_AUTO_TEST_SUITE(treDriverTest)

struct SampleTRE {
  SampleTRE() {
    stream << "(b*(ab*ab*)*)|(a*(ba*ba*)*)";
  }

  std::stringstream stream;
};

struct SimpleTimedExpression {
  SimpleTimedExpression() {
    stream << "(aba)%(1,2)";
  }

  std::stringstream stream;
};


BOOST_AUTO_TEST_SUITE(parse)
BOOST_FIXTURE_TEST_CASE(untimedParse, SampleTRE)
{
  TREDriver driver;
  std::stringstream result;
  driver.parse(stream);
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "(((@|(b+))(@|((a((@|(b+))(a(@|(b+)))))+)))|((@|(a+))(@|((b((@|(a+))(b(@|(a+)))))+))))");
}

BOOST_FIXTURE_TEST_CASE(simpleTimedParse, SimpleTimedExpression)
{
  TREDriver driver;
  std::stringstream result;
  driver.parse(stream);
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((1, 0),(2, 0)))");
}

BOOST_AUTO_TEST_CASE(closeTimedParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%[2,4]";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((2, 1),(4, 1)))");
}

BOOST_AUTO_TEST_CASE(leftOpenTimedParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%(2,4]";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((2, 0),(4, 1)))");
}

BOOST_AUTO_TEST_CASE(rightOpenTimedParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%[2,4)";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((2, 1),(4, 0)))");
}

BOOST_AUTO_TEST_CASE(ltParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%(< 3)";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((0, 1),(3, 0)))");
}

BOOST_AUTO_TEST_CASE(leParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%(<= 3)";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((0, 1),(3, 1)))");
}

BOOST_AUTO_TEST_CASE(eqParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%(= 3)";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((3, 1),(3, 1)))");
}

BOOST_AUTO_TEST_CASE(geParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%(>= 3)";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((3, 1),(inf, 0)))");
}

BOOST_AUTO_TEST_CASE(gtParse) {
  TREDriver driver;
  std::stringstream stream, result;
  stream << "(aba)%(> 3)";
  BOOST_TEST_REQUIRE(driver.parse(stream));
  result << *(driver.getResult());
  BOOST_CHECK_EQUAL(result.str(), "((a(ba))%((3, 0),(inf, 0)))");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
