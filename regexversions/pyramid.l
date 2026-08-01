%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

int expectedStars = -1;
int expectedPluses = -1;
bool patternStarted = false;

void reportError(const string& msg) {
    cout << "Error at line " << yylineno << ": " << msg << endl;
    exit(1);
}

void reportSuccess() {
    cout << "Pattern matched successfully!" << endl;
    exit(0);
}

void processPatternLine(int leftStars, int pluses, int rightStars) {
    if (leftStars != rightStars) {
        reportError("Stars on left and right not equal");
    }

    if (!patternStarted) {
        if (leftStars < 1 || pluses != 1) {
            reportError("First line must have at least one star on each side and exactly one plus");
        }
        expectedStars = leftStars - 1;
        expectedPluses = pluses + 2;
        patternStarted = true;
    } else {
        if (leftStars != expectedStars) {
            reportError("Stars on each side should be " + to_string(expectedStars) + 
                        " but found " + to_string(leftStars));
        }
        if (pluses != expectedPluses) {
            reportError("Pluses should be " + to_string(expectedPluses) + 
                        " but found " + to_string(pluses));
        }

        expectedStars--;
        expectedPluses += 2;
    }
}
%}

%%

 /* Match complete valid pyramid lines: Left Stars, Pluses, Right Stars */
^(\*+)(\++)(`\*+`)[\t ]*\n? {
    // Extract component counts directly via string analysis on yytext
    string text = yytext;
    if (!text.empty() && text.back() == '\n') text.pop_back();
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.pop_back();

    size_t firstPlus = text.find('+');
    size_t lastPlus = text.rfind('+');

    int leftStars = firstPlus;
    int pluses = lastPlus - firstPlus + 1;
    int rightStars = text.length() - lastPlus - 1;

    processPatternLine(leftStars, pluses, rightStars);
}

 /* Match final line containing only pluses */
^(\++)[\t ]*\n? {
    string text = yytext;
    if (!text.empty() && text.back() == '\n') text.pop_back();
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.pop_back();

    int pluses = text.length();

    if (!patternStarted) {
        reportError("Line must start with stars");
    }

    if (expectedStars != 0) {
        reportError("Pattern reached zero stars before final line");
    }

    if (pluses != expectedPluses) {
        reportError("Pluses should be " + to_string(expectedPluses) + 
                    " but found " + to_string(pluses));
    }

    expectedStars = -2; // Mark fully matched
}

 /* Catch empty lines */
^\n reportError("Empty line");

 /* Catch lines with invalid characters or illegal sequence */
^[^*+\n]+\n? reportError("Invalid character in pattern");

 /* Any non-matching pattern structure */
^[^\n]+\n? reportError("Invalid pattern format");

[ \t]+ ;

<<EOF>> {
    if (!patternStarted) {
        reportError("Empty file - no pattern found");
    }
    if (expectedStars != -2) {
        reportError("Pattern incomplete - missing final plus-only line");
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