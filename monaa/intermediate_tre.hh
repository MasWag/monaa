#pragma once

#include <unordered_set>

#include "interval.hh"
#include "tre.hh"

/*!
  @brief The syntactic decision of a DNFTRE or AtomicTRE

  @todo Write what is the syntactic decision
 */
struct SyntacticDecision {
  enum class Decision { Constant, Mixed };
  Decision tag;
  //! @note chars must be sorted
  std::vector<Alphabet> chars;
  SyntacticDecision(const Decision &tag, const std::vector<Alphabet> &chars)
      : tag(tag), chars(chars) {}
  void concat(std::shared_ptr<SyntacticDecision> in);
  //! @brief Check if the decision is Mixed
  inline bool isMixed() const {
    return tag == Decision::Mixed && !chars.empty();
  }
  /*!
    @brief Check if the decision is Constant

    @param[out] singleC Write the constant value if the decision is Constant.
    Otherwise, it is not updated
    @retval true if and only if the decision is Constant
   */
  inline bool isConstant(char &singleC) {
    if (chars.size() == 1) {
      singleC = chars[0];
      return true;
    } else {
      return false;
    }
  }
};

/*!
  @brief A TRE of a single value

  The signal value is defined by @link c @endlink and the length of the signal
  is represented by @link intervals @endlink.

  Formally, we have the followng.

  \f[
  \sigma \in L(\varphi) \iff \exists I \in \mathtt{intervals}, t \in I. \sigma =
  \mathtt{c}^{t} \f]
*/
class SingletonTRE {
public:
  //! @brief The value of the TRE
  Alphabet c;
  /*!
    @brief A set of intervals representing the length of the signals.

    @note This is a naive implemenation. We can optimize the interval
    operations.
  */
  std::vector<std::shared_ptr<Interval>> intervals;
  /*!
    @brief The constructor

    @param [in] c The signal value
    @param [in] intervals The length of the signals.  If this is omitted, any
    length is accepted i.e., \f$ \mathtt{intervals} = [0,\infty)\f$.
  */
  SingletonTRE(Alphabet c, std::vector<std::shared_ptr<Interval>> intervals =
                               {std::make_shared<Interval>()})
      : c(c), intervals(std::move(intervals)) {}
};

/*!
  @brief A TRE without Boolean connectives (&& and ||) at the top-level or the
  children of the top-level within.

  For example, the following expressions are atomic.

  a, ab, (abc)%(1,4), (a|c)+

  For example, the following expressions are NOT atomic.

  a&&ab, (ab|c)%(1,4), a|c
*/
class AtomicTRE {
public:
  //! @brief The top-level operation
  enum class op { singleton, epsilon, concat, plus, within };

