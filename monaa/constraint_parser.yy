%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {MONAA::Parametric}
%define parser_class_name {ConstraintParser}

%code requires{
   #include <memory>
   #include <ppl.hh>
   class ConstraintDriver;
   class ConstraintScanner;
}

%parse-param { ConstraintScanner  &scanner  }
%parse-param { ConstraintDriver  &driver  }
%parse-param { const std::size_t paramDimensions } 

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   #include "constraint_driver.hh"

#undef yylex
#define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

%token END 0  "end of file"
%token BLANK
%token <int>       INT
%token <int>       CLOCK
%token <int>       PARAM
%token
  PLUS        "+"
  MINUS       "-"
  STAR        "*"
  LPAREN      "("
  RPAREN      ")"
  LT          "<"
  LE          "<="
  GE          ">="
  GT          ">"
  EQ          "=="
  COMMA       ","
  BEGIN_TOKEN "{"
;

%locations

%type <Parma_Polyhedra_Library::Constraint_System>   constraint_system
%type <Parma_Polyhedra_Library::Constraint>          constraint
%type <Parma_Polyhedra_Library::Linear_Expression>   expr
%type <std::size_t>                                  variable

%%

unit : BEGIN_TOKEN constraint_system END { driver.result = $2; }

constraint_system : constraint { $$ = Parma_Polyhedra_Library::Constraint_System($1);}
                  | constraint_system COMMA constraint { $1.insert($3); $$ = std::move($1);}

/* The order matters */
constraint : expr LT expr { $$ = $1 < $3; }
           | expr LT INT { $$ = $1 < $3; }
           | INT LT expr { $$ = $1 < $3; }
           | expr GT expr { $$ = $1 > $3; }
           | expr GT INT { $$ = $1 > $3; }
           | INT GT expr { $$ = $1 > $3; }
           | expr LE expr { $$ = $1 <= $3; }
           | expr LE INT { $$ = $1 <= $3; }
           | INT LE expr { $$ = $1 <= $3; }
           | expr GE expr { $$ = $1 >= $3; }
           | expr GE INT { $$ = $1 >= $3; }
           | INT GE expr { $$ = $1 >= $3; }
           | expr EQ expr { $$ = $1 == $3; }
           | expr EQ INT { $$ = $1 == $3; }
           | INT EQ expr { $$ = $1 == $3; }

expr : variable { $$ = Parma_Polyhedra_Library::Variable($1); }
     | expr STAR INT { $$ = $1 * $3; }
     | INT STAR expr { $$ = $1 * $3; }
     | PLUS expr { $$ = +$2; }
     | expr PLUS expr { $$ = $1 + $3; }
     | INT PLUS expr { $$ = $1 + $3; }
     | expr PLUS INT { $$ = $1 + $3; }
     | MINUS expr { $$ = -$2; }
     | expr MINUS expr { $$ = $1 - $3; }
     | INT MINUS expr { $$ = $1 - $3; }
     | expr MINUS INT { $$ = $1 - $3; }



variable : PARAM { $$ = 1 + $1; }
         | CLOCK { $$ = 1 + paramDimensions + $1; }
%%



void MONAA::Parametric::ConstraintParser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
