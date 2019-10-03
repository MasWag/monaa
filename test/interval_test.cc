#include <boost/test/unit_test.hpp>

#include "../libmonaa/interval.hh"

BOOST_AUTO_TEST_SUITE(intervalTests)

BOOST_AUTO_TEST_SUITE(intervalLandTest)
BOOST_AUTO_TEST_CASE(intervalLand1_2)
{
  Interval interval(1,2);
  Interval intervalAll;

  Interval andInterval = interval && intervalAll;
  BOOST_CHECK_EQUAL(andInterval.lowerBound.first, 1);
  BOOST_CHECK_EQUAL(andInterval.lowerBound.second, false);
  BOOST_CHECK_EQUAL(andInterval.upperBound.first, 2);
  BOOST_CHECK_EQUAL(andInterval.upperBound.second, false);
}

BOOST_AUTO_TEST_CASE(intervalsLand1_2)
{
  std::vector<std::shared_ptr<Interval>> intervals = {std::make_shared<Interval>(1,2)};
  std::vector<std::shared_ptr<Interval>> intervalAlls = {std::make_shared<Interval>()};

  land(intervals, intervalAlls);
  BOOST_CHECK_EQUAL(intervals.size(), 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.first, 2);
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.second, false);
}

BOOST_AUTO_TEST_CASE(intervalLand1)
{
  Interval interval(1);
  Interval intervalAll;

  Interval andInterval = interval && intervalAll;
  BOOST_CHECK_EQUAL(andInterval.lowerBound.first, 1);
  BOOST_CHECK_EQUAL(andInterval.lowerBound.second, false);
  BOOST_CHECK_EQUAL(andInterval.upperBound.first, std::numeric_limits<double>::infinity());
  BOOST_CHECK_EQUAL(andInterval.upperBound.second, false);
}

BOOST_AUTO_TEST_CASE(intervalsLand1)
{
  std::vector<std::shared_ptr<Interval>> intervals = {std::make_shared<Interval>(1)};
  std::vector<std::shared_ptr<Interval>> intervalAlls = {std::make_shared<Interval>()};

  land(intervals, intervalAlls);
  BOOST_CHECK_EQUAL(intervals.size(), 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.first, std::numeric_limits<double>::infinity());
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.second, false);
}

BOOST_AUTO_TEST_CASE(intervalsLand1_34_78)
{
  // (1,\infty)
  std::vector<std::shared_ptr<Interval>> intervals = {std::make_shared<Interval>(1)};
  // (3,4), (7,8)
  std::vector<std::shared_ptr<Interval>> interval34_78 = {std::make_shared<Interval>(3,4), std::make_shared<Interval>(7,8)};
  // The result should be (3,4), (7,8)

  land(intervals, interval34_78);
  BOOST_CHECK_EQUAL(intervals.size(), 2);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.first, 3);
  BOOST_CHECK_EQUAL(intervals[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.first, 4);
  BOOST_CHECK_EQUAL(intervals[0]->upperBound.second, false);
  BOOST_CHECK_EQUAL(intervals[1]->lowerBound.first, 7);
  BOOST_CHECK_EQUAL(intervals[1]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(intervals[1]->upperBound.first, 8);
  BOOST_CHECK_EQUAL(intervals[1]->upperBound.second, false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(intervalSumTests)
BOOST_AUTO_TEST_CASE(intervalSum12_34)
{
  const Interval left(1,2), right(3,4);
  const Interval result = left + right;

  BOOST_CHECK_EQUAL(result.lowerBound.first, 4);
  BOOST_CHECK_EQUAL(result.lowerBound.second, false);
  BOOST_CHECK_EQUAL(result.upperBound.first, 6);
  BOOST_CHECK_EQUAL(result.upperBound.second, false);
}
BOOST_AUTO_TEST_CASE(intervalSum1234_5678)
{
  // (1,2), (3,4)
  std::vector<std::shared_ptr<Interval>> left = {std::make_shared<Interval>(1,2), std::make_shared<Interval>(3,4)};
  // (5,6), (7,8)
  std::vector<std::shared_ptr<Interval>> right = {std::make_shared<Interval>(5,6), std::make_shared<Interval>(7,8)};
  left += right;

  BOOST_CHECK_EQUAL(left.size(), 4);
  BOOST_CHECK_EQUAL(left[0]->lowerBound.first, 6);
  BOOST_CHECK_EQUAL(left[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(left[0]->upperBound.first, 8);
  BOOST_CHECK_EQUAL(left[0]->upperBound.second, false);
  BOOST_CHECK_EQUAL(left[1]->lowerBound.first, 8);
  BOOST_CHECK_EQUAL(left[1]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(left[1]->upperBound.first, 10);
  BOOST_CHECK_EQUAL(left[1]->upperBound.second, false);
  BOOST_CHECK_EQUAL(left[2]->lowerBound.first, 8);
  BOOST_CHECK_EQUAL(left[2]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(left[2]->upperBound.first, 10);
  BOOST_CHECK_EQUAL(left[2]->upperBound.second, false);
  BOOST_CHECK_EQUAL(left[3]->lowerBound.first, 10);
  BOOST_CHECK_EQUAL(left[3]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(left[3]->upperBound.first, 12);
  BOOST_CHECK_EQUAL(left[3]->upperBound.second, false);
}
BOOST_AUTO_TEST_SUITE_END()

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

BOOST_AUTO_TEST_CASE(intervalPlus1)
{
  Interval interval(1);
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
