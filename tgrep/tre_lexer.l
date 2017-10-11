%{
#include <string>

/* Implementation of yyFlexScanner */ 
#include "tre_scanner.hh"
#undef  YY_DECL
#define YY_DECL int TREScanner::yylex( tgrep::TREParser::semantic_type * const lval, tgrep::TREParser::location_type *loc )

using token = tgrep::TREParser::token;

#define yyterminate() return( token::END )

#define YY_NO_UNISTD_H
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
%option nodefault
%option yyclass="TREScanner"
%option noyywrap
%option c++

%%
%{          /** Code executed at the beginning of yylex **/
            yylval = lval;
%}

"|"          return token::DISJUNCTION;
"&"          return token::CONJUNCTION;
"*"          return token::STAR;
"+"          return token::PLUS;
"%"          return token::WITHIN;
"("          return token::LPAREN;
")"          return token::RPAREN;
","          return token::COMMA;

[a-zA-Z] {
    yylval->build<char>( yytext[0] );
    return token::ATOM;
}

[0-9]+ {
    yylval->build<int>( atoi(yytext) );
    return token::INT;
}

"\n"         return token::END;

%%


