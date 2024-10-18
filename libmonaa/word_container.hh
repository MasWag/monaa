#pragma once

#include "lazy_deque.hh"
#include <vector>

/*!
  @brief Container class for the input timed word.

  @note This class just defines the interface of the container. The actual
  definition must be done in the class Container passed by template argument. An
  example is @link LazyDeque @endlink.
*/
template <class Container>
class WordContainer {
protected:
  //! @brief The actual container of the timed word.
  Container vec;

public:
  /*!
    @brief The constructor

    @param [in] file The name of the file that the input timed word is in. If
    the container does not use the timed word in a file (e.g., when a result of
    simulation is directly passed), this can be ignored.
    @param [in] isBinary A flag if the input is in a binary file.
  */
  WordContainer(FILE *file, bool isBinary = false)
      : vec(file, isBinary) {}
  /*!
    @brief Access an element of the container.
    @note If the argument is out of range, out_of_range exception can be thrown.

    @param [in] n A position in the timed word
    @return The n-th element in the timed word
  */
  typename Container::value_type operator[](const std::size_t n) {
    return vec[n];
  }
  /*!
    @brief Access an element of the container.
    @note If the argument is out of range, out_of_range exception can be thrown.

    @param [in] n A position in the timed word
    @return The n-th element in the timed word
  */
  typename Container::value_type at(const std::size_t n) { return vec.at(n); }
  /*!
    @brief Returns the length of the timed word.
    @note If the length of the timed word is unknown (e.g., when the stream does
    not reach the end yet), the maximum value of std::size_t is returned.

    @return The length of the timed word, if it is known.
  */
  std::size_t size() const { return vec.size(); }
  /*!
    @brief Discard the elements before n-th position
    @note If n is out of range, out_of_range exception may be thrown.

    @param [in] n A position in the timed word
  */
  void setFront(std::size_t n) { return vec.setFront(n); }
  /*!
    @brief Fetch the timed words by n-th position.

    @param [in] n A position in the timed word
    @return It returns true if and only if the fetch succeeded.
  */
  bool fetch(std::size_t n) { return vec.fetch(n); }
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

template <class T> class Vector : public std::vector<T> {
private:
public:
  Vector(FILE *, bool) {}
  void setFront(std::size_t) {}
  bool fetch(std::size_t n) { return n < this->size(); }
};

template <class T> class WordVector : public WordContainer<Vector<T>> {
public:
  WordVector(FILE *file, bool isBinary = false)
      : WordContainer<Vector<T>>(file, isBinary) {
    const auto getOneElem = isBinary ? getOneBinary : getOne;
    T elem;
    while (getOneElem(file, elem) != EOF) {
      this->vec.push_back(elem);
    }
  }
};
