# Regex Version Problems: All Problem Statements and Solutions

Five practice problems implemented with flex regular expressions. Each section has a
problem statement, sample I/O, and the complete flex solution.

All solutions have been compiled with `flex 2.6.4` and `g++` and tested.

---

## Problem 1: HTML Tag Matcher (`html.l`)

### Problem
Validate the proper nesting of tags in a simplified HTML document. Supported tags:
`html`, `body`, `div`, and `p`.

- Every opening tag needs a matching closing tag, closed in the correct order (use a stack).
- Ignore whitespace and all text content between tags.
- Errors (stop at the first error):
  - `Invalid HTML: unexpected </tag>` - a closing tag with no matching opening tag.
  - `Invalid HTML: tag mismatch </tag>` - a closing tag that does not match the most recent
    opening tag.
  - `Invalid HTML: unclosed tag <tag>` - an opening tag left open at end of input.
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
<html><body></div></body></html>
```
Output:
```
Invalid HTML: tag mismatch </div>
```

Input:
```
<html><body>
```
Output:
```
Invalid HTML: unclosed tag <body>
```

Input:
```
</html>
```
Output:
```
Invalid HTML: unexpected </html>
```

### Solution
```flex
%option noyywrap

%{
#include <stdio.h>
#include <string.h>

char stack[100][20];
int top = -1;
int valid = 1;

void push(char tag[])
{
    top++;
    strcpy(stack[top], tag);
}

void pop(char tag[])
{
    if (top == -1)
    {
        printf("Invalid HTML: unexpected </%s>\n", tag);
        valid = 0;
        return;
    }

    if (strcmp(stack[top], tag) != 0)
    {
        printf("Invalid HTML: tag mismatch </%s>\n", tag);
        valid = 0;
        return;
    }

    top--;
}
%}

%%

"<html>"   { push("html"); if (!valid) return 0; }
"</html>"  { pop("html");  if (!valid) return 0; }

"<body>"   { push("body"); if (!valid) return 0; }
"</body>"  { pop("body");  if (!valid) return 0; }

"<div>"    { push("div");  if (!valid) return 0; }
"</div>"   { pop("div");   if (!valid) return 0; }

"<p>"      { push("p");    if (!valid) return 0; }
"</p>"     { pop("p");     if (!valid) return 0; }

[ \t\n]+   ; /* Ignore whitespace */
.          ; /* Ignore non-tag text */

%%

int main(int argc, char* argv[])
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
            return 1;
        }
    }

    yylex();

    if (valid && top == -1)
        printf("Valid HTML structure\n");
    else if (valid)
        printf("Invalid HTML: unclosed tag <%s>\n", stack[top]);

    return 0;
}
```

---

## Problem 2: HTML Comment Recognizer (`htmlcommnt.l`)

### Problem
Recognize HTML-style block comments `<!-- ... -->` which may span multiple lines.

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

### Solution
```flex
%option noyywrap yylineno

%{
#include <stdio.h>

int start_line = 0;
%}

%x HCOM STR

%%

"<!--"     { start_line = yylineno; BEGIN(HCOM); }

<HCOM>{
    "-->"   { 
        printf("HTML comment from line %d to line %d\n", start_line, yylineno); 
        BEGIN(INITIAL); 
    }
    \n      ;
    .       ;
    <<EOF>> { 
        printf("Error at line %d: Unterminated HTML comment\n", start_line); 
        yyterminate(); 
    }
}

\"         { BEGIN(STR); }

<STR>{
    \\\\    ;
    \\\"    ;
    \"      { BEGIN(INITIAL); }
    \n      ;
    .       ;
}

[ \t\n]+   ;
.          ;

%%

