#include <boost/test/unit_test.hpp>

#include "../tgrep/zone.hh"

BOOST_AUTO_TEST_SUITE(ZoneTest)

BOOST_AUTO_TEST_CASE(makeUnsat) {
  Zone zone = Zone::zero(5);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      zone.value(i, j) = {i * j, true};
    }
  }

  
  BOOST_TEST(zone.isSatisfiable());
  zone.makeUnsat();
  BOOST_TEST(!zone.isSatisfiable());
}

BOOST_AUTO_TEST_SUITE_END()
