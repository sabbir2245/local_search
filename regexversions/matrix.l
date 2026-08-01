%option noyywrap yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int rows = 0, cols = 0;
int rowCount = 0;
int entriesInRow = 0;

void fail(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    exit(0);
}
%}

%x DATA

%%

<INITIAL>{
    [ \t]+      ;
    \n          ;
    [0-9]+      {
        if (rows == 0) {
            rows = atoi(yytext);
        } else if (cols == 0) {
            cols = atoi(yytext);
            if (rows < 1 || cols < 1) fail("Error: Invalid matrix dimensions");
            BEGIN(DATA);
        } else {
            fail("Error: Invalid matrix dimensions");
        }
    }
    .           { fail("Error: Invalid matrix dimensions"); }
    <<EOF>>     { fail("Error: Invalid matrix dimensions"); }
}

<DATA>{
    [ \t]+      ;
    -?[0-9]+    { entriesInRow++; }
    \n          {
        if (entriesInRow == 0) {
            /* blank line - skip */
        } else {
            if (entriesInRow != cols)
                fail("Error at line %d: Row has %d entries, expected %d",
                     yylineno, entriesInRow, cols);
            rowCount++;
            entriesInRow = 0;
            if (rowCount > rows)
                fail("Error at line %d: Too many matrix rows", yylineno);
        }
    }
    .           { fail("Error at line %d: Invalid matrix entry", yylineno); }
    <<EOF>>     {
        if (entriesInRow > 0) {
            if (entriesInRow != cols)
                fail("Error at line %d: Row has %d entries, expected %d",
                     yylineno, entriesInRow, cols);
            rowCount++;
        }
        if (rowCount != rows)
            fail("Error: Matrix has %d rows, expected %d", rowCount, rows);
        printf("Valid matrix of size %d x %d\n", rows, cols);
        yyterminate();
    }
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
