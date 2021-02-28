#include <rapidcheck/boost_test.h>
#include <memory>
#include "objective_tre.hh"
#include "zone.hh"

/*
  @brief define a random generator of TRE for rapidcheck
 */

namespace rc {
  template<>
  struct Arbitrary<Interval> {
    static Gen<Interval> arbitrary() {
      return gen::build<Interval>(gen::set(&Interval::lowerBound), gen::set(&Interval::upperBound));
    }
  };

  template<>
  struct Arbitrary<AtomTRE> {
    static Gen<AtomTRE> arbitrary() {
      return gen::construct<AtomTRE>(gen::positive<Alphabet>());
    }
  };

//   template<>
//   struct Arbitrary<std::shared_ptr<ObjectiveTRE>> {
//     static Gen<std::shared_ptr<ObjectiveTRE>> arbitrary() {
//       const int kind = *rc::gen::inRange(0, 7);
//       assert(kind >= 0 && kind < 7);
//       switch (kind) {
//       case 0:
//         return gen::makeShared<AtomTRE>(gen::positive<Alphabet>());
//       case 1:
//         return gen::makeShared<EpsilonTRE>();
//       case 2:
//         return gen::makeShared<PlusTRE>(gen::arbitrary<std::shared_ptr<ObjectiveTRE>>());
//       case 3:
//         return gen::makeShared<ConcatTRE>(gen::arbitrary<std::shared_ptr<ObjectiveTRE>>(),
//                                           gen::arbitrary<std::shared_ptr<ObjectiveTRE>>());
//         //gen::set(&ConcatTRE::left), gen::set(&ConcatTRE::right));
//       case 4:
//         return gen::makeShared<DisjunctionTRE>(gen::arbitrary<std::shared_ptr<ObjectiveTRE>>(),
//                                                gen::arbitrary<std::shared_ptr<ObjectiveTRE>>());
// // gen::set(&DisjunctionTRE::left), gen::set(&DisjunctionTRE::right));
//       case 5:
//         return gen::makeShared<ConjunctionTRE>(gen::arbitrary<std::shared_ptr<ObjectiveTRE>>(),
//                                                gen::arbitrary<std::shared_ptr<ObjectiveTRE>>());
// // gen::set(&ConjunctionTRE::left), gen::set(&ConjunctionTRE::right));
//       case 6:
//         return gen::makeShared<WithinTRE>(gen::arbitrary<std::shared_ptr<ObjectiveTRE>>(),
// // gen::set(&WithinTRE::child), 
//                                           gen::makeShared<Interval>(gen::build<Bounds>,
//                                                                     gen::build<Bounds>));
//       default:
//         return gen::makeShared<AtomTRE>(gen::positive<Alphabet>());
//       }
//     }
//   };
}

std::shared_ptr<TRE> generate(std::shared_ptr<TRE> expr) {
  const int kind = *rc::gen::inRange(0, 7);
  assert(kind >= 0 && kind < 7);
  switch (kind) {
  case 0:
  case 1:
    return expr;
  case 2:
    return std::make_shared<TRE>(TRE::op::plus, expr);
  case 3:
    return std::make_shared<TRE>(TRE::op::concat, expr, (*rc::gen::arbitrary<AtomTRE>()).toTRE());
  case 4:
    return std::make_shared<TRE>(TRE::op::disjunction, expr, (*rc::gen::arbitrary<AtomTRE>()).toTRE());
  case 5:
    return std::make_shared<TRE>(TRE::op::conjunction, expr, (*rc::gen::arbitrary<AtomTRE>()).toTRE());
  case 6: {
    Bounds a = *rc::gen::construct<Bounds>(rc::gen::positive<double>(), rc::gen::arbitrary<bool>());
    Bounds b = *rc::gen::construct<Bounds>(rc::gen::positive<double>(), rc::gen::arbitrary<bool>());
    if (a <= b) {
      return std::make_shared<TRE>(TRE::op::within, expr,
                                   std::make_shared<Interval>(a, b));
    } else {
      return std::make_shared<TRE>(TRE::op::within, expr,
                                   std::make_shared<Interval>(b, a));
    }
  }
  default:
    return expr;
  }
}
