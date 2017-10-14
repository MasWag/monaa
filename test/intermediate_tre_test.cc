#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../tgrep/tre_driver.hh"
#include "../tgrep/intermediate_tre.hh"

extern void renameToEpsilonTransitions(TimedAutomaton& out);

BOOST_AUTO_TEST_SUITE(IntermediateTRETest)
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

class ConstructDNFTRE : private ParseTRE {
public:
  ConstructDNFTRE() {}
  std::shared_ptr<DNFTRE> dnf;
  void constructNormalForm(const char* tre) {
    parse(tre);
    dnf = std::make_shared<DNFTRE>(driver.getResult());
    dnf->toNormalForm();
  }
};

class ConstructTA : private ParseTRE {
public:
  ConstructTA() {}
  TimedAutomaton TA;
  void constructSignalTA(const char* tre) {
    parse(tre);
    toSignalTA(driver.getResult(), TA);
  }
};

BOOST_AUTO_TEST_SUITE(toNormalFormTest)

BOOST_FIXTURE_TEST_CASE(toNormalFormUntimedSimple, ConstructDNFTRE)
{
  constructNormalForm("ab");
}

BOOST_FIXTURE_TEST_CASE(toNormalFormTimed, ConstructDNFTRE)
{
  constructNormalForm("((aba)%(1,2))*");
}

BOOST_FIXTURE_TEST_CASE(toNormalFormTimedSingleton, ConstructDNFTRE)
{
  constructNormalForm("((a)%(1,2))*");
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedConcat, ConstructDNFTRE)
{
  constructNormalForm("aaaa");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedPlus, ConstructDNFTRE)
{
  constructNormalForm("aa+aa");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedConcatInterval, ConstructDNFTRE)
{
  constructNormalForm("a(a%(1,2))a(a%(2,3))");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedPlusComplex, ConstructDNFTRE)
{
  constructNormalForm("(a|(a+))aa");

  // we do not remove duplicated subformula
  BOOST_CHECK_EQUAL(dnf->list.size(), 2);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
  dnf->list.pop_front();
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormConcatIntervals, ParseTRE)
{
  parse("a(a%(1,2))a(a%(2,3))");
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  //  dnf->toNormalForm();
  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::concat));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->list.size(), 4);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(isMemberTest)

BOOST_FIXTURE_TEST_CASE(isMemberUnTimedConcat, ConstructTA)
{
  constructSignalTA("aaaa");

  BOOST_TEST(TA.isMember({{'a', 2.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMemberUnTimedPlusComplex, ConstructTA)
{
  constructSignalTA("(a|(a+))aa");

  BOOST_TEST(TA.isMember({{'a', 2.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMemberUnTimedPlusComplexMulti, ConstructTA)
{
  constructSignalTA("(a|(c+))ba");

  BOOST_TEST(TA.isMember({{'c', 2.9}, {'b', 2.9}, {'a', 2.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMemberConcatIntervals, ConstructTA)
{
  constructSignalTA("a(a%(1,2))a(a%(2,3))");

  BOOST_TEST(TA.isMember({{'a', 2.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMemberConcatIntervalsPlus, ConstructTA)
{
  constructSignalTA("a(a%(1,2)+)a(a%(2,3)+)");

  BOOST_TEST(TA.isMember({{'a', 2.9}}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(renameToEpsilonTransitionsTest)

BOOST_FIXTURE_TEST_CASE(renameToEpsilonTransitionsConcatIntervals, ParseTRE)
{
  TimedAutomaton TA;
  parse("a(a%(1,2))a(a%(2,3))");

  // Skip Normalize to have epsilon transitions easily
  DNFTRE dnf(driver.getResult());

  // dnf.toNormalForm();
  dnf.toSignalTA(TA);
  renameToEpsilonTransitions(TA);
  BOOST_TEST(TA.isMember({{0, 2.9}, {0, 4.2}, {0, 4.7}, {'a', 7.0}}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

