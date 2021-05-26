#pragma once
#include <iomanip>
#include <iostream>
#include <vector>

#include "zone.hh"

/*!
  @brief Container class for the output zones.

  @note This class just defines the interface of the container. The actual
  definition must be done in the class Container passed by template argument. An
  example is @link PrintContainer @endlink.
  @note This class does not require to contain the zones but just have the
  interface to @link push_back @endlink.
*/
template <class Container> class AnsContainer {
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
  std::size_t size() const { return vec.size(); }
  /*!
    @brief Append a zone to the container.

    @param [in] in A zone to be appended.
  */
  void push_back(typename Container::value_type in) { vec.push_back(in); }
  /*!
    @brief Remove all the contained zones.
  */
  void clear() { vec.clear(); }
  /*!
    @brief Reserve the space to contain zones.
    @note If the container does not need to reserve its space, this function
    does nothing.

    @param [in] n The size of the reserved space of the container.
  */
  void reserve(std::size_t n) { vec.reserve(n); }
};

template <class T> class AnsVec : public AnsContainer<std::vector<T>> {
public:
  typename std::vector<T>::iterator begin() {
    return AnsContainer<std::vector<T>>::vec.begin();
  }
  typename std::vector<T>::iterator end() {
    return AnsContainer<std::vector<T>>::vec.end();
  }
};

template <class T> class IntContainer {
private:
  std::size_t count = 0;

public:
  std::size_t size() const { return count; }
  void push_back(T) { count++; }
  void clear() { count = 0; }
  void reserve(std::size_t) {}
  using value_type = T;
};

template <class T> using AnsNum = AnsContainer<IntContainer<T>>;

/*!
  @brief A pseudo-container class to print the given zone to stdout. This is
  given to @link AnsContainer @endlink.

  @note This class does not contain any zones, but just print to stdout and
  counts the number.
 */
class PrintContainer {
private:
  std::size_t count = 0;
  bool isQuiet = false;

public:
  /*!
    @brief Constructor

    @param [in] isQuiet If isQuiet is true, this class does not print anything.
  */
  PrintContainer(bool isQuiet) : isQuiet(isQuiet) {}
  //! @brief Returns the count of output zones.
  std::size_t size() const { return count; }
  void push_back(const Zone &ans) {
    count++;
    if (!isQuiet) {
      printf("%10lf %8s t %s %10lf\n", -ans.value(0, 1).first,
             (ans.value(0, 1).second ? "<=" : "<"),
             (ans.value(1, 0).second ? "<=" : "<"), ans.value(1, 0).first);
      printf("%10lf %8s t' %s %10lf\n", -ans.value(0, 2).first,
             (ans.value(0, 2).second ? "<=" : "<"),
             (ans.value(2, 0).second ? "<=" : "<"), ans.value(2, 0).first);
      printf("%10lf %8s t' - t %s %10lf\n", -ans.value(1, 2).first,
             (ans.value(1, 2).second ? "<=" : "<"),
             (ans.value(2, 1).second ? "<=" : "<"), ans.value(2, 1).first);
      puts("=============================");
    }
  }
  //! @brief Resets the count of output zones.
  void clear() { count = 0; }
  //! @brief Does nothing.
  void reserve(std::size_t) {}
  using value_type = Zone;
};

using AnsPrinter = AnsContainer<PrintContainer>;
