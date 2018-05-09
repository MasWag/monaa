#pragma once

#include <boost/unordered_map.hpp>
#include "timed_automaton.hh"

/*
  Specifications
  ==============
  * (s,s') is encoded as s + |S| * s'
  * x = x1 or x2 + |C|
  * in1 and in2 can be same.
*/
void intersectionTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out, boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> &toIState, boost::unordered_map<TAState*, uint32_t> &indegree);

void updateInitAccepting(const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out, boost::unordered_map<std::pair<TAState*, TAState*>, std::shared_ptr<TAState>> toIState);

void intersectionSignalTA (const TimedAutomaton &in1, const TimedAutomaton &in2, TimedAutomaton &out);
