#include <numeric>

#include "intermediate_tre.hh"

void concat2(TimedAutomaton &left, const TimedAutomaton &right);

void SyntacticDecision::concat(std::shared_ptr<SyntacticDecision> in) {
  std::vector<Alphabet> ansChars (chars.size() + in->chars.size());
  // chars have epsilon
  if (chars.size() > 0 && chars[0] == 0) {
    if (in->chars.size() > 0 && in->chars[0] == 0) {
      ansChars.resize(std::set_union(chars.begin(), chars.end(),
                                     in->chars.begin(), in->chars.end(), ansChars.begin()) - ansChars.begin());
    } else {
      ansChars = in->chars;
    }
  } else if (in->chars.size() > 0 && in->chars[0] == 0) {
    ansChars = chars;
  } else {
    ansChars.resize(std::set_intersection(chars.begin(), chars.end(),
                                          in->chars.begin(), in->chars.end(), ansChars.begin()) - ansChars.begin());
  }

  if (tag == Decision::Constant && in->tag == Decision::Constant ) {
    int uniqChar[2] = {-1, -1};
    if (chars.size() == 1) {
      uniqChar[0] = chars[0];
    } else if (chars.size() == 2 && chars[0] == 0) {
      uniqChar[0] = chars[1];
    }
    if (in->chars.size() == 1) {
      uniqChar[1] = in->chars[0];
    } else if (in->chars.size() == 2 && in->chars[0] == 0) {
      uniqChar[1] = in->chars[1];
    }
    if (uniqChar[0] < 0 || uniqChar[1] < 0) {
      tag = Decision::Mixed;
    } else if (!uniqChar[0] || !uniqChar[1] || uniqChar[0] == uniqChar[1]) {
      tag = Decision::Constant;
    } else {
      tag = Decision::Mixed;
    }
  } else {
    tag = Decision::Mixed;
  }

  chars = std::move(ansChars);
}

DNFTRE::DNFTRE(const std::shared_ptr<const TRE> tre) {
  switch(tre->tag) {
  case TRE::op::atom: {
    list.clear();
    for (const char c: tre->c) {
      list.push_back({std::make_shared<AtomicTRE>(c)});
    }
    break;
  }
  case TRE::op::epsilon: {
    list = {{std::make_shared<AtomicTRE>()}};
    break;
  }
  case TRE::op::plus: {
    std::shared_ptr<DNFTRE> subfml = std::make_shared<DNFTRE>(tre->regExpr);
    list = {{std::make_shared<AtomicTRE>(subfml)}};
    break;
  }
  case TRE::op::concat: {
    std::shared_ptr<DNFTRE> subfmlLeft = std::make_shared<DNFTRE>(tre->regExprPair.first);
    std::shared_ptr<DNFTRE> subfmlRight = std::make_shared<DNFTRE>(tre->regExprPair.second);

    for (const auto& conjunctionsLeft: subfmlLeft->list) {
      for (const auto& conjunctionsRight: subfmlRight->list) {
        std::list<std::shared_ptr<AtomicTRE>> conjunctions;
        for (const std::shared_ptr<AtomicTRE> left: conjunctionsLeft) {
          for(const std::shared_ptr<AtomicTRE> right: conjunctionsRight) {
            conjunctions.push_back(std::make_shared<AtomicTRE>(left, right));
          }
        }
        if (!conjunctions.empty()) {
          list.emplace_back(std::move(conjunctions));
        }
      }
    }

    break;
  }
  case TRE::op::disjunction: {
    std::shared_ptr<DNFTRE> subfmlLeft = std::make_shared<DNFTRE>(tre->regExprPair.first);
    std::shared_ptr<DNFTRE> subfmlRight = std::make_shared<DNFTRE>(tre->regExprPair.second);
    list = std::move(subfmlLeft->list);
    list.splice(list.end(), subfmlRight->list);

    break;
  }
  case TRE::op::conjunction: {
    std::shared_ptr<DNFTRE> subfmlLeft = std::make_shared<DNFTRE>(tre->regExprPair.first);
    std::shared_ptr<DNFTRE> subfmlRight = std::make_shared<DNFTRE>(tre->regExprPair.second);

    for (const auto& conjunctionsLeft: subfmlLeft->list) {
      for (const auto& conjunctionsRight: subfmlRight->list) {
        std::list<std::shared_ptr<AtomicTRE>> conjunctions = conjunctionsLeft;
        conjunctions.insert(conjunctions.end(), conjunctionsRight.begin(), conjunctionsRight.end());

        if (!conjunctions.empty()) {
          list.emplace_back(std::move(conjunctions));
        }
      }
    }
    
    break;
  }
  case TRE::op::within: {
    DNFTRE subfml (tre->regExprWithin.first);
    list = std::move(subfml.list);
    for (auto& conjunctions: list) {
      for (auto& atomic: conjunctions) {
        atomic = std::make_shared<AtomicTRE>(atomic, tre->regExprWithin.second);
      }
    }

    break;
  }
  }
};

