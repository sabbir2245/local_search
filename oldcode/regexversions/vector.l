%option noyywrap yylineno stack

%{
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

string detectedType = "";
int matchLine = 1;

void reportError(const string& msg) {
    cout << "Error: " << msg << endl;
    exit(1);
}

void reportSuccess(int lineNo, const string& lexeme) {
    cout << "Line No. " << lineNo << ": Token <VECTOR_TYPE> Lexeme " << lexeme << " found" << endl;
}
%}

PRIM_TYPE   (int|float|double|char)

%x IN_VEC EXPECT_TYPE EXPECT_CLOSE

%%

 /* Root vector keyword trigger */
"vector"/[ \t]*"<" {
    detectedType = "vector";
    matchLine = yylineno;
    yy_push_state(IN_VEC);
}

 /* Inside vector context: expects opening angle bracket */
<IN_VEC>{
    [ \t\n]+ { detectedType += yytext; }
    "<" {
        detectedType += "<";
        BEGIN(EXPECT_TYPE);
    }
    . reportError("Invalid character in vector declaration");
    <<EOF>> reportError("Incomplete vector declaration");
}

 /* Expecting a type name or a nested vector */
<EXPECT_TYPE>{
    [ \t\n]+ { detectedType += yytext; }
    "vector"/[ \t]*"<" {
        detectedType += "vector";
        yy_push_state(IN_VEC);
    }
    {PRIM_TYPE} {
        detectedType += yytext;
        BEGIN(EXPECT_CLOSE);
    }
    [a-zA-Z]+ reportError("Invalid type inside vector");
    . reportError("Invalid character in vector declaration");
    <<EOF>> reportError("Incomplete type declaration");
}

 /* Expecting closing '>' or state unwinding */
<EXPECT_CLOSE>{
    [ \t\n]+ { detectedType += yytext; }
    ">" {
        detectedType += ">";
        yy_pop_state();
        
        if (YY_START == INITIAL) {
            reportSuccess(matchLine, detectedType);
            detectedType = "";
        } else {
            BEGIN(EXPECT_CLOSE);
        }
    }
    . reportError("Invalid character after type declaration");
    <<EOF>> reportError("Unclosed angle bracket");
}

[ \t\n]+ ;

. reportError("Invalid input: Expected vector declaration");

<<EOF>> {
    reportError("Invalid input: No vector declaration found");
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