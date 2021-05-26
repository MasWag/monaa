#pragma once

#include <cassert>
#include <cstdio>
#include <deque>
#include <limits>
#include <stdexcept>
#include <utility>

#include "common_types.hh"

static inline int getOne(FILE *file, std::pair<Alphabet, double> &p) {
  return fscanf(file, " %c %lf\n", &p.first, &p.second);
}

static inline int getOneBinary(FILE *file, std::pair<Alphabet, double> &p) {
  if (fread(&p.first, sizeof(char), 1, file)) {
    if (fread(&p.second, sizeof(double), 1, file)) {
      return sizeof(char) + sizeof(double);
    }
  }
  return EOF;
}

/*!
  @brief A Wrapper of FILE Reading. This class is given to @link WordContainer
  @endlink class as its template argument.
 */
class LazyDeque : public std::deque<std::pair<Alphabet, double>> {
private:
  std::size_t front = 0;
  std::size_t N;
  FILE *file;
  int (*getElem)(FILE *, std::pair<Alphabet, double> &);

public:
  /*!
    @param [in] file The FILE-pointer of the file in which the input timed word
    is.
    @param [in] isBinary A flag if the input is in a binary file.
   */
  LazyDeque(FILE *file, bool isBinary = false)
      : N(std::numeric_limits<std::size_t>::max()), file(file) {
    assert(file != nullptr);
    getElem = isBinary ? getOneBinary : getOne;
  }
  std::pair<Alphabet, double> operator[](std::size_t n) {
    const std::size_t indInDeque = n - front;
    if (n < front || n >= N ||
        indInDeque >= std::deque<std::pair<Alphabet, double>>::size()) {
      throw std::out_of_range("thrown at LazyDeque::operator[] ");
    }
    return std::deque<std::pair<Alphabet, double>>::at(indInDeque);
  }
  std::pair<Alphabet, double> at(std::size_t n) { return (*this)[n]; }
  std::size_t size() const { return N; }
  //! @brief Update the internal front. The elements before the front are
  //! removed.
  void setFront(std::size_t newFront) {
    if (newFront < front) {
      throw std::out_of_range("thrown at LazyDeque::setFront ");
    }
    const std::size_t eraseSize = std::min(
        newFront - front, std::deque<std::pair<Alphabet, double>>::size());
    const int readTimes = (newFront - front) - eraseSize;
    front = newFront;
    this->erase(this->begin(), this->begin() + eraseSize);
    for (int i = 0; i < readTimes; i++) {
      std::pair<Alphabet, double> elem;
      getElem(file, elem);
    }
  }
  bool fetch(std::size_t n) noexcept {
    if (n < front || n >= N) {
      return false;
    }
    const std::size_t indInDeque = n - front;
    const int allocTimes =
        indInDeque - std::deque<std::pair<Alphabet, double>>::size() + 1;
    for (int i = 0; i < allocTimes; i++) {
      std::pair<Alphabet, double> elem;
      if (getElem(file, elem) == EOF) {
        N = front + std::deque<std::pair<Alphabet, double>>::size();
        return false;
      }
      this->push_back(elem);
    }
    return true;
  }
};
