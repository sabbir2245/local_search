# 5 Similar Problems with Solutions (Flex/Lex)

Five practice problems in the same spirit as the A1 / B1 / C1 online labs. Each has a
problem statement, sample I/O, and a complete, tested flex solution.

All solutions below have been compiled with `flex 2.6.4` and `g++` and tested.

---

## Problem 1: JSON Object Validator (similar to C1 - Python Dictionary)

### Problem
You will create a `.l` file to validate a simplified JSON object.

- The whole object is enclosed in `{` and `}`.
- Keys must be double-quoted strings.
- A value may be a string, an integer, a float, `true`, `false`, `null`, or a nested object.
- Each key is followed by `:`; a comma separates consecutive pairs.
- A trailing comma after the last pair is optional.

Error messages (stop at the first error):
- `Error: invalid JSON key`
- `Error: missing pair separator`
- `Error: unmatched {`
- `Error: unterminated string`

### Sample I/O
Input:
```
{"name": "alice", "age": 30, "ok": true, "n": null, "obj": {"x": 1.5}}
```
Output:
```
Valid JSON Object
```

Input:
```
{"a": 1 "b": 2}
```
Output:
```
Error: missing pair separator
```

### Solution (`json.l`)
```flex
%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum Mode { EXP_KEY, EXP_COLON, EXP_VALUE, EXP_COMMA };
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
    if (completed || depth == 0) fail("Error: invalid JSON key");
}

void handleString() {
    ensureOpen();
    if (mode == EXP_KEY) { mode = EXP_COLON; return; }
    if (mode == EXP_VALUE) { mode = EXP_COMMA; return; }
    if (mode == EXP_COMMA) fail("Error: missing pair separator");
    fail("Error: invalid JSON key");
}

void handleScalar() {
    ensureOpen();
    if (mode == EXP_VALUE) { mode = EXP_COMMA; return; }
    if (mode == EXP_COMMA) fail("Error: missing pair separator");
    fail("Error: invalid JSON key");
}
%}

%x STR

%%

[ \t\n]+   ;

"{"    {
    if (completed) fail("Error: invalid JSON key");
    if (atStart) { atStart = 0; mode = EXP_KEY; depth = 1; }
    else if (mode == EXP_VALUE) { modeStack.push_back(EXP_COMMA); mode = EXP_KEY; depth++; }
    else fail("Error: invalid JSON key");
}

"}"    {
    if (depth == 0) fail("Error: invalid JSON key");
    if (mode != EXP_COMMA && mode != EXP_KEY) fail("Error: missing pair separator");
    depth--;
    if (depth == 0) { completed = 1; mode = EXP_COMMA; }
    else { mode = modeStack.back(); modeStack.pop_back(); }
}

":"    { ensureOpen(); if (mode == EXP_COLON) mode = EXP_VALUE; else fail("Error: missing pair separator"); }
","    { ensureOpen(); if (mode == EXP_COMMA) mode = EXP_KEY; else fail("Error: missing pair separator"); }

-?[0-9]+(\.[0-9]+)?  { handleScalar(); }

true|false|null      { handleScalar(); }

\"     { BEGIN(STR); }

<STR>{
    \\.     ;
    \"      { BEGIN(INITIAL); handleString(); }
    \n      { fail("Error: unterminated string"); }
    <<EOF>> { fail("Error: unterminated string"); }
    .       ;
}

.      { fail("Error: invalid JSON key"); }

<<EOF>> {
    if (depth > 0) fail("Error: unmatched {");
    if (!atStart && completed) { cout << "Valid JSON Object" << endl; yyterminate(); }
    fail("Error: invalid JSON key");
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

## Problem 2: HTML Tag Matcher (similar to A1 - Yet Another Tag Matching)

### Problem
Validate the nesting of tags in a simplified HTML document. Supported tags:
`html`, `head`, `title`, `body`, `div`, `p`, `br`.

- Every opening tag needs a matching closing tag, closed in the correct order (use a stack).
- `<br>` is self-closing: it may also appear as `<br/>` and requires no closing tag.
- Ignore text content between tags.
- Errors (stop at the first error):
  - `Invalid HTML: unsupported tag <tag>`
  - `Invalid HTML: tag mismatch </tag>`
  - `Invalid HTML: unexpected closing tag </tag>`
  - `Invalid HTML: missing closing tag for <tag>`
- On success print `Valid HTML structure`.

### Sample I/O
Input:
```
<html><body><div><p>Hi</p></div></body></html>
```
Output:
```
Valid HTML structure
```

Input:
```
<p><div></p></div>
```
Output:
```
Invalid HTML: tag mismatch </p>
```

### Solution (`html.l`)
```flex
%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <vector>
using namespace std;

