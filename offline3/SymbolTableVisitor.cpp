#include "SymbolTableVisitor.h"

#include <sstream>
#include <cctype>
#include <functional>

using namespace antlr4;
using namespace std;

extern std::ofstream errorFile;
extern int errorCount;

typedef std::vector<std::string> ArgVec;

static string trim(const string &s) {
    size_t b = s.find_first_not_of(" \t");
    size_t e = s.find_last_not_of(" \t");
    return (b == string::npos) ? "" : s.substr(b, e - b + 1);
}

static any A(const string &t) { return any(t); }
static string T(const any &v) { return any_cast<string>(v); }
static any AV(const ArgVec &v) { return any(v); }
static ArgVec TV(const any &v) { return any_cast<ArgVec>(v); }

SymbolTableVisitor::SymbolTableVisitor(ofstream &log) : logFile(log) {
    scopes.push_back(SymbolTable());
}

SymbolTableVisitor::~SymbolTableVisitor() {}

void SymbolTableVisitor::logRule(const string &rule, const string &code) const {
    logFile << "Rule: " << rule << "  Code: " << code << endl;
}

void SymbolTableVisitor::semError(int line, const string &msg) {
    errorCount++;
    errorFile << "Line no " << line << ": " << msg << endl;
}

void SymbolTableVisitor::semWarning(int line, const string &msg) {
    errorFile << "Line no " << line << ": Warning: " << msg << endl;
}

void SymbolTableVisitor::printSymbolTable() {
    logFile << "Symbol Table:" << endl;
    for (size_t i = 0; i < scopes.size(); i++) {
        logFile << "Scope " << i << ":" << endl;
        scopes[i].print(logFile);
    }
}

SymbolInfo *SymbolTableVisitor::findSymbol(const string &name) {
    for (size_t i = scopes.size(); i-- > 0;) {
        for (SymbolInfo &s : scopes[i].table) {
            if (s.name == name) return &s;
        }
    }
    return nullptr;
}

SymbolInfo *SymbolTableVisitor::findGlobal(const string &name) {
    if (scopes.empty()) return nullptr;
    for (SymbolInfo &s : scopes[0].table) {
        if (s.name == name) return &s;
    }
    return nullptr;
}

vector<SymbolInfo> SymbolTableVisitor::makeParamList(id2205040_CSubsetParser::Parameter_listContext *ctx) {
    vector<SymbolInfo> v;
    if (!ctx) return v;

    std::vector<tree::TerminalNode *> terminals;
    std::function<void(antlr4::tree::ParseTree *)> walk = [&](antlr4::tree::ParseTree *n) {
        if (auto term = dynamic_cast<tree::TerminalNode *>(n)) {
            terminals.push_back(term);
        } else {
            for (auto c : n->children) walk(c);
        }
    };
    for (auto c : ctx->children) walk(c);

    sort(terminals.begin(), terminals.end(), [](const tree::TerminalNode *a, const tree::TerminalNode *b) {
        return a->getSymbol()->getTokenIndex() < b->getSymbol()->getTokenIndex();
    });

    vector<string> types;
    vector<string> ids;
    for (auto t : terminals) {
        size_t tt = t->getSymbol()->getType();
        if (tt == id2205040_CSubsetParser::INT || tt == id2205040_CSubsetParser::FLOAT ||
            tt == id2205040_CSubsetParser::VOID) {
            types.push_back(t->getText());
        } else if (tt == id2205040_CSubsetParser::ID) {
            ids.push_back(t->getText());
        }
    }
    for (size_t i = 0; i < types.size(); i++) {
        string name = (i < ids.size()) ? ids[i] : string("");
        SymbolInfo info(name, types[i]);
        v.push_back(info);
    }
    return v;
}

