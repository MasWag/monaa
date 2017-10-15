#pragma once

#include <iostream>
#include <chrono>
#include <cmath>
#include <climits>
#include <unordered_set>
#include <boost/variant.hpp>

#include "intermediate_zone.hh"
#include "ta2za.hh"
#include "intersection.hh"
#include "word_container.hh"
#include "sunday_skip_value.hh"
#include "kmp_skip_value.hh"
#include "ans_vec.hh"

#include "utils.hh"

// Internal state of BFS
struct InternalState {
  std::shared_ptr<TAState> s;
  // C -> (R or Z)
  std::vector<boost::variant<double, ClockVariables>> resetTime;
  IntermediateZone z;
  InternalState (std::shared_ptr<TAState> s, std::vector<boost::variant<double, ClockVariables>> resetTime, IntermediateZone z) :s(s), resetTime(resetTime), z(z) {}
  InternalState (std::size_t numOfVar, std::shared_ptr<TAState> s, std::pair<double,bool> upperBound, std::pair<double,bool> lowerBound = {0, true}) : s(s), z(Zone::zero(numOfVar + 3), 1) {
    static std::vector<boost::variant<double, ClockVariables>> zeroResetTime(numOfVar);
    std::fill(zeroResetTime.begin(), zeroResetTime.end(), ClockVariables(0));
    resetTime = zeroResetTime;
    lowerBound.first = -lowerBound.first;
    z.value(1, 0) = upperBound;
    z.value(0, 1) = lowerBound;
  }
};

/*!
  @brief Boyer-Moore type algorithm for timed pattern matching
 */