vector<string> tagStack;
string tagName;
string tagBuf;
bool closing = false;
bool selfClose = false;

bool isSupported(const string& name) {
    return name == "html" || name == "head" || name == "title" ||
           name == "body" || name == "div" || name == "p" || name == "br";
}

void report(const string& msg) {
    cout << msg << endl;
    exit(0);
}
%}

%x TAG

%%

<INITIAL><<EOF>> {
    if (!tagStack.empty()) report("Invalid HTML: missing closing tag for <" + tagStack.back() + ">");
    else report("Valid HTML structure");
}

"<"       { tagBuf = "<"; tagName = ""; closing = false; selfClose = false; BEGIN(TAG); }

<TAG>{
    "/"    {
        if (tagName.empty()) closing = true;
        else selfClose = true;
        tagBuf += "/";
    }
    [a-zA-Z_][a-zA-Z0-9_]* { tagBuf += yytext; tagName = yytext; }
    [ \t\n] ;
    ">"    {
        BEGIN(INITIAL);
        tagBuf += ">";
        if (tagName.empty()) report("Invalid HTML: unsupported tag " + tagBuf);
        if (!isSupported(tagName)) report("Invalid HTML: unsupported tag " + tagBuf);
        if (closing) {
            if (tagStack.empty()) report("Invalid HTML: unexpected closing tag " + tagBuf);
            if (tagStack.back() != tagName) report("Invalid HTML: tag mismatch " + tagBuf);
            tagStack.pop_back();
        } else if (!selfClose) {
            tagStack.push_back(tagName);
        }
    }
    .        { tagBuf += ">"; report("Invalid HTML: unsupported tag " + tagBuf); }
    <<EOF>>  { tagBuf += ">"; report("Invalid HTML: unsupported tag " + tagBuf); }
}

[^<]+   ;   /* ignore text content */
.       ;

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

## Problem 3: Binary / Octal / Hex Number Lexer (similar to B1 - Oct, Hex, and Lua)

### Problem
Write a lexer that recognizes integer constants in four bases plus floats.

- Decimal: `42` -> `CONST_INT`
- Float: `3.14` -> `CONST_FLOAT`
- Octal: starts with `0` and has at least one more digit, all `0-7` -> `CONST_OCT`
- Hexadecimal: `0x`/`0X` followed by at least one hex digit -> `CONST_HEX`
- Binary: `0b`/`0B` followed by at least one of `0`/`1` -> `CONST_BIN`

Error checking (an invalid lexeme must be consumed as one error, no token):
- `Invalid octal constant <lexeme>` (e.g. `08`, `019`)
- `Invalid hexadecimal constant <lexeme>` (e.g. `0x`, `0xG1`)
- `Invalid binary constant <lexeme>` (e.g. `0b`, `0b12`)

### Sample I/O
Input:
```
0b101 0B11 017 0x2A 5 3.14
```
Output:
```
<CONST_BIN, 0b101>
<CONST_BIN, 0B11>
<CONST_OCT, 017>
<CONST_HEX, 0x2A>
<CONST_INT, 5>
<CONST_FLOAT, 3.14>
```

Input:
```
0b12 0b 08 0xG
```
Output:
```
Error: Invalid binary constant 0b12
Error: Invalid binary constant 0b
Error: Invalid octal constant 08
Error: Invalid hexadecimal constant 0xG
```

