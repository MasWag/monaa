#include <boost/test/unit_test.hpp>
#include <rapidcheck/boost_test.h>

#include <sstream>
#include "../monaa/tre_driver.hh"
#include "../monaa/tre.hh"

BOOST_AUTO_TEST_SUITE(treTest)

  class ParseTRE {
  public:
    TREDriver driver;

    ParseTRE() {}

    void parse(const char *tre) {
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

    void constructEventTA(const char *tre) {
      parse(tre);
      driver.getResult()->toEventTA(TA);
    }

    void checkMemory() {
      // check if the initial states are in the states
      for (const auto &initState: TA.initialStates) {
        BOOST_TEST((std::find(TA.states.begin(), TA.states.end(), initState) != TA.states.end()));
      }
      // check if the target states are in the states
      for (const auto &state: TA.states) {
        for (const auto &transitions: state->next) {
          for (const auto &transition: transitions.second) {
            BOOST_TEST((std::find_if(TA.states.begin(), TA.states.end(), [&](const std::shared_ptr<TAState> sp) {
              return static_cast<TAState *>(sp.get()) == static_cast<TAState *>(transition.target);
            }) != TA.states.end()));
          }
        }
      }
    }
  };

  BOOST_AUTO_TEST_SUITE(treTest)

    BOOST_FIXTURE_TEST_CASE(toEventTAUntimed, ConstructTA) {
      constructEventTA("ab");

      BOOST_TEST(!TA.isMember({}));
      BOOST_TEST(TA.isMember({{'a', 1.2},
                              {'b', 2.0}}));
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'b', 2.0},
                               {'a', 3.0}}));
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'b', 2.0},
                               {'a', 3.3}}));
    }

    BOOST_FIXTURE_TEST_CASE(toEventTATimed, ConstructTA) {
      constructEventTA("((aba)%(1,2))*");

      BOOST_CHECK_EQUAL(TA.initialStates.size(), 2);
      BOOST_CHECK_EQUAL(bool(TA.initialStates[0]->isMatch) + bool(TA.initialStates[1]->isMatch), 1);
      auto initialState = TA.initialStates[0];
      if (initialState->isMatch) {
        initialState =  TA.initialStates[1];
      }
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
      BOOST_TEST(bool(thirdState->next['a'][0].target->isMatch));
      BOOST_TEST(!bool(thirdState->next['a'][1].target->isMatch));
      const auto toAcceptingState = thirdState->next['a'][1];
      BOOST_CHECK_EQUAL(toAcceptingState.guard.size(), 2);
      BOOST_CHECK_EQUAL(toAcceptingState.guard[0].c, 2);
      BOOST_CHECK_EQUAL(toAcceptingState.guard[1].c, 1);


      BOOST_TEST(TA.isMember({}));
      BOOST_TEST(TA.isMember({{'a', 0.2},
                              {'b', 1.0},
                              {'a', 1.9}}));
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'b', 1.8},
                               {'a', 2.0}}));
    }


    BOOST_FIXTURE_TEST_CASE(isMember1_2, ConstructTA) {
      constructEventTA("a%(1,2)");

      BOOST_TEST(!TA.isMember({{'a', 0.2}}));
      BOOST_TEST(TA.isMember({{'a', 1.2}}));
      BOOST_TEST(!TA.isMember({{'a', 2.9}}));
    }

    BOOST_FIXTURE_TEST_CASE(toEventTATimedSigleton, ConstructTA) {
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
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'b', 2.0},
                               {'a', 3.3}}));
    }

    BOOST_FIXTURE_TEST_CASE(toEventTATimedSigleton1_2, ConstructTA) {
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
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'b', 2.0},
                               {'a', 3.3}}));
      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(toEventTAConcatIntervals, ConstructTA) {
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
      BOOST_TEST(TA.isMember({{'a', 0.2},
                              {'b', 0.4}}));
      BOOST_TEST(TA.isMember({{'a', 0.6},
                              {'b', 1.4}}));
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'a', 1.8}}));
      BOOST_TEST(!TA.isMember({{'a', 1.2},
                               {'b', 2.0},
                               {'a', 3.3}}));
      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(star, ConstructTA) {
      constructEventTA("A*");

      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(concat, ConstructTA) {
      constructEventTA("AB");

      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(disjunction, ConstructTA) {
      constructEventTA("A|B");

      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(conjunction, ConstructTA) {
      constructEventTA("A&B");

      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(starConcat, ConstructTA) {
      constructEventTA("A*B");

      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(simpleStar, ConstructTA) {
      constructEventTA("(A*B)$");

      BOOST_TEST(TA.isMember({{'B', 0.2},
                              {'$', 0.8}}));
      BOOST_TEST(TA.isMember({{'A', 1.2},
                              {'B', 1.8},
                              {'$', 1.9}}));
      BOOST_TEST(TA.isMember({{'A', 1.2},
                              {'A', 1.5},
                              {'B', 1.8},
                              {'$', 1.9}}));
      BOOST_TEST(!TA.isMember({{'A', 1.9},
                               {'$', 2.9}}));
      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(simplePlus, ConstructTA) {
      constructEventTA("(A+B)$");

      BOOST_TEST(!TA.isMember({{'B', 0.2},
                               {'$', 0.8}}));
      BOOST_TEST(TA.isMember({{'A', 1.2},
                              {'B', 1.8},
                              {'$', 1.9}}));
      BOOST_TEST(TA.isMember({{'A', 1.2},
                              {'A', 1.5},
                              {'B', 1.8},
                              {'$', 1.9}}));
      BOOST_TEST(!TA.isMember({{'A', 1.9},
                               {'$', 2.9}}));
      checkMemory();
    }

    BOOST_FIXTURE_TEST_CASE(simpleDisj, ConstructTA) {
      constructEventTA("(A|B)$");

      BOOST_TEST(TA.isMember({{'B', 0.2},
                              {'$', 0.8}}));
      BOOST_TEST(!TA.isMember({{'A', 1.2},
                               {'B', 1.8},
                               {'$', 1.9}}));
      BOOST_TEST(!TA.isMember({{'A', 1.2},
                               {'A', 1.5},
                               {'B', 1.8},
                               {'$', 1.9}}));
      BOOST_TEST(TA.isMember({{'A', 1.9},
                              {'$', 2.9}}));
      checkMemory();
    }

  BOOST_AUTO_TEST_SUITE_END()

  BOOST_AUTO_TEST_SUITE(treAcceptanceTest)
  RC_BOOST_PROP(untimedAccept,
                (const std::vector<std::pair<Alphabet, double>> &wordDiff)) {
    std::vector<std::pair<Alphabet, double>> word;
    double totalTime = 0;
    std::shared_ptr<TRE> concatTRE = std::make_shared<TRE>(TRE::op::epsilon);
    for (const std::pair<Alphabet, double> &actionWithTimeDiff: wordDiff) {
      totalTime += actionWithTimeDiff.second;
      concatTRE = std::make_shared<TRE>(TRE::op::concat, concatTRE, std::make_shared<TRE>(TRE::op::atom, actionWithTimeDiff.first));
      word.emplace_back(actionWithTimeDiff.first, totalTime);
    }
    TimedAutomaton TA;
    concatTRE->toEventTA(TA);
    RC_ASSERT(TA.isMember(word));
  }
  BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
