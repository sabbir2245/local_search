# Intermediate Code Generation - Progress

## Phase 1 (Deadline: August 28, 2026)
- [x] Grammar Specification (C4.g4) — ANTLR4 parser, lexer, visitor generation
- [x] Symbol Table & Memory Tracking (SymbolTable.cpp) — SymbolInfo, ScopeTable, SymbolTable
- [x] Visitor Code Generator (CodeGenarator.h + .cpp) — x86 FASM assembly generation
  - [x] Global variable declarations (data segment)
  - [x] Local variable declarations (stack allocation)
  - [x] Arithmetic expressions (+, -, *, /, %)
  - [x] Unary expressions (+, -, !)
  - [x] Relational expressions (==, !=, <, >, <=, >=)
  - [x] Logical expressions (&&, ||) with short-circuit evaluation
  - [x] Variable assignments (simple and array)
  - [x] INCOP / DECOP (++, --)
  - [x] println(ID) via OUTDEC procedure
  - [x] Source line annotations in comments
- [x] Peephole Optimizer (PeepholeOptimizer.h + .cpp)
  - [x] Redundant MOV removal
  - [x] Redundant PUSH/POP removal
  - [x] Redundant arithmetic removal (ADD 0, SUB 0, MUL 1)
  - [x] Consecutive label removal
- [x] Driver Program (main.cpp) — ANTLR setup, visitor, optimizer
- [x] Build System (Makefile)

## Phase 2 (Deadline: September 11, 2026)
- [x] Function declarations
- [x] Function definitions (prologue/epilogue, callee-cleanup RET N)
- [x] Function parameters ([EBP+8], [EBP+12], etc.)
- [x] Function calls (push args right-to-left, caller cleanup ADD ESP)
- [x] Recursive function calls
- [x] IF / IF-ELSE statements
- [x] FOR loops
- [x] WHILE loops
- [x] RETURN statements
- [x] Compound statements (scope management)
- [x] Global and local arrays
- [x] Array element access and assignment
- [x] Boolean expression short-circuiting (&&, ||)

## Files Written
| File | Status | Description |
|------|--------|-------------|
| CodeGenarator.h | DONE | Visitor class header |
| CodeGenarator.cpp | DONE | Full visitor implementation (all grammar rules) |
| PeepholeOptimizer.h | DONE | Optimizer class header |
| PeepholeOptimizer.cpp | DONE | Peephole optimization passes |
| main.cpp | DONE | CLI driver with ANTLR4 setup |
| Makefile | DONE | Build system for ANTLR + compile + assemble |
| SymbolTable.cpp | DONE | Symbol table with stack tracking |
| C4.g4 | DONE | ANTLR4 grammar |
