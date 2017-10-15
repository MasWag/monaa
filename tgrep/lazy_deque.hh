#pragma once

#include <cstdio>
#include <deque>
#include <stdexcept>
#include <utility>
#include <limits>

#include "common_types.hh"

static inline int getOne(FILE* file, std::pair<Alphabet, double> &p) {
  return fscanf(file, " %c %lf\n", &p.first, &p.second);
}

static inline int getOneBinary(FILE* file, std::pair<Alphabet, double> &p) {
  if (fread(&p.first, sizeof(char), 1, file)) {
    if (fread(&p.second, sizeof(double), 1, file)) {
      return sizeof(char) + sizeof(double);
    }
  }
  return EOF;
}

/*!
  @brief A Wrapper of FILE Reading.
 */
class LazyDeque : public std::deque<std::pair<Alphabet, double>>
{
private:
  std::size_t front = 0;
  std::size_t N;
  FILE* file;
  int (*getElem)(FILE*, std::pair<Alphabet, double>&);
public:
  /*!
    @param N size of the file
   */
  LazyDeque (FILE* file, bool isBinary = false) : N(std::numeric_limits<std::size_t>::max()), file(file) {
    getElem = isBinary ? getOneBinary : getOne;
  }
  std::pair<Alphabet, double> operator[](std::size_t n) {
    if (n < front || n >= N) {
      throw std::out_of_range("thrown at LazyDeque::operator[] ");
    }
    const std::size_t indInDeque = n - front;
    const int allocTimes = indInDeque - std::deque<std::pair<Alphabet, double>>::size() + 1;
    for (int i = 0; i < allocTimes; i++) {
      std::pair<Alphabet, double> elem;
      if(getElem(file, elem) == EOF) {
        N = front + std::deque<std::pair<Alphabet, double>>::size();
        throw std::out_of_range("thrown at LazyDeque::operator[] ");        
      }
      this->push_back(elem);
    }
    return std::deque<std::pair<Alphabet, double>>::at(indInDeque);
  }
  std::pair<Alphabet, double> at(std::size_t n) {
    return (*this)[n];
  }
  std::size_t size() const {
    return N;
  }
  //! @brief Update the internal front. The elements before the front are removed.
  void setFront(std::size_t newFront) {
    if (newFront < front) {
      throw std::out_of_range("thrown at LazyDeque::setFront ");
    }
    const std::size_t eraseSize = std::min(newFront - front, std::deque<std::pair<Alphabet, double>>::size());
    const int readTimes = (newFront - front) - eraseSize;
    front = newFront;
    this->erase(this->begin(), this->begin() + eraseSize);
    for (int i = 0; i < readTimes; i++) {
      std::pair<Alphabet, double> elem;
      getElem(file, elem);
    }
  }
};
