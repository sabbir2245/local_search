%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

/* =============== Global Variables =============== */
int lineNumber = 1;
int expectedStars = -1;
int expectedPluses = -1;
bool firstLine = true;
string currentLine = "";

/* =============== Helper Functions =============== */
void reportError(const string& msg) {
    cout << "Error at line " << lineNumber << ": " << msg << endl;
    exit(1);
}

void reportSuccess() {
    cout << "Pattern matched successfully!" << endl;
    exit(0);
}

/* =============== State Machine Variables =============== */
enum ParseState {
    START_LINE,
    LEFT_STARS,
    PLUSES,
    RIGHT_STARS,
    DONE
};

void validatePyramidPattern(const string& line) {
    if (line.empty()) {
        reportError("Empty line");
        return;
    }
    
    ParseState state = START_LINE;
    int leftStars = 0, rightStars = 0, pluses = 0;
    
    for (char c : line) {
        switch (state) {
            case START_LINE:
            case LEFT_STARS:
                if (c == '*') {
                    leftStars++;
                    state = LEFT_STARS;
                } else if (c == '+') {
                    if (state == START_LINE) {
                        reportError("Line must start with stars");
                    }
                    pluses++;
                    state = PLUSES;
                } else {
                    reportError("Invalid character in pattern");
                }
                break;
                
            case PLUSES:
                if (c == '+') {
                    pluses++;
                } else if (c == '*') {
                    rightStars++;
                    state = RIGHT_STARS;
                } else {
                    reportError("Invalid character in pattern");
                }
                break;
                
            case RIGHT_STARS:
                if (c == '*') {
                    rightStars++;
                } else {
                    reportError("Invalid character after right stars");
                }
                break;
                
            case DONE:
                reportError("Extra characters after pattern");
                break;
        }
    }
    
    // Validate the pattern
    if (state == START_LINE) {
        reportError("Empty line");
    }
    
    if (leftStars != rightStars) {
        reportError("Stars on left and right not equal");
    }
    
    if (firstLine) {
        if (leftStars < 1 || pluses != 1) {
            reportError("First line must have at least one star on each side and exactly one plus");
        }
        expectedStars = leftStars - 1;
        expectedPluses = pluses + 2;
        firstLine = false;
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
        
        if (leftStars == 0) {
            // This should be the last line (all pluses)
            if (expectedStars != -1) {
                reportError("Pattern reached zero stars before final line");
            }
        }
    }
}

%}

%x LINE_CONTENT

%%

 /* =============== Main Rules =============== */
 /* Start of a new line - collect the entire line */
^ {
    currentLine = "";
    BEGIN(LINE_CONTENT);
}

<LINE_CONTENT>{
    /* Collect characters until end of line */
    [^*+\n]+ {
        reportError("Invalid characters in pattern");
    }
    
    [*+]  {
        currentLine += yytext;
    }
    
    \n {
        if (!currentLine.empty()) {
            validatePyramidPattern(currentLine);
            lineNumber++;
        }
        BEGIN(INITIAL);
    }
    
    <<EOF>> {
        if (!currentLine.empty()) {
            validatePyramidPattern(currentLine);
            lineNumber++;
        }
        if (firstLine) {
            reportError("Empty file - no pattern found");
        }
        if (expectedStars > 0 || expectedStars == -1) {
            reportError("Pattern incomplete - missing final plus-only line");
        }
        reportSuccess();
    }
}

 /* Handle EOF in INITIAL state */
<<EOF>> {
    if (firstLine) {
        reportError("Empty file - no pattern found");
    }
    if (expectedStars > 0 || expectedStars == -1) {
        reportError("Pattern incomplete - missing final plus-only line");
    }
    reportSuccess();
}

 /* Whitespace at top level - ignore */
[ \t]+    ;

 /* Any other characters at top level */
. {
    reportError("Invalid pattern format");
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