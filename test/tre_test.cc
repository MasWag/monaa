#include <boost/test/unit_test.hpp>
#include <rapidcheck/boost_test.h>

#include <sstream>
#include "../monaa/tre_driver.hh"
#include "../monaa/tre.hh"

#include "generate_tre.hh"

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

int count = 0;

RC_BOOST_PROP(conjunction, 
              (const std::vector<std::pair<Alphabet, double>> &wordDiff,
               const AtomTRE leftAtomic,
               const AtomTRE rightAtomic)) {
  // The label must not zero since it is the special character for unobservable transitions
  RC_PRE(leftAtomic.c != 0);
  RC_PRE(rightAtomic.c != 0);
  std::shared_ptr<TRE> left = leftAtomic.toTRE();
  std::shared_ptr<TRE> right = rightAtomic.toTRE();

  std::vector<std::pair<Alphabet, double>> word;
  double totalTime = 0;
  for (const std::pair<Alphabet, double> &actionWithTimeDiff: wordDiff) {
    totalTime += actionWithTimeDiff.second;
    word.emplace_back(actionWithTimeDiff.first, totalTime);
  }
  int N = *rc::gen::inRange(1, 20);
  for(int i = 0; i < N; ++i) {
    // construct next expressions
    left = generate(left);
    right = generate(right);
  }
  std::shared_ptr<TRE> conjunction = std::make_shared<TRE>(TRE::op::conjunction, left, right);
  TimedAutomaton leftTA, rightTA, TA;
  std::cout << "hello world" << count++ << std::endl;
  left->toEventTA(leftTA);
  right->toEventTA(rightTA);
  conjunction->toEventTA(TA);
  if ((leftTA.isMember(word) and rightTA.isMember(word)) != TA.isMember(word)) {
    std::cout << "conjunction" << std::endl;
    //    left->print(std::cout);
    std::cout << std::endl;
    //    right->print(std::cout);
    std::cout << std::endl;
    std::cout << leftTA.isMember(word) << " " << rightTA.isMember(word) << " " <<  TA.isMember(word) << std::endl;
  }
  // inLeftTA and inRightTA <--> inTA
  RC_ASSERT((leftTA.isMember(word) and rightTA.isMember(word)) == TA.isMember(word));
}


RC_BOOST_PROP(disjunction,
                            (const std::vector<std::pair<Alphabet, double>> &wordDiff,
               const AtomTRE leftAtomic,
               const AtomTRE rightAtomic)) {
  // The label must not zero since it is the special character for unobservable transitions
  RC_PRE(leftAtomic.c != 0);
  RC_PRE(rightAtomic.c != 0);
  std::shared_ptr<TRE> left = leftAtomic.toTRE();
  std::shared_ptr<TRE> right = rightAtomic.toTRE();

  std::vector<std::pair<Alphabet, double>> word;
  double totalTime = 0;
  for (const std::pair<Alphabet, double> &actionWithTimeDiff: wordDiff) {
    totalTime += actionWithTimeDiff.second;
    word.emplace_back(actionWithTimeDiff.first, totalTime);
  }
  int N = *rc::gen::inRange(1, 20);
  for(int i = 0; i < N; ++i) {
    // construct next expressions
    left = generate(std::move(left));
    right = generate(std::move(right));
  }
  std::shared_ptr<TRE> disjunction = std::make_shared<TRE>(TRE::op::disjunction, left, right);
  TimedAutomaton leftTA, rightTA, TA;
  left->toEventTA(leftTA);
  right->toEventTA(rightTA);
  disjunction->toEventTA(TA);
  if ((leftTA.isMember(word) or rightTA.isMember(word)) != TA.isMember(word)) {
    std::cout << "disjunction" << std::endl;
    // left->print(std::cout);
    std::cout << std::endl;
    // right->print(std::cout);
    std::cout << std::endl;
    std::cout << leftTA.isMember(word) << " " << rightTA.isMember(word) << " " <<  TA.isMember(word) << std::endl;
  }
  // inLeftTA or inRightTA <--> inTA
  RC_ASSERT((leftTA.isMember(word) or rightTA.isMember(word)) == TA.isMember(word));
}

