#pragma once

#include <cstdio>
#include <deque>
#include <stdexcept>
#include <utility>
#include <limits>
#include <cstring>
#include <ppl.hh>

#include "common_types.hh"

template<class T>
int getOne(FILE*, std::pair<Alphabet, T> &);

template<>
inline int getOne(FILE* file, std::pair<Alphabet, double> &p) {
  return fscanf(file, " %c %lf\n", &p.first, &p.second);
}

template<>
inline int getOne(FILE* file, std::pair<Alphabet, mpq_class> &p) {
  unsigned long num, dd, den = 1;
  char str[10];
  const auto ret = fscanf(file, " %c %lu", &p.first, &num);

  if (ret == EOF) {
    return ret;
  } else if (ret != 2) {
    std::cerr << "wrong input format\n";
  }

  switch (fgetc(file)) { 
  case '.': {
    // decimals
    fscanf(file, "%9s", str);
    const std::size_t len = strlen(str);
    sscanf(str, "%lu", &dd);
    for (std::size_t i = 0; i < len; ++i) {
      num *= 10;
      den *= 10;
    }
    num += dd;
    break;
  }
  case '\n':
  case ' ':
  case '\t':
    // integers
    break;
  case EOF:
  case '\0':
    //EOF
    return EOF;
  default:
    std::cerr << "wrong input format\n";
    abort();
  }

  p.second.get_num() = num;
  p.second.get_den() = den;
  return ret;
}

template<class T>
int getOneBinary(FILE* file, std::pair<Alphabet, T> &p) {
  if (fread(&p.first, sizeof(char), 1, file)) {
    if (fread(&p.second, sizeof(T), 1, file)) {
      return sizeof(char) + sizeof(T);
    }
  }
  return EOF;
}

/*!
  @brief A Wrapper of FILE Reading. This class is given to @link WordContainer @endlink class as its template argument.
 */
template<class T>
class LazyDeque : public std::deque<std::pair<Alphabet, T>>
{
private:
  std::size_t front = 0;
  std::size_t N;
  FILE* file;
  int (*getElem)(FILE*, std::pair<Alphabet, T>&);
public:
  using TimeStamp = T;
  /*!
    @param [in] file The FILE-pointer of the file in which the input timed word is.
    @param [in] isBinary A flag if the input is in a binary file.
   */
  LazyDeque (FILE* file, bool isBinary = false) : N(std::numeric_limits<std::size_t>::max()), file(file) {
    getElem = isBinary ? getOneBinary<T> : getOne<T>;
  }
  std::pair<Alphabet, T> operator[](std::size_t n) {
    const std::size_t indInDeque = n - front;
    if (n < front || n >= N || indInDeque >= std::deque<std::pair<Alphabet, T>>::size()) {
      throw std::out_of_range("thrown at LazyDeque::operator[] ");
    }
    return std::deque<std::pair<Alphabet, T>>::at(indInDeque);
  }
  std::pair<Alphabet, T> at(std::size_t n) {
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
    const std::size_t eraseSize = std::min(newFront - front, std::deque<std::pair<Alphabet, T>>::size());
    const int readTimes = (newFront - front) - eraseSize;
    front = newFront;
    this->erase(this->begin(), this->begin() + eraseSize);
    for (int i = 0; i < readTimes; i++) {
      std::pair<Alphabet, T> elem;
      getElem(file, elem);
    }
  }
  bool fetch(std::size_t n) noexcept {
    if (n < front || n >= N) {
      return false;
    }
    const std::size_t indInDeque = n - front;
    const int allocTimes = indInDeque - std::deque<std::pair<Alphabet, T>>::size() + 1;
    for (int i = 0; i < allocTimes; i++) {
      std::pair<Alphabet, T> elem;
      if(getElem(file, elem) == EOF) {
        N = front + std::deque<std::pair<Alphabet, T>>::size();
        return false;
      }
      this->push_back(elem);
    }
    return true;
  }
};
