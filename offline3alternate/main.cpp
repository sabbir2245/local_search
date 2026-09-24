#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "antlr4-runtime.h"
#include "id2205040_CSubsetLexer.h"
#include "id2205040_CSubsetParser.h"
#include "id2205040_CSubsetBaseVisitor.h"
#include "SymbolTable.cpp"

using namespace antlr4;
using namespace std;

ofstream logFile;
ofstream errorFile;
extern ofstream lexLogFile;

static string textOf(tree::ParseTree *ctx) {
    return ctx ? ctx->getText() : "";
}

static string lineText(tree::ParseTree *ctx) {
    if (!ctx) return "";
    return textOf(ctx);
}

static void logRule(const string &rule, tree::ParseTree *ctx) {
    if (logFile.is_open()) {
        logFile << "Line " << (ctx && ctx->getSourceInterval().a >= 0
            ? ctx->getPayload()->getSourceInterval().a + 1 : 0)
                 << ": " << rule << " : " << lineText(ctx) << '\n';
    }
}

static void semanticError(int line, const string &msg) {
    errorFile << "Line no " << line << ": " << msg << '\n';
}

static void semanticWarning(int line, const string &msg) {
    errorFile << "Line no " << line << ": Warning: " << msg << '\n';
}

class SemanticVisitor : public id2205040_CSubsetBaseVisitor {
    SymbolTable table;
    string currentFunction;
    string currentFunctionReturnType;
    vector<string> currentParameterTypes;
    bool insideFunction = false;

    string typeOf(tree::ParseTree *ctx) {
        if (!ctx) return "unknown";
        if (dynamic_cast<id2205040_CSubsetParser::CONST_INTContext*>(ctx))
            return "int";
        if (dynamic_cast<id2205040_CSubsetParser::CONST_FLOATContext*>(ctx))
            return "float";

        if (auto *v = dynamic_cast<id2205040_CSubsetParser::VariableContext*>(ctx)) {
            string name = v->ID()->getText();
            SymbolInfo *s = table.lookup(name);
            if (!s) return "unknown";

            if (v->expression()) {
                if (!s->isArray) return "invalid";
                string idx = typeOf(v->expression());
                if (idx != "int" && idx != "unknown")
                    semanticError(v->getStart()->getLine(),
                                  "Array index of '" + name + "' must be integer");
            } else if (s->isArray) {
                return "invalid";
            }
            return s->type;
        }

        if (auto *f = dynamic_cast<id2205040_CSubsetParser::FactorContext*>(ctx)) {
            if (f->ID() && f->argument_list()) {
                SymbolInfo *s = table.lookup(f->ID()->getText());
                if (!s) {
                    semanticError(f->getStart()->getLine(),
                                  "Undeclared function '" + f->ID()->getText() + "'");
                    return "unknown";
                }
                if (!s->isFunction) {
                    semanticError(f->getStart()->getLine(),
                                  "'" + f->ID()->getText() + "' is not a function");
                    return "unknown";
                }

                vector<string> args = argumentTypes(f->argument_list());
                if (args.size() != s->parameterTypes.size()) {
                    semanticError(f->getStart()->getLine(),
                                  "Function '" + s->name + "' argument count mismatch");
                } else {
                    for (size_t i = 0; i < args.size(); ++i) {
                        if (args[i] != "unknown" && args[i] != s->parameterTypes[i]) {
                            semanticError(f->getStart()->getLine(),
                                "Type mismatch for argument " + to_string(i + 1) +
                                " of function '" + s->name + "'");
                        }
                    }
                }

                if (s->returnType == "void") {
                    semanticError(f->getStart()->getLine(),
                                  "Void function '" + s->name +
                                  "' cannot be used in an expression");
                    return "void";
                }
                return s->returnType;
            }

            if (f->variable()) return typeOf(f->variable());
            if (f->expression()) return typeOf(f->expression());
        }

        if (auto *u = dynamic_cast<id2205040_CSubsetParser::Unary_expressionContext*>(ctx)) {
            if (u->factor()) return typeOf(u->factor());
            if (u->unary_expression()) return typeOf(u->unary_expression());
        }

        if (auto *t = dynamic_cast<id2205040_CSubsetParser::TermContext*>(ctx)) {
            if (t->unary_expression()) return typeOf(t->unary_expression());
            if (t->term()) {
                string l = typeOf(t->term());
                string r = typeOf(t->unary_expression());
                if (t->MULOP() && t->MULOP()->getText() == "%") {
                    if (l != "int" || r != "int")
                        semanticError(t->getStart()->getLine(),
                                      "Operands of % must be integers");
                    return "int";
                }
                return combineNumeric(l, r);
            }
        }

        if (auto *s = dynamic_cast<id2205040_CSubsetParser::Simple_expressionContext*>(ctx)) {
            if (s->term()) return typeOf(s->term());
            if (s->simple_expression()) return combineNumeric(
                typeOf(s->simple_expression()), typeOf(s->term()));
        }

        if (auto *r = dynamic_cast<id2205040_CSubsetParser::Rel_expressionContext*>(ctx)) {
            if (r->simple_expression() && r->RELOP())
                return "int";
            return r->simple_expression() ? typeOf(r->simple_expression()) : "unknown";
        }

        if (auto *l = dynamic_cast<id2205040_CSubsetParser::Logic_expressionContext*>(ctx)) {
            if (l->LOGICOP()) return "int";
            return l->rel_expression() ? typeOf(l->rel_expression()) : "unknown";
        }

        if (auto *e = dynamic_cast<id2205040_CSubsetParser::ExpressionContext*>(ctx)) {
            if (e->ASSIGNOP()) {
                string left = typeOf(e->variable());
                string right = typeOf(e->logic_expression());
                if (left != "unknown" && right != "unknown" &&
                    left != "invalid" && right != "invalid" &&
                    left != right) {
                    semanticError(e->getStart()->getLine(),
                                  "Type mismatch in assignment");
                }
                return left;
            }
            return e->logic_expression() ? typeOf(e->logic_expression()) : "unknown";
        }

        return "unknown";
    }

