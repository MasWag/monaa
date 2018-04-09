#pragma once

#include <vector>
#include "lazy_deque.hh"

/*!
  @brief Abstract class for the input timed word

  @note This class just defines the interface of the container.
*/
class AbstractTimedWordContainer
{
public:
  /*!
    @brief Access an element of the container.
    @note If the argument is out of range, out_of_range exception can be thrown.

    @param [in] n A position in the timed word
    @return The n-th element in the timed word
  */
  virtual std::pair<Alphabet, double> operator[](const std::size_t n) = 0;

  /*!
    @brief Access an element of the container.
    @note If the argument is out of range, out_of_range exception can be thrown.

    @param [in] n A position in the timed word
    @return The n-th element in the timed word
  */
  virtual std::pair<Alphabet, double> at(const std::size_t n) = 0;

  /*!
    @brief Returns the length of the timed word.
    @note If the length of the timed word is unknown (e.g., when the stream does not reach the end yet), the maximum value of std::size_t is returned.
    
    @return The length of the timed word, if it is known.
  */
  virtual std::size_t size() const = 0;

  /*!
    @brief Discard the elements before n-th position
    @note If n is out of range, out_of_range exception may be thrown.

    @param [in] n A position in the timed word
  */
  virtual void setFront(std::size_t n) = 0;

  /*!
    @brief Fetch the timed words by n-th position.

    @param [in] n A position in the timed word
    @return It returns true if and only if the fetch succeeded.
  */
  virtual bool fetch(std::size_t n) = 0;
};
