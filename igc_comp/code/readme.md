Here is your project summary and architecture map formatted as a Markdown document. You can save this directly as README.md in your project root.
Intermediate Code Generation (ICG) - Project Overview & Status
1. Assignment Requirements Summary

    Target Intermediate Representation (IR): 32-bit x86 Linux Assembly for FASM (Flat Assembler).

    Architecture: Parse tree traversal using the Visitor Pattern (generated via ANTLR4). No embedded actions allowed in grammar files.

    Grammar & Scope: Use the Assignment 3 C-subset grammar. Submissions are split into two phases:

        Phase 1 (Deadline: August 28, 2026): Variable declarations, arithmetic expressions, assignments, expression statements, and println(ID).

        Phase 2 (Deadline: September 11, 2026): Functions, control structures (if, if-else, for, while), arrays, logical operations, and recursive calls.

    Code Generation Rules:

        Code generated on-the-fly to output files (code.asm and optimized_code.asm).

        All local variables and parameters managed exclusively via the Stack Frame (ebp relative indexing). No local variables declared in the .data segment.

        Annotate assembly output with line numbers from source .c files as comments.

        Implement boolean expression short-circuiting.

        Call custom procedure for println(ID) handling.

    Peephole Optimization Rules:

        Remove redundant consecutive MOV instructions (e.g., MOV AX, a followed by MOV a, AX).

        Remove redundant consecutive PUSH / POP operations.

        Eliminate neutral arithmetic operations (e.g., ADD AX, 0, MUL AX, 1).

        Clean up unused/duplicate jump labels.

2. Progress Tracker

    [x] Grammar Specification (C4.g4) — Configured for ANTLR4 parser, lexer, and visitor generation.

    [x] Symbol Table & Memory Tracking (SymbolTable.h) — Merged single-header implementation containing SymbolInfo, ScopeTable, and SymbolTable with 32-bit stack offset calculations.

    [ ] Visitor Code Generator (CodeGenerator.h) — Next Step

    [ ] Peephole Optimizer (PeepholeOptimizer.h) — Pending Phase 1 completion

    [ ] Driver File (main.cpp) — Pending

    [ ] Build & Test Automation (Makefile / run.sh) — Pending

3. Project File Map & Responsibilities
Plaintext

igc_comp/
│
├── C4.g4                  ───► [ANTLR4 Grammar] Defines language tokens, productions, 
│                               and triggers visitor class generation.
│
├── SymbolTable.h          ───► [Memory & Scope Management] Unified single-header engine.
│                               ├── SymbolInfo: Stores symbol name, type, array size, 
│                               │               and stack offset ([ebp - offset]).
│                               ├── ScopeTable: Manages hash bucket table per scope level 
│                               │               and accumulates stack allocation size.
│                               └── SymbolTable: Enters/exits scopes, looks up variables, 
│                                               and computes 4-byte aligned stack slots.
│
├── CodeGenerator.h        ───► [Assembly Code Generator] ANTLR visitor implementation.
│   (To Be Created)             Traverses AST nodes, emits x86 32-bit FASM code, 
│                               manages register allocation, and writes to `code.asm`.
│
├── PeepholeOptimizer.h    ───► [Post-Processing Optimizer] Inspects generated code 
│   (To Be Created)             line-by-line to remove redundant MOVs, PUSH/POPs, 
│                               zero-adds, and dead labels into `optimized_code.asm`.
│
├── main.cpp               ───► [Driver Program] CLI entry point. Initializes ANTLR streams,
│   (To Be Created)             runs visitor, triggers peephole optimizer, and handles I/O.
│
└── Makefile               ───► [Build System] Runs ANTLR tool, compiles C++ sources, 
    (To Be Created)             links target binary, and runs FASM assembler tests.