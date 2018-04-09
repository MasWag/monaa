#pragma once

#include <vector>
#include <iostream>
#include <iomanip>

#include "zone.hh"
#include "zone_container.hh"

/*!
  @brief Container class for the output zones.

  @note This class just defines the interface of the container. The actual definition must be done in the class Container passed by template argument. An example is @link PrintContainer @endlink.
  @note This class does not require to contain the zones but just have the interface to @link push_back @endlink.
*/
template<class Container>
class AnsContainer : public AbstractAnsZoneContainer
{
protected:
  //! @brief The actual container of the zones.
  Container vec;
public:
  //! @brief Constructor
  AnsContainer(const Container vec) : vec(vec) {}
  //! @brief Constructor
  AnsContainer() : vec() {}
  /*!
    @brief Returns the number of the contained zones.

    @return The number of the contained zones.
  */
  std::size_t size() const {
    return vec.size();
  }
  /*!
    @brief Append a zone to the container.

    @param [in] in A zone to be appended.
  */
  void push_back(const typename Container::value_type &in) {
    vec.push_back(in);
  }
  /*!
    @brief Remove all the contained zones.
  */
  void clear() {
    vec.clear();
  }
  /*!
    @brief Reserve the space to contain zones.
    @note If the container does not need to reserve its space, this function does nothing.

    @param [in] n The size of the reserved space of the container.
  */
  void reserve(std::size_t n) {
    vec.reserve(n);
  }
};

template<class T>
class AnsVec : public AnsContainer<std::vector<T>>
{
public:
  typename std::vector<T>::iterator begin() {
    return AnsContainer<std::vector<T>>::vec.begin();
  }
  typename std::vector<T>::iterator end() {
    return AnsContainer<std::vector<T>>::vec.end();
  }
};


template<class T>
class IntContainer 
{
private:
  std::size_t count = 0;
public:
  std::size_t size() const {
    return count;
  }
  void push_back(T) {
    count++;
  }
  void clear() {
    count = 0;
  }
  void reserve(std::size_t) {}
  using value_type = T;
};

template<class T>
using AnsNum = AnsContainer<IntContainer<T>>;
