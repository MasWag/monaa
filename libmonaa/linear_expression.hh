#include <ppl.hh>
#include <iostream>

/*!
  @brief A class for a linear constraint

  This class is a wrapper of Parma_Polyhedra_Library::Linear_Expression so that coefficients can be float or mpq_class (rationals in GMP).
 */
class LinearExpression {
private:
  using Coefficient = Parma_Polyhedra_Library::Coefficient;
  Coefficient denominator;
  Parma_Polyhedra_Library::Linear_Expression expr;
public:
  LinearExpression(const Parma_Polyhedra_Library::Linear_Expression &expr) : expr(std::move(expr)) {
    denominator = 1;
  }
  template<class T>
  Parma_Polyhedra_Library::Constraint operator>(const T &d) const;
  template<class T>
  Parma_Polyhedra_Library::Constraint operator<(const T &d) const;
  template<class T>
  Parma_Polyhedra_Library::Constraint operator>=(const T &d) const;
  template<class T>
  Parma_Polyhedra_Library::Constraint operator<=(const T &d) const;
  template<class T>
  Parma_Polyhedra_Library::Constraint operator==(const T &d) const;
  template<class T>
  LinearExpression operator+(const T &d) const;
  template<class T>
  LinearExpression operator*(const T &d) const;
};

template<class T>
Parma_Polyhedra_Library::Constraint LinearExpression::operator>(const T &d) const {
  return expr > Parma_Polyhedra_Library::Linear_Expression(d) * denominator;
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator>(const mpq_class &_r) const {
  mpq_class r = _r;
  r.canonicalize();    
  LinearExpression tmpExpr = *this;
  Coefficient newDenominator;
  Parma_Polyhedra_Library::lcm_assign(newDenominator, denominator, r.get_den());
  tmpExpr.denominator = newDenominator;
  return tmpExpr.expr * (newDenominator / denominator) > r.get_num() * (newDenominator / r.get_den());
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator>(const double &d) const {
  mpq_class r(d);
  return *this > r;
}

template<class T>
Parma_Polyhedra_Library::Constraint LinearExpression::operator<(const T &d) const {
  return expr < Parma_Polyhedra_Library::Linear_Expression(d) * denominator;
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator<(const mpq_class &_r) const {
  mpq_class r = _r;
  r.canonicalize();    
  LinearExpression tmpExpr = *this;
  Coefficient newDenominator;
  Parma_Polyhedra_Library::lcm_assign(newDenominator, denominator, r.get_den());
  tmpExpr.denominator = newDenominator;
  return tmpExpr.expr * (newDenominator / denominator) < r.get_num() * (newDenominator / r.get_den());
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator<(const double &d) const {
  mpq_class r(d);
  return *this < r;
}

template<class T>
Parma_Polyhedra_Library::Constraint LinearExpression::operator>=(const T &d) const {
  return expr >= Parma_Polyhedra_Library::Linear_Expression(d) * denominator;
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator>=(const mpq_class &_r) const {
  mpq_class r = _r;
  r.canonicalize();    
  LinearExpression tmpExpr = *this;
  Coefficient newDenominator;
  Parma_Polyhedra_Library::lcm_assign(newDenominator, denominator, r.get_den());
  tmpExpr.denominator = newDenominator;
  return tmpExpr.expr * (newDenominator / denominator) >= r.get_num() * (newDenominator / r.get_den());
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator>=(const double &d) const {
  mpq_class r(d);
  return *this >= r;
}

template<class T>
Parma_Polyhedra_Library::Constraint LinearExpression::operator<=(const T &d) const {
  return expr <= Parma_Polyhedra_Library::Linear_Expression(d) * denominator;
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator<=(const mpq_class &_r) const {
  mpq_class r = _r;
  r.canonicalize();    
  LinearExpression tmpExpr = *this;
  Coefficient newDenominator;
  Parma_Polyhedra_Library::lcm_assign(newDenominator, denominator, r.get_den());
  tmpExpr.denominator = newDenominator;
  return tmpExpr.expr * (newDenominator / denominator) <= r.get_num() * (newDenominator / r.get_den());
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator<=(const double &d) const {
  mpq_class r(d);
  return *this <= r;
}

template<class T>
Parma_Polyhedra_Library::Constraint LinearExpression::operator==(const T &d) const {
  return expr == Parma_Polyhedra_Library::Linear_Expression(d) * denominator;
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator==(const mpq_class &_r) const {
  mpq_class r = _r;
  r.canonicalize();    
  LinearExpression tmpExpr = *this;
  Coefficient newDenominator;
  Parma_Polyhedra_Library::lcm_assign(newDenominator, denominator, r.get_den());
  tmpExpr.denominator = newDenominator;
  return tmpExpr.expr * (newDenominator / denominator) == r.get_num() * (newDenominator / r.get_den());
}
template<>
Parma_Polyhedra_Library::Constraint LinearExpression::operator==(const double &d) const {
  mpq_class r(d);
  return *this == r;
}

template<class T>
LinearExpression  LinearExpression::operator+(const T &d) const {
  LinearExpression tmpExpr = *this;
  tmpExpr.expr = tmpExpr.expr + Parma_Polyhedra_Library::Linear_Expression(d) * denominator;
  return tmpExpr;
}
template<>
LinearExpression LinearExpression::operator+(const mpq_class &_r) const {
  mpq_class r = _r;
  r.canonicalize();    
  LinearExpression tmpExpr = *this;
  Coefficient newDenominator;
  Parma_Polyhedra_Library::lcm_assign(newDenominator, denominator, r.get_den());
  tmpExpr.denominator = newDenominator;
  tmpExpr.expr = tmpExpr.expr * (newDenominator / denominator) + r.get_num() * (newDenominator / r.get_den());
  return tmpExpr;
}
template<>
LinearExpression  LinearExpression::operator+(const double &d) const {
  mpq_class r(d);
  r.canonicalize();
  return *this + r;
}
