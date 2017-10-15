#include <boost/test/unit_test.hpp>

#include "../tgrep/interval.hh"

BOOST_AUTO_TEST_SUITE(intervalTests)

BOOST_AUTO_TEST_SUITE(intervalPlusTests)
BOOST_AUTO_TEST_CASE(intervalPlus1_2)
{
  Interval interval(1,2);
  std::vector<std::shared_ptr<Interval>> intervals;
  interval.plus(intervals);
  BOOST_CHECK_EQUAL(intervals.size(), 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.second, false);
  BOOST_TEST(std::isinf(intervals[0]->upperBound.first));
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.second, false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(intervalsPlusTests)
BOOST_AUTO_TEST_CASE(intervalsSinglePlus)
{
  std::vector<std::shared_ptr<Interval>> intervals = {std::make_shared<Interval>(1,2)};
  plus(intervals);
  BOOST_CHECK_EQUAL(intervals.size(), 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.second, false);
  BOOST_TEST(std::isinf(intervals[0]->upperBound.first));
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.second, false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
