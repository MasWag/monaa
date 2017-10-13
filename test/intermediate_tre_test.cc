#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../tgrep/tre_driver.hh"
#include "../tgrep/intermediate_tre.hh"

BOOST_AUTO_TEST_SUITE(IntermediateTRETest)
struct SimpleUnTimedExpression {
  SimpleUnTimedExpression() {
    stream << "ab";
  }

  std::stringstream stream;
};

struct SimpleConcatUnTimedExpression {
  SimpleConcatUnTimedExpression() {
    stream << "aaaa";
  }

  std::stringstream stream;
};

struct ConcatIntervalsUnTimedExpression {
  ConcatIntervalsUnTimedExpression() {
    stream << "a(a)%(1,2)aa%(2,3)";
  }

  std::stringstream stream;
};

struct SimplePlusUnTimedExpression {
  SimplePlusUnTimedExpression() {
    stream << "aa+aa";
  }

  std::stringstream stream;
};

struct PlusUnTimedExpression {
  PlusUnTimedExpression() {
    stream << "(a|(a+))aa";
  }

  std::stringstream stream;
};

struct SimpleTimedExpression {
  SimpleTimedExpression() {
    stream << "((aba)%(1,2))*";
  }

  std::stringstream stream;
};

struct SingletonTimedExpression {
  SingletonTimedExpression() {
    stream << "((a)%(1,2))*";
  }

  std::stringstream stream;
};

struct UnTimedExpression {
  UnTimedExpression() {
    stream << "((a|b)d&c)*";
  }

  std::stringstream stream;
};

BOOST_AUTO_TEST_SUITE(toNormalFormTest)

BOOST_FIXTURE_TEST_CASE(toNormalFormUntimedSimple, SimpleUnTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
}

BOOST_FIXTURE_TEST_CASE(toNormalFormTimed, SimpleTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
}

BOOST_FIXTURE_TEST_CASE(toNormalFormTimedSingleton, SingletonTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimed, SingletonTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedConcat, SimpleConcatUnTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedPlus, SimplePlusUnTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedConcatInterval, ConcatIntervalsUnTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedPlusComplex, PlusUnTimedExpression)
{
  TREDriver driver;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();

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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(isMemberTest)

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedConcat, SimpleConcatUnTimedExpression)
{
  TREDriver driver;
  TimedAutomaton TA;
  driver.parse(stream);
  std::shared_ptr<DNFTRE> dnf = std::make_shared<DNFTRE>(driver.getResult());

  dnf->toNormalForm();
  dnf->toSignalTA(TA);
  BOOST_TEST(TA.isMember({{0, 1.2}, {0, 1.3}, {0, 1.6}, {'a', 2.9}}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

