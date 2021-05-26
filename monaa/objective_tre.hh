#include "tre.hh"

class ObjectiveTRE {
public:
  virtual std::shared_ptr<TRE> toTRE() const = 0;
  virtual std::ostream &print(std::ostream &os) = 0;
  virtual ~ObjectiveTRE() {}
};

class AtomTRE final : public ObjectiveTRE {
public:
  Alphabet c;
  AtomTRE() = default;
  explicit AtomTRE(const Alphabet c) { this->c = c; }
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::atom, c);
  }
  virtual std::ostream &print(std::ostream &os) override {
    os << "AtomTRE: " << static_cast<int>(c);
    return os;
  }
};

class EpsilonTRE final : public ObjectiveTRE {
public:
  Alphabet c;
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::epsilon);
  }
  virtual std::ostream &print(std::ostream &os) override {
    os << "EpsilonTRE";
    return os;
  }
};

class UnaryTRE : public ObjectiveTRE {
public:
  std::shared_ptr<ObjectiveTRE> child;
};

class BinaryTRE : public ObjectiveTRE {
public:
  std::shared_ptr<ObjectiveTRE> left;
  std::shared_ptr<ObjectiveTRE> right;
};

class ConcatTRE final : public BinaryTRE {
public:
  ConcatTRE(std::shared_ptr<ObjectiveTRE> left,
            std::shared_ptr<ObjectiveTRE> right) {
    this->left = left;
    this->right = right;
  }
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::concat, left->toTRE(),
                                 right->toTRE());
  }
  virtual std::ostream &print(std::ostream &os) override {
    os << "ConcatTRE{ ";
    left->print(os);
    os << ", ";
    right->print(os);
    os << " }";
    return os;
  }
};

class DisjunctionTRE final : public BinaryTRE {
public:
  DisjunctionTRE(std::shared_ptr<ObjectiveTRE> left,
                 std::shared_ptr<ObjectiveTRE> right) {
    this->left = left;
    this->right = right;
  }
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::disjunction, left->toTRE(),
                                 right->toTRE());
  }
  virtual std::ostream &print(std::ostream &os) override {
    os << "DisjunctionTRE{ ";
    left->print(os);
    os << ", ";
    right->print(os);
    os << " }";
    return os;
  }
};

class ConjunctionTRE final : public BinaryTRE {
public:
  ConjunctionTRE(std::shared_ptr<ObjectiveTRE> left,
                 std::shared_ptr<ObjectiveTRE> right) {
    this->left = left;
    this->right = right;
  }
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::conjunction, left->toTRE(),
                                 right->toTRE());
  }
  virtual std::ostream &print(std::ostream &os) override {
    os << "ConjunctionTRE{ ";
    left->print(os);
    os << ", ";
    right->print(os);
    os << " }";
    return os;
  }
};

class PlusTRE final : public UnaryTRE {
public:
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::plus, child->toTRE());
  }
  PlusTRE(std::shared_ptr<ObjectiveTRE> child) { this->child = child; }
  virtual std::ostream &print(std::ostream &os) override {
    os << "PlusTRE{ ";
    child->print(os);
    os << " }";
    return os;
  }
};

class WithinTRE final : public UnaryTRE {
public:
  std::shared_ptr<Interval> interval;
  virtual std::shared_ptr<TRE> toTRE() const override {
    return std::make_shared<TRE>(TRE::op::within, child->toTRE(), interval);
  }
  WithinTRE(std::shared_ptr<ObjectiveTRE> child,
            std::shared_ptr<Interval> interval) {
    this->child = child;
    this->interval = interval;
  }
  virtual std::ostream &print(std::ostream &os) override {
    os << "WithinTRE{ ";
    child->print(os);
    os << " }(";
    os << interval->lowerBound;
    os << ",";
    os << interval->upperBound;
    os << ")";
    return os;
  }
};