  //! @brief The constructor for \f$\varepsilon\f$
  AtomicTRE() : tag(op::epsilon) {}
  //! @brief The constructor for \f$c\f$ where \f$c \in \Sigma\f$
  AtomicTRE(const Alphabet c)
      : tag(op::singleton), singleton(std::make_shared<SingletonTRE>(c)) {}
  //! @brief The constructor for a singleton TRE i.e., \f$\bigcup_{I \in
  //! \mathcal{I}} \langle c \rangle_{I} \f$ where \f$c \in \Sigma\f$
  AtomicTRE(const std::shared_ptr<SingletonTRE> singleton)
      : tag(op::singleton), singleton(singleton) {}
  //! @brief The constructor for a concatenation of three formulas
  AtomicTRE(const std::list<std::shared_ptr<AtomicTRE>> &left,
            const std::shared_ptr<AtomicTRE> mid,
            const std::list<std::shared_ptr<AtomicTRE>> &right)
      : tag(op::concat), list() {
    list = left;
    list.push_back(mid);
    list.insert(list.end(), right.begin(), right.end());
  }
  /*!
    @brief The constructor for a concatenation of two formulas

    @note Thanks to the optimization, the constructed formula may not be a
    concatenation e.g., an epsilon formula is given.
   */
  AtomicTRE(const std::shared_ptr<AtomicTRE> left,
            const std::shared_ptr<AtomicTRE> right)
      : tag(op::concat), list() {
    if (left->tag == op::concat) {
      list = left->list;
    } else if (left->tag != op::epsilon) {
      list = {left};
    }
    if (right->tag == op::concat) {
      list.insert(list.end(), right->list.begin(), right->list.end());
    } else if (right->tag != op::epsilon) {
      list.push_back(right);
    }
    // Optimizatin: When the list is not concat anymore, use the original one
    if (list.size() == 0) {
      // list.size() == 0 <==> Both left and right are empty
      tag = op::epsilon;
      list.~list();
    } else if (list.size() == 1) {
      auto origExpr = list.front();
      tag = origExpr->tag;
      list.~list();
      switch (tag) {
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
  }
  //! @brief The constructor for the plus operator
  AtomicTRE(const std::shared_ptr<DNFTRE> expr) : tag(op::plus), expr(expr) {}
  //! @brief The constructor for the witin operator
  AtomicTRE(const std::shared_ptr<AtomicTRE> atomic,
            const std::shared_ptr<Interval> interval)
      : tag(atomic->tag == op::singleton ? op::singleton : op::within) {
    if (atomic->tag == op::singleton) {
      new (&singleton) std::shared_ptr<SingletonTRE>(
          std::make_shared<SingletonTRE>(*(atomic->singleton)));
      land(singleton->intervals, *interval);
    } else {
      new (&within)
          std::pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>(
              atomic, interval);
    }
  }

  //! @brief The destructer
  ~AtomicTRE() {
    switch (tag) {
    case op::epsilon:
      break;
    case op::singleton:
      singleton.reset();
      break;
    case op::plus:
      expr.reset();
      break;
    case op::within:
      within.~pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>>();
      break;
    case op::concat:
      list.~list();
      break;
    }
  }

  op tag;

  union {
    std::shared_ptr<SingletonTRE> singleton;
    std::shared_ptr<DNFTRE> expr;
    std::pair<std::shared_ptr<AtomicTRE>, std::shared_ptr<Interval>> within;
    std::list<std::shared_ptr<AtomicTRE>> list;
  };
  //! @brief The SyntacticDecision for this AtomicTRE
  std::shared_ptr<SyntacticDecision> decision;
  //! @todo write what is the normal form
  void toNormalForm();
  //! @todo write what is the SNF
  bool makeSNF(const char singleC);
  void toSignalTA(TimedAutomaton &out) const;
};

/*!
  @brief A timed regular expression of "DNF" form

  The contained expression is
  \f$\bigvee_{\Phi\in\texttt{list}} \bigwedge_{\varphi\in\Phi}\varphi\f$, where
  \f$\varphi\f$ is an AtomicTRE.

  @todo Masaki is writing here
*/
class DNFTRE {
public:
  std::list<std::list<std::shared_ptr<AtomicTRE>>> list;
  //! @brief the default constructor
  DNFTRE() {}
  /*!
    @brief The constructor for
    \f$\bigwedge_{\varphi\in\texttt{conjunctions}}\varphi\f$, where
    \f$\varphi\f$ is an AtomicTRE.
   */
  DNFTRE(const std::list<std::shared_ptr<AtomicTRE>> &conjunctions)
      : list({conjunctions}) {}
  //! @brief Constricts a DNFTRE from TRE
  DNFTRE(const std::shared_ptr<const TRE> tre);
  //! @brief The SyntacticDecision for this DNFTRE
  std::shared_ptr<SyntacticDecision> decision;
  //! @todo write what is the normal form
  void toNormalForm();
  //! @todo write what is the SNF
  bool makeSNF(const char singleC);
  /*!
    @brief Constract a signal timed automaton

    @param [out] out The constructed (signal) timed automaton
   */
  void toSignalTA(TimedAutomaton &out) const;
};
