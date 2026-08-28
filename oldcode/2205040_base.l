%option noyywrap yylineno

%{
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include "general.h"
#include "symbol_table.cpp"

using namespace std;

SymbolTable st(7);
ostringstream flog;
ofstream ftoken;
int errorCount = 0;

string buf;
int startLine = 0;

string decoded;
char charVal;
int charCount;

vector<pair<string, string>> kwTable = {
    {"if", "IF"}, {"else", "ELSE"}, {"for", "FOR"}, {"while", "WHILE"},
    {"do", "DO"}, {"break", "BREAK"}, {"continue", "CONTINUE"}, {"return", "RETURN"},
    {"int", "INT"}, {"float", "FLOAT"}, {"double", "DOUBLE"}, {"char", "CHAR"},
    {"void", "VOID"}, {"short", "SHORT"}, {"long", "LONG"}, {"static", "STATIC"},
    {"unsigned", "UNSIGNED"}, {"switch", "SWITCH"}, {"case", "CASE"},
    {"default", "DEFAULT"}, {"goto", "GOTO"}
};

char resolveEscape(char esc) {
    switch (esc) {
        case 'n':  return '\n'; case 't':  return '\t'; case 'r':  return '\r';
        case '0':  return '\0'; case '\\': return '\\'; case '\'': return '\'';
        case '"':  return '"';  case 'a':  return '\a'; case 'b':  return '\b';
        case 'f':  return '\f'; case 'v':  return '\v';
        default:   return esc;
    }
}

void printLog(const string& tokenType, const string& lexeme);
void printError(const string& msg);
void handleSymbolInsert(const string& name, const string& type);
void scanNumber();

%}

%x COMMENT STRING CHAR LINE_COMMENT

%%

[ \t]+                ;
\n                    ;

"//"                  { BEGIN(LINE_COMMENT); buf = "//"; }

<LINE_COMMENT>{
    \\\n              { buf += "\\\n"; }
    \n                { BEGIN(INITIAL); printLog("COMMENT", buf); }
    .                 { buf += yytext[0]; }
    <<EOF>>           { BEGIN(INITIAL); printLog("COMMENT", buf); }
}

"/*"                  { buf = "/*"; startLine = yylineno; BEGIN(COMMENT); }

<COMMENT>{
    "*/"              { buf += "*/"; BEGIN(INITIAL); printLog("COMMENT", buf); }
    <<EOF>>           { flog << "Error at line no " << startLine << ": Unterminated comment " << buf << endl << endl; errorCount++; BEGIN(INITIAL); }
    \n                { buf += '\n'; }
    .                 { buf += yytext[0]; }
}

\"                    { buf = "\""; decoded = ""; startLine = yylineno; BEGIN(STRING); }

<STRING>{
    \"                { buf += '"'; BEGIN(INITIAL);
        ftoken << "<STRING, " << decoded << "> ";
        flog << "Line no " << startLine << ": Token <STRING> Lexeme " << buf
             << " found --> <STRING, " << decoded << ">" << endl << endl;
    }
    \\n                { decoded += '\n'; buf += "\\n"; }
    \\t                { decoded += '\t'; buf += "\\t"; }
    \\r                { decoded += '\r'; buf += "\\r"; }
    \\0                { decoded += '\0'; buf += "\\0"; }
    \\\\               { decoded += '\\'; buf += "\\\\"; }
    \\\"               { decoded += '"';  buf += "\\\""; }
    \\\'               { decoded += '\''; buf += "\\\'"; }
    \\a                { decoded += '\a'; buf += "\\a"; }
    \\b                { decoded += '\b'; buf += "\\b"; }
    \\f                { decoded += '\f'; buf += "\\f"; }
    \\v                { decoded += '\v'; buf += "\\v"; }
    \\.                { decoded += yytext[1]; buf += yytext; }
    \\\n               { buf += "\\\n"; }
    \n                 { unput('\n'); BEGIN(INITIAL);
        flog << "Error at line no " << startLine << ": Unterminated string " << buf << endl << endl;
        errorCount++;
    }
    .                  { decoded += yytext[0]; buf += yytext[0]; }
    <<EOF>>            { BEGIN(INITIAL);
        flog << "Error at line no " << startLine << ": Unterminated string " << buf << endl << endl;
        errorCount++;
    }
}

