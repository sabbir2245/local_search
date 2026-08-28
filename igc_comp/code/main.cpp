#include <iostream>
#include <fstream>
#include <string>
#include "antlr4-runtime.h"
#include "C4Lexer.h"
#include "C4Parser.h"
#include "CodeGenarator.h"
#include "PeepholeOptimizer.h"

using namespace antlr4;
using namespace std;

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file.c>" << endl;
        return 1;
    }

    string inputFileName = argv[1];

    // Step 1: Read input file
    ifstream inputStream(inputFileName);
    if (!inputStream.is_open()) {
        cerr << "Error: Cannot open input file " << inputFileName << endl;
        return 1;
    }

    // Step 2: ANTLR4 lexing and parsing
    ANTLRInputStream input(inputStream);
    C4Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    C4Parser parser(&tokens);

    // Step 3: Parse the input
    C4Parser::StartContext* tree = parser.start();

    // Step 4: Generate assembly code
    string codeFileName = "code.asm";
    CodeGenarator codeGen(codeFileName);

    // visitProgram handles: header, data segment, code segment, OUTDEC, and all functions
    codeGen.visit(tree);

    codeGen.finalize();
    inputStream.close();

    cout << "Assembly code generated: " << codeFileName << endl;

    // Step 5: Peephole optimization
    string optimizedFileName = "optimized_code.asm";
    PeepholeOptimizer optimizer;
    optimizer.loadFile(codeFileName);
    optimizer.optimize();
    optimizer.writeOutput(optimizedFileName);

    cout << "Optimized assembly code generated: " << optimizedFileName << endl;
    cout << "Peephole optimizations applied: " << optimizer.getOptimizationsCount() << endl;

    return 0;
}
