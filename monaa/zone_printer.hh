#pragma once

#include <cstdio>
#include "zone_container.hh"

/*!
  @brief A pseudo-container class to print the given zone to stdout. This is given to @link AnsContainer @endlink.

  @note This class does not contain any zones, but just print to stdout and counts the number.
 */
class ZonePrinter : public AbstractAnsZoneContainer
{
private:
  std::size_t count = 0;
  bool isQuiet = false;
public:
  /*!
    @brief Constructor
    
    @param [in] isQuiet If isQuiet is true, this class does not print anything.
  */
  ZonePrinter(bool isQuiet) : isQuiet(isQuiet) {}
  //! @brief Returns the count of output zones.
  std::size_t size() const {
    return count;
  }
  void push_back(const Zone &ans) {
    count++;
    if (!isQuiet) {
      printf("%10lf %8s t %s %10lf\n", -ans.value(0, 1).first,
             (ans.value(0, 1).second ? "<=" : "<"),
             (ans.value(1, 0).second ? "<=" : "<"),
             ans.value(1, 0).first);
      printf("%10lf %8s t' %s %10lf\n", -ans.value(0, 2).first,
             (ans.value(0, 2).second ? "<=" : "<"),
             (ans.value(2, 0).second ? "<=" : "<"),
             ans.value(2, 0).first);
      printf("%10lf %8s t' - t %s %10lf\n", -ans.value(1, 2).first,
             (ans.value(1, 2).second ? "<=" : "<"),
             (ans.value(2, 1).second ? "<=" : "<"),
             ans.value(2, 1).first);
      puts("=============================");
    }
  }
  //! @brief Resets the count of output zones.
  void clear() {
    count = 0;
  }
  //! @brief Does nothing.
  void reserve(std::size_t) {}
};