\'                    { BEGIN(CHAR); buf = "'"; charVal = 0; charCount = 0; startLine = yylineno; }

<CHAR>{
    \\n                { charVal = '\n'; buf += "\\n"; charCount++; }
    \\t                { charVal = '\t'; buf += "\\t"; charCount++; }
    \\r                { charVal = '\r'; buf += "\\r"; charCount++; }
    \\0                { charVal = '\0'; buf += "\\0"; charCount++; }
    \\\\               { charVal = '\\'; buf += "\\\\"; charCount++; }
    \\\'               { charVal = '\''; buf += "\\\'"; charCount++; }
    \\\"               { charVal = '"';  buf += "\\\""; charCount++; }
    \\a                { charVal = '\a'; buf += "\\a"; charCount++; }
    \\b                { charVal = '\b'; buf += "\\b"; charCount++; }
    \\f                { charVal = '\f'; buf += "\\f"; charCount++; }
    \\v                { charVal = '\v'; buf += "\\v"; charCount++; }
    \\.                { charVal = yytext[1]; buf += yytext; charCount++; }
    \n                 { BEGIN(INITIAL);
        flog << "Error at line no " << startLine << ": Unterminated character " << buf << endl << endl;
        errorCount++;
    }
    \'                 { BEGIN(INITIAL); buf += '\'';
        if (charCount == 0) {
            flog << "Error at line no " << startLine << ": Empty character constant error ''" << endl << endl;
            errorCount++;
        } else if (charCount == 1) {
            string actualStr(1, charVal);
            ftoken << "<CONST_CHAR, " << actualStr << "> ";
            flog << "Line no " << startLine << ": Token <CONST_CHAR> Lexeme " << buf
                 << " found --> <CONST_CHAR, " << actualStr << ">" << endl << endl;
            handleSymbolInsert(buf, "CONST_CHAR");
        } else {
            flog << "Error at line no " << startLine << ": Multi character constant error " << buf << endl << endl;
            errorCount++;
        }
    }
    .                  { if (charCount == 0) charVal = yytext[0]; buf += yytext[0]; charCount++; }
    <<EOF>>            { BEGIN(INITIAL);
        flog << "Error at line no " << startLine << ": Unterminated character " << buf << endl << endl;
        errorCount++;
    }
}

"<="|">="|"=="|"!="|"&&"|"||"|"++"|"--" {
    string s = yytext;
    string ty;
    if (s == "<=" || s == ">=" || s == "==" || s == "!=") ty = "RELOP";
    else if (s == "&&" || s == "||") ty = "LOGICOP";
    else if (s == "++" || s == "--") ty = "INCOP";
    ftoken << "<" << ty << ", " << s << "> ";
    printLog(ty, s);
}

"+"|"-"|"*"|"/"|"%"|"="|"!"|"<"|">"|"("|")"|"{"|"}"|"["|"]"|","|";" {
    string ty;
    switch (yytext[0]) {
        case '+': case '-': ty = "ADDOP"; break;
        case '*': case '/': case '%': ty = "MULOP"; break;
        case '=': ty = "ASSIGNOP"; break;
        case '!': ty = "NOT"; break;
        case '<': case '>': ty = "RELOP"; break;
        case '(': ty = "LPAREN"; break;
        case ')': ty = "RPAREN"; break;
        case '{': ty = "LCURL"; break;
        case '}': ty = "RCURL"; break;
        case '[': ty = "LTHIRD"; break;
        case ']': ty = "RTHIRD"; break;
        case ',': ty = "COMMA"; break;
        case ';': ty = "SEMICOLON"; break;
    }
    string lexeme(1, yytext[0]);
    ftoken << "<" << ty << ", " << lexeme << "> ";
    printLog(ty, lexeme);
    if (yytext[0] == '{') st.EnterScope();
    else if (yytext[0] == '}') st.ExitScope();
}

