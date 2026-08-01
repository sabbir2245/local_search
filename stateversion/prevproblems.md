# Previous Problems: A1, B1, C1 (Statements + Solutions)

Combined problem statements and complete solutions for the three online labs.
Each solution has been compiled with `flex 2.6.4` and `g++` and tested.

---

# A1: Yet Another Tag Matching (XML)

## Problem Statement

Validate the proper nesting of tags in a simplified XML-like document. Each opening tag
must have a corresponding closing tag, and the tags must be closed in the correct order.

Supported tags (only these are allowed):

- `<book>` and `</book>`
- `<title>` and `</title>`
- `<author>` and `</author>`

Requirements:

- Detect and validate the supported opening and closing tags.
- Each closing tag must match the most recently encountered unmatched opening tag
  (use a stack).
- Ignore all text content between tags.
- Any tag other than `book`, `title`, `author` is reported as **unsupported**.
- A closing tag that does not match the most recent opening tag is a **tag mismatch**.
- A closing tag without a corresponding opening tag is an **unexpected closing tag**.
- Unmatched opening tags at end of input are reported as a **missing closing tag**.
- Stop parsing immediately after detecting the first error.

## Sample Input / Output

| Input | Output |
|---|---|
| `<book><title>Compiler Design</title><author>A. Aho</author></book>` | `Valid XML structure` |
| `<book><title>Compiler Design</author></book>` | `Invalid XML: tag mismatch </author>` |
| `<book><title>Compiler Design</title>` | `Invalid XML: missing closing tag for <book>` |
| `<book><publisher>ABC Press</publisher></book>` | `Invalid XML: unsupported tag <publisher>` |
| `<book></author></book>` | `Invalid XML: tag mismatch </author>` |

## Solution (`2205040_A1.l`)

```flex
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

 /* tag matching with a stack */
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
```

---

# B1: Oct, Hex, and Lua

## Problem Statement

Extend the lexical analyzer from the offline lexical-analysis assignment with two features.
All previously specified tokenization, symbol-table, line-counting, and error-reporting
rules remain unchanged.

### 1. Octal and Hexadecimal Integer Constants

- An octal constant starts with `0`, has at least one additional digit, and all following
  digits are in `0-7`. Examples: `00`, `017`, `0755`.
- A single `0` continues to be recognized as `CONST_INT`.
- A hexadecimal constant starts with `0x` or `0X` and has at least one hex digit from
  `0-9`, `a-f`, `A-F`. Examples: `0x2A`, `0Xff`, `0x10B7`.

Token formats: `0755 -> <CONST_OCT, 0755>`, `0x2A -> <CONST_HEX, 0x2A>`.

Log format:
```
Line no 3: Token <CONST_OCT> Lexeme 0755 found
Line no 4: Token <CONST_HEX> Lexeme 0x2A found
```

Error checking:

- `Line no <n>: Invalid octal constant <lexeme>` for numbers starting with `0` that contain
  `8` or `9` (e.g. `078`, `0197`, `0089`).
- `Line no <n>: Invalid hexadecimal constant <lexeme>` when `0x`/`0X` is not followed by a
  hex digit, or an alphanumeric continuation has a non-hex character (e.g. `0x`, `0XG1`, `0x2AZ`).
- An invalid numeric lexeme is consumed and reported as one error; it is not split into
  valid tokens, no token is written, and octal/hex constants are **not** inserted into the
  symbol table.

### 2. Lua-Style Comments

- Multiline Lua comment starts with `--[[` and ends at the first following `]]`. It may span
  multiple lines.
- Lua comments produce no token and are not inserted into the symbol table.
- Log message: `Line no <start>: Lua multiline comment ending at line <end> found`
- Comment delimiters inside string literals or character constants are ordinary characters.
- Error: `Line no <start>: Unfinished Lua multiline comment` if EOF is reached before the
  comment closes (counted in the total error count).

## Sample I/O

Sample 1 (valid):
```
int permission = 0755;
int mask = 0x2Af;
--[[ Lua comment
spanning two lines ]]
```
Token file: `<CONST_OCT, 0755> <CONST_HEX, 0x2Af>`
Log file:
```
Line no 1: Token <CONST_OCT> Lexeme 0755 found
Line no 2: Token <CONST_HEX> Lexeme 0x2Af found
Line no 3: Lua multiline comment ending at line 4 found
```

Sample 2 (invalid):
```
int a = 0789;
int b = 0x2G7;
int c = 0X;
```
Log file:
```
Line no 1: Invalid octal constant 0789
Line no 2: Invalid hexadecimal constant 0x2G7
Line no 3: Invalid hexadecimal constant 0X
```

