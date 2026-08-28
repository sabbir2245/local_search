%option noyywrap yylineno

%{
#include <stdio.h>

void print_num(const char* type, const char* lexeme) {
    printf("<%s, %s>\n", type, lexeme);
}

void report_error(const char* type, const char* lexeme) {
    printf("Error: Invalid %s constant %s\n", type, lexeme);
}
%}

HEX_VALID   0[xX][0-9a-fA-F]+
BIN_VALID   0[bB][01]+
OCT_VALID   0[0-7]+
INT_VALID   [0-9]+
FLOAT_VALID [0-9]+\.[0-9]+

HEX_ERR     0[xX][a-zA-Z0-9]*
BIN_ERR     0[bB][a-zA-Z0-9]*
OCT_ERR     0[0-9]+

%%

{HEX_VALID}     { print_num("CONST_HEX", yytext); }
{BIN_VALID}     { print_num("CONST_BIN", yytext); }
{FLOAT_VALID}   { print_num("CONST_FLOAT", yytext); }
{OCT_VALID}     { print_num("CONST_OCT", yytext); }
{INT_VALID}     { print_num("CONST_INT", yytext); }

{HEX_ERR}       { report_error("hexadecimal", yytext); }
{BIN_ERR}       { report_error("binary", yytext); }
{OCT_ERR}       { report_error("octal", yytext); }

[ \t\n]+        ;
.               ;

%%

int main(int argc, char* argv[])
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
            return 1;
        }
    }
    yylex();
    return 0;
}