[a-zA-Z_][a-zA-Z0-9_]* {
    string tok = yytext;
    bool isKw = false;
    for (auto& p : kwTable) {
        if (p.first == tok) {
            printLog(p.second, tok);
            ftoken << "<" << p.second << "> ";
            isKw = true;
            break;
        }
    }
    if (!isKw) {
        ftoken << "<ID, " << tok << "> ";
        printLog("ID", tok);
        handleSymbolInsert(tok, "ID");
    }
}

[0-9]                 { scanNumber(); }
"."[0-9]              { unput('.'); scanNumber(); }

.                     {
    string err(1, yytext[0]);
    printError("Unrecognized character " + err);
}

%%

/* ========== C++ helper implementations ========== */

void printLog(const string& tokenType, const string& lexeme) {
    flog << "Line no " << yylineno << ": Token <" << tokenType << "> Lexeme " << lexeme << " found" << endl << endl;
}

void printError(const string& msg) {
    flog << "Error at line no " << yylineno << ": " << msg << endl << endl;
    errorCount++;
}

void handleSymbolInsert(const string& name, const string& type) {
    auto lookup = st.LookupAll(name);
    if (lookup.first != -1) {
        flog << "< " << name << " : " << type << " > already exists in ScopeTable# "
             << st.getCurrentId() << " at position " << lookup.first << ", " << lookup.second << endl << endl;
    } else {
        st.Insert(name, type);
    }
    st.PrintAll(flog);
    flog << endl;
}

void scanNumber() {
    string buf;
    buf += yytext[0];

    bool isFloat = false;
    int dotCount = 0;
    bool hasExponent = false;
    string errorType;

    int c;
    while ((c = yyinput()) != EOF) {
        if (isdigit(c)) {
            buf += (char)c;

        } else if (c == '.') {
            buf += (char)c;
            if (hasExponent) {
                while ((c = yyinput()) != EOF && isdigit(c)) buf += (char)c;
                if (c != EOF) unput(c);
                errorType = "ill formed";
                break;
            }
            isFloat = true;
            dotCount++;
            if (dotCount > 1) { errorType = "multiple decimal"; break; }

        } else if ((c == 'E' || c == 'e') && !hasExponent) {
            buf += (char)c;
            hasExponent = true;
            isFloat = true;
            int d = yyinput();
            if (d == '+' || d == '-') buf += (char)d;
            else if (d != EOF) unput(d);

        } else if (c == '_' || (isalpha(c) && c != 'E' && c != 'e')) {
            buf += (char)c;
            while ((c = yyinput()) != EOF && (isalnum(c) || c == '.' || c == '_')) buf += (char)c;
            if (c != EOF) unput(c);
            errorType = "invalid suffix";
            break;

        } else {
            if (c != EOF) unput(c);
            break;
        }
    }

    if (!errorType.empty()) {
        while ((c = yyinput()) != EOF && !isspace(c) && c != '\n') buf += (char)c;
        if (c != EOF) unput(c);
        if (errorType == "multiple decimal") printError("Too many decimal points " + buf);
        else if (errorType == "ill formed") printError("Ill formed number " + buf);
        else printError("Invalid prefix on ID or invalid suffix on Number " + buf);
    } else {
        string type = isFloat ? "CONST_FLOAT" : "CONST_INT";
        ftoken << "<" << type << ", " << buf << "> ";
        printLog(type, buf);
        handleSymbolInsert(buf, type);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string baseName = "2205040";
    string tokenFile = baseName + "_token.txt";
    string logFile = baseName + "_log.txt";

    ifstream fin(inputFile);
    if (!fin) { cerr << "Cannot open input file: " << inputFile << endl; return 1; }

    string content((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    ftoken.open(tokenFile);

    yy_scan_string(content.c_str());
    yylex();

    flog << "Total lines: " << yylineno << endl;
    flog << "Total errors: " << errorCount << endl;

    ofstream fout(logFile);
    fout << flog.str();
    fout.close();
    ftoken.close();

    return 0;
}
