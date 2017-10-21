%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {MONAA}
%define parser_class_name {TREParser}

%code requires{
   #include <memory>
   #include "interval.hh"
   class TRE;
   class TREDriver;
   class TREScanner;
}

%parse-param { TREScanner  &scanner  }
%parse-param { TREDriver  &driver  }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   #include "tre_driver.hh"

#undef yylex
#define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

%token END 0  "end of file"
%token <char>        ATOM
%token <int>        INT
%token
  DISJUNCTION "|"
  CONJUNCTION "&"
  STAR        "*"
  PLUS        "+"
  WITHIN      "%"
  LPAREN      "("
  RPAREN      ")"
  COMMA       ","
;

%locations

%type <std::shared_ptr<TRE>>      expr
%type <std::shared_ptr<Interval>>      interval

%%

unit : expr END { driver.result = $1; }

/* The order matters */
expr : ATOM { $$ = std::make_shared<TRE>(TRE::op::atom, $1); }
     | LPAREN expr RPAREN { $$ = $2; }
     | expr PLUS { $$ = std::make_shared<TRE>(TRE::op::plus, $1); }
     | expr STAR { $$ = std::make_shared<TRE>(TRE::op::disjunction, 
                                              TRE::epsilon,
                                              std::make_shared<TRE>(TRE::op::plus, $1)); }
     | expr expr { $$ = std::make_shared<TRE>(TRE::op::concat, $1, $2); }
     | expr DISJUNCTION expr { $$ = std::make_shared<TRE>(TRE::op::disjunction, $1, $3); }
     | expr CONJUNCTION expr { $$ = std::make_shared<TRE>(TRE::op::conjunction, $1, $3); }
     | expr WITHIN interval { $$ = std::make_shared<TRE>(TRE::op::within, $1, $3); }
     ;

/* We support only open intervals now */
interval : LPAREN INT COMMA INT RPAREN { $$ = std::make_shared<Interval>($2, $4); }
%%



void MONAA::TREParser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
