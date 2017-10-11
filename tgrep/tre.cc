#include <ostream>
#include "tre.hh"

std::ostream& 
operator<<( std::ostream &stream, const TRE& expr)
{
  switch(expr.tag) {
  case TRE::op::atom:
    stream << expr.c;
    break;
  case TRE::op::epsilon:
    stream << '@';
    break;
  case TRE::op::plus:
    stream << "(" << *(expr.regExpr) << "+)";
    break;
  case TRE::op::concat:
    stream << "(" << *(expr.regExprPair.first) << *(expr.regExprPair.second) << ")";
    break;
  case TRE::op::disjunction:
    stream << "(" << *(expr.regExprPair.first) << "|" << *(expr.regExprPair.second) << ")";
    break;
  case TRE::op::conjunction:
    stream << "(" << *(expr.regExprPair.first) << "&" << *(expr.regExprPair.second) << ")";
    break;
  case TRE::op::within:
    stream << "(" << *(expr.regExprWithin.first) << "%" << *(expr.regExprWithin.second) << ")";
    break;
  }

  return stream;
}

const std::shared_ptr<TRE> TRE::epsilon = std::make_shared<TRE>(op::epsilon);