void AtomicTRE::toNormalForm()
{
  switch (tag) {
  case op::singleton: {
    decision = std::make_shared<SyntacticDecision>(SyntacticDecision::Decision::Constant, std::vector<Alphabet>{singleton->c});
    break;
  }
  case op::epsilon: {
    decision = std::make_shared<SyntacticDecision>(SyntacticDecision::Decision::Constant, std::vector<Alphabet>{0});
    break;
  }
  case op::concat: {
    decision = std::make_shared<SyntacticDecision>(SyntacticDecision::Decision::Constant, std::vector<Alphabet>{0});
    for (std::shared_ptr<AtomicTRE> expr: list) {
      expr->toNormalForm();
      decision->concat(expr->decision);
    }

    // merge unnecessary expressions
    for (auto it = list.begin(); it != list.end(); ) {
      if (list.size() > 1 && (*it)->tag == AtomicTRE::op::epsilon) {
        it = list.erase(it);
      } else if ((*it)->tag == AtomicTRE::op::singleton &&
                 std::next(it) != list.end() && 
                 (*std::next(it))->tag == AtomicTRE::op::singleton &&
                 (*it)->singleton->c == (*std::next(it))->singleton->c) {
        (*it)->singleton->intervals += (*std::next(it))->singleton->intervals;
        list.erase(std::next(it));
      } else {
        it++;
      }
    }

    if (list.size() == 1) {
      auto origExpr = list.front();
      list.~list();
      switch (tag = origExpr->tag) {
      case op::singleton: {
        new (&singleton) std::shared_ptr<SingletonTRE>(origExpr->singleton);
        break;
      }
      case op::epsilon: {
        break;
      }
      case op::concat: {
        new (&list) std::list<std::shared_ptr<AtomicTRE>>(origExpr->list);
        break;
      }
      case op::plus: {
        new (&expr) std::shared_ptr<DNFTRE>(origExpr->expr);
        break;
      }
      case op::within: {
        new (&within) std::pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>(origExpr->within);
        break;
      }
      }
    }
    if (tag == op::concat) {
      auto it = find_if(list.begin(), list.end(), [](std::shared_ptr<AtomicTRE> tre) {
          return tre->tag == op::plus && tre->expr->decision->isMixed();
        });
      if (it != list.end()) {
        // when the subformula is splitted (not plus anymore)
        // propergate within
        std::list<std::shared_ptr<AtomicTRE>> prevList = {list.begin(), std::prev(it)};
        std::list<std::shared_ptr<AtomicTRE>> afterList = {std::next(it), list.end()};
        auto origDecision =  (*it)->decision;
        auto origExpr = (*it)->expr;
        list.~list();
        tag = op::plus;
        new (&expr) std::shared_ptr<DNFTRE>(std::move(origExpr));
        for (auto &conjunctions: expr->list) {
          for (auto &tmpExpr: conjunctions) {
            tmpExpr = std::make_shared<AtomicTRE>(prevList, tmpExpr, afterList);
          }
        }
      }
    }
    break;
  }
  case op::plus: {
    expr->toNormalForm();
    decision = std::make_shared<SyntacticDecision>(SyntacticDecision::Decision::Mixed, expr->decision->chars);
    if (expr->decision->tag == SyntacticDecision::Decision::Constant && (decision->chars.size() == 1 || (decision->chars.size() == 2 && decision->chars[0] == 0))) {
      decision->tag = SyntacticDecision::Decision::Constant;
    }

    // When mixed, we execute sparation
    //! @note This increase the size of formula exponentially
    if (decision->isMixed()) {
      if (expr->list.size() >= 32) {
        throw "too large expression";
      }
      std::shared_ptr<DNFTRE> separatedTRE = std::make_shared<DNFTRE>();
      const uint32_t subsetSize = 1 << expr->list.size();
      expr->list.sort();
      std::vector<std::list<std::shared_ptr<AtomicTRE>>> vec(expr->list.begin(), expr->list.end());
      for (uint32_t i = 1; i < subsetSize; i++) {
        std::vector<std::list<std::shared_ptr<AtomicTRE>>> subSetVec;
        subSetVec.reserve(vec.size());
        for (std::size_t j = 0; j < vec.size(); j++) {
          if( (1<<j) & i){
            subSetVec.push_back(vec[j]);
          }
        }
        std::shared_ptr<DNFTRE> dnftre = std::make_shared<DNFTRE>();
        do {
          std::shared_ptr<AtomicTRE> atomic = std::make_shared<AtomicTRE>();
          for (auto &conjunctions: subSetVec) {
            atomic = std::make_shared<AtomicTRE>(atomic, std::make_shared<AtomicTRE>(std::make_shared<DNFTRE>(conjunctions)));
          }
          dnftre->list.push_back({atomic});
        } while (std::next_permutation(subSetVec.begin(), subSetVec.end()));
        dnftre->toNormalForm();
        separatedTRE->list.push_back({std::make_shared<AtomicTRE>(dnftre)});
      }
      expr = std::move(separatedTRE);
    }
    //! reduce to SNF
    Alphabet singleC;
    if (decision->isConstant(singleC)) {
      makeSNF(singleC);
    }
    break;
  }
  case op::within: {
    within.first->toNormalForm();
    decision = std::make_shared<SyntacticDecision>(within.first->decision->tag, within.first->decision->chars);
    switch (within.first->tag) {
    case op::singleton: {
      auto origExpr = within.first;
      land(origExpr->singleton->intervals, *(within.second));
      within.~pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>();
      tag = op::singleton;
      new (&singleton) std::shared_ptr<SingletonTRE>(std::move(origExpr->singleton));
      break;
    }
    case op::epsilon: {
      within.~pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>();
      tag = op::epsilon;
      break;
    }
    case op::plus: {
      if (within.first->decision->isMixed()) {
        // when the subformula is splitted (not plus anymore)
        // propergate within
        decision = within.first->decision;
        auto origExpr = within.first;
        auto origInterval = within.second;
        within.~pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>();
        expr = origExpr->expr;
        for (auto &conjunctions: expr->list) {
          for (auto &tmpExpr: conjunctions) {
            tmpExpr = std::make_shared<AtomicTRE>(tmpExpr, origInterval);
          }
        }
      }
    }
    case op::concat:
    case op::within: {
      break;
    }
    }
    break;
  }
  }
}

