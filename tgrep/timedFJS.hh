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
  const TAState *s;
  // C -> (R or Z)
  std::vector<boost::variant<double, ClockVariables>> resetTime;
  IntermediateZone z;
  InternalState (const TAState *s, const std::vector<boost::variant<double, ClockVariables>> &resetTime, const IntermediateZone &z) :s(std::move(s)), resetTime(std::move(resetTime)), z(std::move(z)) {}
  InternalState (const std::size_t numOfVar, const TAState *s, const Interval &interval) : s(std::move(s)), z(std::move(interval)) {
    static const std::vector<boost::variant<double, ClockVariables>> zeroResetTime(numOfVar, ClockVariables(1));
    // Every clock variables are reset at t1 ( = t)
    resetTime = zeroResetTime;
  }
};
struct IntervalInternalState {
  using Variables = char;
  const TAState *s;
  std::vector<double> resetTime;
  std::pair<double,bool> upperConstraint;
  std::pair<double,bool> lowerConstraint;
};

//! @brief Check if the given constraint is non empty.
inline bool isValidConstraint (const std::pair<double,bool>& upperConstraint,
                               const std::pair<double,bool>& lowerConstraint)
{
  return upperConstraint.first > lowerConstraint.first ||
    (upperConstraint.first == lowerConstraint.first &&
     upperConstraint.second &&
     lowerConstraint.second);
}

/*!
  @brief update the assuming interval to satisfy the constraint
 */
