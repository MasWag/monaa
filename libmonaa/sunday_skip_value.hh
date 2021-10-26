#pragma once

#include <array>
#include <climits>
#include <iostream>

#include "ta2za.hh"
#include "timed_automaton.hh"
#include "zone_automaton.hh"

/*!
 * @brief The skip value function based on Sunday's quick search
 *
 * @note We construct the table maintaining all the skip values in the constructor for the efficiency at runtime.
 * @sa https://doi.org/10.1145/79173.79184
 */
class SundaySkipValue {
private:
  //! @brief Minimum length of the recognized language
  int m;
  std::array<unsigned int, CHAR_MAX> delta{};
  //! @brief The set of the m-th characters of the untimed projection of the recognized language
  std::unordered_set<char> endChars;

public:
  explicit SundaySkipValue(const TimedAutomaton &TA) {
    ZoneAutomaton ZA;
    ta2za(TA, ZA);
    ZA.removeDeadStates();

    std::vector<std::unordered_set<char>> charSet;
    bool accepted = false;
    m = 0;
    std::vector<std::shared_ptr<ZAState>> CStates = ZA.initialStates;
    while (!accepted) {
      if (CStates.empty()) {
        std::cerr << "monaa: empty pattern" << std::endl;
        exit(10);
      }
      std::vector<std::shared_ptr<ZAState>> NStates;
      m++;
      charSet.resize(m);
      for (const auto& zaState : CStates) {
        std::unordered_set<std::shared_ptr<ZAState>> closure;
        closure.insert(zaState);
        epsilonClosure(closure);
        for (const auto& state : closure) {
          for (char c = 1; c < CHAR_MAX; c++) {
            for (const auto& nextState : state->next[c]) {
              auto sharedNext = nextState.lock();
              if (!sharedNext) {
                continue;
              }
              accepted = accepted || sharedNext->isMatch;
              NStates.push_back(sharedNext);
              charSet[m - 1].insert(c);
            }
          }
        }
      }
      CStates = NStates;
    }

    // Construct the table of Sunday's skip value
    delta.fill(m + 1);
    for (int i = 0; i <= m - 1; i++) {
      for (char s : charSet[i]) {
        delta[s] = m - i;
      }
    }
    endChars = charSet[m - 1];
  }
  unsigned int at(std::size_t n) const { return delta.at(n); }
  unsigned int operator[](std::size_t n) const { return delta[n]; }
  //! @brief Minimum length of the recognized language
  int getM() const { return m; }
  void getEndChars(std::unordered_set<char> &endCharsHolder) const {
    endCharsHolder = this->endChars;
  }
};
