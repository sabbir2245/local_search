%option noyywrap yylineno

%{
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

/* =============== Global Variables =============== */
string detectedType = "";    // Stores the complete vector type
int angleBracketDepth = 0;   // Tracks nesting level of < and >
bool insideVector = false;   // Whether we're currently parsing a vector
bool validType = true;       // Whether the type is valid

/* =============== Constants =============== */
const vector<string> VALID_TYPES = {"int", "float", "double", "char"};

/* =============== Helper Functions =============== */
bool isValidBaseType(const string& type) {
    for (const string& t : VALID_TYPES) {
        if (t == type) return true;
    }
    return false;
}

void resetState() {
    detectedType = "";
    angleBracketDepth = 0;
    insideVector = false;
    validType = true;
}

void reportError(const string& msg) {
    cout << "Error: " << msg << endl;
    exit(1);
}

void reportSuccess(int lineNo, const string& lexeme) {
    cout << "Line No. " << lineNo << ": Token <VECTOR_TYPE> Lexeme " << lexeme << " found" << endl;
}

%}

%x IN_VECTOR
%x IN_TYPE
%x AFTER_VECTOR

%%

 /* =============== INITIAL State =============== */
<INITIAL>{
    /* Starting a vector declaration */
    "vector" {
        insideVector = true;
        detectedType = "vector";
        angleBracketDepth = 0;
        validType = true;
        BEGIN(IN_VECTOR);
    }

    /* Any other input is invalid */
    . {
        reportError("Invalid input: Expected vector declaration");
    }

    <<EOF>> {
        /* Empty input or whitespace only is considered invalid */
        reportError("Invalid input: No vector declaration found");
    }
}

 /* =============== IN_VECTOR State =============== */
<IN_VECTOR>{
    /* Opening angle bracket */
    "<" {
        angleBracketDepth++;
        detectedType += "<";
        
        /* If this is the first opening bracket, look for a type */
        if (angleBracketDepth == 1) {
            BEGIN(IN_TYPE);
        }
    }

    /* Closing angle bracket */
    ">" {
        if (angleBracketDepth == 0) {
            reportError("Unexpected '>' without matching '<'");
        }
        
        angleBracketDepth--;
        detectedType += ">";
        
        /* If we're back to depth 0, vector is complete */
        if (angleBracketDepth == 0) {
            /* Check if we have a valid type */
            if (!validType) {
                reportError("Invalid type inside vector");
            }
            
            reportSuccess(yylineno, detectedType);
            resetState();
            BEGIN(INITIAL);
        }
    }

    /* Type name (int, float, double, char) */
    [a-zA-Z]+ {
        if (angleBracketDepth > 0) {
            string typeName = yytext;
            
            /* Check if this is a valid base type */
            if (!isValidBaseType(typeName)) {
                validType = false;
            }
            
            detectedType += typeName;
            
            /* After reading the type, expect '>' or more nesting */
            BEGIN(AFTER_VECTOR);
        } else {
            reportError("Unexpected type outside angle brackets");
        }
    }

    /* Whitespace inside vector declaration - ignore */
    [ \t\n]+ { 
        /* Preserve whitespace in detected type for accurate lexeme */
        detectedType += yytext;
    }

    /* Invalid characters inside vector */
    . {
        reportError("Invalid character in vector declaration");
    }

    <<EOF>> {
        if (angleBracketDepth > 0) {
            reportError("Unclosed angle bracket");
        }
        reportError("Incomplete vector declaration");
    }
}

 /* =============== IN_TYPE State - looking for a type name =============== */
<IN_TYPE>{
    /* Type name */
    [a-zA-Z]+ {
        string typeName = yytext;
        
        /* Check if this is a valid base type */
        if (!isValidBaseType(typeName)) {
            validType = false;
        }
        
        detectedType += typeName;
        BEGIN(AFTER_VECTOR);
    }

    /* Nested vector - push back to IN_VECTOR to handle the vector keyword */
    "vector" {
        detectedType += "vector";
        /* We stay in IN_TYPE but will detect the nested vector */
        /* This handles vector<vector<int>> properly */
        BEGIN(IN_VECTOR);
    }

    /* Whitespace - ignore */
    [ \t\n]+ {
        detectedType += yytext;
    }

    /* Any other character is invalid */
    . {
        reportError("Invalid type name in vector");
    }

    <<EOF>> {
        reportError("Incomplete type declaration");
    }
}

 /* =============== AFTER_VECTOR State - after reading a type, expect > or more nesting =============== */
<AFTER_VECTOR>{
    /* Closing angle bracket */
    ">" {
        if (angleBracketDepth == 0) {
            reportError("Unexpected '>' without matching '<'");
        }
        
        angleBracketDepth--;
        detectedType += ">";
        
        /* If we're back to depth 0, vector is complete */
        if (angleBracketDepth == 0) {
            if (!validType) {
                reportError("Invalid type inside vector");
            }
            
            reportSuccess(yylineno, detectedType);
            resetState();
            BEGIN(INITIAL);
        }
    }

    /* Opening angle bracket for more nesting */
    "<" {
        angleBracketDepth++;
        detectedType += "<";
        BEGIN(IN_TYPE);
    }

    /* Whitespace - ignore */
    [ \t\n]+ {
        detectedType += yytext;
    }

    /* Any other character is invalid */
    . {
        reportError("Invalid character after type declaration");
    }

    <<EOF>> {
        if (angleBracketDepth > 0) {
            reportError("Unclosed angle bracket");
        }
        reportError("Incomplete vector declaration");
    }
}

 /* =============== Additional whitespace handling =============== */
[ \t\n]+ {
    /* Whitespace at top level - ignore */
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