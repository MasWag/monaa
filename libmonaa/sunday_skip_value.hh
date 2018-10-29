#pragma once

#include <iostream>
#include <array>
#include <climits>

#include "timed_automaton.hh"
#include "zone_automaton.hh"
#include "ta2za.hh"

template<class TimedAutomaton, class Zone>
class SundaySkipValueTemplate {
  using ZoneAutomaton = AbstractZoneAutomaton<typename TimedAutomaton::State, Zone>;
private:
  int m;
  std::array<unsigned int, CHAR_MAX> delta;
  std::unordered_set<char> endChars;
public:
  SundaySkipValueTemplate(TimedAutomaton TA) {
    ZoneAutomaton ZA;
    ta2za(TA,ZA);
    ZA.removeDeadStates();

    std::vector<std::unordered_set<char>> charSet;
    bool accepted = false;
    m = 0;
    std::vector<std::shared_ptr<typename ZoneAutomaton::State>> CStates = ZA.initialStates;
    while (!accepted) {
      if (CStates.empty()) {
        std::cerr << "monaa: empty pattern" << std::endl;
        exit(10);
      }
      std::vector<std::shared_ptr<typename ZoneAutomaton::State>> NStates;
      m++;
      charSet.resize(m);
      for (auto zstate: CStates) {
        std::unordered_set<std::shared_ptr<typename ZoneAutomaton::State>> closure;
        closure.insert(zstate);
        epsilonClosure(closure);
        for (auto state: closure) {
          for (char c = 1; c < CHAR_MAX; c++) {
            for (auto nextState: state->next[c]) {
              auto sharedNext = nextState.lock();
              if (!sharedNext) {
                continue;
              }
              accepted = accepted || sharedNext->isMatch;
              NStates.push_back(sharedNext);
              charSet[m-1].insert(c);
            }
          }
        }
      }
      CStates = NStates;
    }

    // Calc Sunday's Skip Value
    delta.fill(m+1);
    for (int i = 0; i <= m - 1; i++) {
      for (char s: charSet[i]) {
        delta[s] = m - i;
      }
    }
    endChars = charSet[m-1];
  }
  unsigned int at(std::size_t n) const {
    return delta.at(n);
  }
  unsigned int operator[](std::size_t n) const {
    return delta[n];
  }
  //! @brief Minumum length of the language
  int getM() const {
    return m;
  }
  void getEndChars (std::unordered_set<char> &endChars) const {
    endChars = this->endChars;
  }
};

using SundaySkipValue = SundaySkipValueTemplate<TimedAutomaton, Zone>;
