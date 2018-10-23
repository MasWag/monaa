#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "../build/constraint/constraint_parser.tab.hh"
#include "../build/constraint/location.hh"

class ConstraintScanner : public yyFlexLexer {
public:
   
  ConstraintScanner(std::istream *in) : yyFlexLexer(in) {};
  virtual ~ConstraintScanner() {};

  //get rid of override virtual function warning
  using FlexLexer::yylex;

  virtual
  int yylex( MONAA::Parametric::ConstraintParser::semantic_type * const lval, 
             MONAA::Parametric::ConstraintParser::location_type *location );

private:
  MONAA::Parametric::ConstraintParser::semantic_type *yylval = nullptr;
};

