#pragma once

#include <unordered_map>
#include <numeric>
#include <ppl.hh>

#include "timed_automaton.hh"
#include "zone_automaton.hh"
#include "intersection.hh"
#include "ta2za.hh"

template<class T, class Autom>
struct GuardHelper;

template<>
struct GuardHelper<std::vector<Constraint>, TimedAutomaton> {
 const std::vector<Constraint> empty;
  static inline void widen(std::vector<Constraint> &guard) {
    ::widen(guard);
  }
  const std::size_t clockSize;
  GuardHelper(const TimedAutomaton& TA) : empty(), clockSize(TA.clockSize()) {}
};

#include "parametric_timed_automaton.hh"
#include "polyhedron_zone_automaton.hh"
#include "pta2pza.hh"

template<>
struct GuardHelper<Parma_Polyhedra_Library::NNC_Polyhedron, ParametricTimedAutomaton> {
  const Parma_Polyhedra_Library::NNC_Polyhedron empty;
  static inline void widen(Parma_Polyhedra_Library::NNC_Polyhedron &) {
  }
  const std::size_t clockSize;
  GuardHelper(const ParametricTimedAutomaton& TA) :
    empty(Parma_Polyhedra_Library::NNC_Polyhedron(1 + TA.paramDimensions + TA.clockDimensions)),
    clockSize(TA.clockDimensions) {}
};

/*!
  @brief Class for KMP-style skip value function
 */
template<class TAutomaton, class Zone>
class KMPSkipValueTemplate {
  using ZoneAutomaton = AbstractZoneAutomaton<typename TAutomaton::State, Zone>;
private:
  std::unordered_map<const typename TAutomaton::State*, int> beta; //< container for the calculated skip value function

public:
  /*!
    @brief construct the automaton A_{+n}^* in the paper

    What we do is: 1) construct a dummy accepting state, and 2) construct m-dummy states to the original intial states.

    @param [in] TA The input timed automaton 
    @param [in] m Size of the extention
    @param [out] A0 The output timed automaton 
    @param [out] extendedInitialStates Vector of the extended initial states
   */
  static void makeAn(const TAutomaton &TA, const int m, TAutomaton &A0, std::vector<std::shared_ptr<typename TAutomaton::State>> &extendedInitialStates) {
    std::unordered_map<typename TAutomaton::State*, std::shared_ptr<typename TAutomaton::State>> old2new0;
    TA.deepCopy(A0, old2new0);
    GuardHelper<typename TAutomaton::TATransition::Guard, TAutomaton> gh = GuardHelper<typename TAutomaton::TATransition::Guard, TAutomaton>(TA);

    for (auto &es: extendedInitialStates) {
      es = std::make_shared<typename TAutomaton::State>();
    }

    std::vector<ClockVariables> allClocks(gh.clockSize);
    std::iota(allClocks.begin(), allClocks.end(), 0);
    for (auto initialState: A0.initialStates) {
      extendedInitialStates[0]->next[0].push_back({initialState.get(), allClocks, gh.empty});
    }

    for (int i = 1; i <= m; ++i) {
      for (char c = 1; c < CHAR_MAX; ++c) {
        extendedInitialStates[i]->next[c].push_back({extendedInitialStates[i-1].get(), {}, gh.empty});
      }      
    }
    auto dummyAcceptingState = std::make_shared<typename TAutomaton::State>(true);
    // add self loop
    for (char c = 1; c < CHAR_MAX; ++c) {
      dummyAcceptingState->next[c].push_back({dummyAcceptingState.get(), {}, gh.empty});
    }
    for (auto &state: A0.states) {
      for (auto it = state->next.begin(); it != state->next.end(); it++) {
        for (typename TAutomaton::TATransition edge: it->second) {
          const auto target = edge.target;
          // We can modify edge because it is copied
          if (target && target->isMatch) {
            edge.target = dummyAcceptingState.get();
            GuardHelper<typename TAutomaton::TATransition::Guard, TAutomaton>::widen(edge.guard);
            it->second.emplace_back(std::move(edge));
          }
        }
      }      
    }
    A0.states.reserve(A0.states.size() + 1 + (m + 1));
    A0.states.push_back(dummyAcceptingState);
    A0.states.insert(A0.states.end(), extendedInitialStates.begin(), extendedInitialStates.end());
  }
  /*!
    @brief construct the automaton A_{s}^* in the paper

    What we do is to construct a dummy state for each state s.

    @param [in] TA The input timed automaton 
    @param [out] As The output timed automaton 
    @param [out] old2newS Mapping from the input TA to the output TA
    @param [out] toDummyState Mapping from a state of TA and the corresponding dummy state
  */
  static void makeAs(const TAutomaton &TA, TAutomaton &As,
                     std::unordered_map<typename TAutomaton::State*, std::shared_ptr<typename TAutomaton::State>>  &old2newS,
                     std::unordered_map<std::shared_ptr<typename TAutomaton::State>, std::shared_ptr<typename TAutomaton::State>> &toDummyState) {
    GuardHelper<typename TAutomaton::TATransition::Guard, TAutomaton> gh = GuardHelper<typename TAutomaton::TATransition::Guard, TAutomaton>(TA);
    old2newS.reserve(TA.states.size());
    TA.deepCopy(As, old2newS);
    toDummyState.reserve(TA.states.size());
    for (auto state: As.states) {
      toDummyState[state] = std::make_shared<typename TAutomaton::State>();
      state->next[0].push_back({toDummyState[state].get(), {}, gh.empty});
      // add self loop
      for (char c = 1; c < CHAR_MAX; ++c) {
        toDummyState[state]->next[c].push_back({toDummyState[state].get(), {}, gh.empty});
      }
    }
    As.states.reserve(As.states.size() * 2);
    for (auto dummyState: toDummyState) {
      As.states.push_back(dummyState.second);
    }
  }

