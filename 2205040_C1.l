%option noyywrap yylineno

%{
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

/* =============== critical: dictionary validation mode machine =============== */
enum Mode {
    EXP_KEY = 0,    // expect a key or '}'
    EXP_COLON = 1,  // expect ':'
    EXP_VALUE = 2,  // expect a value
    EXP_COMMA = 3   // expect ',' or '}'
};

vector<int> modeStack;
int mode = EXP_KEY;
int depth = 0;
int atStart = 1;
int completed = 0;

void fail(const string& msg) {
    cout << msg << endl;
    exit(0);
}

void ensureOpen() {
    if (completed || depth == 0) fail("Error: invalid dictionary key");
}

void handleString() {
    ensureOpen();
    if (mode == EXP_KEY) { mode = EXP_COLON; return; }
    if (mode == EXP_VALUE) { mode = EXP_COMMA; return; }
    if (mode == EXP_COMMA) fail("Error: missing pair separator");
    fail("Error: invalid dictionary key");
}

void handleInt() {
    ensureOpen();
    if (mode == EXP_KEY) { mode = EXP_COLON; return; }
    if (mode == EXP_VALUE) { mode = EXP_COMMA; return; }
    if (mode == EXP_COMMA) fail("Error: missing pair separator");
    fail("Error: invalid dictionary key");
}

void handleKeyword(const string& name) {
    ensureOpen();
    if (mode == EXP_VALUE) {
        if (name == "True" || name == "False" || name == "None") { mode = EXP_COMMA; return; }
        fail("Error: invalid dictionary key");
    }
    if (mode == EXP_COMMA) fail("Error: missing pair separator");
    fail("Error: invalid dictionary key");
}

void handleOpenBrace() {
    if (completed) fail("Error: invalid dictionary key");
    if (atStart) {
        atStart = 0;
        mode = EXP_KEY;
        depth = 1;
        return;
    }
    if (mode == EXP_VALUE) {
        modeStack.push_back(EXP_COMMA);
        mode = EXP_KEY;
        depth++;
        return;
    }
    fail("Error: invalid dictionary key");
}

void handleCloseBrace() {
    if (depth == 0) fail("Error: invalid dictionary key");
    if (mode != EXP_COMMA && mode != EXP_KEY) fail("Error: missing pair separator");
    depth--;
    if (depth == 0) {
        completed = 1;
        mode = EXP_COMMA;
    } else {
        mode = modeStack.back();
        modeStack.pop_back();
    }
}

%}

%x STRD STRS

%%

 /* =============== critical: flex rules driving dictionary validation =============== */
[ \t\n]+              ;
"{"                   { handleOpenBrace(); }
"}"                   { handleCloseBrace(); }
":"                   { ensureOpen(); if (mode == EXP_COLON) { mode = EXP_VALUE; } else fail("Error: missing pair separator"); }
","                   { ensureOpen(); if (mode == EXP_COMMA) { mode = EXP_KEY; } else fail("Error: missing pair separator"); }

-?[0-9]+              { handleInt(); }

[a-zA-Z_][a-zA-Z0-9_]* { handleKeyword(yytext); }

\"                    { BEGIN(STRD); }
\'                    { BEGIN(STRS); }

<STRD>{
    \\.               ;
    \"                { BEGIN(INITIAL); handleString(); }
    .                 ;
    \n                { fail("Error: unterminated string"); }
    <<EOF>>           { fail("Error: unterminated string"); }
}

<STRS>{
    \\.               ;
    \'                { BEGIN(INITIAL); handleString(); }
    .                 ;
    \n                { fail("Error: unterminated string"); }
    <<EOF>>           { fail("Error: unterminated string"); }
}

.                     { fail("Error: invalid dictionary key"); }

<<EOF>> {
    if (depth > 0) fail("Error: unmatched {");
    if (!atStart && completed) { cout << "Valid Python Dictionary" << endl; yyterminate(); }
    fail("Error: invalid dictionary key");
}

%%

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        cerr << "Cannot open input file: " << argv[1] << endl;
        return 1;
    }
    yylex();
    return 0;
}
