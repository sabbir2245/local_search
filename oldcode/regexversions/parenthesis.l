%option noyywrap yylineno

%{
#include <stdio.h>
#include <stdlib.h>

char stack[100];
int top = -1;

void push(char c) {
    top++;
    stack[top] = c;
}

char get_expected(char open) {
    if (open == '(') return ')';
    if (open == '[') return ']';
    return '}';
}

void report(const char* msg) {
    printf("%s\n", msg);
    exit(0);
}
%}

%x STR COM

%%

"/*"   { BEGIN(COM); }

<COM>{
    "*/"    { BEGIN(INITIAL); }
    \n      ;
    .       ;
    <<EOF>> { report("Unterminated comment"); }
}

\"     { BEGIN(STR); }

<STR>{
    \\\\    ;
    \\\"    ;
    \"      { BEGIN(INITIAL); }
    \n      ;
    .       ;
    <<EOF>> { report("Unterminated string"); }
}

"("|"["|"{"   { push(yytext[0]); }

")"|"]"|"}"   {
    char c = yytext[0];
    if (top == -1) {
        printf("Unexpected closing '%c'\n", c);
        exit(0);
    }
    char o = stack[top];
    char exp = get_expected(o);
    if (c != exp) {
        printf("Mismatch: expected '%c' but found '%c'\n", exp, c);
        exit(0);
    }
    top--;
}

[ \t\n]+  ;
[^ \t\n]  ;

<<EOF>> {
    if (top != -1) {
        printf("Unclosed '%c' at end of input\n", stack[top]);
    } else {
        printf("Balanced expression\n");
    }
    yyterminate();
}

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