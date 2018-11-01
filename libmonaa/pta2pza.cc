#include <cstdlib>
#include <utility>
#include <tuple>
#include <numeric>

#include "pta2pza.hh"

void ta2za (const ParametricTimedAutomaton &PTA,
            PZoneAutomaton &PZA,
            Parma_Polyhedra_Library::NNC_Polyhedron initialZone,
            std::size_t maxStates)
{
  if(initialZone.space_dimension() == 0) {
    initialZone = Parma_Polyhedra_Library::NNC_Polyhedron(PTA.paramDimensions + PTA.clockDimensions + 1);
    Parma_Polyhedra_Library::Variable t(0);
    initialZone.add_constraint(t == 0);
    for (unsigned long long i = 0; i < PTA.clockDimensions; i++) {
      Parma_Polyhedra_Library::Variable x(i + 1 + PTA.paramDimensions);
      initialZone.add_constraint(x == 0);
    }
  }
  Parma_Polyhedra_Library::NNC_Polyhedron elapsePolyhedron = Parma_Polyhedra_Library::NNC_Polyhedron(PTA.paramDimensions + PTA.clockDimensions + 1);
  Parma_Polyhedra_Library::Variable t(0);
  elapsePolyhedron.add_constraint(t == 1);
  for (unsigned long long i = 0; i < PTA.paramDimensions; i++) {
    Parma_Polyhedra_Library::Variable p(i + 1);
    elapsePolyhedron.add_constraint(p == 0);
  }
  for (unsigned long long i = 0; i < PTA.clockDimensions; i++) {
    Parma_Polyhedra_Library::Variable x(i + 1 + PTA.paramDimensions);
    elapsePolyhedron.add_constraint(x == 1);
  }


  auto initialStates = PTA.initialStates;
  // remove the states already computed
  if (!PZA.states.empty()) {
    for (auto it = initialStates.begin(); it != initialStates.end();) {
      if (std::find_if(PZA.states.begin(), PZA.states.end(), [&it, &initialZone](std::shared_ptr<PZAState> zaState) { 
            return zaState->taState == it->get() && initialZone.contains(zaState->zone);
          }) != PZA.states.end()) {
        it = initialStates.erase(it);
      } else {
        it++;
      }
    }
  }
  if (initialStates.empty()) {
    return;    
  }

  /*!
    @brief Make initial state, that is Current configuration of BFS
  */
  std::vector<std::shared_ptr<PZAState> > nextConf;
  nextConf.reserve(initialStates.size());
  PZA.states.reserve(PZA.stateSize() + initialStates.size());
  PZA.initialStates.reserve(PZA.initialStates.size() + initialStates.size());
  for (const auto &taState : initialStates ) {
    PZA.states.push_back(std::make_shared<PZAState>(taState.get(), initialZone));
    PZA.initialStates.push_back(PZA.states.back());
    nextConf.push_back(PZA.states.back());
  }

  /*! 
    @brief translater from TAState and Zone to its corresponding state in PZA.

    The type is like this.
    (TAState,Zone) -> ZAState
  */
  while (!nextConf.empty ()) {
    std::vector<std::shared_ptr<PZAState>> currentConf = std::move(nextConf);
    nextConf.clear();
    for (const auto &conf : currentConf) {
      PTAState *taState = conf->taState;
      Parma_Polyhedra_Library::NNC_Polyhedron nowZone = conf->zone;
      nowZone.positive_time_elapse_assign(elapsePolyhedron);
      for (auto it = taState->next.begin(); it != taState->next.end(); it++) {
        const Alphabet c = it->first;
        for (const auto &edge : it->second) {
          auto nextZone = nowZone;
          auto nextState = edge.target;
          if (!nextState) {
            continue;
          }          
          nextZone.intersection_assign(edge.guard);

          if (!nextZone.is_empty()) {
            for (auto x : edge.resetVars) {
              nextZone.unconstrain(Parma_Polyhedra_Library::Variable(1 + PTA.paramDimensions + x));
              nextZone.add_constraint(Parma_Polyhedra_Library::Variable(1 + PTA.paramDimensions + x) == 0);
            }
            // check if nextZone state is new
            const auto targetStateInZA = std::find_if(PZA.states.begin(), PZA.states.end(), [&nextState, &nextZone] (std::shared_ptr<PZAState> zaState) { 
                return zaState->taState == nextState && zaState->zone.contains(nextZone);
              });

            // targetStateInZA is already added
            if (targetStateInZA != PZA.states.end()) {
              conf->next[c].push_back(*targetStateInZA);

              //! @todo check if this is necessary
              // PZA.edgeMap[newEdge.toTuple()] = taEdge;
            } else {
              // targetStateInZA is new
              PZA.states.push_back(std::make_shared<PZAState>(nextState, nextZone));
              conf->next[c].push_back(PZA.states.back());

              // PZA.edgeMap[newEdge.toTuple()] = taEdge;
            
              nextConf.push_back (PZA.states.back());
            }
          }
        }
      }
    }

    if (PZA.states.size() > maxStates) {
      // reachability analysis does not terminate. do overapproximation
      auto acceptingState = std::find_if(PTA.states.begin(), PTA.states.end(), [] (std::shared_ptr<PTAState> taState) {
          return taState->isMatch;
        });
      if (acceptingState != PTA.states.end()) {
        auto dummyAcceptingZAState = std::make_shared<PZAState>(acceptingState->get(), initialZone);
        for (const auto &conf : nextConf) {
          PTAState *taState = conf->taState;
          for (auto it = taState->next.begin(); it != taState->next.end(); it++) {
            const Alphabet c = it->first;
            if (!it->second.empty()) {
              conf->next[c].push_back(dummyAcceptingZAState);
            }
          }
        }
      }
      return;
    }
  }
}
