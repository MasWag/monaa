#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../monaa/tre_driver.hh"
#include "../monaa/tre.hh"

BOOST_AUTO_TEST_SUITE(treTest)
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

class ConstructTA : private ParseTRE {
public:
  ConstructTA() {}
  TimedAutomaton TA;
  void constructEventTA(const char* tre) {
    parse(tre);
    boost::unordered_map<TAState*, uint32_t> indegree;
    driver.getResult()->toEventTA(TA, indegree);
  }
};

BOOST_AUTO_TEST_SUITE(treTest)

BOOST_FIXTURE_TEST_CASE(toEventTAUntimed, ConstructTA)
{
  constructEventTA("ab");

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 2.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.0}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTATimed, ConstructTA)
{
  constructEventTA("((aba)%(1,2))*");

  BOOST_CHECK_EQUAL(TA.initialStates.size(), 2);
  BOOST_TEST( bool(TA.initialStates[0]->isMatch));
  BOOST_TEST(!bool(TA.initialStates[1]->isMatch));
  const auto initialState = TA.initialStates[1];
  BOOST_CHECK_EQUAL(initialState->next['a'].size(), 1);
  BOOST_TEST(!bool(initialState->next['a'][0].target->isMatch));
  const auto toSecondState = initialState->next['a'][0];
  const auto secondState = toSecondState.target;
  BOOST_CHECK_EQUAL(secondState->next['b'].size(), 1);
  BOOST_TEST(!bool(secondState->next['b'][0].target->isMatch));
  BOOST_TEST(!bool(secondState->next['b'][1].target->isMatch));
  const auto toThirdState = secondState->next['b'][1];
  const auto thirdState = toThirdState.target;
  BOOST_CHECK_EQUAL(thirdState->next['a'].size(), 2);
  BOOST_TEST( bool(thirdState->next['a'][0].target->isMatch));
  BOOST_TEST(!bool(thirdState->next['a'][1].target->isMatch));
  const auto toAcceptingState = thirdState->next['a'][1];
  BOOST_CHECK_EQUAL(toAcceptingState.guard.size(), 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].c, 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].c, 1);


  BOOST_TEST(TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 1.3}, {'a', 1.9}}));
  BOOST_TEST(TA.isMember({{'a', 0.2}, {'b', 0.3}, {'a', 1.1}, {'a', 1.2}, {'b', 1.3}, {'a', 2.3}}));
  BOOST_TEST(TA.isMember({{'a', 1.2}, {'b', 1.3}, {'a', 1.9}, {'a', 2.2}, {'b', 2.3}, {'a', 3.2}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}


BOOST_FIXTURE_TEST_CASE(isMember1_2, ConstructTA)
{
  constructEventTA("a%(1,2)");

  BOOST_TEST(!TA.isMember({{'a', 0.2}}));
  BOOST_TEST(TA.isMember({{'a', 1.2}}));
  BOOST_TEST(!TA.isMember({{'a', 2.9}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTATimedSigleton, ConstructTA)
{
  constructEventTA("(a)%(0,1)");

  BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
  const auto initialState = TA.initialStates[0];
  BOOST_CHECK_EQUAL(initialState->next['a'].size(), 1);
  BOOST_TEST(bool(initialState->next['a'][0].target->isMatch));
  const auto toAcceptingState = initialState->next['a'][0];
  BOOST_CHECK_EQUAL(toAcceptingState.guard.size(), 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].c, 1);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].c, 0);

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 0.2}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTATimedSigleton1_2, ConstructTA)
{
  constructEventTA("(a)%(1,2)");

  BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
  const auto initialState = TA.initialStates[0];
  BOOST_CHECK_EQUAL(initialState->next['a'].size(), 1);
  BOOST_TEST(bool(initialState->next['a'][0].target->isMatch));
  const auto toAcceptingState = initialState->next['a'][0];
  BOOST_CHECK_EQUAL(toAcceptingState.guard.size(), 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].c, 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].c, 1);

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(!TA.isMember({{'a', 0.2}}));
  BOOST_TEST(TA.isMember({{'a', 1.2}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_FIXTURE_TEST_CASE(toEventTAConcatIntervals, ConstructTA)
{
  constructEventTA("(a%(0,1))(b%(0,1))");

  BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
  const auto initialState = TA.initialStates[0];
  BOOST_CHECK_EQUAL(initialState->next['a'].size(), 1);
  BOOST_TEST(!bool(initialState->next['a'][0].target->isMatch));
  const auto toSecondState = initialState->next['a'][0];
  const auto secondState = toSecondState.target;
  BOOST_CHECK_EQUAL(toSecondState.resetVars.size(), 1);
  BOOST_CHECK_EQUAL(toSecondState.guard.size(), 2);
  BOOST_CHECK_EQUAL(toSecondState.guard[0].c, 1);
  BOOST_CHECK_EQUAL(toSecondState.guard[1].c, 0);

  BOOST_CHECK_EQUAL(secondState->next['b'].size(), 1);
  BOOST_TEST(bool(secondState->next['b'][0].target->isMatch));
  const auto toAcceptingState = secondState->next['b'][0];
  BOOST_CHECK_EQUAL(toAcceptingState.guard.size(), 2);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[0].c, 1);
  BOOST_CHECK_EQUAL(toAcceptingState.guard[1].c, 0);
  BOOST_TEST(toAcceptingState.target->isMatch);

  BOOST_TEST(!TA.isMember({}));
  BOOST_TEST(TA.isMember({{'a', 0.2}, {'b', 0.4}}));
  BOOST_TEST(TA.isMember({{'a', 0.6}, {'b', 1.4}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'a', 1.8}}));
  BOOST_TEST(!TA.isMember({{'a', 1.2}, {'b', 2.0}, {'a', 3.3}}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
