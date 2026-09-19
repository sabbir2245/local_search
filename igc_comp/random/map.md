actula c code on right side
Function → What C Code It Handles
CodeGenarator.cpp
Function	C Code It Handles
newLabel	(labels for) if, else, while, for, &&, ||, !, <,>,==,…
getOperandAddr	Any variable use: x, a, b, i, j, k (global & local)
getArrayAddr	arr[m] address computation
relOpToJump	a < b, a > b, a <= b, a >= b, a == b, a != b
generateHeader	(program preamble)
generateDataSegment	int g; at global scope
generateCodeSegmentStart	(segment directive)
generateOutdecProcedure	println(k) (the print routine)
generateFunctionPrologue	int foo(...) { — function entry
generateFunctionEpilogue	return x; / end of foo, main exit
visitProgram	Whole program (globals + functions)
visitUnit	Top-level declaration or function
visitFunc_declaration	int foo(int a, int b); (prototype)
visitFunc_definition	int foo(int a, int b) { ... }
visitCompound_statement	{ ... } block
visitVar_declaration	int i,j,k; / int arr[10];
visitStatements	Sequence of statements inside { }
visitStatement	for(...), while(...), if(...), if...else, println(x), return x;, {...}, x = ...;
visitExpression_statement	x = 5; / i++; / foo(a,b); / ;
visitVariable	Read arr[m]
visitExpression	x = expr; and write arr[m] = expr;
visitLogic_expression	a && b, a || b
visitRel_expression	a < b, a > b, a <= b, a >= b, a == b, a != b
visitSimple_expression	a + b, a - b
visitTerm	a * b, a / b, a % b
visitUnary_expression	-a, !a, +a
visitFactor	Constants 7, variables x, arr[m]++/arr[m]--, function calls foo(a,b), (expr)
visitArgument_list / visitArguments	(argument collection for calls — done via lambda in visitFactor)
PeepholeOptimizer.cpp

(This file works on assembly, not C code. So "C code" here means the C construct whose generated assembly gets optimized.)
Function	C Code Whose Assembly It Optimizes
isEmptyLine	Any code — removes blank lines between statements
isLabel	if, while, for, &&, || — any construct that emits a label
isRedundantMOV	a = b; b = a; style assignment pairs
isRedundantPushPop	f(x); where a push/pop of the same reg cancels
isRedundantArithmetic	x = x + 0;, x = x - 0;, x = x * 1;, x = x & -1;
isConsecutiveLabels	Nested/adjacent if/while/for producing back-to-back labels
isRedundantJmpToNextLabel	if (...) {} fall-through, empty else
isRedundantLoadAfterStore	arr[i] = x; y = arr[i]; adjacent pairs
isConstantStoreSimplifiable	int i; i = 7; (const assignment to a local/global)
foldPushOverwritePop	int a = x; f(); a = 7; — arg push then overwrite
findMatchingPop	f(a); — matching PUSH a / POP a around a call
isDeadRegisterOverwrite	x = f(); x = 5; — value overwritten before use
isLabelReferenced	Unreferenced labels from if/while/for that nobody jumps to
getLabelName	(helper for label handling)
optimize()	The entire optimized C program's assembly — fixed-point pass
loadFile / writeOutput	(I/O of the .asm file)
Quick One-Line Summary
Function (CodeGen)	C construct
visitExpression	x = ... and arr[m] = ...
visitVariable	arr[m] (read)
visitFactor	foo(a,b), arr[m]++, 7, (e)
visitLogic_expression	a && b, a || b
visitRel_expression	a < b, a == b, …
visitSimple_expression	a + b, a - b
visitTerm	a * b, a / b, a % b
visitUnary_expression	-a, !a
visitStatement	if, else, while, for, println, return
visitVar_declaration	int i,j,k;, int arr[10];
visitFunc_definition	int foo(int a, int b) { ... }