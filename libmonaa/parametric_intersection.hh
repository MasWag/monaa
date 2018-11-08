#pragma once

#include <boost/unordered_map.hpp>
#include "parametric_timed_automaton.hh"

/*
  Specifications
  ==============
  * (s,s') is encoded as s + |S| * s'
  * x = x1 or x2 + |C|
  * p = p1 or p2 + |P|
  * in1 and in2 can be same.
*/
/*!
  @brief Computes product of two PTAs

  @param [in] in1 First PTA
  @param [in] in2 Second PTA
  @param [out] out Product PTA
  @param [out] toIState Mapping from the pair (s1,s2) of the original automata to the state in the product PTA.

 */
void intersectionTA (const ParametricTimedAutomaton &in1, const ParametricTimedAutomaton &in2, ParametricTimedAutomaton &out, boost::unordered_map<std::pair<PTAState*, PTAState*>, std::shared_ptr<PTAState>> &toIState);
