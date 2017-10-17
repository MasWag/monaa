#include <boost/test/unit_test.hpp>

#include "../tgrep/intermediate_zone.hh"

BOOST_AUTO_TEST_SUITE(IntermediateZoneTest)

BOOST_AUTO_TEST_CASE(toAns) {
  IntermediateZone in = Zone::zero(5);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      in.value(i, j) = {i * j, true};
    }
  }
  Zone out;
  in.toAns(out);

  // Check
  BOOST_TEST(bool(out.value(0, 1) == in.value(0, 1)));
  BOOST_TEST(bool(out.value(1, 0) == in.value(1, 0)));
  BOOST_TEST(bool(out.value(0, 2) == in.value(0, 4)));
  BOOST_TEST(bool(out.value(2, 0) == in.value(4, 0)));
  BOOST_TEST(bool(out.value(1, 2) == in.value(1, 4)));
  BOOST_TEST(bool(out.value(2, 1) == in.value(4, 1)));
}

BOOST_AUTO_TEST_SUITE(tightenTests)

BOOST_AUTO_TEST_CASE(tightenTest) {
  IntermediateZone in = Zone::zero(2);
  in.value(1, 0) = {5.3, false};
  in.value(0, 1) = {-4.7, true};

  BOOST_TEST(in.isSatisfiable());
}

BOOST_AUTO_TEST_CASE(tightenAllocTest) {
  IntermediateZone in = {Zone::zero(3), 1};
  in.value(1, 0) = {5.3, false};
  in.value(0, 1) = {-4.7, true};
  BOOST_TEST(in.isSatisfiable());
  BOOST_TEST(bool(in.value(0, 0) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(0, 1) == Bounds(-4.7, true)));
  BOOST_TEST(bool(in.value(1, 0) == Bounds(5.3, false)));
  BOOST_TEST(bool(in.value(1, 1) == Bounds(0, true)));

  // new clock sould be allocated
  // Now we do not need explicit canonize after allocation
  BOOST_CHECK_EQUAL(in.alloc({5.8, true}, {5.3, false}), 2);
  BOOST_TEST(bool(in.value(0, 0) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(0, 1) == Bounds(-4.7, true)));
  BOOST_TEST(bool(in.value(0, 2) == Bounds(-5.3, false)));
  BOOST_TEST(bool(in.value(1, 0) == Bounds(5.3, false)));
  BOOST_TEST(bool(in.value(1, 1) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(1, 2) == Bounds(0, false)));
  BOOST_TEST(bool(in.value(2, 0) == Bounds(5.8, true)));
  BOOST_TEST(bool(in.value(2, 1) == Bounds(5.8 - 4.7, true)));
  BOOST_TEST(bool(in.value(2, 2) == Bounds(0, true)));
  in.canonize();
  BOOST_TEST(bool(in.value(0, 0) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(0, 1) == Bounds(-4.7, true)));
  BOOST_TEST(bool(in.value(0, 2) == Bounds(-5.3, false)));
  BOOST_TEST(bool(in.value(1, 0) == Bounds(5.3, false)));
  BOOST_TEST(bool(in.value(1, 1) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(1, 2) == Bounds(0, false)));
  BOOST_TEST(bool(in.value(2, 0) == Bounds(5.8, true)));
  BOOST_TEST(bool(in.value(2, 1) == Bounds(5.8 - 4.7, true)));
  BOOST_TEST(bool(in.value(2, 2) == Bounds(0, true)));
  BOOST_TEST(in.isSatisfiable());

  std::vector<boost::variant<double, ClockVariables>> resetTime = {double(5.3)};
  Constraint guard = {ConstraintMaker(0) < 1};
  in.tighten({guard}, resetTime);
  BOOST_TEST(bool(in.value(0, 0) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(0, 1) == Bounds(-4.7, true)));
  BOOST_TEST(bool(in.value(0, 2) == Bounds(-5.3, false)));
  BOOST_TEST(bool(in.value(1, 0) == Bounds(5.3, false)));
  BOOST_TEST(bool(in.value(1, 1) == Bounds(0, true)));
  BOOST_TEST(bool(in.value(1, 2) == Bounds(0, false)));
  BOOST_TEST(bool(in.value(2, 0) == Bounds(5.8, true)));
  BOOST_TEST(bool(in.value(2, 1) == Bounds(5.8 - 4.7, true)));
  BOOST_TEST(bool(in.value(2, 2) == Bounds(0, true)));
  BOOST_TEST(in.isSatisfiable());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
