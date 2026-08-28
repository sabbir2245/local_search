%option noyywrap yylineno

%{
#include <stdio.h>

int start_line = 0;
%}

%x HCOM STR

%%

"<!--"     { start_line = yylineno; BEGIN(HCOM); }

<HCOM>{
    "-->"   { 
        printf("HTML comment from line %d to line %d\n", start_line, yylineno); 
        BEGIN(INITIAL); 
    }
    \n      ;
    .       ;
    <<EOF>> { 
        printf("Error at line %d: Unterminated HTML comment\n", start_line); 
        yyterminate(); 
    }
}

\"         { BEGIN(STR); }

<STR>{
    \\\\    ;
    \\\"    ;
    \"      { BEGIN(INITIAL); }
    \n      ;
    .       ;
}

[ \t\n]+   ;
.          ;

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