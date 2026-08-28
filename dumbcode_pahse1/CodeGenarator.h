#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "C4BaseVisitor.h"
#include "SymbolTable.cpp"

using namespace std;

class CodeGenarator : public C4BaseVisitor {
private:
    SymbolTable symbolTable;
    ofstream asmFile;
    ostream* out;
    int labelCount;
    int currentLine;
    string currentFunctionName;
    int paramCount;
    vector<string> dataSegmentGlobals;

    string newLabel(const string& prefix = "L");
    void emit(const string& instruction);
    string getOperandAddr(SymbolInfo* sym);
    string relOpToJump(const string& op, bool negate = false);

public:
    CodeGenarator(const string& outputFileName);
    ~CodeGenarator();

    void generateHeader();
    void generateDataSegment();
    void generateCodeSegmentStart();
    void generateOutdecProcedure();
    void generateFunctionPrologue(const string& funcName);
    void generateFunctionEpilogue(const string& funcName, int localSize, int paramSize);
    void finalize();

    antlrcpp::Any visitStart(C4Parser::StartContext* ctx) override;
    antlrcpp::Any visitProgram(C4Parser::ProgramContext* ctx) override;
    antlrcpp::Any visitUnit(C4Parser::UnitContext* ctx) override;
    antlrcpp::Any visitFunc_declaration(C4Parser::Func_declarationContext* ctx) override;
    antlrcpp::Any visitFunc_definition(C4Parser::Func_definitionContext* ctx) override;
    antlrcpp::Any visitParameter_list(C4Parser::Parameter_listContext* ctx) override;
    antlrcpp::Any visitCompound_statement(C4Parser::Compound_statementContext* ctx) override;
    antlrcpp::Any visitVar_declaration(C4Parser::Var_declarationContext* ctx) override;
    antlrcpp::Any visitType_specifier(C4Parser::Type_specifierContext* ctx) override;
    antlrcpp::Any visitDeclaration_list(C4Parser::Declaration_listContext* ctx) override;
    antlrcpp::Any visitStatements(C4Parser::StatementsContext* ctx) override;
    antlrcpp::Any visitStatement(C4Parser::StatementContext* ctx) override;
    antlrcpp::Any visitExpression_statement(C4Parser::Expression_statementContext* ctx) override;
    antlrcpp::Any visitVariable(C4Parser::VariableContext* ctx) override;
    antlrcpp::Any visitExpression(C4Parser::ExpressionContext* ctx) override;
    antlrcpp::Any visitLogic_expression(C4Parser::Logic_expressionContext* ctx) override;
    antlrcpp::Any visitRel_expression(C4Parser::Rel_expressionContext* ctx) override;
    antlrcpp::Any visitSimple_expression(C4Parser::Simple_expressionContext* ctx) override;
    antlrcpp::Any visitTerm(C4Parser::TermContext* ctx) override;
    antlrcpp::Any visitUnary_expression(C4Parser::Unary_expressionContext* ctx) override;
    antlrcpp::Any visitFactor(C4Parser::FactorContext* ctx) override;
    antlrcpp::Any visitArgument_list(C4Parser::Argument_listContext* ctx) override;
    antlrcpp::Any visitArguments(C4Parser::ArgumentsContext* ctx) override;
};

#endif
