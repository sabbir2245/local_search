%option noyywrap yylineno

%{
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

vector<string> tagStack;
string tagName;
string tagBuf;

bool isSupported(const string& name) {
    return name == "book" || name == "title" || name == "author";
}

void report(const string& msg) {
    cout << msg << endl;
    exit(0);
}

%}

%x TAG

%%

 /* =============== critical: tag matching with a stack =============== */
<INITIAL><<EOF>> {
    if (!tagStack.empty()) {
        report("Invalid XML: missing closing tag for <" + tagStack.back() + ">");
    } else {
        report("Valid XML structure");
    }
}

"<"                 { tagBuf = "<"; tagName = ""; BEGIN(TAG); }

<TAG>{
    "/"             { tagBuf += "/"; }
    [a-zA-Z_][a-zA-Z0-9_]* { tagBuf += yytext; tagName = yytext; }
    [ \t\n]         ;
    ">"             {
        BEGIN(INITIAL);
        tagBuf += ">";
        if (tagName.empty()) { report("Invalid XML: unsupported tag " + tagBuf); }
        if (tagBuf.find('/') == string::npos) {
            // opening tag
            if (!isSupported(tagName)) { report("Invalid XML: unsupported tag " + tagBuf); }
            tagStack.push_back(tagName);
        } else {
            // closing tag
            if (!isSupported(tagName)) { report("Invalid XML: unsupported tag " + tagBuf); }
            if (tagStack.empty()) { report("Invalid XML: unexpected closing tag " + tagBuf); }
            if (tagStack.back() != tagName) { report("Invalid XML: tag mismatch " + tagBuf); }
            tagStack.pop_back();
        }
    }
    .               { tagBuf += ">"; report("Invalid XML: unsupported tag " + tagBuf); }
    <<EOF>>         { tagBuf += ">"; report("Invalid XML: unsupported tag " + tagBuf); }
}

[^<]+               ;   /* ignore text content */
.                   ;

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
