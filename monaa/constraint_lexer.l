%{
#include <string>

/* Implementation of yyFlexScanner */ 
#include "constraint_scanner.hh"
#undef  YY_DECL
#define YY_DECL int ConstraintScanner::yylex( MONAA::Parametric::ConstraintParser::semantic_type * const lval, MONAA::Parametric::ConstraintParser::location_type *loc )

using token = MONAA::Parametric::ConstraintParser::token;

#define yyterminate() return( token::END )

#define YY_NO_UNISTD_H
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
%option nodefault
%option yyclass="ConstraintScanner"
%option noyywrap
%option c++
%option prefix="Constraint"
%option yylineno


%%
%{          /** Code executed at the beginning of yylex **/
            yylval = lval;
%}

"+"          return token::PLUS;
"-"          return token::MINUS;
"*"          return token::STAR;
"("          return token::LPAREN;
")"          return token::RPAREN;
"<"          return token::LT;
"<="         return token::LE;
">="         return token::GE;
">"          return token::GT;
"=="         return token::EQ;
","          return token::COMMA;
"{"          return token::BEGIN_TOKEN;
"}"          return token::END;

x[0-9]+ {
    yylval->build<int>( atoi(yytext + 1) );
    return token::CLOCK;
}

p[0-9]+ {
    yylval->build<int>( atoi(yytext + 1) );
    return token::PARAM;
}

[0-9]+ {
    yylval->build<int>( atoi(yytext) );
    return token::INT;
}

[[:blank:]]+
";"         return token::END;
"\n"         return token::END;

.	printf("bad input character '%s' at line %d\n", yytext, yylineno);

%%