  /*!
    @param [in] TA timed automaton
    @param [in] m length of the minimum timed words accepted by TA
  */
  KMPSkipValueTemplate(const TAutomaton &TA, int m) {
    ZoneAutomaton ZA2;
    TAutomaton A2;
    boost::unordered_map<std::pair<typename TAutomaton::State*, typename TAutomaton::State*>, std::shared_ptr<typename TAutomaton::State>> toIState;
    TAutomaton A0;
    // Vector of extended initial states. if the accepting state is extendedInitialStates[n], the TA reads n-additional events.
    std::vector<std::shared_ptr<typename TAutomaton::State>> extendedInitialStates(m+1);
    makeAn(TA, m, A0, extendedInitialStates);
    
    // As is the automaton in A_{s}^* in the paper. What we do is to construct a dummy state for each state s.
    TAutomaton As;
    std::unordered_map<typename TAutomaton::State*, std::shared_ptr<typename TAutomaton::State>> old2newS;
    std::unordered_map<std::shared_ptr<typename TAutomaton::State>, std::shared_ptr<typename TAutomaton::State>> toDummyState;

    makeAs(TA, As, old2newS, toDummyState);

    intersectionTA (A0, As, A2, toIState);

    // Calculate KMP-type skip value
    for (auto origState: TA.states) {
      for (auto &stateAs: As.states) {
        stateAs->isMatch = stateAs == old2newS[origState.get()] || stateAs == toDummyState[old2newS[origState.get()]];
      }
      // Find the minumum n such that the intersection of the two languages is not empty.
      for (int n = 1; n <= m; n++) {
        A0.initialStates = {extendedInitialStates[n]};
        updateInitAccepting(A0, As, A2, toIState);
        std::sort(A2.initialStates.begin(), A2.initialStates.end());
        ZA2.updateInitAccepting(A2.initialStates);
        ta2za(A2, ZA2);
        if (!ZA2.empty()) {
          beta[origState.get()] = n;
          break;
        }
      }
      // When the emptiness checking always failed, we set m
      if (beta.find(origState.get()) == beta.end()) {
        beta[origState.get()] = m;
      }
    }
  }

  inline
  int at(const typename TAutomaton::State *s) const {
    return beta.at(s);
  }
  inline
  int operator[](const typename TAutomaton::State *s) const {
    return beta.at(s);
  }
  inline
  int at(std::shared_ptr<typename TAutomaton::State> s) const {
    return beta.at(s.get());
  }
  inline
  int operator[](std::shared_ptr<typename TAutomaton::State> s) const {
    return beta.at(s.get());
  }
};

using KMPSkipValue = KMPSkipValueTemplate<TimedAutomaton, Zone>;