    string combineNumeric(const string &a, const string &b) {
        if (a == "unknown" || b == "unknown") return "unknown";
        if (a == "float" || b == "float") return "float";
        if (a == "int" && b == "int") return "int";
        return "unknown";
    }

    vector<string> argumentTypes(id2205040_CSubsetParser::Argument_listContext *ctx) {
        vector<string> result;
        if (!ctx || !ctx->arguments()) return result;

        for (auto *arg : ctx->arguments()->logic_expression())
            result.push_back(typeOf(arg));

        return result;
    }

    vector<string> parameterTypes(id2205040_CSubsetParser::Parameter_listContext *ctx) {
        vector<string> result;
        if (!ctx) return result;

        for (auto *t : ctx->type_specifier()) {
            string ty = t->getText();
            result.push_back(ty);
        }
        return result;
    }

    string declaredType(id2205040_CSubsetParser::Type_specifierContext *ctx) {
        return ctx ? ctx->getText() : "unknown";
    }

    void processFunction(int line, const string &name, const string &ret,
                         const vector<string> &params, bool definition) {
        SymbolInfo *old = table.lookupCurrent(name);

        if (!old) {
            SymbolInfo s(name, ret);
            s.isFunction = true;
            s.returnType = ret;
            s.parameterTypes = params;
            s.defined = definition;
            table.insert(s);
        } else if (!old->isFunction) {
            semanticError(line, "Function '" + name +
                                "' conflicts with an existing variable");
        } else {
            if (old->returnType != ret)
                semanticError(line, "Return type mismatch for function '" + name + "'");
            if (old->parameterTypes != params)
                semanticError(line, "Parameter list mismatch for function '" + name + "'");
            if (definition && old->defined)
                semanticError(line, "Multiple definitions of function '" + name + "'");
            if (definition)
                old->defined = true;
        }
    }

public:
    SemanticVisitor() : table() {}

    any visitStart(id2205040_CSubsetParser::StartContext *ctx) override {
        logRule("start", ctx);
        table.enterScope("1");
        auto r = visitChildren(ctx);
        table.printAll(logFile);
        return r;
    }