void DNFTRE::toNormalForm()
{
  for (auto &conjunctions: list) {
    for (auto &expr: conjunctions) {
      expr->toNormalForm();
    }
  }
  for (auto it = list.begin(); it != list.end();) {
    auto itc = std::find_if(it->begin(), it->end(), [](std::shared_ptr<AtomicTRE> expr) {
        return expr->tag == AtomicTRE::op::plus && expr->decision->isMixed();
      });
    // When plus and mixed, we execute sparation
    if (itc != it->end()) {
      std::shared_ptr<AtomicTRE> expr = *itc;
      std::list<std::shared_ptr<AtomicTRE>> others = {it->begin(), std::prev(itc)};
      others.insert(others.end(), std::next(itc), it->end());
      for (auto &subExpr: expr->expr->list) {
        subExpr.insert(subExpr.end(), others.begin(), others.end());
      }
      list.insert(list.end(), expr->expr->list.begin(), expr->expr->list.end());
      it = list.erase(it);
    } else {
      it++;
    }
  }

  // decision of each conjunction
  std::vector<std::shared_ptr<SyntacticDecision>> decisions;
  decisions.reserve(list.size());
  for (auto &conjunctions: list) {
    auto it = conjunctions.begin();
    decisions.push_back(std::make_shared<SyntacticDecision>(*((*it)->decision)));
    for (it++;it != conjunctions.end();) {
      decisions.back()->tag = (decisions.back()->tag == SyntacticDecision::Decision::Constant || (*it)->decision->tag == SyntacticDecision::Decision::Constant) ? SyntacticDecision::Decision::Constant : SyntacticDecision::Decision::Mixed;
      std::vector<Alphabet> chars (decisions.back()->chars.size());
      chars.resize(std::set_intersection(decisions.back()->chars.begin(), decisions.back()->chars.end(),
                                         (*it)->decision->chars.begin(), (*it)->decision->chars.end(), chars.begin()) - chars.begin());
      decisions.back()->chars = std::move(chars);
    }
  }

  // decision of conjunctions
  auto it = decisions.begin();
  decision = std::make_shared<SyntacticDecision>(**it);
  for (it++;it != decisions.end();it++) {
    decision->tag = (decision->tag == SyntacticDecision::Decision::Constant && (*it)->tag == SyntacticDecision::Decision::Constant) ? SyntacticDecision::Decision::Constant : SyntacticDecision::Decision::Mixed;
    std::vector<Alphabet> chars(decision->chars.size());
    chars.resize(std::set_intersection(decision->chars.begin(), decision->chars.end(),
                                       (*it)->chars.begin(), (*it)->chars.end(), chars.begin()) - chars.begin());
    decision->chars = std::move(chars);
  }
}

