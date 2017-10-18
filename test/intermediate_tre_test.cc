#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../tgrep/tre_driver.hh"
#include "../tgrep/intermediate_tre.hh"

extern void renameToEpsilonTransitions(TimedAutomaton& out);
extern void concat2(TimedAutomaton &left, const TimedAutomaton &right);

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

BOOST_AUTO_TEST_SUITE(AtomicTRETest)
BOOST_AUTO_TEST_CASE(AtomicTREWithin)
{
  std::shared_ptr<AtomicTRE> singleton = std::make_shared<AtomicTRE>('a');
  std::shared_ptr<Interval> interval = std::make_shared<Interval>(1,2);
  std::shared_ptr<AtomicTRE> within = std::make_shared<AtomicTRE>(singleton, interval);

  BOOST_CHECK_EQUAL(static_cast<int>(within->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(within->singleton->c, 'a');
  BOOST_CHECK_EQUAL(within->singleton->intervals.size(), 1);
  BOOST_CHECK_EQUAL(within->singleton->intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(within->singleton->intervals[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(within->singleton->intervals[0]->upperBound.first, 2);
  BOOST_CHECK_EQUAL(within->singleton->intervals[0]->upperBound.second, false);
}
BOOST_AUTO_TEST_SUITE_END()

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

BOOST_FIXTURE_TEST_CASE(toNormalFormPlus, ConstructDNFTRE)
{
  constructNormalForm("a+");

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

BOOST_FIXTURE_TEST_CASE(toNormalFormUnTimedConcatPlusInterval, ConstructDNFTRE)
{
  constructNormalForm("a(a%(1,2)+)a(a%(2,3)+)");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->c, 'a');
  BOOST_CHECK_EQUAL(dnf->list.front().front()->singleton->intervals.size(), 2);
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

BOOST_FIXTURE_TEST_CASE(toNormalForm1_2, ConstructDNFTRE)
{
  constructNormalForm("a%(1,2)");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  auto singleton = dnf->list.front().front()->singleton;
  BOOST_CHECK_EQUAL(singleton->c, 'a');
  BOOST_CHECK_EQUAL(singleton->intervals.size(), 1);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->upperBound.first, 2);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->upperBound.second, false);
}

BOOST_FIXTURE_TEST_CASE(toNormalForm1_2Plus, ConstructDNFTRE)
{
  constructNormalForm("a%(1,2)+");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  auto singleton = dnf->list.front().front()->singleton;
  BOOST_CHECK_EQUAL(singleton->c, 'a');
  BOOST_CHECK_EQUAL(singleton->intervals.size(), 1);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.second, false);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormConcat1_2Plus, ConstructDNFTRE)
{
  constructNormalForm("a(a%(1,2)+)");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  auto singleton = dnf->list.front().front()->singleton;
  BOOST_CHECK_EQUAL(singleton->c, 'a');
  BOOST_CHECK_EQUAL(singleton->intervals.size(), 1);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.first, 1);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.second, false);
}

BOOST_FIXTURE_TEST_CASE(toNormalFormConcat2_3Plus, ConstructDNFTRE)
{
  constructNormalForm("a(a%(2,3)+)");

  BOOST_CHECK_EQUAL(dnf->list.size(), 1);
  BOOST_CHECK_EQUAL(dnf->list.front().size(), 1);
  BOOST_CHECK_EQUAL(static_cast<int>(dnf->list.front().front()->tag), static_cast<int>(AtomicTRE::op::singleton));
  auto singleton = dnf->list.front().front()->singleton;
  BOOST_CHECK_EQUAL(singleton->c, 'a');
  BOOST_CHECK_EQUAL(singleton->intervals.size(), 2);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.first, 2);
  BOOST_CHECK_EQUAL(singleton->intervals[0]->lowerBound.second, false);
  BOOST_CHECK_EQUAL(singleton->intervals[1]->lowerBound.first, 4);
  BOOST_CHECK_EQUAL(singleton->intervals[1]->lowerBound.second, false);
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

  BOOST_TEST(TA.isMember({{'a', 3.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMemberConcatIntervalsPlus, ConstructTA)
{
  constructSignalTA("a(a%(1,2)+)a(a%(2,3)+)");

  BOOST_TEST(!TA.isMember({{'a', 2.9}}));
  BOOST_TEST(TA.isMember({{'a', 3.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMember1_2, ConstructTA)
{
  constructSignalTA("a%(1,2)");

  BOOST_TEST(!TA.isMember({{'a', 0.2}}));
  BOOST_TEST(TA.isMember({{'a', 1.2}}));
  BOOST_TEST(!TA.isMember({{'a', 2.9}}));
}

BOOST_FIXTURE_TEST_CASE(isMember1_2Plus, ConstructTA)
{
  constructSignalTA("a%(1,2)+");

  BOOST_TEST(!TA.isMember({{'a', 0.2}}));
  BOOST_TEST(TA.isMember({{'a', 1.2}}));
  BOOST_TEST(TA.isMember({{'a', 2.9}}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(renameToEpsilonTransitionsTest)

BOOST_AUTO_TEST_CASE(renameToEpsilonTransitionsSimpleConcat)
{
  TimedAutomaton TA;
  TA.states.resize(3);
  for (auto& state: TA.states) {
    state = std::make_shared<TAState>();
  }
  TA.states[2]->isMatch = true;
  TA.initialStates = {TA.states[0]};

  TA.states[0]->next['a'].emplace_back(TATransition{TA.states[1].get(), {0}, {TimedAutomaton::X(0) < 10}});
  TA.states[1]->next['a'].emplace_back(TATransition{TA.states[2].get(), {1}, {TimedAutomaton::X(1) > 1}});
  TA.maxConstraints = {10};

  renameToEpsilonTransitions(TA);
  BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
  const auto initialState = TA.initialStates[0];
  BOOST_CHECK_EQUAL(initialState->next[0].size(), 1);
  const auto firstTransition = initialState->next[0][0];
  BOOST_CHECK_EQUAL(firstTransition.guard.size(), 1);
  BOOST_CHECK_EQUAL(firstTransition.resetVars.size(), 1);
  const auto secondState = firstTransition.target;
  BOOST_CHECK_EQUAL(secondState->next['a'].size(), 1);
  const auto secondTransition = secondState->next['a'][0];
  BOOST_CHECK_EQUAL(secondTransition.guard.size(), 1);
  BOOST_CHECK_EQUAL(secondTransition.resetVars.size(), 1);
  const auto acceptingState = secondTransition.target;
  BOOST_TEST(acceptingState->isMatch);
}

BOOST_FIXTURE_TEST_CASE(renameToEpsilonTransitionsConcat, ParseTRE)
{
  TimedAutomaton TA;
  parse("a(a%(1,2))");

  // Skip Normalize to have epsilon transitions easily
  DNFTRE dnf(driver.getResult());

  // dnf.toNormalForm();
  dnf.toSignalTA(TA);
  renameToEpsilonTransitions(TA);
  BOOST_TEST(!TA.isMember({{0, 0.9}, {'a', 1.2}}));
  BOOST_TEST(TA.isMember({{0, 2.9}, {'a', 4.2}}));
}

BOOST_FIXTURE_TEST_CASE(renameToEpsilonTransitionsConcatIntervals, ParseTRE)
{
  TimedAutomaton TA;
  parse("a(a%(1,2))a(a%(2,3))");

  // Skip Normalize to have epsilon transitions easily
  DNFTRE dnf(driver.getResult());

  // dnf.toNormalForm();
  dnf.toSignalTA(TA);
  renameToEpsilonTransitions(TA);
  BOOST_TEST(!TA.isMember({{0, 0.9}, {0, 1.2}, {0, 1.7}, {'a', 2.5}}));
  BOOST_TEST(TA.isMember({{0, 2.9}, {0, 4.2}, {0, 4.7}, {'a', 7.0}}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(concat2Test)
BOOST_AUTO_TEST_CASE(renameToEpsilonTransitionsSimpleConcat)
{
  std::array<TimedAutomaton, 2> TA;
  for (auto &A: TA) {
    A.states.resize(2);
    for (auto& state: A.states) {
      state = std::make_shared<TAState>();
    }
    A.states[1]->isMatch = true;
    A.initialStates = {A.states[0]};

    A.states[0]->next['a'].emplace_back(TATransition{A.states[1].get(), {0}, {TimedAutomaton::X(0) < 10}});
    A.maxConstraints = {10};
  }

  concat2(TA[0], TA[1]);
  BOOST_CHECK_EQUAL(TA[0].initialStates.size(), 1);
  const auto initialState = TA[0].initialStates[0];
  BOOST_CHECK_EQUAL(initialState->next['a'].size(), 2);
  const auto firstTransition = initialState->next['a'][1];
  BOOST_CHECK_EQUAL(firstTransition.guard.size(), 1);
  BOOST_CHECK_EQUAL(firstTransition.resetVars.size(), 1);
  const auto secondState = firstTransition.target;
  BOOST_CHECK_EQUAL(secondState->next['a'].size(), 1);
  const auto secondTransition = secondState->next['a'][0];
  BOOST_CHECK_EQUAL(secondTransition.guard.size(), 1);
  BOOST_CHECK_EQUAL(secondTransition.resetVars.size(), 1);
  const auto acceptingState = secondTransition.target;
  BOOST_TEST(acceptingState->isMatch);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