template <class InputContainer, class OutputContainer>
void timedFranekJenningsSmyth (WordContainer<InputContainer> word,
                               TimedAutomaton A,
                               AnsContainer<OutputContainer> &ans)
{
  // Sunday's Skip value
  // Char -> Skip Value
  const SundaySkipValue delta = SundaySkipValue(A);
  const int m = delta.getM();
  std::unordered_set<Alphabet> endChars;
  delta.getEndChars(endChars);

  // KMP-Type Skip value
  //! A.State -> SkipValue
  const KMPSkipValue beta(A, m);

  // main computation
  {
    auto start = std::chrono::system_clock::now();  

    std::size_t i = 0;
    std::array<std::vector<IntermediateZone>, CHAR_MAX> init;
    std::vector<InternalState> CStates;
    std::vector<InternalState> LastStates;
  
    // When there can be immidiate accepting
    // @todo This optimization is not yet when we have epsilon transitions
#if 0
    if (m == 1) {
      for (const auto initState: A.initialStates) {
        for (char c = 0; c < CHAR_MAX; c++) {
          for (const auto &edge: initState->next[c]) {
            if (edge.target.lock()->isMatch) {
              // solve delta
              IntermediateZone zone = Zone::zero(2);
              for (const auto &constraint: edge.guard) {
                zone.tighten(1, constraint);
              }
              if (zone.isSatisfiable()) {
                init[c].push_back(std::move(zone));
              }
            }
          }
        }
      }
    }
#endif

    ans.clear();
    std::size_t j;
    while (i + m <= word.size()) {
      bool tooLarge = false;
      // Sunday Shift
      try {
#if 0
        if (m == 1 && init[word[i].first].size() > 0) {
          // When there can be immidiate accepting
          // @todo This optimization is not yet
          ans.reserve(ans.size() + init[word[i].first].size());
          if (i <= 0) {
            for (auto zone: init[word[i].first]) {
              Zone ansZone;
              zone.value.col(0).fill({word[i].second, false});
              if (zone.isSatisfiable()) {
                zone.toAns(ansZone);
                ans.push_back(std::move(ansZone));
              }
            }
          } else {
            for (auto zone: init[word[i].first]) {
              Zone ansZone;
              zone.value.col(0).fill({word[i].second, false});
              zone.value.row(0).fill({-word[i-1].second, true});
              if (zone.isSatisfiable()) {
                zone.toAns(ansZone);
                ans.push_back(std::move(ansZone));
              }
            }
          }
        } else
#endif
          if (m > 1) {
            while (endChars.find(word[i + m - 1].first) == endChars.end() ) {                    if (i + m >= word.size()) {
                tooLarge = true;
                break;
              }
              // increment i
              i += delta[ word[i + m].first ];
              word.setFront(i - 1);
              if (i + m > word.size())  {
                tooLarge = true;
                break;
              }
            }
          }
      } catch (const std::out_of_range& e) {
        // if we hit EOF, i is too large
        tooLarge = true;
      }

      if (tooLarge) break;

      // KMP like Matching
      CStates.clear ();
      CStates.reserve(A.initialStates.size());
      try {
        if (i <= 0) {
          for (const auto& s: A.initialStates) {
            CStates.push_back({A.clockSize(), s, {word[i].second, false}});
          }
        } else {
          for (const auto& s: A.initialStates) {
            CStates.push_back({A.clockSize(), s, {word[i].second, false}, {word[i-1].second, true}});
          }
        }
      } catch (const std::out_of_range&) {
        break;
      }
      j = i;
      try {
        while (!CStates.empty () && j < word.size ()) {
          // try unobservable transitions
          std::vector<InternalState> CurrEpsilonConf = CStates;
          while (!CurrEpsilonConf.empty()) {
            std::vector<InternalState> PrevEpsilonConf = std::move(CurrEpsilonConf);
            CurrEpsilonConf.clear();
            for (const auto &econfig: PrevEpsilonConf) {
              for (const auto &edge: econfig.s->next[0]) {
                auto target = edge.target.lock();
                if (!target) {
                  continue;
                }
                IntermediateZone tmpZ = econfig.z;
                ClockVariables newClock;
                if (j > 0) {
                  newClock = tmpZ.alloc({word[j].second, true}, {word[j-1].second, false});
                } else {
                  newClock = tmpZ.alloc({word[j].second, true});
                }
                tmpZ.tighten(edge.guard, econfig.resetTime);
                if (tmpZ.isSatisfiable()) {
                  auto tmpResetTime = econfig.resetTime;
                  for (ClockVariables x: edge.resetVars) {
                    tmpResetTime[x] = newClock;
                  }
                  tmpZ.update(tmpResetTime);
                  CurrEpsilonConf.emplace_back(target, tmpResetTime, tmpZ);
                }
              }
            }
            CStates.insert(CStates.end(), CurrEpsilonConf.begin(), CurrEpsilonConf.end());
          }



          const Alphabet c = word[j].first;
          const double t = word[j].second;
      
          // try to go to an accepting state
          for (const auto &config : CStates) {
            const std::shared_ptr<TAState> s = config.s;
            for (const auto &edge : s->next[c]) {
              auto target = edge.target.lock();
              if (!target || !target->isMatch) {
                continue;
              }
              IntermediateZone tmpZ = config.z;
              if (j > 0) {
                tmpZ.alloc({word[j].second, false}, {word[j-1].second, true});
              } else {
                tmpZ.alloc({word[j].second, false});
              }
              tmpZ.tighten(edge.guard, config.resetTime);
              if (tmpZ.isSatisfiable()) {
                Zone ansZone;
                tmpZ.toAns(ansZone);
                ans.push_back(ansZone);
              }
            }
          }

          // try observable transitios (usual)
          LastStates = std::move(CStates);
          for (const auto &config : LastStates) {
            const std::shared_ptr<TAState> s = config.s;
            for (const auto &edge : s->next[c]) {
              auto target = edge.target.lock();
              if (!target) {
                continue;
              }
              IntermediateZone tmpZ = config.z;
              tmpZ.tighten(edge.guard, config.resetTime, t);
              if (tmpZ.isSatisfiable()) {
                auto tmpResetTime = config.resetTime;
                for (ClockVariables x: edge.resetVars) {
                  tmpResetTime[x] = t;
                }
                tmpZ.update(tmpResetTime);
                CStates.emplace_back(target, tmpResetTime, tmpZ);
              }
            }
          }
          j++;
        }
      } catch (const std::out_of_range&) {
        LastStates = std::move(CStates);
      }
      if (j >= word.size ()) {
        LastStates = std::move(CStates);
      }
      // KMP like skip value
      int greatestN = 1;
      for (const InternalState& istate: LastStates) {
        greatestN = std::max(beta[istate.s],greatestN);
      }
      // increment i
      i += greatestN;
      word.setFront(i - 1);
    }

    auto end = std::chrono::system_clock::now();
    auto dur = end - start;
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    std::cout << "main computation: " << nsec / 1000000.0 << " ms" << std::endl;
  }
}
