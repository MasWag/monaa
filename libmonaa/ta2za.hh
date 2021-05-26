#pragma once

#include "timed_automaton.hh"
#include "zone_automaton.hh"

/*!
  @brief Generate a zone automaton from a timed automaton
  @tparam NVar the number of variable in TA

  TA to ZA adds states with BFS. Initial configuration is the initial states of
  ZA. The ZA contain only the states reachable from initial states.
 */
void ta2za(const TimedAutomaton &TA, ZoneAutomaton &ZA,
           Zone initialZone = Zone::zero(0));
