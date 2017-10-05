#pragma once

#include <iostream>
#include <chrono>
#include <cmath>
#include <climits>
#include <unordered_set>
// #include "automaton_operations.hh"
// #include "calcLs.hh"
#include "ta2za.hh"
// #include "partial_run_checker.hh"
// #include "intersection.hh"
#include "word_container.hh"
#include "sunday_skip_value.hh"
#include "ans_vec.hh"

#include "utils.hh"

struct ansZone {
  std::pair<double,bool> upperBeginConstraint;
  std::pair<double,bool> lowerBeginConstraint;
  std::pair<double,bool> upperEndConstraint;
  std::pair<double,bool> lowerEndConstraint;
  std::pair<double,bool> upperDeltaConstraint;
  std::pair<double,bool> lowerDeltaConstraint;  
  inline bool operator == (const ansZone z) const {
    return upperBeginConstraint == z.upperBeginConstraint &&
      lowerBeginConstraint == z.lowerBeginConstraint &&
      upperEndConstraint == z.upperEndConstraint &&
      lowerEndConstraint == z.lowerEndConstraint &&
      upperDeltaConstraint == z.upperDeltaConstraint &&
      lowerDeltaConstraint == z.lowerDeltaConstraint;
  }
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
                               AnsContainer<OutputContainer> &ans,
                               int &hashCalcCount)
{
  // Internal state of BFS
  struct InternalState{
    using Variables = char;
    std::shared_ptr<TAState> s;
    //! @todo fix
    std::vector<double> resetTime; // C -> (R or Z)
    //! @todo this should be a zone
    std::pair<double,bool> upperConstraint;
    std::pair<double,bool> lowerConstraint;
  };

#ifdef SKIP_PREPROCESSING
  return;
#endif

  //! ZA = R^r(A)
  ZoneAutomaton ZA;
  ZA.states.clear();
  // KMP-Type Skip value
  //! A.State -> SkipValue
  std::vector<int> beta;
  std::unordered_set<Alphabet> endChars;
  // Sunday's Skip value
  SundaySkipValue delta = SundaySkipValue(A);
  const int m = delta.getM();
  //
  int tSizeP;
  // precomputation
  {
    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    auto dur = end - start;
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();

#if 0 // TOOD: around here will be modified a lot
    // make R(A)
    printDuration(ta2za(A,ZA), "ta2za: ");
    //! L = L'    
    std::vector< std::pair<std::vector<ZAState>, std::string> > L;
    std::vector< std::vector<ZAState> > L_first;
    std::vector<std::string> L_second;
    //! Ls = L'_s
    std::vector< std::vector<ZAState> > Ls;
    printDuration(calcL (ZA,L,m, endChars), "calcL: ");
    // sort uniq
    std::transform(L.begin(), L.end(), std::back_inserter(L_first),
                   [](std::pair<std::vector<ZAState>, std::string> x) { return x.first; });
    std::sort (L_first.begin(), L_first.end ());
    L_first.erase( std::unique(L_first.begin(), L_first.end()), L_first.end() );

    std::transform(L.begin(), L.end(), std::back_inserter(L_second),
                   [](std::pair<std::vector<ZAState>, std::string> x) { return x.second; });
    std::sort (L_second.begin(), L_second.end ());
    L_second.erase( std::unique(L_second.begin(), L_second.end()), L_second.end() );
    beta.resize (A.states.size(),0);
    tSizeP = word.size() - m + 1;

    // Calc Sunday's Skip value
    delta.fill (m);
    for (int i = 0; i < m-1; i++) {
      for (auto s: L) {
        delta[ s.second[i] ] = m - i - 1;
      }
    }
#endif

    ZoneAutomaton ZA2;
    ZA2.abstractedStates.clear();
    // make R'(A2)
    //! A2 = A x A (product)
    TimedAutomaton A2;
    TimedAutomaton A0 = A;
    TimedAutomaton As = A;
    intersectionTA (A0,As,A2);

    for (TAState s = 0; s < A.edges.size();s++) {
      hashCalcCount++;
      int ms;
      printDuration(calcLs (ZA,Ls,s,m,ms), "calcL" << s << ": ");
      int n;
      start = std::chrono::system_clock::now();
      for (n = 1;n < ms-1; n++) {
        const auto rend = std::min (ms - n,m);
        const auto rsend = std::min (m + n,ms);
        // examine each r,rs
        if (rend == 0 || std::any_of(L_first.begin(), L_first.end(),[&](const std::vector<ZAState> &r) {
              return std::any_of(Ls.begin(), Ls.end(),[&](const std::vector<ZAState>&rs){
                  A2.initialStates = std::vector<TAState>{ZA.abstractedStates[r.front()].first + ZA.abstractedStates[rs[n]].first * TAState(A.stateSize())};
                  Zone initialZone;
                  initialZone.value.resize(NVar * 2 + 1, NVar * 2 + 1);
                  initialZone.value.fill(Bounds(std::numeric_limits<double>::infinity(),false));
                  initialZone.value.block(0,0,NVar+1,NVar+1) = ZA.abstractedStates[r.front()].second.value;
                  initialZone.value.block(NVar+1,NVar+1,NVar,NVar) = ZA.abstractedStates[rs[n]].second.value.block(1,1,NVar,NVar);
                  initialZone.value.block(0,NVar+1,1,NVar) = ZA.abstractedStates[rs[n]].second.value.block(0,1,1,NVar);
                  initialZone.value.block(NVar+1,0,NVar,1) = ZA.abstractedStates[rs[n]].second.value.block(1,0,NVar,1);
                  initialZone.M = ZA.abstractedStates[r.front()].second.M;
                  initialZone.canonize();
                  ta2za(A2,ZA2,initialZone);
                  //                  printDuration(, "ta2za");
                  PartialRunChecker<Zone> isPartialRun2(ZA2,ZA,A.edges.size());
                  return isPartialRun2 ({r.begin(),r.begin() + rend},
                                        {rs.begin() + n,rs.begin() + rsend});
                });})) {
          break;
        }
      }
#ifndef PRODUCT
      end = std::chrono::system_clock::now();
      dur = end - start;
      nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
      std::cout << std::scientific << "beta[" << s << "]: " << nsec / 1000000.0 << " ms" << std::endl;
      std::cout << "beta[" << s << "]: " << n << std::endl;
#endif
      beta[s] = n;
    }
#ifdef PRODUCT
    end = std::chrono::system_clock::now();
    dur = end - start;
    nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    std::cout << "precomputation: " << nsec / 1000000.0 << " ms" << std::endl;
#endif
  }

#ifdef SKIP_MAIN
  return;
#endif

  // main computation
  {
    auto start = std::chrono::system_clock::now();  

    int i = 0;
    std::vector<std::pair<std::pair<double,bool>,std::pair<double,bool> > > init;
    std::vector<InternalState> CStates;
    std::vector<InternalState> LastStates;
    const std::pair<double,bool> upperMaxConstraint = {INFINITY,false};
    const std::pair<double,bool> lowerMinConstraint = {0,true};
  
    // When there can be immidiate accepting
    if (m == 1) {
      for (const auto &s: A.initialStates) {
        for (const auto &e: A.edges[s]) {
          if (binary_search (A.acceptingStates.begin(), A.acceptingStates.end(),e.target) and e.c == '$') {
            // solve delta
            std::pair<double,bool> upperConstraint = upperMaxConstraint;
            std::pair<double,bool> lowerConstraint = lowerMinConstraint;
            for (const auto & constraint: e.guard) {
              updateConstraint (upperConstraint,lowerConstraint,constraint,constraint.c);

              if (isValidConstraint (upperConstraint,lowerConstraint)) {
                init.push_back ({upperConstraint,lowerConstraint});
              }
            }
          }
        }
      }
    }

    ans.clear();
    int j;
    while (i <= tSizeP ) {
      bool tooLarge = false;
      // Sunday Shift
      if (m == 1) {
        // When there can be immidiate accepting
        ans.reserve(ans.size() + init.size());
        if (i <= 0) {
          for (const auto& t: init) {
            ans.push_back ({{word[i].first,false},{0,true},{word[i].first,true},{0,false},t.first,t.second});
          }
        } else {
          for (const auto& t: init) {
            ans.push_back ({{word[i].first,false},{word[i-1].first,true},{word[i].first,true},{word[i-1].first,false},t.first,t.second});
          }
        }
        if (i > tSizeP) break;
      } else {
        while (endChars.find(word[i + m - 2].first) == endChars.end() ) {
          if (i >= tSizeP) {
            tooLarge = true;
            break;
          }
          // increment i
          i += delta[ word[i + m - 1].first ];
          word.setFront(i - 1);
          if (i > tSizeP)  {
            tooLarge = true;
            break;
          }
        }
      }
      if (tooLarge) break;

      // KMP like Matching
      CStates.clear ();
      CStates.reserve(A.initialStates.size());
      if (i <= 0) {
        for (const auto& s: A.initialStates) {
          CStates.push_back ({s,{},{word[i].second,false},{0,true}});
        }
      } else {
        for (const auto& s: A.initialStates) {
          CStates.push_back ({s,{},{word[i].second,false},{word[i-1].second,true}});
        }
      }
      j = i;
      while (!CStates.empty () && j < word.size ()) {
        LastStates = CStates;
        CStates.clear ();
        const Alphabet c = word[j].first;
        const double t = word[j].second;
      
        for (const auto &state : LastStates) {
          const State s = state.s;

          for (const auto &edge : A.edges[s]) {
            if (edge.c == c) {

              std::pair<double,bool> upperBeginConstraint = state.upperConstraint;
              std::pair<double,bool> lowerBeginConstraint = state.lowerConstraint;
              bool transitable = true;

              for (const auto& delta: edge.guard) {
                if (state.resetTime[delta.x]) {
                  if (!delta.satisfy(t - state.resetTime[delta.x])) {
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
            
              auto tmpResetTime = state.resetTime;
              for (auto i : edge.resetVars) {
                tmpResetTime[i] = t;
              }
            
              CStates.push_back ({edge.target,tmpResetTime,upperBeginConstraint,lowerBeginConstraint});
            
              for (const auto &edge_f: A.edges[edge.target]) {
                if (std::binary_search(A.acceptingStates.begin(),
                                       A.acceptingStates.end(),edge_f.target) and edge_f.c == '$') {
                  std::pair<double,bool> upperEndConstraint = upperMaxConstraint;
                  std::pair<double,bool> lowerEndConstraint = {word[j].second,false};
                  std::pair<double,bool> upperDeltaConstraint = upperMaxConstraint;
                  std::pair<double,bool> lowerDeltaConstraint = lowerMinConstraint;
                
                  if (j != word.size() - 1) {
                    upperEndConstraint = {word[j+1].second,true};
                  }

                  // solve delta
                  for (const auto& delta: edge_f.guard) {
                    if (tmpResetTime[delta.x]) {
                      const double delta_t = delta.c + tmpResetTime[delta.x];
                      updateConstraint (upperEndConstraint,lowerEndConstraint,delta,delta_t);
                    } else {
                      updateConstraint (upperDeltaConstraint,lowerDeltaConstraint,delta,delta.c);
                    }
                  }

                  if (!isValidConstraint (upperBeginConstraint,lowerBeginConstraint) ||
                      !isValidConstraint (upperEndConstraint,lowerEndConstraint) ||
                      !isValidConstraint (upperDeltaConstraint,lowerDeltaConstraint)) {
                    continue;
                  }

                  ans.push_back ({upperBeginConstraint,lowerBeginConstraint,
                        upperEndConstraint,lowerEndConstraint,
                        upperDeltaConstraint,lowerDeltaConstraint});
                }
              }
            }
          }
        }
        j++;
      }
      if (j >= word.size ()) {
        LastStates = std::move(CStates);
      }
      // KMP like skip value
      int greatestN = 1;
      for (const InternalState& istate: LastStates) {
        int& n = beta[istate.s];
        greatestN = std::max(n,greatestN);
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