RC_BOOST_PROP(concat,
              (const std::vector<std::pair<Alphabet, double>> &leftWordDiff,
               const std::vector<std::pair<Alphabet, double>> &rightWordDiff,
               const AtomTRE leftAtomic,
               const AtomTRE rightAtomic)) {
  // The label must not zero since it is the special character for unobservable transitions
  RC_PRE(leftAtomic.c != 0);
  RC_PRE(rightAtomic.c != 0);
  std::shared_ptr<TRE> left = leftAtomic.toTRE();
  std::shared_ptr<TRE> right = rightAtomic.toTRE();

  std::vector<std::pair<Alphabet, double>> word, leftWord, rightWord;
  double totalTime = 0;
  double rightTotalTime = 0;
  for (const std::pair<Alphabet, double> &actionWithTimeDiff: leftWordDiff) {
    totalTime += actionWithTimeDiff.second;
    word.emplace_back(actionWithTimeDiff.first, totalTime);
    leftWord.emplace_back(actionWithTimeDiff.first, totalTime);
  }
  for (const std::pair<Alphabet, double> &actionWithTimeDiff: rightWordDiff) {
    rightTotalTime += actionWithTimeDiff.second;
    totalTime += actionWithTimeDiff.second;
    word.emplace_back(actionWithTimeDiff.first, totalTime);
    rightWord.emplace_back(actionWithTimeDiff.first, rightTotalTime);
  }
  int N = *rc::gen::inRange(1, 20);
  for(int i = 0; i < N; ++i) {
    // construct next expressions
    left = generate(std::move(left));
    right = generate(std::move(right));
  }
  std::shared_ptr<TRE> concat = std::make_shared<TRE>(TRE::op::concat, left, right);
  TimedAutomaton leftTA, rightTA, TA;
  left->toEventTA(leftTA);
  right->toEventTA(rightTA);
  concat->toEventTA(TA);
  if ((leftTA.isMember(leftWord) && rightTA.isMember(rightWord)) != TA.isMember(word)) {
    std::cout << "concat" << std::endl;
    // left->print(std::cout);
    std::cout << std::endl;
    // right->print(std::cout);
    std::cout << std::endl;
    std::cout << leftTA.isMember(word) << " " << rightTA.isMember(word) << " " <<  TA.isMember(word) << std::endl;
  }
  // inLeftTA and inRightTA <--> inTA
  RC_ASSERT((leftTA.isMember(leftWord) && rightTA.isMember(rightWord)) == TA.isMember(word));
}

RC_BOOST_PROP(plus,
              (const std::vector<std::pair<Alphabet, double>> &wordDiff,
               const AtomTRE treAtomic)) {
  // The label must not zero since it is the special character for unobservable transitions
  RC_PRE(treAtomic.c != 0);
  std::shared_ptr<TRE> tre = treAtomic.toTRE();

  std::vector<std::pair<Alphabet, double>> word, originalWord;
  double totalTime = 0;
  int repeatNum = *rc::gen::inRange(1, 20);
  for (int i = 0; i < repeatNum; ++i) {
    for (const std::pair<Alphabet, double> &actionWithTimeDiff: wordDiff) {
      totalTime += actionWithTimeDiff.second;
      word.emplace_back(actionWithTimeDiff.first, totalTime);
      if (i == 0) {
        originalWord.emplace_back(actionWithTimeDiff.first, totalTime);
      }
    }
  }
  int N = *rc::gen::inRange(1, 20);
  for(int i = 0; i < N; ++i) {
    // construct next expressions
    tre = generate(std::move(tre));
  }
  std::shared_ptr<TRE> plus = std::make_shared<TRE>(TRE::op::plus, tre);
  TimedAutomaton plusTA, TA;
  tre->toEventTA(TA);
  plus->toEventTA(plusTA);

  if (plusTA.isMember(word) != TA.isMember(originalWord)) {
    std::cout << "plus" << std::endl;
    std::cout << "word.size(): " << word.size() << std::endl;
    std::cout << "originalWord.size(): " << originalWord.size() << std::endl;
    std::cout << plusTA.isMember(word) << std::endl;
    std::cout << TA.isMember(originalWord) << std::endl;
  }
  // inTA -> inPlusTA (the opposite does not hold in general)
  RC_ASSERT(!TA.isMember(originalWord) or plusTA.isMember(word));
}

RC_BOOST_PROP(within,
              (const std::vector<std::pair<Alphabet, int>> &wordDiff,
               const AtomTRE treAtomic)) {
  // We use integer-valued time differences so that the time does not become too large
  // The label must not zero since it is the special character for unobservable transitions
  RC_PRE(treAtomic.c != 0);
  std::shared_ptr<TRE> tre = treAtomic.toTRE();

  std::vector<std::pair<Alphabet, double>> word;
  double totalTime = 0;
  for (const std::pair<Alphabet, int> &actionWithTimeDiff: wordDiff) {
    totalTime += static_cast<double>(actionWithTimeDiff.second) / 3;
    word.emplace_back(actionWithTimeDiff.first, totalTime);
  }
  RC_PRE(2 * totalTime < INT_MAX);
  int N = *rc::gen::inRange(1, 20);
  for(int i = 0; i < N; ++i) {
    // construct next expressions
    tre = generate(std::move(tre));
  }
  Bounds lowerBound = *rc::gen::construct<Bounds>(rc::gen::inRange<int>(0, totalTime * 2), rc::gen::arbitrary<bool>());
  Bounds upperBound = *rc::gen::construct<Bounds>(rc::gen::inRange<int>(lowerBound.first, totalTime * 2), rc::gen::arbitrary<bool>());
  auto interval = std::make_shared<Interval>(lowerBound, upperBound);
  std::shared_ptr<TRE> within = std::make_shared<TRE>(TRE::op::within, tre, interval);
  TimedAutomaton withinTA, TA;
  tre->toEventTA(TA);
  within->toEventTA(withinTA);

  if (withinTA.isMember(word) != TA.isMember(word)) {
    std::cout << "within" << std::endl;
    std::cout << "word.size(): " << word.size() << std::endl;
    std::cout << withinTA.isMember(word) << std::endl;
    std::cout << TA.isMember(word) << std::endl;
    std::cout << totalTime << std::endl;
    std::cout << *interval << std::endl;
  }
  // (inTA && lowerBound <= totalTime <= upperBound) <--> inWithinTA
  RC_ASSERT(!(TA.isMember(word) && interval->contain(totalTime)) or withinTA.isMember(word));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
