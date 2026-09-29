# CodeGenarator — Function → Sample C Code Map

This file maps each function in `CodeGenarator.cpp` to a **sample C/C-like source construct handled by that function**. The sample in the second column is the kind of code that reaches that visitor/helper; it is not necessarily the exact input used by the compiler.

The generator is an ANTLR visitor: grammar constructs are visited and corresponding x86-style assembly is emitted. For example, `visitStatement()` handles `for`, `if`, `while`, `println`, and `return` cases. fileciteturn1file0L402-L497

| Function name | Sample C code handled in this function |
|---|---|
| `newLabel()` | `if (x < 10) { ... }  // needs generated labels` |
| `emit()` | `— internal helper used while generating any statement/expression` |
| `emitWithComment()` | `— internal helper when generated ASM needs a comment` |
| `getOperandAddr()` | `int x;  // global or local variable address` |
| `getArrayAddr()` | `int a[10]; a[i];` |
| `relOpToJump()` | `if (a < b) x = 1;  // relational operators: == != < > <= >=` |
| `generateHeader()` | `— generated automatically for the whole program` |
| `generateDataSegment()` | `int x; int a[10];  // global variables/arrays` |
| `generateCodeSegmentStart()` | `— generated automatically before function code` |
| `generateOutdecProcedure()` | `println(x);  // runtime procedure used for integer output` |
| `generateFunctionPrologue()` | `int main() { ... }  // start of a function` |
| `generateFunctionEpilogue()` | `int f(int x) { return x; }  // end of function` |
| `finalize()` | `— end of code generation` |
| `visitStart()` | `start rule of the C program` |
| `visitProgram()` | `int x; int main() { return 0; }  // whole program` |
| `visitUnit()` | `int x;  OR  int f();  OR  int f() { ... }` |
| `visitFunc_declaration()` | `int add(int a, int b);` |
| `visitFunc_definition()` | `int add(int a, int b) { return a + b; }` |
| `visitParameter_list()` | `int add(int a, int b) { ... }` |
| `visitCompound_statement()` | `{ int x; x = 5; }` |
| `visitVar_declaration()` | `int x; int a[10];` |
| `visitType_specifier()` | `int x;  // type-specifier node` |
| `visitDeclaration_list()` | `int a, b, c;  // declaration-list node` |
| `visitStatements()` | `{ x = 1; y = 2; return x; }` |
| `visitStatement()` | `if (x) y = 1; else y = 2;
while (x) x = x - 1;
for (i = 0; i < 10; i++) x = x + 1;
println(x);
return x;` |
| `visitExpression_statement()` | `x = 5;` |
| `visitVariable()` | `x;
a[i];` |
| `visitExpression()` | `x = 5;
a[i] = x + 1;` |
| `visitLogic_expression()` | `x && y;
x || y;` |
| `visitRel_expression()` | `a < b;
a == b;
a >= b;` |
| `visitSimple_expression()` | `a + b;
a - b;` |
| `visitTerm()` | `a * b;
a / b;
a % b;` |
| `visitUnary_expression()` | `-x;
!x;` |
| `visitFactor()` | `x;
a[i];
x++;
--x;
foo(a, b);
( a + b );
42;
3.14;` |
| `visitArgument_list()` | `foo();  // argument-list node` |
| `visitArguments()` | `foo(a, b, c);  // arguments node` |

## Visitor map by C construct

| C construct | Main function handling it |
|---|---|
| Whole program | `visitProgram()` |
| Global/local variable declaration | `visitVar_declaration()` |
| Function declaration | `visitFunc_declaration()` |
| Function definition | `visitFunc_definition()` |
| Compound block `{ ... }` | `visitCompound_statement()` |
| `for (...)` | `visitStatement()` |
| `if (...)` | `visitStatement()` |
| `if (...) ... else ...` | `visitStatement()` |
| `while (...)` | `visitStatement()` |
| `println(x)` | `visitStatement()` |
| `return expr` | `visitStatement()` |
| Assignment `x = expr` | `visitExpression()` |
| Array assignment `a[i] = expr` | `visitExpression()` |
| Array access `a[i]` | `visitVariable()` / `visitFactor()` |
| `&&` / `||` | `visitLogic_expression()` |
| Relational operators `< > <= >= == !=` | `visitRel_expression()` |
| `+` / `-` | `visitSimple_expression()` |
| `*` / `/` / `%` | `visitTerm()` |
| Unary `-x` | `visitUnary_expression()` |
| Logical NOT `!x` | `visitUnary_expression()` |
| `x++` / `x--` | `visitFactor()` |
| Function call `foo(a,b)` | `visitFactor()` |
| Integer constant | `visitFactor()` |
| Float constant | `visitFactor()` |