    any visitProgram(id2205040_CSubsetParser::ProgramContext *ctx) override {
        logRule("program", ctx);
        return visitChildren(ctx);
    }

    any visitUnit(id2205040_CSubsetParser::UnitContext *ctx) override {
        logRule("unit", ctx);
        return visitChildren(ctx);
    }

    any visitVar_declaration(id2205040_CSubsetParser::Var_declarationContext *ctx) override {
        logRule("var_declaration", ctx);

        string ty = declaredType(ctx->type_specifier());

        for (auto *d : ctx->declaration_list()->ID()) {
            string name = d->getText();
            SymbolInfo s(name, ty);
            s.isArray = false;

            // Detect array declarators by inspecting the declaration-list text.
            // The grammar represents each declaration recursively, so the ID
            // immediately followed by '[' belongs to an array declaration.
            string whole = ctx->declaration_list()->getText();
            size_t p = whole.find(name + "[");
            if (p != string::npos) {
                s.isArray = true;
                size_t q = whole.find('[', p);
                size_t z = whole.find(']', q);
                if (q != string::npos && z != string::npos)
                    s.arraySize = stoi(whole.substr(q + 1, z - q - 1));
            }

            if (!table.insert(s))
                semanticError(d->getSymbol()->getLine(),
                              "Multiple declaration of '" + name + "' in the same scope");
        }
        return visitChildren(ctx);
    }

    any visitFunc_declaration(id2205040_CSubsetParser::Func_declarationContext *ctx) override {
        logRule("func_declaration", ctx);

        string ret = declaredType(ctx->type_specifier());
        string name = ctx->ID()->getText();
        vector<string> params = parameterTypes(ctx->parameter_list());
        processFunction(ctx->getStart()->getLine(), name, ret, params, false);
        return visitChildren(ctx);
    }

    any visitFunc_definition(id2205040_CSubsetParser::Func_definitionContext *ctx) override {
        logRule("func_definition", ctx);

        string ret = declaredType(ctx->type_specifier());
        string name = ctx->ID()->getText();
        vector<string> params = parameterTypes(ctx->parameter_list());

        // Functions must be declared/defined before a call. A definition
        // itself introduces the function before its body is visited.
        processFunction(ctx->getStart()->getLine(), name, ret, params, true);

        currentFunction = name;
        currentFunctionReturnType = ret;
        currentParameterTypes = params;
        insideFunction = true;

        table.enterScope("function_" + name);

        if (ctx->parameter_list()) {
            auto *pctx = ctx->parameter_list();
            for (size_t i = 0; i < pctx->ID().size(); ++i) {
                string pn = pctx->ID(i)->getText();
                string pt = pctx->type_specifier(i)->getText();
                SymbolInfo ps(pn, pt);
                if (!table.insert(ps))
                    semanticError(pctx->ID(i)->getSymbol()->getLine(),
                                  "Multiple declaration of parameter '" + pn + "'");
            }
        }

        visit(ctx->compound_statement());

        table.printCurrent(logFile);
        table.exitScope();

        insideFunction = false;
        currentFunction.clear();
        currentFunctionReturnType.clear();
        return nullptr;
    }

    any visitCompound_statement(id2205040_CSubsetParser::Compound_statementContext *ctx) override {
        logRule("compound_statement", ctx);

        // The function-definition visitor already creates the function scope.
        // Nested blocks create their own scopes.
        bool functionBody = insideFunction &&
                            ctx->parent &&
                            dynamic_cast<id2205040_CSubsetParser::Func_definitionContext*>(ctx->parent);

        if (!functionBody) {
            table.enterScope("block");
        }

        auto r = visitChildren(ctx);

        if (!functionBody) {
            table.printCurrent(logFile);
            table.exitScope();
        }
        return r;
    }

    any visitVariable(id2205040_CSubsetParser::VariableContext *ctx) override {
        logRule("variable", ctx);

        string name = ctx->ID()->getText();
        SymbolInfo *s = table.lookup(name);

        if (!s) {
            semanticError(ctx->getStart()->getLine(),
                          "Undeclared variable '" + name + "'");
            return nullptr;
        }

        if (ctx->expression()) {
            if (!s->isArray)
                semanticError(ctx->getStart()->getLine(),
                              "'" + name + "' is not an array");
            typeOf(ctx->expression());
        } else if (s->isArray) {
            semanticError(ctx->getStart()->getLine(),
                          "Array '" + name + "' used without index");
        }
        return nullptr;
    }

