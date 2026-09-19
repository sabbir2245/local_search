# PeepholeOptimizer — Assembly Function Map

This map focuses on the **assembly transformation** performed by each optimization-related function.

| Function name | Unoptimized ASM code | Optimized ASM code |
|---|---|---|
| `trim()` | *(C++ helper — no ASM pattern)* | *(No ASM change)* |
| `isLabel()` | `LOOP:` | `LOOP:` *(recognized as a label; no direct rewrite)* |
| `isEmptyOrComment()` | `; this is a comment`<br>` ` | *(Line can be ignored/removed)* |
| `normalizeInstruction()` | `  mov eax, ebx ; comment` | `MOV EAX, EBX` |
| `isRedundantMOV()` | `MOV EAX, EBX`<br>`MOV EBX, EAX` | `MOV EAX, EBX` |
| `isRedundantPushPop()` | `PUSH EAX`<br>`POP EAX` | *(both removed)* |
| `isRedundantArithmetic()` | `ADD EAX, 0` | *(removed)* |
| `isRedundantArithmetic()` | `SUB EAX, 0` | *(removed)* |
| `isRedundantArithmetic()` | `MUL EAX, 1` | *(removed)* |
| `isRedundantArithmetic()` | `AND EAX, -1` | *(removed)* |
| `isConsecutiveLabels()` | `L1:`<br>`L2:` | `L1:`<br>`L2:` *(then unused labels may be removed)* |
| `isRedundantJmpToNextLabel()` | `JMP NEXT`<br>`NEXT:` | `NEXT:` |
| `isRedundantLoadAfterStore()` | `MOV [EAX], EBX`<br>`MOV ECX, [EAX]` | `MOV [EAX], EBX` |
| `isConstantStoreSimplifiable()` | `MOV EAX, 5`<br>`MOV [EBX], EAX` | `MOV dword [EBX], 5` |
| `foldPushOverwritePop()` | `MOV EAX, EBX`<br>`PUSH EAX`<br>`MOV EAX, 10`<br>`POP ECX` | `MOV ECX, EBX` |
| `findMatchingPop()` | `PUSH EAX`<br>`...`<br>`POP EAX` | *(No direct rewrite; identifies the matching `POP`)* |
| `isDeadRegisterOverwrite()` | `PUSH EAX`<br>`...`<br>`POP EAX`<br>`MOV EAX, EBX` | `MOV EAX, EBX` |
| `isEmptyLine()` | *(blank line)* | *(blank line removed)* |
| `getLabelName()` | `LOOP:` | `LOOP` *(name extracted internally; no ASM rewrite)* |
| `isLabelReferenced()` | `JMP LOOP`<br>`LOOP:` | `LOOP:` is kept because it is referenced |
| `loadFile()` | Input assembly file | Loads ASM lines into `lines` *(no optimization)* |
| `optimize()` | All supported unoptimized ASM patterns | Applies the matching transformations repeatedly until no more changes occur |
| `writeOutput()` | Optimized `lines` | Writes optimized ASM to output file |
| `getOptimizationsCount()` | *(No ASM)* | *(Returns optimization count)* |

## Optimization-only quick map

| Function | Unoptimized ASM | Optimized ASM |
|---|---|---|
| `isRedundantMOV()` | `MOV A, B`<br>`MOV B, A` | `MOV A, B` |
| `isRedundantPushPop()` | `PUSH X`<br>`POP X` | *(nothing)* |
| `isRedundantArithmetic()` | `ADD X, 0` | *(nothing)* |
| `isRedundantArithmetic()` | `SUB X, 0` | *(nothing)* |
| `isRedundantArithmetic()` | `MUL X, 1` | *(nothing)* |
| `isRedundantArithmetic()` | `AND X, -1` | *(nothing)* |
| `isRedundantJmpToNextLabel()` | `JMP L`<br>`L:` | `L:` |
| `isRedundantLoadAfterStore()` | `MOV [M], R`<br>`MOV R2, [M]` | `MOV [M], R` |
| `isConstantStoreSimplifiable()` | `MOV R, 5`<br>`MOV [M], R` | `MOV dword [M], 5` |
| `foldPushOverwritePop()` | `MOV R1, X`<br>`PUSH R1`<br>`MOV R1, 10`<br>`POP R2` | `MOV R2, X` |
| `isDeadRegisterOverwrite()` + `findMatchingPop()` | `PUSH R`<br>`...`<br>`POP R`<br>`MOV R, X` | `MOV R, X` |
| consecutive-label cleanup | `L1:`<br>`L2:` where `L2` is unused | `L1:` |

## Important

The examples above represent the **ASM patterns that the C++ functions recognize**. They are based on the actual conditions in `PeepholeOptimizer.cpp`; the file's `optimize()` function performs the removals/replacements and repeats the pass until no further changes occur.

The main optimization sequence is:

1. Remove empty lines
2. Fold the 4-line `PUSH`/overwrite/`POP` pattern
3. Remove dead `PUSH`/`POP`
4. Remove redundant `MOV`
5. Remove redundant `PUSH`/`POP`
6. Remove neutral arithmetic
7. Remove `JMP` to the immediately following label
8. Remove redundant load after store
9. Simplify constant stores
10. Remove unreferenced consecutive labels