Sample 3 (ordinary tokens keep working): `count--;` still produces the `INCOP` token,
`017 -> <CONST_OCT, 017>`, `0XABC9 -> <CONST_HEX, 0XABC9>`.

Sample 4 (unfinished comment):
```
int x = 0x1F;
--[[ this comment
never ends
```
Token file: `<CONST_HEX, 0x1F>`
Log file:
```
Line no 1: Token <CONST_HEX> Lexeme 0x1F found
Line no 2: Unfinished Lua multiline comment
```

## Solution (`2205040_B1.l`)

```flex
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

%x COMMENT STRING CHAR LINE_COMMENT LUA_COMMENT

%%

[ \t]+                ;
\n                    ;

 /* Lua-style multiline comment --[[ ... ]] */
"--[["                { BEGIN(LUA_COMMENT); startLine = yylineno; }

<LUA_COMMENT>{
    "]]"              { flog << "Line no " << startLine << ": Lua multiline comment ending at line "
                             << yylineno << " found" << endl << endl; BEGIN(INITIAL); }
    \n                ;
    .                 ;
    <<EOF>>           { BEGIN(INITIAL);
        flog << "Line no " << startLine << ": Unfinished Lua multiline comment" << endl << endl;
        errorCount++;
    }
}

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

    /* octal & hexadecimal integer constants */
    // Hex constant detection: 0x / 0X
    int c0 = yyinput();
    if (yytext[0] == '0' && (c0 == 'x' || c0 == 'X')) {
        buf += (char)c0;
        bool anyHex = false;
        bool hasInvalid = false;
        int c;
        while ((c = yyinput()) != EOF) {
            if (isxdigit(c)) {
                buf += (char)c;
                anyHex = true;
            } else if (isalnum(c) || c == '_') {
                buf += (char)c;
                hasInvalid = true;
            } else {
                if (c != EOF) unput(c);
                break;
            }
        }
        if (hasInvalid || !anyHex) {
            printError("Invalid hexadecimal constant " + buf);
            return;
        }
        ftoken << "<CONST_HEX, " << buf << "> ";
        printLog("CONST_HEX", buf);
        return;
    }
    if (c0 != EOF) unput(c0);

    // Decimal / Octal detection
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
        while ((c = yyinput()) != EOF && (isalnum(c) || c == '.' || c == '_')) buf += (char)c;
        if (c != EOF) unput(c);
        if (errorType == "multiple decimal") printError("Too many decimal points " + buf);
        else if (errorType == "ill formed") printError("Ill formed number " + buf);
        else printError("Invalid prefix on ID or invalid suffix on Number " + buf);
        return;
    }

    // Octal constant detection: starts with 0 and has >= 1 more digit
    if (yytext[0] == '0' && buf.length() > 1 && !isFloat && !hasExponent) {
        bool validOct = true;
        for (size_t i = 0; i < buf.size(); i++) {
            char ch = buf[i];
            if (ch < '0' || ch > '7') { validOct = false; break; }
        }
        if (validOct) {
            ftoken << "<CONST_OCT, " << buf << "> ";
            printLog("CONST_OCT", buf);
            return;
        } else {
            printError("Invalid octal constant " + buf);
            return;
        }
    }

    string type = isFloat ? "CONST_FLOAT" : "CONST_INT";
    ftoken << "<" << type << ", " << buf << "> ";
    printLog(type, buf);
    handleSymbolInsert(buf, type);
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
```

---

# C1: Python Dictionary

## Problem Statement

Create a `.l` file to validate a simplified Python dictionary. The input is a single
dictionary containing key-value pairs.

Rules for a valid dictionary:

- The complete dictionary is enclosed within `{` and `}`.
- A key may be a quoted string or an integer constant.
- A quoted string may use either single quotes `'...'` or double quotes `"..."`.
- Each key is followed by a colon `:`.
- A value may be a quoted string, an integer constant, `True`, `False`, `None`, or another
  dictionary.
- A comma `,` separates consecutive key-value pairs.
- A comma after the final key-value pair is optional.

Validate:

- proper matching and nesting of `{` and `}`;
- proper opening and closing of single and double quotation marks;
- valid string or integer keys;
- correct use of `:` between a key and its value;
- correct use of `,` between consecutive key-value pairs.

Error checking (report only the first error):

- `Error: invalid dictionary key` - a key is neither a quoted string nor an integer.
- `Error: missing pair separator` - two pairs are not separated by `,`.
- `Error: unmatched {` - an opening brace remains unmatched at end of input.

## Sample I/O

Sample 1 (valid):
```
{
    'name': "alice",
    101: { "city": 'sylhet', "active": True },
    "phone": None
}
```
Output: `Valid Python Dictionary`