    any visitExpression(id2205040_CSubsetParser::ExpressionContext *ctx) override {
        logRule("expression", ctx);
        typeOf(ctx);
        return visitChildren(ctx);
    }

    any visitFactor(id2205040_CSubsetParser::FactorContext *ctx) override {
        logRule("factor", ctx);
        typeOf(ctx);
        return visitChildren(ctx);
    }

    any visitTerm(id2205040_CSubsetParser::TermContext *ctx) override {
        logRule("term", ctx);
        typeOf(ctx);
        return visitChildren(ctx);
    }

    any visitSimple_expression(id2205040_CSubsetParser::Simple_expressionContext *ctx) override {
        logRule("simple_expression", ctx);
        return visitChildren(ctx);
    }

    any visitRel_expression(id2205040_CSubsetParser::Rel_expressionContext *ctx) override {
        logRule("rel_expression", ctx);
        return visitChildren(ctx);
    }

    any visitLogic_expression(id2205040_CSubsetParser::Logic_expressionContext *ctx) override {
        logRule("logic_expression", ctx);
        return visitChildren(ctx);
    }

    any visitReturn(id2205040_CSubsetParser::StatementContext *ctx) {
        if (ctx && ctx->RETURN()) {
            string actual = typeOf(ctx->expression());
            if (insideFunction && currentFunctionReturnType == "void") {
                semanticError(ctx->getStart()->getLine(),
                              "Void function '" + currentFunction +
                              "' cannot return a value");
            } else if (insideFunction && actual != "unknown" &&
                       actual != currentFunctionReturnType) {
                semanticError(ctx->getStart()->getLine(),
                              "Return type mismatch in function '" + currentFunction + "'");
            }
        }
        return nullptr;
    }

    any visitStatement(id2205040_CSubsetParser::StatementContext *ctx) override {
        logRule("statement", ctx);
        visitReturn(ctx);
        return visitChildren(ctx);
    }

    any visitArgument_list(id2205040_CSubsetParser::Argument_listContext *ctx) override {
        logRule("argument_list", ctx);
        return visitChildren(ctx);
    }

    any visitArguments(id2205040_CSubsetParser::ArgumentsContext *ctx) override {
        logRule("arguments", ctx);
        return visitChildren(ctx);
    }

    any visitChildren(tree::ParseTree *node) {
        return id2205040_CSubsetBaseVisitor::visitChildren(node);
    }
};

class SyntaxErrorListener : public BaseErrorListener {
public:
    void syntaxError(Recognizer *, Token *offendingSymbol,
                     size_t line, size_t charPositionInLine,
                     const string &msg, exception_ptr) override {
        errorFile << "Line no " << line << ": Syntax error: " << msg << '\n';
    }
};

ofstream lexLogFile;

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " input.c\n";
        return 1;
    }

    ifstream input(argv[1]);
    if (!input) {
        cerr << "Cannot open input file: " << argv[1] << '\n';
        return 1;
    }

    logFile.open("log.txt");
    errorFile.open("error.txt");
    lexLogFile.open("lexLogFile.txt");

    ANTLRInputStream inputStream(input);
    id2205040_CSubsetLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    id2205040_CSubsetParser parser(&tokens);

    SyntaxErrorListener syntaxListener;
    lexer.removeErrorListeners();
    parser.removeErrorListeners();
    lexer.addErrorListener(&syntaxListener);
    parser.addErrorListener(&syntaxListener);

    tree::ParseTree *tree = parser.start();

    SemanticVisitor visitor;
    visitor.visit(tree);

    logFile << "Line count: " << inputStream.LA(0) << '\n';
    logFile << "Number of syntax/semantic errors: "
            << parser.getNumberOfSyntaxErrors() << '\n';

    logFile.close();
    errorFile.close();
    lexLogFile.close();

    return parser.getNumberOfSyntaxErrors() ? 1 : 0;
}
