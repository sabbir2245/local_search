%option noyywrap yylineno stack

%{
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

void fail(const string& msg) {
    cout << msg << endl;
    exit(0);
}
%}

%x DICT EXP_VAL STRD STRS

%%

[ \t\n]+   ;

 /* Root opening brace */
"{" { yy_push_state(DICT); }

 /* Inside dictionary expecting key or closing brace */
<DICT>{
    [ \t\n]+   ;
    "}" {
        yy_pop_state();
        if (YY_START == EXP_VAL) BEGIN(DICT);
    }
    \"   { yy_push_state(STRD); }
    \'   { yy_push_state(STRS); }
    -?[0-9]+ { BEGIN(EXP_VAL); }
    .    { fail("Error: invalid dictionary key"); }
}

 /* Colon separator between key and value */
<DICT,EXP_VAL>":" {
    if (YY_START == DICT) BEGIN(EXP_VAL);
    else fail("Error: missing pair separator");
}

 /* Expecting a value after colon */
<EXP_VAL>{
    [ \t\n]+   ;
    \"                  { yy_push_state(STRD); }
    \'                  { yy_push_state(STRS); }
    -?[0-9]+            { BEGIN(DICT); }
    True|False|None     { BEGIN(DICT); }
    "{"                 { yy_push_state(DICT); }
    .                   { fail("Error: invalid dictionary key"); }
}

 /* Comma separator between pairs */
<DICT>"," ;

 /* Double-quoted string handler */
<STRD>{
    \\.        ;
    \"         { yy_pop_state(); if (YY_START == EXP_VAL) BEGIN(DICT); }
    [^\"\n\\]+ ;
    \n         { fail("Error: unterminated string"); }
    <<EOF>>    { fail("Error: unterminated string"); }
}

 /* Single-quoted string handler */
<STRS>{
    \\.        ;
    \'         { yy_pop_state(); if (YY_START == EXP_VAL) BEGIN(DICT); }
    [^'\n\\]+  ;
    \n         { fail("Error: unterminated string"); }
    <<EOF>>    { fail("Error: unterminated string"); }
}

. { fail("Error: invalid dictionary key"); }

<<EOF>> {
    if (YY_START != INITIAL) fail("Error: unmatched {");
    cout << "Valid Python Dictionary" << endl;
    yyterminate();
}

%%

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    yyin = fopen(argv[1], "r");
    if (!yyin) return 1;
    yylex();
    return 0;
}