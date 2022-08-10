%option noyywrap

%{

#include "../include/symbol_table.hpp" 
#include "../include/ast.hpp"
#include "compiler_parser.tab.hpp"

#include <string>
#include <iostream>
#include <regex>

inline int lineno = 1; 
extern void yyerror(char const *s);


std::string remove_f(std::string text){
    std::regex reg("f");
    return std::regex_replace(text, reg, "");
}

%}


%%

"int"       { return(TYPE_INT); }
"float"       { return(TYPE_FLOAT); }
"double"    {return(TYPE_DOUBLE);}


"for"       { return(FOR); }
"if"        { return(IF); }
"else"      { return(ELSE); }
"return"    { return(RETURN); }
"while"     { return(WHILE); }

"+"         {  yylval.val.ival = ADD; return '+'; }
"-"         {  yylval.val.ival = SUB; return '-'; }
"*"         { return '*'; }   
"/"         { return '/' ; }
"%"         { return '%' ; }
"++"        { yylval.val.ival = INC; return(OP_INC) ; }
"--"        { yylval.val.ival = DEC; return(OP_DEC) ; }

"&&"        { return(OP_AND) ; }
"||"        { return(OP_OR) ; }
"!"         { return '!' ; }

"&"        { return(BITOP_AND) ; }
"|"        { return(BITOP_OR) ; }
"^"         { return '^' ; }
"~"         { return '~' ; }
"<<"        { return(L_SHIFT) ; }
">>"        { return(R_SHIFT) ; }


"<"         { yylval.val.ival = LESS; return '<' ; }
"<="        { yylval.val.ival = LESS_EQUAL; return(OP_LTE) ; }
">"         { yylval.val.ival = GREATER;  return '>' ; }
">="        { yylval.val.ival = GREATER_EQUAL; return(OP_GTE) ; }
"=="        { yylval.val.ival = EQUAL; return(OP_EQ) ; }
"!="        { yylval.val.ival = NOT_EQUAL; return(OP_NEQ) ; }

"="         { return '=' ; }

"?"         { return '?' ; }
":"         { return ':' ; }

"&"         { return '&' ; }

";"         { return ';' ; }
","         { return ',' ; }
"("         { return '(' ; }
")"         { return ')' ; }
"["         { return '[' ; }
"]"         { return ']' ; }
"{"         { return '{' ; }
"}"         { return '}' ; }

[0-9]+           { yylval.val.ival = atoi(yytext); return(CONST_INT); }
[0-9]+[.][0-9]+  {yylval.val.fval = atof(yytext); return(CONST_FLOAT);}
0|[0-9]+[.][0-9]+"f"  {yylval.val.fval = stof(remove_f(yytext), 0); return(CONST_FLOAT);}
[0-9]+[.]?[0-9]*  {yylval.val.fval = atof(yytext); return(CONST_DOUBLE);}

"0x"[A-Fa-f0-9]+ { yylval.val.ival = strtoul(yytext, NULL, 16); return(CONST_INT); }

[a-zA-Z_][a-zA-Z0-9_]*  {insert(yytext, strlen(yytext), UNDEF, lineno);
                        yylval.symbol_table_item = lookup(yytext);
                        return(ID); }


"\n"              { lineno += 1; }

[ \t\r]+		      {}

.                 { yyerror("Invalid token\n"); }

%%




 

