#pragma once

#include <ppl.hh>

#include "zone_automaton.hh"
#include "parametric_timed_automaton.hh"

using PZAState = AbstractZAState<PTAState, typename Parma_Polyhedra_Library::NNC_Polyhedron>;
using PZoneAutomaton = AbstractZoneAutomaton<PTAState, typename Parma_Polyhedra_Library::NNC_Polyhedron>;
