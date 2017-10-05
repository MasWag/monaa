#pragma once

#include <vector>
#include "lazy_deque.hh"

template <class Container>
class WordContainer
{
protected:
  Container vec;
public:
  WordContainer<Container>(const std::size_t N, FILE* file,bool isBinary = false) : vec (N, file, isBinary) {}
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

template <class T>
using WordLazyDeque = WordContainer<LazyDeque<T>>;

/*!
  @class WordVector
  @brief Word container without any runtime memory alloc / free.
*/


template<class T>
class Vector : public std::vector<T>
{
private:
public:
  Vector(const std::size_t N, FILE*, bool) {
    this->resize(N);
  }
  void setFront(std::size_t) {}
};

template <class T>
class WordVector : public WordContainer<Vector<T>>
{
public:
  WordVector (const std::size_t N, FILE* file, bool isBinary = false) : WordContainer<Vector<T>>(N, file, isBinary) {
    const auto getOneElem = isBinary ? getOneBinary : getOne;
    this->vec.resize (N);
    for (auto &p : this->vec) {
      T elem;
      getOneElem(file, elem);
      p = elem;
    }
  }
};
