# Build
make all

# Clean (removes all generated files including ANTLR output)
make clean

after make all 


# Run with a specific input
./icg_compiler input/test1_i.c && fasm code.asm code && ./code
# Or use the make shortcut (runs on test1_i.c)
make run

# Run all tests
make test

# Assemble generated code with FASM
fasm code.asm
fasm optimized_code.asm
Note after make clean: You need to regenerate ANTLR files before building:
java -jar antlr-4.13.1-complete.jar -Dlanguage=Cpp -visitor -no-listener C4.g4
make all
Or simply combine it:
make clean && java -jar antlr-4.13.1-complete.jar -Dlanguage=Cpp -visitor -no-listener C4.g4 && make all



Based on the assignment spec (CSE_310_Januay_2026_ICG_Spec.txt lines 100-135), you need to submit a zip file containing:
1. Source files:
- C4.g4 — ANTLR4 grammar
- SymbolTable.cpp — Symbol table / scope management
- CodeGenarator.cpp — Visitor code generator (note the typo in filename is intentional)
- CodeGenarator.h — Header for code generator
- PeepholeOptimizer.cpp — Peephole optimizer
- PeepholeOptimizer.h — Header for optimizer
- main.cpp — Driver program
- Makefile — Build system
2. ANTLR-generated files (or include the jar + script to regenerate):
- C4Lexer.cpp / C4Lexer.h
- C4Parser.cpp / C4Parser.h
- C4BaseVisitor.cpp / C4BaseVisitor.h
- C4Visitor.cpp / C4Visitor.h
3. A script to compile and run your code (the Makefile serves this purpose)
4. If FASM can't assemble all sample inputs, provide your custom input files for which it works
Zip naming: Your 7-digit student ID followed by _P1.zip (e.g., 2205001_P1.zip)
TheANTLR-generated .cpp/.h files can either be included directly or regenerated via the Makefile's antlr target (which requires the antlr-4.13.1-complete.jar). Since the Makefile already has an antlr target to regenerate them, you likely just need the .g4 + jar + Makefile.