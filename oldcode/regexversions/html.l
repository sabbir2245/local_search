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