int main(int argc, char* argv[])
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
            return 1;
        }
    }
    yylex();
    return 0;
}
```

---

## Problem 3: JSON Object Validator (`json.l`)

### Problem
Validate a simplified JSON object.

- The whole object is enclosed in `{` and `}`.
- Keys must be double-quoted strings.
- A value may be a string, an integer, a float, `true`, `false`, `null`, or a nested object.
- Each key is followed by `:`; a comma separates consecutive pairs.

Error messages (stop at the first error):
- `Error: invalid JSON key`
- `Error: missing pair separator`
- `Error: unterminated string`
- `Error: unmatched {`

On success print `Valid JSON Object`.

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

Input:
```
{ name: "alice" }
```
Output:
```
Error: invalid JSON key
```

Input:
```
{"a": 1
```
Output:
```
Error: unmatched {
```

Input:
```
{"a": "unterminated
```
Output:
```
Error: unterminated string
```

### Solution
```flex
%option noyywrap yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mode_stack[100];
int stack_top = -1;

void push_mode(int mode) {
    stack_top++;
    mode_stack[stack_top] = mode;
}

int pop_mode() {
    if (stack_top == -1) return 0;
    int m = mode_stack[stack_top];
    stack_top--;
    return m;
}

void fail(const char* msg) {
    printf("%s\n", msg);
    exit(0);
}
%}

%x OBJ EXP_VAL STR

%%

[ \t\n]+   ;

"{" {
    push_mode(OBJ);
    BEGIN(OBJ);
}

<OBJ>{
    [ \t\n]+   ;
    "}" {
        pop_mode();
        if (stack_top == -1) {
            BEGIN(INITIAL);
        } else {
            BEGIN(mode_stack[stack_top]);
        }
    }
    \" { BEGIN(STR); }
    .  { fail("Error: invalid JSON key"); }
}

<OBJ,EXP_VAL>":" {
    if (YY_START == OBJ) {
        BEGIN(EXP_VAL);
    } else {
        fail("Error: missing pair separator");
    }
}

<EXP_VAL>{
    [ \t\n]+   ;
    -?[0-9]+(\.[0-9]+)? { BEGIN(OBJ); }
    true|false|null     { BEGIN(OBJ); }
    \"                  { BEGIN(STR); }
    "{"                 { push_mode(OBJ); BEGIN(OBJ); }
    .                   { fail("Error: invalid JSON key"); }
}

<OBJ>"," ;

<STR>{
    \\\"        ;
    [^\"\n\\]+  ;
    \"          { 
        if (stack_top >= 0 && mode_stack[stack_top] == OBJ) {
            BEGIN(OBJ);
        } else {
            BEGIN(EXP_VAL);
        }
    }
    \n          { fail("Error: unterminated string"); }
    <<EOF>>     { fail("Error: unterminated string"); }
}

. { fail("Error: invalid JSON key"); }

<<EOF>> {
    if (stack_top != -1) fail("Error: unmatched {");
    printf("Valid JSON Object\n");
    yyterminate();
}

%%

int main(int argc, char* argv[])
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
            return 1;
        }
    }
    yylex();
    return 0;
}
```

---

## Problem 4: Binary / Octal / Hex / Float Number Lexer (`number.l`)

### Problem
Recognize integer constants in several bases plus floats, and report invalid constants.

- Decimal: `42` -> `CONST_INT`
- Float: `3.14` -> `CONST_FLOAT`
- Octal: starts with `0` and has at least one more digit, all `0-7` -> `CONST_OCT`
- Hexadecimal: `0x`/`0X` followed by at least one hex digit -> `CONST_HEX`
- Binary: `0b`/`0B` followed by at least one of `0`/`1` -> `CONST_BIN`

Error checking (an invalid lexeme is consumed as one error, no valid token):
- `Error: Invalid hexadecimal constant <lexeme>` (e.g. `0x`, `0xG1`)
- `Error: Invalid binary constant <lexeme>` (e.g. `0b`, `0b12`)
- `Error: Invalid octal constant <lexeme>` (e.g. `08`, `019`)

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

### Solution
```flex
%option noyywrap yylineno

%{
#include <stdio.h>

void print_num(const char* type, const char* lexeme) {
    printf("<%s, %s>\n", type, lexeme);
}

void report_error(const char* type, const char* lexeme) {
    printf("Error: Invalid %s constant %s\n", type, lexeme);
}
%}

HEX_VALID   0[xX][0-9a-fA-F]+
BIN_VALID   0[bB][01]+
OCT_VALID   0[0-7]+
INT_VALID   [0-9]+
FLOAT_VALID [0-9]+\.[0-9]+

HEX_ERR     0[xX][a-zA-Z0-9]*
BIN_ERR     0[bB][a-zA-Z0-9]*
OCT_ERR     0[0-9]+

%%

{HEX_VALID}     { print_num("CONST_HEX", yytext); }
{BIN_VALID}     { print_num("CONST_BIN", yytext); }
{FLOAT_VALID}   { print_num("CONST_FLOAT", yytext); }
{OCT_VALID}     { print_num("CONST_OCT", yytext); }
{INT_VALID}     { print_num("CONST_INT", yytext); }

{HEX_ERR}       { report_error("hexadecimal", yytext); }
{BIN_ERR}       { report_error("binary", yytext); }
{OCT_ERR}       { report_error("octal", yytext); }

[ \t\n]+        ;
.               ;

%%

int main(int argc, char* argv[])
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
            return 1;
        }
    }
    yylex();
    return 0;
}
```

---

## Problem 5: Expression Bracket Matcher (`parenthesis.l`)

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

Input:
```
]x(
```
Output:
```
Unexpected closing ']'
```

### Solution
```flex
%option noyywrap yylineno

%{
#include <stdio.h>
#include <stdlib.h>

char stack[100];
int top = -1;

void push(char c) {
    top++;
    stack[top] = c;
}

char get_expected(char open) {
    if (open == '(') return ')';
    if (open == '[') return ']';
    return '}';
}

void report(const char* msg) {
    printf("%s\n", msg);
    exit(0);
}
%}

%x STR COM

%%

"/*"   { BEGIN(COM); }

<COM>{
    "*/"    { BEGIN(INITIAL); }
    \n      ;
    .       ;
    <<EOF>> { report("Unterminated comment"); }
}

\"     { BEGIN(STR); }

<STR>{
    \\\\    ;
    \\\"    ;
    \"      { BEGIN(INITIAL); }
    \n      ;
    .       ;
    <<EOF>> { report("Unterminated string"); }
}

"("|"["|"{"   { push(yytext[0]); }

")"|"]"|"}"   {
    char c = yytext[0];
    if (top == -1) {
        printf("Unexpected closing '%c'\n", c);
        exit(0);
    }
    char o = stack[top];
    char exp = get_expected(o);
    if (c != exp) {
        printf("Mismatch: expected '%c' but found '%c'\n", exp, c);
        exit(0);
    }
    top--;
}

[ \t\n]+  ;
[^ \t\n]  ;

<<EOF>> {
    if (top != -1) {
        printf("Unclosed '%c' at end of input\n", stack[top]);
    } else {
        printf("Balanced expression\n");
    }
    yyterminate();
}

%%

int main(int argc, char* argv[])
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
            return 1;
        }
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
