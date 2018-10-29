#pragma once

#include "parametric_timed_automaton.hh"
#include "polyhedron_zone_automaton.hh"

/*!
  @brief Generate a polyhedron zone automaton from a parametric timed automaton

  Adds states with BFS. Initial configuration is the initial states of ZA. The ZA contain only the states reachable from initial states.

  @param [in] TA The PTA to construct PZA
  @param [out] ZA The constructed PZA
  @param [in] initialZone The initial Zone (Polyhedron)
  @param [in] maxStates the maximum number of states of ZA. When the BFS does not terminate before reaching this number, it connects to an accepting state to overapproximate.
 */
void ta2za (const ParametricTimedAutomaton &,
            PZoneAutomaton &,
            Parma_Polyhedra_Library::NNC_Polyhedron initialZone = Parma_Polyhedra_Library::NNC_Polyhedron(0),
            std::size_t maxStates = 100);
