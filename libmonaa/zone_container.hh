#pragma once

#include <vector>

#include "zone.hh"

/*!
  @brief Abstract class for the output zones.

  @note This class just defines the interface of the container. 
  @note This class does not require to contain the zones but just have the interface to @link push_back @endlink.
*/
class AbstractAnsZoneContainer
{
public:
  /*!
    @brief Returns the number of the contained zones.

    @return The number of the contained zones.
  */
  virtual std::size_t size() const = 0;

  /*!
    @brief Append a zone to the container.

    @param [in] in A zone to be appended.
  */
  virtual void push_back(const Zone & in) = 0;

  /*!
    @brief Remove all the contained zones.
  */
  virtual void clear() = 0;

  /*!
    @brief Reserve the space to contain zones.
    @note If the container does not need to reserve its space, this function does nothing.

    @param [in] n The size of the reserved space of the container.
  */
  virtual void reserve(std::size_t n) = 0;
};