bool AtomicTRE::makeSNF(const char singleC)
{
  switch(tag) {
  case op::singleton: {
    return singleC == singleton->c;
  }
  case op::epsilon: {
    return false;
  }
  case op::concat: {
    if (std::all_of(list.begin(), list.end(), [&](std::shared_ptr<AtomicTRE> tre) {
          return tre->makeSNF(singleC);
        })) {
      auto tmpTre = std::make_shared<SingletonTRE>(singleC, std::vector<std::shared_ptr<Interval>>{std::make_shared<Interval>(Bounds{0, true}, Bounds{0, true})});
      for (auto &singleton: list) {
        tmpTre->intervals += singleton->singleton->intervals;
      }
      tag = op::singleton;
      list.~list();
      new (&singleton) std::shared_ptr<SingletonTRE>(std::move(tmpTre));
      return true;
    } else {
      return false;
    }
  }
  case op::plus: {
    if (!expr->makeSNF(singleC)) {
      return false;
    }
    auto tmpTRE = expr->list.front().front()->singleton;
    expr.reset();
    tag = op::singleton;
    new (&singleton) std::shared_ptr<SingletonTRE>(std::move(tmpTRE));
    plus(singleton->intervals);
    return true;
  }
  case op::within: {
    if (!within.first->makeSNF(singleC)) {
      return false;
    }
    auto tmpTRE = within.first->singleton;
    auto tmpInterval = within.second;
    within.~pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>();
    tag = op::singleton;
    new (&singleton) std::shared_ptr<SingletonTRE>(std::move(tmpTRE));
    land(singleton->intervals, *tmpInterval);

    return true;
  }
  }
  return false;
}

bool DNFTRE::makeSNF(const char singleC)
{
  auto tmpTre = std::make_shared<SingletonTRE>(singleC, std::vector<std::shared_ptr<Interval>>{});
  for (auto it = list.begin(); it != list.end();) {
    bool remainConjunctions = true;
    for (auto &expr: *it) {
      if (!(remainConjunctions = remainConjunctions && expr->makeSNF(singleC))) {
        break;
      }
    }
    if (remainConjunctions) {
      it++;
    } else {
      it = list.erase(it);
    }
  }
  // make SNF
  for (const auto &conjunctions: list) {
    std::vector<std::shared_ptr<Interval>> tmpIntervals = {std::make_shared<Interval>()};
    for (const auto &expr: conjunctions) {
      land(tmpIntervals, expr->singleton->intervals);
    }
    tmpTre->intervals.reserve(tmpTre->intervals.size() + tmpIntervals.size());
    tmpTre->intervals.insert(tmpTre->intervals.end(), tmpIntervals.begin(), tmpIntervals.end());
  }
  list = {{std::make_shared<AtomicTRE>(std::move(tmpTre))}};
  return true;
}

