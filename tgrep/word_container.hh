#pragma once

#include <vector>
#include "lazy_deque.hh"

template <class Container>
class WordContainer
{
protected:
  Container vec;
public:
  WordContainer<Container>(FILE* file,bool isBinary = false) : vec (file, isBinary) {}
  typename Container::value_type operator[](const std::size_t n) {
    return vec[n];
  }
  typename Container::value_type at(const std::size_t n) {
    return vec.at(n);
  }
  std::size_t size() const {
    return vec.size();
  }
  void setFront(std::size_t n) {
    return vec.setFront(n);
  }
};

/*!
  @class WordLazyDeque
  @brief Word container with runtime allocation and free.
*/

using WordLazyDeque = WordContainer<LazyDeque>;

/*!
  @class WordVector
  @brief Word container without any runtime memory alloc / free.
*/


template<class T>
class Vector : public std::vector<T>
{
private:
public:
  Vector(FILE*, bool) {
  }
  void setFront(std::size_t) {}
};

template <class T>
class WordVector : public WordContainer<Vector<T>>
{
public:
  WordVector (FILE* file, bool isBinary = false) : WordContainer<Vector<T>>(file, isBinary) {
    const auto getOneElem = isBinary ? getOneBinary : getOne;
    T elem;
    while (getOneElem(file, elem) != EOF) {
      this->vec.push_back(elem);
    }
  }
};
