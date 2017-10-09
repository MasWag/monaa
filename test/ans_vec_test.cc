#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../tgrep/ans_vec.hh"

BOOST_AUTO_TEST_SUITE(AnsContainerTest)

typedef boost::mpl::list<AnsVec<int>, AnsNum<int>> testTypesInt;


template<class T>
void push_one(AnsContainer<T> &ans)
{
  ans.push_back(1);
}

// We can instanciate
BOOST_AUTO_TEST_CASE_TEMPLATE( push, T, testTypesInt)
{
  T ans;
  ans.push_back(0);
  push_one(ans);
  BOOST_CHECK_EQUAL(ans.size(), 2);
  ans.push_back(0);
  push_one(ans);
  BOOST_CHECK_EQUAL(ans.size(), 4);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(AnsVecTest)

// We can instanciate
BOOST_AUTO_TEST_CASE( sum )
{
  AnsVec<int> ans;
  ans.push_back(1);
  ans.push_back(2);
  ans.push_back(3);
  ans.push_back(4);
  int sum = 0;
  for (auto item: ans) {
    sum += item;
  }
  BOOST_CHECK_EQUAL(sum, 10);
}

BOOST_AUTO_TEST_SUITE_END()
