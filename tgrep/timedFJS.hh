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
  InternalState (const std::shared_ptr<TAState> &s, const std::vector<boost::variant<double, ClockVariables>> &resetTime, const IntermediateZone &z) :s(std::move(s)), resetTime(std::move(resetTime)), z(std::move(z)) {}
  InternalState (const std::size_t numOfVar, const std::shared_ptr<TAState> &s, const Interval &interval) : s(std::move(s)), z(std::move(interval)) {
    static const std::vector<boost::variant<double, ClockVariables>> zeroResetTime(numOfVar, ClockVariables(1));
    // Every clock variables are reset at t1 ( = t)
    resetTime = zeroResetTime;
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
              if (zone.isSatisfiableCanonized()) {
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
    while (word.fetch(i + m - 1)) {
      bool tooLarge = false;
      // Sunday Shift
#if 0
      if (m == 1 && init[word[i].first].size() > 0) {
        // When there can be immidiate accepting
        // @todo This optimization is not yet
        ans.reserve(ans.size() + init[word[i].first].size());
        if (i <= 0) {
          for (auto zone: init[word[i].first]) {
            Zone ansZone;
            zone.value.col(0).fill({word[i].second, false});
            if (zone.isSatisfiableCanonized()) {
              zone.toAns(ansZone);
              ans.push_back(std::move(ansZone));
            }
          }
        } else {
          for (auto zone: init[word[i].first]) {
            Zone ansZone;
            zone.value.col(0).fill({word[i].second, false});
            zone.value.row(0).fill({-word[i-1].second, true});
            if (zone.isSatisfiableCanonized()) {
              zone.toAns(ansZone);
              ans.push_back(std::move(ansZone));
            }
          }
        }
      } else
#endif
        if (m > 1 && word.fetch(i + m - 1)) {
          while (endChars.find(word[i + m - 1].first) == endChars.end() ) {
            if (!word.fetch(i + m)) {
              tooLarge = true;
              break;
            }
            // increment i
            i += delta[ word[i + m].first ];
            word.setFront(i - 1);
            if (!word.fetch(i + m - 1))  {
              tooLarge = true;
              break;
            }
          }
        }

      if (tooLarge) break;

      // KMP like Matching
      CStates.clear ();
      if (word.fetch(i)) {
        InternalState istate = {A.clockSize(),
                                std::make_shared<TAState>(),
                                Interval{((i <= 0) ? Bounds{0, true} :
                                          Bounds{word[i-1].second, true}),
                                         {word[i].second, false}}};
        CStates.resize(A.initialStates.size(), istate);
        for (std::size_t k = 0; k < A.initialStates.size(); k++) {
          CStates[k].s = A.initialStates[k];
        }
      } else {
        break;
      }
      j = i;
      while (!CStates.empty () && word.fetch(j)) {
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
              if (tmpZ.isSatisfiableCanonized()) {
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
            if (tmpZ.isSatisfiableCanonized()) {
              Zone ansZone;
              tmpZ.toAns(ansZone);
              ans.push_back(std::move(ansZone));
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
            if (tmpZ.isSatisfiableCanonized()) {
              auto tmpResetTime = config.resetTime;
              for (ClockVariables x: edge.resetVars) {
                tmpResetTime[x] = t;
              }
              tmpZ.update(tmpResetTime);
              CStates.emplace_back(target, std::move(tmpResetTime), std::move(tmpZ));
            }
          }
        }
        j++;
      }
      if (!word.fetch(j)) {
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
