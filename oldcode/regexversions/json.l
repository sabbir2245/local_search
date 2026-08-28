%option noyywrap yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mode_stack[100];
int stack_top = -1;

void push_mode(int mode) {
    stack_top++;
    mode_stack[stack_top] = mode;
}

int pop_mode() {
    if (stack_top == -1) return 0;
    int m = mode_stack[stack_top];
    stack_top--;
    return m;
}

void fail(const char* msg) {
    printf("%s\n", msg);
    exit(0);
}
%}

%x OBJ EXP_VAL STR

%%

[ \t\n]+   ;

"{" {
    push_mode(OBJ);
    BEGIN(OBJ);
}

<OBJ>{
    [ \t\n]+   ;
    "}" {
        pop_mode();
        if (stack_top == -1) {
            BEGIN(INITIAL);
        } else {
            BEGIN(mode_stack[stack_top]);
        }
    }
    \" { BEGIN(STR); }
    .  { fail("Error: invalid JSON key"); }
}

<OBJ,EXP_VAL>":" {
    if (YY_START == OBJ) {
        BEGIN(EXP_VAL);
    } else {
        fail("Error: missing pair separator");
    }
}

<EXP_VAL>{
    [ \t\n]+   ;
    -?[0-9]+(\.[0-9]+)? { BEGIN(OBJ); }
    true|false|null     { BEGIN(OBJ); }
    \"                  { BEGIN(STR); }
    "{"                 { push_mode(OBJ); BEGIN(OBJ); }
    .                   { fail("Error: invalid JSON key"); }
}

<OBJ>"," ;

<STR>{
    \\\"        ;
    [^\"\n\\]+  ;
    \"          { 
        if (stack_top >= 0 && mode_stack[stack_top] == OBJ) {
            BEGIN(OBJ);
        } else {
            BEGIN(EXP_VAL);
        }
    }
    \n          { fail("Error: unterminated string"); }
    <<EOF>>     { fail("Error: unterminated string"); }
}

. { fail("Error: invalid JSON key"); }

<<EOF>> {
    if (stack_top != -1) fail("Error: unmatched {");
    printf("Valid JSON Object\n");
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