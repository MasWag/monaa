#pragma once

#include <unordered_map>
#include "parametric_timed_automaton.hh"
#include "polyhedron_zone_automaton.hh"
#include "parametric_intersection.hh"
#include "intersection.hh"
#include "pta2pza.hh"

#include "kmp_skip_value.hh"

class ParametricKMPSkipValue {
private:
  std::unordered_map<const PTAState*,
                     std::vector<Parma_Polyhedra_Library::Pointset_Powerset<Parma_Polyhedra_Library::NNC_Polyhedron>>> beta; //< container for the calculated skip value function  
public:
  /*!
    @param [in] TA timed automaton
    @param [in] m length of the minimum timed words accepted by TA
  */
  ParametricKMPSkipValue(const ParametricTimedAutomaton &TA, int m) {
    const Parma_Polyhedra_Library::Variables_Set removedVariablesSet(Parma_Polyhedra_Library::Variable(1 + TA.paramDimensions), Parma_Polyhedra_Library::Variable(1 + 2 * TA.paramDimensions + 2 * TA.clockDimensions - 1));
    PZoneAutomaton ZA2;
    ParametricTimedAutomaton A2;
    boost::unordered_map<std::pair<typename ParametricTimedAutomaton::State*, typename ParametricTimedAutomaton::State*>, std::shared_ptr<typename ParametricTimedAutomaton::State>> toIState;
    ParametricTimedAutomaton A0;
    // Vector of extended initial states. if the accepting state is extendedInitialStates[n], the TA reads n-additional events.
    std::vector<std::shared_ptr<typename ParametricTimedAutomaton::State>> extendedInitialStates(m+1);
    KMPSkipValueTemplate<ParametricTimedAutomaton, Parma_Polyhedra_Library::NNC_Polyhedron>::makeAn(TA, m, A0, extendedInitialStates);
    
    // As is the automaton in A_{s}^* in the paper. What we do is to construct a dummy state for each state s.
    ParametricTimedAutomaton As;
    std::unordered_map<typename ParametricTimedAutomaton::State*, std::shared_ptr<typename ParametricTimedAutomaton::State>> old2newS;
    std::unordered_map<std::shared_ptr<typename ParametricTimedAutomaton::State>, std::shared_ptr<typename ParametricTimedAutomaton::State>> toDummyState;

    KMPSkipValueTemplate<ParametricTimedAutomaton, Parma_Polyhedra_Library::NNC_Polyhedron>::makeAs(TA, As, old2newS, toDummyState);

    intersectionTA (As, A0, A2, toIState);

    // Calculate KMP-type skip value
    for (auto origState: TA.states) {
      for (auto &stateAs: As.states) {
        stateAs->isMatch = stateAs == old2newS[origState.get()] || stateAs == toDummyState[old2newS[origState.get()]];
      }
      beta[origState.get()].reserve(m);
      // Find the minumum n such that the intersection of the two languages is not empty.
      for (int n = 1; n <= m; n++) {
        A0.initialStates = {extendedInitialStates[n]};
        updateInitAccepting(As, A0, A2, toIState);
        std::sort(A2.initialStates.begin(), A2.initialStates.end());
        ZA2.updateInitAccepting(A2.initialStates);
        ta2za(A2, ZA2);

        // We have twice number of params
        Parma_Polyhedra_Library::Pointset_Powerset<Parma_Polyhedra_Library::NNC_Polyhedron> feasibleParameters(TA.paramDimensions + 1, Parma_Polyhedra_Library::EMPTY);
        {
          // Synthesize the parameters
          std::vector<std::shared_ptr<PZAState>> currentStates = ZA2.initialStates;
          std::unordered_set<std::shared_ptr<PZAState>> visited = {ZA2.initialStates.begin(), ZA2.initialStates.end()};
          while (!currentStates.empty()) {
            std::vector<std::shared_ptr<PZAState>> nextStates;
            for (auto state: currentStates) {
              if (state->isMatch) {
                Parma_Polyhedra_Library::NNC_Polyhedron zone = state->zone;
                zone.remove_space_dimensions(removedVariablesSet);
                feasibleParameters.add_disjunct(std::move(zone));
              }
              for (const auto &edges: state->next) {
                for (const auto edge: edges) {
                  auto target = edge.lock();
                  if (target && visited.find(target) == visited.end()) {
                    // We have not visited the state
                    nextStates.push_back(target);
                    visited.insert(target);
                  }
                }
              }
        
            }
            currentStates = std::move(nextStates);
          }
        }
        feasibleParameters.omega_reduce();
        beta[origState.get()].emplace_back(std::move(feasibleParameters));
        if (feasibleParameters.is_universe()) {
          break;
        }
      }
    }
  }

  /*!
    @brief Accessor to the Parametric KMP skip values

    @param [in] s State of the timed automaton
    @param [in] zone Zone representing the available parameters
  */
  inline
  std::size_t at(const typename ParametricTimedAutomaton::State *s,
                 const Parma_Polyhedra_Library::NNC_Polyhedron &zone) const {
    const Parma_Polyhedra_Library::Pointset_Powerset<Parma_Polyhedra_Library::NNC_Polyhedron> powersetZone = Parma_Polyhedra_Library::Pointset_Powerset<Parma_Polyhedra_Library::NNC_Polyhedron>(zone);
    for (std::size_t i = 0; i < beta.at(s).size(); i++) {
      Parma_Polyhedra_Library::Pointset_Powerset<Parma_Polyhedra_Library::NNC_Polyhedron> currentZone = beta.at(s).at(i);
      currentZone.intersection_assign(powersetZone);
      if (!currentZone.is_empty()) {
        return i + 1;
      }
    }
    return beta.at(s).size() + 1;
  }

  /*!
    @brief Accessor to the Parametric KMP skip values

    @param [in] s State of the timed automaton
    @param [in] zone Zone representing the available parameters
  */
  inline
  std::size_t at(std::shared_ptr<typename ParametricTimedAutomaton::State> s,
                 const Parma_Polyhedra_Library::NNC_Polyhedron &zone) const {
    return at(s.get(), zone);
  }
};
