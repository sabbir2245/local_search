#!/bin/bash
set -e

GRAMMAR="id2205040_CSubset.g4"
LEXER="id2205040_Lexer.g4"
RUNTIME_INCLUDE="/usr/local/include/antlr4-runtime"

echo "[1/4] Generating ANTLR C++ files..."

rm -f id2205040_CSubset*.cpp id2205040_CSubset*.h
rm -f id2205040_Lexer.cpp id2205040_Lexer.h

antlr4 -Dlanguage=Cpp -visitor "$GRAMMAR"

echo "[2/4] Compiling..."

g++ -std=c++17 -I"$RUNTIME_INCLUDE" \
    main.cpp \
    id2205040_CSubset.cpp \
    id2205040_CSubsetBaseVisitor.cpp \
    id2205040_Lexer.cpp \
    -lantlr4-runtime \
    -pthread \
    -o parser

echo "[3/4] Running..."

if [ -z "$1" ]; then
    echo "Usage: ./run.sh input.c"
    exit 1
fi

./parser "$1"

echo "[4/4] Done."
echo "Generated:"
echo "  log.txt"
echo "  error.txt"
echo "  lexLogFile.txt"
