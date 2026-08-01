%option noyywrap yylineno

%{
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

/* Stack to track brace nesting depth */
vector<int> braceStack;  // Stores the line number where each brace was opened

string currentCommand = "";
int braceDepth = 0;
bool commandPending = false;  // True if we just saw a command and need a brace block

/* Supported commands */
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

%x COMMAND
%x BRACE_BLOCK

%%

 /* =============== INITIAL State =============== */
<INITIAL>{
    /* Backslash indicates start of a command */
    "\\" {
        currentCommand = "";
        BEGIN(COMMAND);
    }

    /* Braces without command should be an error */
    "{" {
        report("unmatched {");
    }
    "}" {
        report("unmatched }");
    }

    /* Regular text content (anything else) */
    [^\\{}]+    ;  /* ignore text */

    <<EOF>> {
        if (commandPending) {
            report("command not followed by brace block");
        }
        if (braceDepth > 0) {
            report("unmatched {");
        }
        reportSuccess();
    }
}

 /* =============== COMMAND State - parsing command name =============== */
<COMMAND>{
    /* Command name: letters only (no numbers or underscores for simplicity) */
    [a-zA-Z]+ {
        currentCommand = yytext;
        commandPending = true;
        
        if (!isSupported(currentCommand)) {
            report("unsupported command");
        }
        
        BEGIN(INITIAL);
    }

    /* If we see a brace immediately after backslash, there's no command name */
    "{" {
        report("unsupported command");  // Empty command name
    }

    /* Any other character after backslash is invalid */
    . {
        report("unsupported command");
    }

    <<EOF>> {
        report("unsupported command");
    }
}

 /* =============== BRACE_BLOCK State - inside braces =============== */
<BRACE_BLOCK>{
    /* Opening brace increases depth */
    "{" {
        braceDepth++;
    }

    /* Closing brace decreases depth */
    "}" {
        braceDepth--;
        if (braceDepth == 0) {
            /* Exited the current brace block */
            commandPending = false;
            BEGIN(INITIAL);
        }
    }

    /* Backslash inside braces - could be nested command */
    "\\" {
        /* Start a nested command */
        currentCommand = "";
        BEGIN(COMMAND);
    }

    /* Any text inside braces */
    [^\\{}]+    ;  /* ignore text */

    <<EOF>> {
        report("unmatched {");
    }
}

 /* =============== Global Rules =============== */

/* After finishing a command, we should immediately enter BRACE_BLOCK */
<INITIAL>{
    /* This is the key: when commandPending is true and we see '{' */
    "{" {
        if (commandPending) {
            /* Enter the brace block for the command */
            braceDepth = 1;
            BEGIN(BRACE_BLOCK);
            commandPending = false;
        } else {
            /* Opening brace without command */
            report("unmatched {");
        }
    }

    /* If we see anything other than '{' when commandPending is true */
    [^\\{]+ {
        if (commandPending) {
            report("command not followed by brace block");
        }
        /* If no pending command, just ignore text */
    }
}

 /* =============== Catch-all =============== */
. {
    /* Should not reach here, but just in case */
    ;
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