inline void updateConstraint(std::pair<double,bool>& upperConstraint,
                             std::pair<double,bool>& lowerConstraint,
                             const Constraint &delta,
                             const double comparedValue)
{
  switch (delta.odr) {
  case Constraint::Order::gt:
    if (lowerConstraint.first < comparedValue) {
      lowerConstraint.first = comparedValue;
      lowerConstraint.second = 0;
    } else if (lowerConstraint.first == comparedValue) {
      lowerConstraint.second = 0;
    }
    break;
  case Constraint::Order::ge:
    if (lowerConstraint.first < comparedValue) {
      lowerConstraint.first = comparedValue;
      lowerConstraint.second = 1;
    }
    break;
  case Constraint::Order::lt:
    if (upperConstraint.first > comparedValue) {
      upperConstraint.first = comparedValue;
      upperConstraint.second = 0;
    } else if (upperConstraint.first == comparedValue) {
      upperConstraint.second = 0;
    }
    break;
  case Constraint::Order::le:
    if (upperConstraint.first > comparedValue) {
      upperConstraint.first = comparedValue;
      upperConstraint.second = 1;
    }
    break;
  }
}

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
  if (std::all_of(A.states.begin(), A.states.end(), [](std::shared_ptr<TAState> s) {
        return s->next.find(0) == s->next.end();
      })) {
    // When there is no epsilon transition
    auto start = std::chrono::system_clock::now();

    std::size_t i = 0;
    std::vector<std::pair<std::pair<double,bool>,std::pair<double,bool> > > init;
    std::vector<IntervalInternalState> CStates;
    std::vector<IntervalInternalState> LastStates;
    const std::pair<double,bool> upperMaxConstraint = {INFINITY,false};
    const std::pair<double,bool> lowerMinConstraint = {0,true};
  
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
      CStates.reserve(A.initialStates.size());
      std::vector<double> zeroResetTime(A.clockSize(), 0);
      if (word.fetch(i)) {
        if (i <= 0) {
          for (const auto& s: A.initialStates) {
            CStates.push_back ({s.get(), zeroResetTime,{word[i].second,false},{0,true}});
          }
        } else {
          for (const auto& s: A.initialStates) {
            CStates.push_back ({s.get(), zeroResetTime,{word[i].second,false},{word[i-1].second,true}});
          }
        }
      } else {
        break;
      }
      j = i;
      while (!CStates.empty () && word.fetch(j)) {
        const Alphabet c = word[j].first;
        const double t = word[j].second;
      
        // try to go to an accepting state
        for (const auto &config : CStates) {
          const TAState *s = config.s;
          auto it = s->next.find(c);
          if (it == s->next.end()) {
            continue;
          }
          for (const auto &edge : it->second) {
            auto target = edge.target;
            if (!target || !target->isMatch) {
              continue;
            }
            std::pair<double,bool> upperBeginConstraint = config.upperConstraint;
            std::pair<double,bool> lowerBeginConstraint = config.lowerConstraint;
            std::pair<double,bool> upperEndConstraint = upperMaxConstraint;
            std::pair<double,bool> lowerEndConstraint = {word[j].second,false};
                
            if (word.size() <= j || word.fetch(j+1)) {
              upperEndConstraint = {word[j+1].second,true};
            }

            std::pair<double,bool> upperDeltaConstraint = upperEndConstraint - lowerBeginConstraint;
            std::pair<double,bool> lowerDeltaConstraint = std::max(lowerEndConstraint - upperBeginConstraint, Bounds{0, true});

            auto tmpResetTime = config.resetTime;
            // solve delta
            for (const auto& delta: edge.guard) {
              if (tmpResetTime[delta.x]) {
                const double delta_t = delta.c + tmpResetTime[delta.x];
                updateConstraint (upperEndConstraint,lowerEndConstraint,delta,delta_t);
                upperDeltaConstraint = std::min(upperDeltaConstraint, upperEndConstraint - lowerBeginConstraint);
                lowerDeltaConstraint = std::max(lowerDeltaConstraint, lowerEndConstraint - upperBeginConstraint);
              } else {
                updateConstraint (upperDeltaConstraint,lowerDeltaConstraint,delta,delta.c);
                upperEndConstraint = std::min(upperEndConstraint, upperDeltaConstraint + upperBeginConstraint);
                lowerEndConstraint = std::max(lowerEndConstraint, lowerDeltaConstraint + lowerBeginConstraint);
              }
            }

            if (!isValidConstraint (upperBeginConstraint,lowerBeginConstraint) ||
                !isValidConstraint (upperEndConstraint,lowerEndConstraint) ||
                !isValidConstraint (upperDeltaConstraint,lowerDeltaConstraint)) {
              continue;
            }

            Zone ansZone = Zone::zero(3);
            ansZone.value(0, 1) = std::move(lowerBeginConstraint);
            ansZone.value(1, 0) = std::move(upperBeginConstraint);
            ansZone.value(0, 2) = std::move(lowerEndConstraint);
            ansZone.value(2, 0) = std::move(upperEndConstraint);
            ansZone.value(1, 2) = std::move(lowerDeltaConstraint);
            ansZone.value(2, 1) = std::move(upperDeltaConstraint);

            ansZone.value(0, 1).first *= -1;
            ansZone.value(0, 2).first *= -1;
            ansZone.value(1, 2).first *= -1;

            ans.push_back(std::move(ansZone));
          }
        }

        // try observable transitios (usual)
        LastStates = std::move(CStates);
        for (const auto &config : LastStates) {
          const TAState *s = config.s;
          auto it = s->next.find(c);
          if (it == s->next.end()) {
            continue;
          }
          for (const auto &edge : it->second) {
            auto target = edge.target;
            if (!target) {
              continue;
            }


            std::pair<double,bool> upperBeginConstraint = config.upperConstraint;
            std::pair<double,bool> lowerBeginConstraint = config.lowerConstraint;
            bool transitable = true;

            for (const auto& delta: edge.guard) {
              if (config.resetTime[delta.x]) {
                if (!delta.satisfy(t - config.resetTime[delta.x])) {
                  transitable = false;
                  break;
                }
              } else {
                const double delta_t = t - delta.c;
                switch (delta.odr) {
                case Constraint::Order::lt:
                case Constraint::Order::le:
                  if (lowerBeginConstraint.first < delta_t) {
                    lowerBeginConstraint.first = delta_t;
                    lowerBeginConstraint.second = 
                      delta.odr == Constraint::Order::le;
                  } else if (lowerBeginConstraint.first == delta_t) {
                    lowerBeginConstraint.second = 
                      lowerBeginConstraint.second && delta.odr == Constraint::Order::le;
                  }
                  break;
                case Constraint::Order::gt:
                case Constraint::Order::ge:
                  if (upperBeginConstraint.first > delta_t) {
                    upperBeginConstraint.first = delta_t;
                    upperBeginConstraint.second = 
                      delta.odr == Constraint::Order::ge;
                  } else if (upperBeginConstraint.first == delta_t) {
                    upperBeginConstraint.second = 
                      upperBeginConstraint.second && delta.odr == Constraint::Order::ge;
                  }
                  break;
                }
              }
            }

            if (!transitable || !isValidConstraint (upperBeginConstraint,lowerBeginConstraint)) {
              continue;
            }
            
            auto tmpResetTime = config.resetTime;
            for (auto i : edge.resetVars) {
              tmpResetTime[i] = t;
            }
            
            CStates.push_back ({edge.target,tmpResetTime,upperBeginConstraint,lowerBeginConstraint});
          }
        }
        j++;
      }
      if (!word.fetch(j)) {
        LastStates = std::move(CStates);
      }
      // KMP like skip value
      int greatestN = 1;
      for (const IntervalInternalState& istate: LastStates) {
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
  } else {
    // When there are some epsilon transitions
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
                                nullptr,
                                Interval{((i <= 0) ? Bounds{0, true} :
                                          Bounds{word[i-1].second, true}),
                                         {word[i].second, false}}};
        CStates.resize(A.initialStates.size(), istate);
        for (std::size_t k = 0; k < A.initialStates.size(); k++) {
          CStates[k].s = A.initialStates[k].get();
        }
      } else {
        break;
      }
      j = i;
      while (!CStates.empty () && word.fetch(j)) {
        // try unobservable transitions

        // Correct the states have epsilon transitions
        std::vector<InternalState> CurrEpsilonConf;
        CurrEpsilonConf.reserve(CStates.size());
        for (auto &istate: CStates) {
          auto it = istate.s->next.find(0);
          if (it != istate.s->next.end()) {
            CurrEpsilonConf.push_back(istate);
          }
        }
        while (!CurrEpsilonConf.empty()) {
          std::vector<InternalState> PrevEpsilonConf = std::move(CurrEpsilonConf);
          CurrEpsilonConf.clear();
          for (const auto &econfig: PrevEpsilonConf) {
            auto it = econfig.s->next.find(0);
            if (it == econfig.s->next.end()) {
              continue;
            }
            for (const auto &edge: it->second) {
              auto target = edge.target;
              if (!target) {
                continue;
              }
              IntermediateZone tmpZ = econfig.z;
              ClockVariables newClock;
              tmpZ.alloc({word[j].second, true},
                         ((j > 0) ? Bounds{-word[j-1].second, false} : Bounds{0, true}));
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
          const TAState *s = config.s;
          auto it = s->next.find(c);
          if (it == s->next.end()) {
            continue;
          }
          for (const auto &edge : it->second) {
            auto target = edge.target;
            if (!target || !target->isMatch) {
              continue;
            }
            IntermediateZone tmpZ = config.z;
            tmpZ.alloc({word[j].second, false},
                       ((j > 0) ? Bounds{-word[j-1].second, true} : Bounds{0, true}));
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
          const TAState *s = config.s;
          auto it = s->next.find(c);
          if (it == s->next.end()) {
            continue;
          }
          for (const auto &edge : it->second) {
            auto target = edge.target;
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
