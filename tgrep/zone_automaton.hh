#pragma once

#include "timed_automaton.hh"
#include "zone.hh"

namespace {
  struct ZAState {
    bool isMatch;
    std::array<std::vector<std::weak_ptr<ZAState>>, CHAR_MAX> next;
    std::shared_ptr<TAState> taState;
    Zone zone;
    ZAState () : isMatch(false), next({}) {}
    ZAState (std::shared_ptr<TAState> taState, Zone zone) : isMatch(taState->isMatch), next({}), taState(taState), zone(std::move(zone)) {}
    ZAState (bool isMatch) : isMatch(isMatch), next({}) {}
    ZAState (bool isMatch, std::array<std::vector<std::weak_ptr<ZAState>>, CHAR_MAX> next) : isMatch(isMatch), next(next) {}
    bool operator==(std::pair<std::shared_ptr<TAState>, Zone> pair) {
      return pair.first == taState && pair.second == zone;
    }
  };
}


struct ZoneAutomaton : public Automaton<ZAState> {
  using ZAState = ZAState;
};