any SymbolTableVisitor::visitStart(id2205040_CSubsetParser::StartContext *ctx) {
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitProgram(id2205040_CSubsetParser::ProgramContext *ctx) {
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitUnit(id2205040_CSubsetParser::UnitContext *ctx) {
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitFunc_declaration(id2205040_CSubsetParser::Func_declarationContext *ctx) {
    int line = ctx->getStart()->getLine();
    string ret = ctx->type_specifier()->getText();
    string fname = ctx->ID()->getText();
    logRule("func_declaration", ctx->getText());
    vector<SymbolInfo> params = makeParamList(ctx->parameter_list());
    SymbolInfo *g = findGlobal(fname);
    if (g) {
        if (g->isFunction) {
            if (g->type != ret) semError(line, "Return type mismatch in declaration of function '" + fname + "'");
            if ((int)params.size() != g->paramCount)
                semError(line, "Parameter count mismatch in declaration of function '" + fname + "'");
            else
                for (size_t i = 0; i < params.size(); i++)
                    if (params[i].type != g->paramTypes[i]) {
                        semError(line, "Parameter type mismatch in declaration of function '" + fname + "'");
                        break;
                    }
            if (g->defined) semError(line, "Function '" + fname + "' is already defined");
        } else {
            semError(line, "'" + fname + "' conflicts with an existing variable in the same scope");
        }
    } else {
        SymbolInfo fi(fname, ret);
        fi.isFunction = true;
        fi.defined = false;
        fi.paramCount = (int)params.size();
        for (auto &p : params) fi.paramTypes.push_back(p.type);
        scopes[0].insert(fi);
    }
    pendingParams.clear();
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitFunc_definition(id2205040_CSubsetParser::Func_definitionContext *ctx) {
    int line = ctx->getStart()->getLine();
    string ret = ctx->type_specifier()->getText();
    string fname = ctx->ID()->getText();
    logRule("func_definition", ctx->getText());
    vector<SymbolInfo> params = makeParamList(ctx->parameter_list());
    SymbolInfo *g = findGlobal(fname);
    if (g) {
        if (g->isFunction) {
            if (g->type != ret) semError(line, "Return type mismatch for function '" + fname + "'");
            if ((int)params.size() != g->paramCount)
                semError(line, "Parameter count mismatch for function '" + fname + "'");
            else
                for (size_t i = 0; i < params.size(); i++)
                    if (params[i].type != g->paramTypes[i]) {
                        semError(line, "Parameter type mismatch for function '" + fname + "'");
                        break;
                    }
            if (g->defined) semError(line, "Multiple definitions of function '" + fname + "'");
            else g->defined = true;
        } else {
            semError(line, "Function name '" + fname + "' conflicts with an existing variable");
        }
    } else {
        SymbolInfo fi(fname, ret);
        fi.isFunction = true;
        fi.defined = true;
        fi.paramCount = (int)params.size();
        for (auto &p : params) fi.paramTypes.push_back(p.type);
        scopes[0].insert(fi);
    }
    pendingParams = params;
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitParameter_list(id2205040_CSubsetParser::Parameter_listContext *ctx) {
    logRule("parameter_list", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitCompound_statement(id2205040_CSubsetParser::Compound_statementContext *ctx) {
    scopes.push_back(SymbolTable());
    for (SymbolInfo &p : pendingParams) {
        if (!p.name.empty()) scopes.back().insert(p);
    }
    pendingParams.clear();
    logRule("compound_statement", ctx->getText());
    visitChildren(ctx);
    logFile << "Exiting scope. Symbol table:" << endl;
    printSymbolTable();
    scopes.pop_back();
    return A("void");
}

any SymbolTableVisitor::visitVar_declaration(id2205040_CSubsetParser::Var_declarationContext *ctx) {
    int line = ctx->getStart()->getLine();
    string type = ctx->type_specifier()->getText();

    string text = ctx->declaration_list()->getText();
    stringstream ss(text);
    string decl;
    while (getline(ss, decl, ',')) {
        string token = trim(decl);
        string name;
        bool isArray = false;
        int size = 0;
        size_t lbrack = token.find('[');
        if (lbrack != string::npos) {
            isArray = true;
            name = token.substr(0, lbrack);
            size_t start = token.find_first_of("0123456789", lbrack);
            size_t end = token.find(']', lbrack);
            if (start != string::npos && end != string::npos) {
                size = stoi(token.substr(start, end - start));
            }
        } else {
            name = token;
        }
        SymbolInfo info(name, type);
        info.isArray = isArray;
        info.arraySize = size;
        bool ok = scopes.back().insert(info);
        if (!ok) {
            semError(line, "Multiple declaration of '" + name + "' in the same scope");
        }
    }

    logRule("var_declaration", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitType_specifier(id2205040_CSubsetParser::Type_specifierContext *ctx) {
    logRule("type_specifier", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitDeclaration_list(id2205040_CSubsetParser::Declaration_listContext *ctx) {
    logRule("declaration_list", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitStatements(id2205040_CSubsetParser::StatementsContext *ctx) {
    logRule("statements", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitStatement(id2205040_CSubsetParser::StatementContext *ctx) {
    logRule("statement", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitExpression_statement(id2205040_CSubsetParser::Expression_statementContext *ctx) {
    logRule("expression_statement", ctx->getText());
    return visitChildren(ctx);
}

any SymbolTableVisitor::visitVariable(id2205040_CSubsetParser::VariableContext *ctx) {
    int line = ctx->getStart()->getLine();
    string name = ctx->ID()->getText();
    logRule("variable", ctx->getText());
    SymbolInfo *s = findSymbol(name);
    if (!s) {
        semError(line, "Undeclared variable '" + name + "'");
        return A("error");
    }
    if (ctx->LTHIRD()) {
        if (!s->isArray) {
            semError(line, "Index used with non-array variable '" + name + "'");
        }
        string it = "int";
        if (ctx->expression()) it = T(visit(ctx->expression()));
        if (it != "int" && it != "error") {
            semError(line, "Array index of '" + name + "' must be an integer");
        }
        return A(s->type);
    }
    return A(s->type);
}

any SymbolTableVisitor::visitExpression(id2205040_CSubsetParser::ExpressionContext *ctx) {
    logRule("expression", ctx->getText());
    if (ctx->ASSIGNOP()) {
        int line = ctx->getStart()->getLine();
        string lv = T(visit(ctx->variable()));
        string rv = T(visit(ctx->logic_expression()));
        if (lv == "error" || rv == "error") return A("error");
        if (lv == "void") { semError(line, "Cannot assign to a function"); return A(lv); }
        if (rv == "void") { semError(line, "Cannot assign a void value"); return A("error"); }
        if (lv != rv) {
            if (lv == "int" && rv == "float") {
                semWarning(line, "Possible loss of data: assigning float to int");
            } else if (lv == "float" && rv == "int") {
                // int implicitly promoted to float
            } else {
                semError(line, "Assignment type mismatch between '" + lv + "' and '" + rv + "'");
            }
        }
        return A(lv);
    }
    return visit(ctx->logic_expression());
}

any SymbolTableVisitor::visitLogic_expression(id2205040_CSubsetParser::Logic_expressionContext *ctx) {
    logRule("logic_expression", ctx->getText());
    if (ctx->LOGICOP()) {
        T(visit(ctx->rel_expression(0)));
        T(visit(ctx->rel_expression(1)));
        return A("int");
    }
    return visit(ctx->rel_expression(0));
}

any SymbolTableVisitor::visitRel_expression(id2205040_CSubsetParser::Rel_expressionContext *ctx) {
    logRule("rel_expression", ctx->getText());
    if (ctx->RELOP()) {
        T(visit(ctx->simple_expression(0)));
        T(visit(ctx->simple_expression(1)));
        return A("int");
    }
    return visit(ctx->simple_expression(0));
}

any SymbolTableVisitor::visitSimple_expression(id2205040_CSubsetParser::Simple_expressionContext *ctx) {
    logRule("simple_expression", ctx->getText());
    if (ctx->ADDOP()) {
        string l = T(visit(ctx->simple_expression()));
        string r = T(visit(ctx->term()));
        if (l == "float" || r == "float") return A("float");
        return A("int");
    }
    return visit(ctx->term());
}

any SymbolTableVisitor::visitTerm(id2205040_CSubsetParser::TermContext *ctx) {
    logRule("term", ctx->getText());
    if (ctx->MULOP()) {
        int line = ctx->getStart()->getLine();
        string l = T(visit(ctx->term()));
        string r = T(visit(ctx->unary_expression()));
        string op = ctx->MULOP()->getText();
        if (op == "%") {
            if (l != "int" || r != "int") {
                semError(line, "Operands of modulus operator must be integers");
            }
            return A("int");
        }
        if (l == "float" || r == "float") return A("float");
        return A("int");
    }
    return visit(ctx->unary_expression());
}

any SymbolTableVisitor::visitUnary_expression(id2205040_CSubsetParser::Unary_expressionContext *ctx) {
    logRule("unary_expression", ctx->getText());
    if (ctx->NOT()) {
        T(visit(ctx->unary_expression()));
        return A("int");
    }
    if (ctx->ADDOP()) {
        string t = T(visit(ctx->unary_expression()));
        return A((t == "float") ? "float" : "int");
    }
    return visit(ctx->factor());
}

any SymbolTableVisitor::visitFactor(id2205040_CSubsetParser::FactorContext *ctx) {
    int line = ctx->getStart()->getLine();
    logRule("factor", ctx->getText());
    if (ctx->CONST_INT()) return A("int");
    if (ctx->CONST_FLOAT()) return A("float");
    if (ctx->LPAREN() && ctx->expression()) return visit(ctx->expression());
    if (ctx->ID()) {
        string fname = ctx->ID()->getText();
        SymbolInfo *s = findSymbol(fname);
        if (!s) {
            semError(line, "Undeclared function '" + fname + "'");
            return A("error");
        }
        if (!s->isFunction) {
            semError(line, "'" + fname + "' is not a function");
            return A("error");
        }
        ArgVec at;
        if (ctx->argument_list()) at = TV(visit(ctx->argument_list()));
        if ((int)at.size() != s->paramCount) {
            semError(line, "Function '" + fname + "' expects " + to_string(s->paramCount) +
                           " arguments but " + to_string(at.size()) + " given");
        } else {
            for (size_t i = 0; i < at.size(); i++) {
                if (at[i] != s->paramTypes[i]) {
                    semError(line, "Type mismatch for argument " + to_string(i + 1) +
                                   " in call to '" + fname + "'");
                    break;
                }
            }
        }
        if (s->type == "void") {
            semError(line, "Void function '" + fname + "' used in an expression");
        }
        return A(s->type);
    }
    if (ctx->variable()) {
        string vname = ctx->variable()->ID()->getText();
        SymbolInfo *s = findSymbol(vname);
        if (s && s->isFunction) {
            semError(line, "'" + vname + "' is a function used as a variable");
            return A("error");
        }
        if (s && s->isArray && !ctx->INCOP() && !ctx->DECOP() && !ctx->variable()->LTHIRD()) {
            semError(line, "Array '" + vname + "' used without an index");
        }
        return visit(ctx->variable());
    }
    return A("error");
}

any SymbolTableVisitor::visitArgument_list(id2205040_CSubsetParser::Argument_listContext *ctx) {
    logRule("argument_list", ctx->getText());
    if (ctx->arguments()) return visit(ctx->arguments());
    return AV(ArgVec{});
}

any SymbolTableVisitor::visitArguments(id2205040_CSubsetParser::ArgumentsContext *ctx) {
    logRule("arguments", ctx->getText());
    ArgVec types;
    if (ctx->arguments()) types = TV(visit(ctx->arguments()));
    types.push_back(T(visit(ctx->logic_expression())));
    return AV(types);
}