Sample 2 (invalid unquoted key): `{ name: "alice", "city": "dhaka" }`
Output: `Error: invalid dictionary key`

Sample 3 (missing pair separator): `{ "name": "alice" "city": "dhaka" }`
Output: `Error: missing pair separator`

Sample 4 (unmatched brace):
```
{ "name": "alice", "address": { "city": "sylhet" }
```
Output: `Error: unmatched {`

## Solution (`2205040_C1.l`)

```flex
%option noyywrap yylineno

%{
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

/* dictionary validation mode machine */
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

 /* flex rules driving dictionary validation */
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
```

---

# Build Commands

```sh
# A1
flex -o a1.c 2205040_A1.l && g++ -o a1 a1.c

# B1 (needs the headers)
flex -o b1.c 2205040_B1.l && g++ -o b1 b1.c -I.

# C1
flex -o c1.c 2205040_C1.l && g++ -o c1 c1.c
```

Run each program with an input file: `./a1 input.txt`. See `howtorunandtest.md` for the
full test procedure.

---

# D1: LaTeX Command Validator

## Problem Statement

Write a flex program to validate a simplified LaTeX document. Only three commands are
supported: `\textbf`, `\textit`, and `\section`.

Rules:

- A command is written as `\name` and must be followed immediately by a brace block
  `{...}` containing its argument.
- Braces must be balanced and properly nested.
- Commands may be nested inside a brace block (e.g. `\textbf{\textit{...}}`).
- A `{` or `}` that does not belong to a command is an error.
- Any command other than the three supported ones is an error.
- Stop parsing immediately after detecting the first error.

Error messages:

- `Error: unsupported command` - a command other than `textbf`, `textit`, `section`.
- `Error: command not followed by brace block` - a command that is not immediately
  followed by `{`.
- `Error: unmatched {` - an unclosed or orphan opening brace.
- `Error: unmatched }` - a closing brace with no matching opening brace.

On success print `Valid LaTeX syntax`.

## Sample Input / Output

| Input | Output |
|---|---|
| `\textbf{Bold text}` | `Valid LaTeX syntax` |
| `\textbf{bold and \textit{nested} text}` | `Valid LaTeX syntax` |
| `\section{Title}` | `Valid LaTeX syntax` |
| `\textbf{unclosed` | `Error: unmatched {` |
| `\textbf broken` | `Error: command not followed by brace block` |
| `\emph{text}` | `Error: unsupported command` |
| `{orphan}` | `Error: unmatched {` |

## Solution (`latex.l`)

```flex
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
```

---

# D2: Star-Plus Pyramid Pattern

## Problem Statement

Write a flex program to validate a pyramid-shaped pattern built from stars `*` and
pluses `+`. The pattern is read line by line.

Rules:

- Each line has the form `left stars, then pluses, then right stars`, where the number of
  stars on the left equals the number on the right.
- The first line must have at least one star on each side and exactly one plus.
- Each following line must have one fewer star per side and two more pluses than the
  previous line.
- The pattern ends when the star count reaches zero (a line of only pluses).

Error checking (stop at the first error, report the line number):

- `Error at line N: Empty line`
- `Error at line N: Line must start with stars`
- `Error at line N: Invalid character in pattern`
- `Error at line N: Invalid character after right stars`
- `Error at line N: Extra characters after pattern`
- `Error at line N: Stars on left and right not equal`
- `Error at line N: First line must have at least one star on each side and exactly one plus`
- `Error at line N: Stars on each side should be X but found Y`
- `Error at line N: Pluses should be X but found Y`
- `Error at line N: Pattern reached zero stars before final line`
- `Error at line N: Invalid characters in pattern`
- `Error at line N: Invalid pattern format`
- `Error at line N: Empty file - no pattern found`
- `Error at line N: Pattern incomplete - missing final plus-only line`

On success print `Pattern matched successfully!`.

## Sample Input / Output

| Input | Output |
|---|---|
| `**+**`<br>`*+++*` | `Pattern matched successfully!` |
| `*+*` | `Pattern matched successfully!` |
| `*++*` | `Error at line 1: First line must have at least one star on each side and exactly one plus` |
| `**+*` | `Error at line 1: Stars on left and right not equal` |
| `*+*+*` | `Error at line 1: Invalid character after right stars` |
| `+*+` | `Error at line 1: Line must start with stars` |
| `**+**`<br>`*+*` | `Error at line 2: Pluses should be 3 but found 1` |
| `ax*+*` | `Error at line 1: Invalid characters in pattern` |

## Solution (`pyrami.l`)

```flex
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
```