### Solution (`binary.l`)
```flex
%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <cctype>
using namespace std;

void printNum(const string& type, const string& lexeme) {
    cout << "<" << type << ", " << lexeme << ">" << endl;
}

void error(const string& msg) {
    cout << "Error: " << msg << endl;
}

void scanNumber();
%}

%%

[0-9]      { scanNumber(); }
[ \t\n]+   ;
.          ;

%%

void scanNumber() {
    string buf(1, yytext[0]);

    int c = yyinput();

    if (yytext[0] == '0' && (c == 'x' || c == 'X')) {
        buf += (char)c;
        bool any = false, invalid = false;
        while ((c = yyinput()) != EOF) {
            if (isxdigit(c)) { buf += (char)c; any = true; }
            else if (isalnum(c)) { buf += (char)c; invalid = true; }
            else { unput(c); break; }
        }
        if (!any || invalid) error("Invalid hexadecimal constant " + buf);
        else printNum("CONST_HEX", buf);
        return;
    }

    if (yytext[0] == '0' && (c == 'b' || c == 'B')) {
        buf += (char)c;
        bool any = false, invalid = false;
        while ((c = yyinput()) != EOF) {
            if (c == '0' || c == '1') { buf += (char)c; any = true; }
            else if (isalnum(c)) { buf += (char)c; invalid = true; }
            else { unput(c); break; }
        }
        if (!any || invalid) error("Invalid binary constant " + buf);
        else printNum("CONST_BIN", buf);
        return;
    }

    if (c != EOF) unput(c);

    bool isFloat = false;
    while ((c = yyinput()) != EOF) {
        if (isdigit(c)) buf += (char)c;
        else if (c == '.') { buf += (char)c; isFloat = true; }
        else { unput(c); break; }
    }

    if (yytext[0] == '0' && buf.length() > 1 && !isFloat) {
        bool ok = true;
        for (char d : buf) if (d < '0' || d > '7') ok = false;
        if (ok) printNum("CONST_OCT", buf);
        else error("Invalid octal constant " + buf);
        return;
    }

    printNum(isFloat ? "CONST_FLOAT" : "CONST_INT", buf);
}

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

## Problem 4: HTML `<!-- ... -->` Comments (similar to B1 - Lua-Style Comments)

### Problem
Extend a lexer to recognize HTML-style block comments `<!-- ... -->` which may span
multiple lines.

- Comment delimiters appearing inside double-quoted strings must be treated as ordinary
  characters (so `"a <!-- b -->"` is just a string, not a comment).
- For each complete comment print:
  `HTML comment from line <start> to line <end>`
- If EOF is reached before the comment is closed, print:
  `Error at line <start>: Unterminated HTML comment`

### Sample I/O
Input:
```
a <!-- comment
spanning lines --> b "x <!-- y -->" z
```
Output:
```
HTML comment from line 1 to line 2
```

Input:
```
<!-- never ends
more text
```
Output:
```
Error at line 1: Unterminated HTML comment
```

### Solution (`htmlcomment.l`)
```flex
%option noyywrap yylineno

%{
#include <iostream>
#include <string>
using namespace std;
int startLine = 0;
%}

%x HCOM STR

%%

[ \t\n]+   ;

"<!--"     { startLine = yylineno; BEGIN(HCOM); }

<HCOM>{
    "-->"   { cout << "HTML comment from line " << startLine << " to line " << yylineno << endl; BEGIN(INITIAL); }
    \n      ;
    .       ;
    <<EOF>> { cout << "Error at line " << startLine << ": Unterminated HTML comment" << endl; yyterminate(); }
}

\"         { BEGIN(STR); }

<STR>{
    \\.     ;
    \"      { BEGIN(INITIAL); }
    \n      ;
    .       ;
}

.          ;

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

## Problem 5: Expression Bracket Matcher (similar to A1 / C1 stack approach)

### Problem
Validate that the brackets `()`, `[]`, `{}` in an expression are balanced and properly
nested. Ignore string literals (`"..."`) and C comments (`/* ... */`) entirely.

- On success print `Balanced expression`.
- Errors (stop at the first error):
  - `Unexpected closing '<c>'`
  - `Mismatch: expected '<c>' but found '<c>'`
  - `Unclosed '<c>' at end of input`
  - `Unterminated string` / `Unterminated comment`

### Sample I/O
Input:
```
(a[b]{c})
```
Output:
```
Balanced expression
```

Input:
```
([)]
```
Output:
```
Mismatch: expected ']' but found ')'
```

Input:
```
(a[b
```
Output:
```
Unclosed '[' at end of input
```

### Solution (`brackets.l`)
```flex
%option noyywrap yylineno

%{
#include <iostream>
#include <string>
#include <vector>
using namespace std;

vector<char> stack;

char expect(char open) {
    switch (open) {
        case '(': return ')';
        case '[': return ']';
        default:  return '}';
    }
}

bool matches(char open, char close) {
    return close == expect(open);
}

void report(const string& msg) {
    cout << msg << endl;
    exit(0);
}
%}

%x STR COM

%%

"/*"   { BEGIN(COM); }

<COM>{
    "*/"  { BEGIN(INITIAL); }
    \n    ;
    .     ;
    <<EOF>> { report("Unterminated comment"); }
}

\"     { BEGIN(STR); }

<STR>{
    \\.   ;
    \"    { BEGIN(INITIAL); }
    \n    ;
    .     ;
    <<EOF>> { report("Unterminated string"); }
}

"("|"["|"{"   { stack.push_back(yytext[0]); }

")"|"]"|"}"   {
    char c = yytext[0];
    if (stack.empty()) report(string("Unexpected closing '") + c + "'");
    char o = stack.back();
    if (!matches(o, c)) report(string("Mismatch: expected '") + expect(o) + "' but found '" + c + "'");
    stack.pop_back();
}

\n        ;
[^ \t\n]  ;

<INITIAL><<EOF>> {
    if (!stack.empty()) report(string("Unclosed '") + stack.back() + "' at end of input");
    else report("Balanced expression");
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

## Build / run commands (any problem)

```sh
flex -o sol.c sol.l && g++ -o sol sol.c
./sol input.txt
```
