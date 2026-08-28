# ============================================================
# Makefile for ICG Compiler - C4 Grammar
# Target: 32-bit x86 Linux Assembly (FASM)
# ============================================================

# Tools
ANTLR_JAR = antlr-4.13.1-complete.jar
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
JAVA = java

# ANTLR generated files
ANTLR_GEN = C4Lexer C4Parser C4BaseVisitor C4Visitor

# Source files
SRC = main.cpp CodeGenarator.cpp PeepholeOptimizer.cpp \
      $(foreach f,$(ANTLR_GEN),$(f).cpp) \
      $(foreach f,$(ANTLR_GEN),$(f).h)

# Object files
OBJ = $(SRC:.cpp=.o)

# Target binary
TARGET = icg_compiler

# ============================================================
# Targets
# ============================================================

.PHONY: all clean antlr run assemble test

all: $(TARGET)

# Generate ANTLR parser/lexer/visitor from grammar
antlr:
	$(JAVA) -jar $(ANTLR_JAR) -Dlanguage=Cpp -visitor -no-listener C4.g4

# Compile the compiler
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lantlr4-runtime

# Compile individual object files
main.o: main.cpp C4Lexer.h C4Parser.h CodeGenarator.h PeepholeOptimizer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

CodeGenarator.o: CodeGenarator.cpp CodeGenarator.h C4BaseVisitor.h C4Parser.h SymbolTable.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

PeepholeOptimizer.o: PeepholeOptimizer.cpp PeepholeOptimizer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generic compile rule for ANTLR-generated files
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the compiler on a test file
run: $(TARGET)
	./$(TARGET) input/test1_i.c

# Assemble with FASM
assemble: code.asm
	fasm code.asm
	fasm optimized_code.asm

# Run all tests
test: $(TARGET)
	@echo "=== Test 1 ===" && ./$(TARGET) input/test1_i.c
	@echo "=== Test 2 ===" && ./$(TARGET) input/test2_i.c
	@echo "=== Test 3 ===" && ./$(TARGET) input/test3_i.c
	@echo "=== Test 4 ===" && ./$(TARGET) input/test4_i.c
	@echo "=== Test 5 ===" && ./$(TARGET) input/test5_i.c
	@echo "=== Test 6 ===" && ./$(TARGET) input/test6_i.c
	@echo "=== Test 7 ===" && ./$(TARGET) input/test7_i.c
	@echo "=== Recursion 1 ===" && ./$(TARGET) input/recursion1_i.c
	@echo "=== Recursion 2 ===" && ./$(TARGET) input/recursion2_i.c

# Clean generated files
clean:
	rm -f $(OBJ) $(TARGET) code.asm optimized_code.asm
	rm -f C4Lexer.cpp C4Parser.cpp C4BaseVisitor.cpp C4Visitor.cpp
	rm -f C4Lexer.h C4Parser.h C4BaseVisitor.h C4Visitor.h
	rm -f C4.tokens C4Lexer.tokens
