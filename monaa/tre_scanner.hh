#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "tre_parser.tab.hh"
#include "location.hh"

class TREScanner : public yyFlexLexer {
public:
   
  TREScanner(std::istream *in) : yyFlexLexer(in) {};
  virtual ~TREScanner() {};

  //get rid of override virtual function warning
  using FlexLexer::yylex;

  virtual
  int yylex( MONAA::TREParser::semantic_type * const lval, 
             MONAA::TREParser::location_type *location );

private:
  MONAA::TREParser::semantic_type *yylval = nullptr;
};