void AtomicTRE::toSignalTA(TimedAutomaton& out) const {
  switch(tag) {
  case op::singleton: {
    if (singleton->intervals.empty()) {
      out.states.clear();
      out.initialStates.clear();
      return;
    }
    out.states.resize(2);
    for (auto &state: out.states) {
      state = std::make_shared<TAState>();
    }
    out.initialStates = {out.states[0]};

    out.states[0]->isMatch = false;
    out.states[1]->isMatch = true;

    // make constraints form intervals
    out.states[0]->next[singleton->c].reserve(singleton->intervals.size());
    int maxConstraint = -1;
    for (const auto &interval: singleton->intervals) {
      std::vector<Constraint> guard;
      guard.reserve(2);
      if (interval->lowerBound.first != 0 || interval->lowerBound.second != true) {
        if (interval->lowerBound.second) {
          guard.emplace_back(TimedAutomaton::X(0) >= interval->lowerBound.first);
        } else {
          guard.emplace_back(TimedAutomaton::X(0) > interval->lowerBound.first);          
        }
        maxConstraint = std::max(maxConstraint, int(interval->lowerBound.first));
      }
      if (!std::isinf(interval->upperBound.first)) {
        if (interval->upperBound.second) {
          guard.emplace_back(TimedAutomaton::X(0) <= interval->upperBound.first);
        } else {
          guard.emplace_back(TimedAutomaton::X(0) < interval->upperBound.first);          
        }
        maxConstraint = std::max(maxConstraint, int(interval->upperBound.first));
      }
      out.states[0]->next[singleton->c].push_back({out.states[1].get(), {}, guard});
    }

    if (maxConstraint != -1) {
      out.maxConstraints.push_back(maxConstraint);
    } else {
      out.maxConstraints.clear();
    }
    break;
  }
  case op::epsilon: {
    out.states.resize(1);
    out.states[0] = std::make_shared<TAState>();
    out.initialStates = {out.states[0]};
    out.states[0]->isMatch = true;
    out.maxConstraints.clear();
    break;
  }
  case op::plus: {
    expr->toSignalTA(out);
    for (auto &s: out.states) {
      for (auto &edges: s->next) {
        for (auto &edge: edges.second) {
          TAState *target = edge.target;
          if (target && target->isMatch) {
            edges.second.reserve(edges.second.size() + out.initialStates.size());
            for (auto initState: out.initialStates) {
              TATransition transition = edge;
              edge.target = target;
              edges.second.emplace_back(std::move(transition));
            }
          }
        }
      }
    }
    break;
  }
  case op::concat: {
    auto it = list.begin();
    (*it)->toSignalTA(out);
    for (it++; it != list.end(); it++) {
      TimedAutomaton another; 
      (*it)->toSignalTA(another);
      concat2(out, another);
    }
    break;
  }
  case op::within: {
    within.first->toSignalTA(out);
    // add dummy initial state
    std::shared_ptr<TAState> dummyInitialState = std::make_shared<TAState>();
    for (auto initialState: out.initialStates) {
      for (const auto &initTransitionsPair: initialState->next) {
        const Alphabet c = initTransitionsPair.first;
        dummyInitialState->next[c].reserve(dummyInitialState->next[c].size() + initTransitionsPair.second.size());
        for (auto& edge: initTransitionsPair.second) {
          TATransition transition = edge;
          transition.resetVars.push_back(out.clockSize());
          dummyInitialState->next[c].emplace_back(std::move(transition));
        }
      }
    }
    // add dummy accepting state
    std::shared_ptr<TAState> dummyAcceptingState = std::make_shared<TAState>();
    dummyAcceptingState->isMatch = true;
    for (auto state: out.states) {
      for (auto& edges: state->next) {
        for (auto& edge: edges.second) {
          auto target = edge.target;
          if (target && target->isMatch) {
            TATransition transition = edge;
            transition.target = dummyAcceptingState.get();
            transition.guard.reserve(transition.guard.size() + 2);
            // upper bound
            if (within.second->upperBound.second) {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) <= within.second->upperBound.first);
            } else {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) < within.second->upperBound.first);
            }
            // lower bound
            if (within.second->lowerBound.second) {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) >= within.second->lowerBound.first);
            } else {
              transition.guard.emplace_back(TimedAutomaton::X(out.clockSize()) > within.second->lowerBound.first);
            }
            edges.second.emplace_back(std::move(transition));
          }
        }
      }
    }
    for (auto state: out.states) {
      state->isMatch = false;
    }
    out.initialStates = {dummyInitialState};
    out.states.reserve(out.stateSize() + 2);
    out.states.emplace_back(std::move(dummyInitialState));
    out.states.emplace_back(std::move(dummyAcceptingState));
    out.maxConstraints.emplace_back(within.second->upperBound.first);
    break;
  }
  }
}

void DNFTRE::toSignalTA(TimedAutomaton& out) const {
  for (auto &conjunctions: list) {
    auto it = conjunctions.begin();
    TimedAutomaton another;
    (*it)->toSignalTA(another);
    for (it++; it != conjunctions.end(); it++) {
      TimedAutomaton tmpTA, TA2;
      (*it)->toSignalTA(tmpTA);
      intersectionSignalTA(another, tmpTA, TA2);
    }
    out.states.insert(out.states.end(), another.states.begin(), another.states.end());
    out.initialStates.insert(out.initialStates.end(), another.initialStates.begin(), another.initialStates.end());

    // we can reuse variables since we have no overwrapping constraints
    out.maxConstraints.resize(std::max(out.maxConstraints.size(), another.maxConstraints.size()));
    another.maxConstraints.resize(std::max(out.maxConstraints.size(), another.maxConstraints.size()));
    for (std::size_t i = 0; i < out.maxConstraints.size(); ++i) {
      out.maxConstraints[i] = std::max(out.maxConstraints[i], another.maxConstraints[i]);
    }
  }
}
