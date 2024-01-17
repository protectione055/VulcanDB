%{
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "express_defs.h"

struct ParserContext;

#include "yacc_exp.tab.h"
extern int atoi();
extern double atof();

#if YYDEBUG > 0
#define debug_printf  printf
#else
#define debug_printf(...)
#endif // YYDEBUG

#define RETURN_TOKEN(token) debug_printf("%s\n",#token);return token
%}

/* Prevent the need for linking with -lfl */
%option noyywrap
%option bison-bridge
%option reentrant

WHITE_SAPCE      [\ \t\b\f]
DIGIT            [0-9]+
ID               [A-Za-z_]+[A-Za-z0-9_]*
DOT              \.
QUOTE            [\'\"]
ENTITY_NAME      "Ifc"[A-Za-z0-9]+
LABEL            [A-Za-z0-9_]+
QUOTED_STRING    {QUOTE}[\0-\46\50-\177]*{QUOTE}
EXPRESSION       ([\-]?[\+\-\*\/\(\)A-Za-z0-9\._]+[\+\-\*/]+[A-Za-z0-9\._]+[\(\)A-Za-z0-9\.\+\-\*/_]*)|(\-+[\(\)A-Za-z0-9\._]+)

%x STR IGNORE COMMENT

%%

{WHITE_SAPCE}                                       ;// ignore whitespace
"\(\*"                                              BEGIN(COMMENT); // ignore comments
<COMMENT>"\*\)"                                     BEGIN(INITIAL); // ignore comments
<COMMENT>.|\n                                       ;

[Ii][Nn][Vv][Ee][Rr][Ss][Ee]	                    BEGIN(IGNORE);
[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]	                BEGIN(IGNORE);
[Rr][Uu][Ll][Ee]	                                BEGIN(IGNORE);
[Ww][Hh][Ee][Rr][Ee]	                            BEGIN(IGNORE);
[Dd][Ee][Rr][Ii][Vv][Ee]	                        BEGIN(IGNORE);
<IGNORE>"END_TYPE"                                  BEGIN(INITIAL); RETURN_TOKEN(END_TYPE);  
<IGNORE>"END_ENTITY"                                BEGIN(INITIAL); RETURN_TOKEN(END_ENTITY);
<IGNORE>"END_FUNCTION;"                             BEGIN(INITIAL); 
<IGNORE>"END_RULE;"                                 BEGIN(INITIAL); 
<IGNORE>.|\n                                        ;


\n                                                   ;

[\-]?{DIGIT}+                                       yylval->integer=atoi(yytext); RETURN_TOKEN(INTEGER_VALUE);

{ENTITY_NAME}                                       yylval->string=strdup(yytext); RETURN_TOKEN(ENTITY_NAME);


[Ss][Cc][Hh][Ee][Mm][Aa]                            RETURN_TOKEN(SCHEMA);
[Tt][Yy][Pp][Ee]	                                RETURN_TOKEN(TYPE);
[Ee][Nn][Tt][Ii][Tt][Yy]	                        RETURN_TOKEN(ENTITY);
[Ee][Nn][Dd][_][Tt][Yy][Pp][Ee]	                    RETURN_TOKEN(END_TYPE);
[Aa][Bb][Ss][Tt][Rr][Aa][Cc][Tt]	                RETURN_TOKEN(ABSTRACT);
[Ss][Uu][Pp][Ee][Rr][Tt][Yy][Pp][Ee]	            RETURN_TOKEN(SUPERTYPE);
[Ss][Uu][Bb][Tt][Yy][Pp][Ee]	                    RETURN_TOKEN(SUBTYPE);
[Ee][Nn][Dd][_][Ee][Nn][Tt][Ii][Tt][Yy]	            RETURN_TOKEN(END_ENTITY);
[Oo][Ff]	                                        RETURN_TOKEN(OF);
[Oo][Nn][Ee][Oo][Ff]	                            RETURN_TOKEN(ONEOF);
[Ff][Oo][Rr]	                                    RETURN_TOKEN(FOR);
[Uu][Nn][Ii][Qq][Uu][Ee]	                        RETURN_TOKEN(UNIQUE);
[Ee][Nn][Dd][_][Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]	    RETURN_TOKEN(END_FUNCTION);
[Ee][Nn][Dd][_][Rr][Uu][Ll][Ee]	                    RETURN_TOKEN(END_RULE);
[Ff][Ii][Xx][Ee][Dd]	                            RETURN_TOKEN(FIXED);
[Ee][Nn][Dd][_][Ss][Cc][Hh][Ee][Mm][Aa]	            RETURN_TOKEN(END_SCHEMA);
[Ss][Ee][Ll][Ff]	                                RETURN_TOKEN(SELF);
[Oo][Pp][Tt][Ii][Oo][Nn][Aa][Ll]	                RETURN_TOKEN(OPTIONAL);

[Rr][Ee][Aa][Ll]	                                RETURN_TOKEN(REAL);
[Bb][Oo][Oo][Ll][Ee][Aa][Nn]	                    RETURN_TOKEN(BOOLEAN);
[Ss][Tt][Rr][Ii][Nn][Gg]	                        RETURN_TOKEN(STRING);
[Ii][Nn][Tt][Ee][Gg][Ee][Rr]	                    RETURN_TOKEN(INTEGER);
[Nn][Uu][Mm][Bb][Ee][Rr]	                        RETURN_TOKEN(NUMBER);
[Ee][Nn][Uu][Mm][Ee][Rr][Aa][Tt][Ii][Oo][Nn]	    RETURN_TOKEN(ENUMERATION);
[Ll][Ii][Ss][Tt]	                                RETURN_TOKEN(LIST);
[Ss][Ee][Tt]	                                    RETURN_TOKEN(SET);
[Ss][Ee][Ll][Ee][Cc][Tt]	                        RETURN_TOKEN(SELECT);
[Ll][Oo][Gg][Ii][Cc][Aa][Ll]	                    RETURN_TOKEN(LOGICAL);
[Aa][Rr][Rr][Aa][Yy]	                            RETURN_TOKEN(ARRAY);
[Bb][Ii][Nn][Aa][Rr][Yy]	                        RETURN_TOKEN(BINARY);

";"                                                  RETURN_TOKEN(SEMICOLON);
{DOT}                                                RETURN_TOKEN(DOT);
"*"                                                  RETURN_TOKEN(STAR);
"["                                                  RETURN_TOKEN(LBRACKET);
"]"                                                  RETURN_TOKEN(RBRACKET);
"||"                                                 RETURN_TOKEN(OR);
":="                                                 RETURN_TOKEN(COLON_EQ);
"("                                                  RETURN_TOKEN(LBRACE);
")"                                                  RETURN_TOKEN(RBRACE);
","                                                  RETURN_TOKEN(COMMA);
"="                                                  RETURN_TOKEN(EQ);
"<="                                                 RETURN_TOKEN(LE);
"<>"                                                 RETURN_TOKEN(NE);
"!="                                                 RETURN_TOKEN(NE);
"<"                                                  RETURN_TOKEN(LT);
">="                                                 RETURN_TOKEN(GE);
">"                                                  RETURN_TOKEN(GT);
"?"                                                  RETURN_TOKEN(QUESTION);
":"                                                  RETURN_TOKEN(COLON);
[Aa][Nn][Dd]	                                     RETURN_TOKEN(AND);
[Ii][Nn]	                                         RETURN_TOKEN(IN);

{QUOTED_STRING}                                     yylval->string=strdup(yytext); RETURN_TOKEN(SSS);
{EXPRESSION}                                        yylval->string=strdup(yytext);RETURN_TOKEN(EXPRESSION);

{LABEL}                                             yylval->string=strdup(yytext); RETURN_TOKEN(LABEL);

.                                                    printf("Unknown character [%c]\n",yytext[0]); return yytext[0];
%%

void scan_string(const char *str, yyscan_t scanner) {
  yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
}
