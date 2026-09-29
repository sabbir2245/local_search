#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "antlr4-runtime.h"
#include "id2205040_CSubsetLexer.h"
#include "id2205040_CSubsetParser.h"
#include "SymbolTableVisitor.h"

using namespace antlr4;
using namespace std;

ofstream lexLogFile;        // used by Lexer.g4's writeIntoLexLogFile()
ofstream errorFile;         // shared with SymbolTableVisitor
int errorCount = 0;         // shared with SymbolTableVisitor

class ProjectErrorListener : public BaseErrorListener {
public:
    virtual void syntaxError(Recognizer *recognizer, Token *offendingSymbol,
                             size_t line, size_t charPositionInLine,
                             const std::string &msg, std::exception_ptr e) override {
        errorCount++;
        errorFile << "Line no " << line << ": " << msg << endl;
    }
};

static int countLines(const string &data) {
    if (data.empty()) return 0;
    int lines = 1;
    for (char c : data) {
        if (c == '\n') lines++;
    }
    return lines;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ofstream logFile("log.txt");
    if (!logFile.is_open()) {
        cerr << "Error opening log.txt" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        cerr << "Error opening input file: " << argv[1] << endl;
        return 1;
    }

    stringstream ss;
    ss << inputFile.rdbuf();
    string data = ss.str();
    inputFile.close();

    int lineCount = countLines(data);

    ANTLRInputStream input(data);
    id2205040_CSubsetLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    id2205040_CSubsetParser parser(&tokens);

    errorFile.open("error.txt");

    parser.removeErrorListeners();
    ProjectErrorListener errorListener;
    parser.addErrorListener(&errorListener);

    id2205040_CSubsetParser::StartContext* tree = parser.start();
    (void)tree;

    SymbolTableVisitor visitor(logFile);
    visitor.visit(tree);

    visitor.printSymbolTable();
    logFile << "Total lines: " << lineCount << endl;
    logFile << "Total errors: " << errorCount << endl;

    logFile.close();
    errorFile.close();
    return 0;
}