/* -*- mode: bison; -*- */
%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {MONAA}
%define api.parser.class {TREParser}

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
  LSQPAREN    "["
  RSQPAREN    "]"
  COMMA       ","
  GT          ">"
  GE          ">="
  LE          "<="
  LT          "<"
  EQ          "="
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
         | LSQPAREN INT COMMA INT RPAREN { $$ = std::make_shared<Interval>(Bounds($2, 1), Bounds($4, 0)); }
         | LPAREN INT COMMA INT RSQPAREN { $$ = std::make_shared<Interval>(Bounds($2, 0), Bounds($4, 1)); }
         | LSQPAREN INT COMMA INT RSQPAREN { $$ = std::make_shared<Interval>(Bounds($2, 1), Bounds($4, 1)); }
         | LT INT { $$ = std::make_shared<Interval>(Bounds(0, 1), Bounds($2, 0)); }
         | LE INT { $$ = std::make_shared<Interval>(Bounds(0, 1), Bounds($2, 1)); }
         | EQ INT { $$ = std::make_shared<Interval>(Bounds($2, 1), Bounds($2, 1)); }
         | GE INT { $$ = std::make_shared<Interval>(Bounds($2, 1), Bounds(std::numeric_limits<double>::infinity(), 0)); }
         | GT INT { $$ = std::make_shared<Interval>($2); }
         | LPAREN interval RPAREN { $$ = $2; }
%%



void MONAA::TREParser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
