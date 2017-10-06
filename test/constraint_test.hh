#pragma once
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../tgrep/constraint.hh"

BOOST_AUTO_TEST_SUITE(ConstraintTest)

// We can instanciate
BOOST_AUTO_TEST_CASE( widenTest )
{
  std::vector<Constraint> guard = {{ConstraintMaker(0) > 10},
                                   {ConstraintMaker(1) >= 9},
                                   {ConstraintMaker(2) <= 8},
                                   {ConstraintMaker(3) < 7}};
  BOOST_CHECK_EQUAL(guard.size(), 4);
  widen(guard);
  BOOST_CHECK_EQUAL(guard.size(), 2);
  BOOST_CHECK_EQUAL(guard[0].c, 8);
  BOOST_CHECK_EQUAL(guard[1].c, 7);
}

BOOST_AUTO_TEST_SUITE_END()
