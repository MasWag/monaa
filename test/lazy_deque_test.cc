#include <boost/test/unit_test.hpp>
#include <cstdio>
#include "../tgrep/lazy_deque.hh"

BOOST_AUTO_TEST_SUITE(LazyDequeTest)

class DQASCIIFixture {
private:
  FILE* file;
public:
  // "a 1.1 b 3.2 c 5.3 d 7.4 e 9.5 f 11.6 g 13.7 h 15.8"
  DQASCIIFixture() : file(fopen("../test/ascii_test.txt", "r")), dqAscii(8, file) {}
  LazyDeque dqAscii;
};

class DQBinaryFixture {
private:
  FILE* file;
public:
  // "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"
  DQBinaryFixture() : file(fopen("../test/binary_test.bin", "r")), dqBinary(8, file, true) {}
  LazyDeque dqBinary;
};

class DQFixture : public DQASCIIFixture, public DQBinaryFixture {
};

// We can instanciate
BOOST_FIXTURE_TEST_CASE( instance, DQFixture )
{
  BOOST_CHECK_EQUAL(dqAscii[5].first, 'f');
  BOOST_CHECK_EQUAL(dqBinary[5].first, 'f');
}

// When available areas are accessed, it returns valid value.
BOOST_FIXTURE_TEST_CASE( access, DQASCIIFixture )
{
  BOOST_CHECK_EQUAL(dqAscii[5].first, 'f');
  BOOST_CHECK_EQUAL(dqAscii[7].first, 'h');
}

// When backward areas are accessed, it throws out_of_range.
BOOST_FIXTURE_TEST_CASE( backward_of_range_access, DQASCIIFixture )
{
  BOOST_CHECK_THROW(dqAscii[10], std::out_of_range);
  BOOST_CHECK_THROW(dqAscii[14], std::out_of_range);
}

// When forward areas are accessed, it throws out_of_range.
BOOST_FIXTURE_TEST_CASE( forward_of_range_access, DQASCIIFixture )
{
  dqAscii.setFront(4);
  BOOST_CHECK_EQUAL(dqAscii[5].first, 'f');
  dqAscii.setFront(6);
  BOOST_CHECK_THROW(dqAscii[5], std::out_of_range);
  BOOST_CHECK_EQUAL(dqAscii[7].second, 15.8);
  dqAscii.setFront(8);
  BOOST_CHECK_THROW(dqAscii[7], std::out_of_range);
  dqAscii.setFront(10);
  BOOST_CHECK_THROW(dqAscii[9], std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
