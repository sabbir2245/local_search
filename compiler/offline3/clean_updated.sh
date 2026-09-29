#!/bin/bash

# Remove runtime output files
rm -f log.txt error.txt lexLogFile.txt compiler.out

# Remove ANTLR-generated files
rm -f id2205040_CSubsetLexer.h id2205040_CSubsetLexer.cpp
rm -f id2205040_CSubsetParser.h id2205040_CSubsetParser.cpp
rm -f id2205040_CSubsetVisitor.h id2205040_CSubsetVisitor.cpp
rm -f id2205040_CSubsetBaseVisitor.h id2205040_CSubsetBaseVisitor.cpp
rm -f id2205040_CSubsetLexer.tokens id2205040_CSubsetLexer.interp
rm -f id2205040_CSubset.tokens id2205040_CSubset.interp
rm -f id2205040_Lexer.tokens id2205040_Lexer.interp

# Remove ANTLR working directory
rm -rf .antlr

# ID-prefixed grammars are hand-written sources and are preserved.
echo "Cleaned. Hand-written sources and id2205040_*.g4 grammars preserved."