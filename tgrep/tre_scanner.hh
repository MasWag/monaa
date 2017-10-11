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
  int yylex( tgrep::TREParser::semantic_type * const lval, 
             tgrep::TREParser::location_type *location );

private:
  tgrep::TREParser::semantic_type *yylval = nullptr;
};

