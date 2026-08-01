%option noyywrap yylineno stack

%{
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

bool isSupported(const string& cmd) {
    return cmd == "textbf" || cmd == "textit" || cmd == "section";
}

void report(const string& msg) {
    cout << "Error: " << msg << endl;
    exit(0);
}

void reportSuccess() {
    cout << "Valid LaTeX syntax" << endl;
    exit(0);
}
%}

NAME    [a-zA-Z]+

%x BRACE

%%

 /* Valid command followed by its required brace block */
"\\"{NAME}/"{" {
    string cmd = string(yytext + 1);
    if (!isSupported(cmd)) {
        report("unsupported command");
    }
}

 /* Opening brace of a command's block */
"{" {
    yy_push_state(BRACE);
}

 /* Naked closing brace without opening */
"}" {
    report("unmatched }");
}

 /* Command not immediately followed by '{' */
"\\"{NAME} {
    string cmd = string(yytext + 1);
    if (!isSupported(cmd)) {
        report("unsupported command");
    }
    report("command not followed by brace block");
}

 /* Malformed or empty command (e.g., '\', '\{', '\123') */
"\\"[^a-zA-Z]? {
    report("unsupported command");
}

 /* Inside LaTeX brace blocks */
<BRACE>{
    "{" {
        yy_push_state(BRACE); /* Push state for nested braces */
    }
    "}" {
        yy_pop_state(); /* Pop when closing brace matched */
    }
    "\\" {
        /* Catch trailing backslash inside braces before EOF */
    }
    [^\\{}]+ ; /* Ignore inner text */
    <<EOF>> {
        report("unmatched {");
    }
}

 /* Ignore standard ambient text */
[^\\{}]+ ;

<<EOF>> {
    if (YY_START != INITIAL) {
        report("unmatched {");
    }
    reportSuccess();
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