%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

vector<string> tagStack;

bool isSupported(const string& name) {
    return name == "book" || name == "title" || name == "author";
}

void report(const string& msg) {
    cout << msg << endl;
    exit(0);
}
%}

NAME    [a-zA-Z_][a-zA-Z0-9_]*

%%

 /* Match valid opening tags: <book>, <title>, <author> */
"<"{NAME}">" {
    string name = string(yytext + 1, yyleng - 2);
    if (!isSupported(name)) {
        report("Invalid XML: unsupported tag <" + name + ">");
    }
    tagStack.push_back(name);
}

 /* Match valid closing tags: </book>, </title>, </author> */
"</"{NAME}">" {
    string name = string(yytext + 2, yyleng - 3);
    if (!isSupported(name)) {
        report("Invalid XML: unsupported tag </" + name + ">");
    }
    if (tagStack.empty()) {
        report("Invalid XML: unexpected closing tag </" + name + ">");
    }
    if (tagStack.back() != name) {
        report("Invalid XML: tag mismatch </" + name + ">");
    }
    tagStack.pop_back();
}

 /* Catch malformed or unsupported tags ending with '>' */
"<"[^>\n]*">" {
    report("Invalid XML: unsupported tag " + string(yytext));
}

 /* Catch unclosed/truncated tags reaching newline or EOF */
"<"[^>\n]* {
    report("Invalid XML: unsupported tag " + string(yytext) + ">");
}

[^<]+   ; /* Ignore text content between tags */

<<EOF>> {
    if (!tagStack.empty()) {
        report("Invalid XML: missing closing tag for <" + tagStack.back() + ">");
    }
    cout << "Valid XML structure" << endl;
    yyterminate();
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