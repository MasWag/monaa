#pragma once
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <sstream>
#include "../tgrep/word_container.hh"

BOOST_AUTO_TEST_SUITE(WordContainerTest)

typedef boost::mpl::list<WordLazyDeque<int>, WordVector<int>> testTypesInt;
typedef boost::mpl::list<WordLazyDeque<std::pair<int, double>>,
                         WordVector<std::pair<int, double>>> testTypesPair;

template<class T>
class DQFixture {
private:
  std::stringstream stream;
public:
  DQFixture() : stream("0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"), dq(10, stream) {}
  T dq;
};

// We can instanciate
BOOST_AUTO_TEST_CASE_TEMPLATE( instanceInt, T, testTypesInt)
{
  DQFixture<T> f;
  BOOST_CHECK_EQUAL(f.dq[5], 5);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( instancePair, T, testTypesPair)
{
  DQFixture<T> f;
  BOOST_CHECK_EQUAL(f.dq[5].first, 10);
}

// When available areas are accessed, it returns valid value.
BOOST_AUTO_TEST_CASE_TEMPLATE( access, T, testTypesInt )
{
  DQFixture<T> f;
  BOOST_CHECK_EQUAL(f.dq[5], 5);
  BOOST_CHECK_EQUAL(f.dq[8], 8);
  BOOST_CHECK_EQUAL(f.dq[9], 9);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(WordLazyDequeTest)


// When backward areas are accessed, it throws out_of_range.
BOOST_FIXTURE_TEST_CASE( backward_of_range_access, WordContainerTest::DQFixture<WordLazyDeque<int>>)
{
  BOOST_CHECK_THROW(dq[10], std::out_of_range);
  BOOST_CHECK_THROW(dq[14], std::out_of_range);
}

// When forward areas are accessed, it throws out_of_range.
BOOST_FIXTURE_TEST_CASE( forward_of_range_access, WordContainerTest::DQFixture<WordLazyDeque<int>>)
{
  BOOST_CHECK_EQUAL(dq[5], 5);
  BOOST_CHECK_EQUAL(dq[8], 8);
  BOOST_CHECK_EQUAL(dq[9], 9);
  dq.setFront(4);
  BOOST_CHECK_EQUAL(dq[5], 5);
  dq.setFront(6);
  BOOST_CHECK_THROW(dq[5], std::out_of_range);
  dq.setFront(8);
  BOOST_CHECK_EQUAL(dq[8], 8);
  dq.setFront(10);
  BOOST_CHECK_THROW(dq[8], std::out_of_range);
  BOOST_CHECK_THROW(dq[9], std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(WordVectorTest)

// When forward areas are accessed, it throws out_of_range.
BOOST_FIXTURE_TEST_CASE( forward_of_range_access, WordContainerTest::DQFixture<WordVector<int>>)
{
  BOOST_CHECK_EQUAL(dq[5], 5);
  BOOST_CHECK_EQUAL(dq[8], 8);
  BOOST_CHECK_EQUAL(dq[9], 9);
  dq.setFront(4);
  BOOST_CHECK_EQUAL(dq[5], 5);
  dq.setFront(6);
  BOOST_CHECK_NO_THROW(dq[5]);
  dq.setFront(8);
  BOOST_CHECK_EQUAL(dq[8], 8);
  dq.setFront(10);
  BOOST_CHECK_NO_THROW(dq[8]);
  BOOST_CHECK_NO_THROW(dq[9]);
}

BOOST_AUTO_TEST_SUITE_END()
