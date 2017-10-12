#include "intermediate_tre.hh"

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
