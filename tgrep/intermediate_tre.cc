#include "intermediate_tre.hh"

void SyntacticDecision::concat(std::shared_ptr<SyntacticDecision> in) {
  std::vector<Alphabet> ansChars;    
  // chars have epsilon
  if (chars.size() > 0 && chars[0] == 0) {
    if (in->chars.size() > 0 && in->chars[0] == 0) {
      std::set_union(chars.begin(), chars.end(),
                     in->chars.begin(), in->chars.end(), ansChars.begin());
    } else {
      ansChars = in->chars;
    }
  } else if (in->chars.size() > 0 && in->chars[0] == 0) {
    ansChars = chars;
  } else {
    std::set_intersection(chars.begin(), chars.end(),
                          in->chars.begin(), in->chars.end(), ansChars.begin());
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

DNFTRE::DNFTRE(const std::shared_ptr<TRE> tre) {
  switch(tre->tag) {
  case TRE::op::atom: {
    list = {{std::make_shared<AtomicTRE>(tre->c)}};
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
        //! @todo normalize conjunctions

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

        //! @todo normalize conjunctions

        if (!conjunctions.empty()) {
          list.emplace_back(std::move(conjunctions));
        }
      }
    }
    
    break;
  }
  case TRE::op::within: {
    std::shared_ptr<DNFTRE> subfml = std::make_shared<DNFTRE>(tre->regExprWithin.first);
    list = subfml->list;
    for (auto& conjunctions: subfml->list) {
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
        it = list.erase(std::next(it));
      } else {
        it++;
      }
    }

    if (list.size() == 1) {
      auto origExpr = list.front();
      list.~list();
      switch (tag = origExpr->tag) {
      case op::singleton: {
        singleton = origExpr->singleton;
        break;
      }
      case op::epsilon: {
        break;
      }
      case op::concat: {
        list = origExpr->list;
        break;
      }
      case op::plus: {
        expr = origExpr->expr;
        break;
      }
      case op::within: {
        within = origExpr->within;
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
        expr = std::move(origExpr);
        tag = op::plus;
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
    if (decision->chars.size() == 1 || (decision->chars.size() == 2 && decision->chars[0] == 0)) {
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
      for (uint32_t i = 0; i < subsetSize; i++) {
        std::vector<std::list<std::shared_ptr<AtomicTRE>>> subSetVec;
        subSetVec.reserve(vec.size());
        for (int j = 0; j < vec.size(); j++) {
          if( (1<<j) & i){
            subSetVec.push_back(vec[i]);
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
    //! @todo reduce to SNF
    Alphabet singleC;
    if (decision->isConstant(singleC)) {
      singleton = expr->makeSNF(singleC);
      tag = op::singleton;
    }
    break;
  }
  case op::within: {
    within.first->toNormalForm();
    decision = std::make_shared<SyntacticDecision>(expr->decision->tag, expr->decision->chars);
    switch (within.first->tag) {
    case op::singleton: {
      auto origExpr = within.first;
      land(origExpr->singleton->intervals, *(within.second));
      within.~pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>();
      singleton = std::move(origExpr->singleton);
      tag = op::singleton;
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
      // When plus and mixed, we execute sparation
      //! @todo not yet
      if (expr->tag == AtomicTRE::op::plus && expr->decision->isMixed()) {
        
      }
    }
    //! @todo decision of a conjunction
  }
  //! @todo decision of conjunctions
}
