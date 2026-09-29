#pragma once

#include <any>
#include <fstream>
#include <string>
#include <vector>
#include "id2205040_CSubsetBaseVisitor.h"
#include "SymbolTable.h"

class SymbolTableVisitor : public id2205040_CSubsetBaseVisitor {
public:
    SymbolTableVisitor(std::ofstream &logFile);
    ~SymbolTableVisitor();

    void printSymbolTable();

    std::any visitStart(id2205040_CSubsetParser::StartContext *ctx) override;
    std::any visitProgram(id2205040_CSubsetParser::ProgramContext *ctx) override;
    std::any visitUnit(id2205040_CSubsetParser::UnitContext *ctx) override;
    std::any visitFunc_declaration(id2205040_CSubsetParser::Func_declarationContext *ctx) override;
    std::any visitFunc_definition(id2205040_CSubsetParser::Func_definitionContext *ctx) override;
    std::any visitParameter_list(id2205040_CSubsetParser::Parameter_listContext *ctx) override;
    std::any visitCompound_statement(id2205040_CSubsetParser::Compound_statementContext *ctx) override;
    std::any visitVar_declaration(id2205040_CSubsetParser::Var_declarationContext *ctx) override;
    std::any visitType_specifier(id2205040_CSubsetParser::Type_specifierContext *ctx) override;
    std::any visitDeclaration_list(id2205040_CSubsetParser::Declaration_listContext *ctx) override;
    std::any visitStatements(id2205040_CSubsetParser::StatementsContext *ctx) override;
    std::any visitStatement(id2205040_CSubsetParser::StatementContext *ctx) override;
    std::any visitExpression_statement(id2205040_CSubsetParser::Expression_statementContext *ctx) override;
    std::any visitVariable(id2205040_CSubsetParser::VariableContext *ctx) override;
    std::any visitExpression(id2205040_CSubsetParser::ExpressionContext *ctx) override;
    std::any visitLogic_expression(id2205040_CSubsetParser::Logic_expressionContext *ctx) override;
    std::any visitRel_expression(id2205040_CSubsetParser::Rel_expressionContext *ctx) override;
    std::any visitSimple_expression(id2205040_CSubsetParser::Simple_expressionContext *ctx) override;
    std::any visitTerm(id2205040_CSubsetParser::TermContext *ctx) override;
    std::any visitUnary_expression(id2205040_CSubsetParser::Unary_expressionContext *ctx) override;
    std::any visitFactor(id2205040_CSubsetParser::FactorContext *ctx) override;
    std::any visitArgument_list(id2205040_CSubsetParser::Argument_listContext *ctx) override;
    std::any visitArguments(id2205040_CSubsetParser::ArgumentsContext *ctx) override;

private:
    std::ofstream &logFile;
    std::vector<SymbolTable> scopes;
    std::vector<SymbolInfo> pendingParams;

    void logRule(const std::string &rule, const std::string &code) const;
    void semError(int line, const std::string &msg);
    void semWarning(int line, const std::string &msg);
    SymbolInfo *findSymbol(const std::string &name);
    SymbolInfo *findGlobal(const std::string &name);
    std::vector<SymbolInfo> makeParamList(id2205040_CSubsetParser::Parameter_listContext *ctx);
};