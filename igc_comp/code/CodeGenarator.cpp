#include "CodeGenarator.h"

// ==========================================
// Constructor / Destructor
// ==========================================

CodeGenarator::CodeGenarator(const string& outputFileName)
    : labelCount(0), currentLine(1), paramCount(0) {
    asmFile.open(outputFileName);
    if (!asmFile.is_open()) {
        cerr << "Error: Cannot open output file " << outputFileName << endl;
    }
}

CodeGenarator::~CodeGenarator() {
    if (asmFile.is_open()) asmFile.close();
}

// ==========================================
// Helper Methods
// ==========================================

string CodeGenarator::newLabel(const string& prefix) {
    return prefix + "_" + to_string(labelCount++);
}

void CodeGenarator::emit(const string& instruction) {
    asmFile << "    " << instruction << endl;
}

void CodeGenarator::emitWithComment(const string& instruction, const string& comment) {
    asmFile << "    " << instruction << "    ; " << comment << endl;
}

string CodeGenarator::getOperandAddr(SymbolInfo* sym) {
    if (sym->getIsGlobal()) {
        return "[" + sym->getName() + "]";
    }
    return "[EBP-" + to_string(sym->getStackOffset()) + "]";
}

string CodeGenarator::getArrayAddr(SymbolInfo* sym, const string& indexReg) {
    if (sym->getIsGlobal()) {
        return "[" + sym->getName() + " + " + indexReg + "]";
    }
    int baseOffset = sym->getStackOffset();
    return "[EBP + " + indexReg + " - " + to_string(baseOffset) + "]";
}

string CodeGenarator::relOpToJump(const string& op, bool negate) {
    if (!negate) {
        if (op == "==") return "JNE";
        if (op == "!=") return "JE";
        if (op == "<")  return "JGE";
        if (op == ">")  return "JLE";
        if (op == "<=") return "JG";
        if (op == ">=") return "JL";
    } else {
        if (op == "==") return "JE";
        if (op == "!=") return "JNE";
        if (op == "<")  return "JL";
        if (op == ">")  return "JG";
        if (op == "<=") return "JLE";
        if (op == ">=") return "JGE";
    }
    return "JMP";
}

// ==========================================
// Assembly Structure Methods
// ==========================================

void CodeGenarator::generateHeader() {
    asmFile << "format ELF executable 3" << endl;
    asmFile << "entry main" << endl;
}

void CodeGenarator::generateDataSegment() {
    if (!dataSegmentGlobals.empty()) {
        asmFile << endl;
        asmFile << "segment readable writeable" << endl;
        for (const string& name : dataSegmentGlobals) {
            asmFile << "    " << name << " dd 1 dup (0)" << endl;
        }
    }
}

void CodeGenarator::generateCodeSegmentStart() {
    asmFile << endl;
    asmFile << "segment readable executable" << endl;
}

void CodeGenarator::generateOutdecProcedure() {
    asmFile << endl;
    asmFile << "; ---- OUTDEC: print integer in EAX as decimal, followed by newline ----" << endl;
    asmFile << "OUTDEC:" << endl;
    asmFile << "    PUSH EBX" << endl;
    asmFile << "    PUSH ECX" << endl;
    asmFile << "    PUSH EDX" << endl;
    asmFile << "    PUSH ESI" << endl;
    asmFile << "    OR   EAX, EAX" << endl;
    asmFile << "    JGE  OUTDEC_POSITIVE" << endl;
    asmFile << "    NEG  EAX" << endl;
    asmFile << "    PUSH EAX" << endl;
    asmFile << "    SUB  ESP, 4" << endl;
    asmFile << "    MOV  byte [ESP], '-'" << endl;
    asmFile << "    MOV  EAX, 4" << endl;
    asmFile << "    MOV  EBX, 1" << endl;
    asmFile << "    MOV  ECX, ESP" << endl;
    asmFile << "    MOV  EDX, 1" << endl;
    asmFile << "    INT  0x80" << endl;
    asmFile << "    ADD  ESP, 4" << endl;
    asmFile << "    POP  EAX" << endl;
    asmFile << "OUTDEC_POSITIVE:" << endl;
    asmFile << "    XOR  ECX, ECX" << endl;
    asmFile << "    MOV  EBX, 10" << endl;
    asmFile << "OUTDEC_DIGIT_LOOP:" << endl;
    asmFile << "    XOR  EDX, EDX" << endl;
    asmFile << "    DIV  EBX" << endl;
    asmFile << "    ADD  DL, 30h" << endl;
    asmFile << "    PUSH EDX" << endl;
    asmFile << "    INC  ECX" << endl;
    asmFile << "    TEST EAX, EAX" << endl;
    asmFile << "    JNZ  OUTDEC_DIGIT_LOOP" << endl;
    asmFile << "    MOV  ESI, ECX" << endl;
    asmFile << "    MOV  EBX, 1" << endl;
    asmFile << "    MOV  EDX, 1" << endl;
    asmFile << "OUTDEC_PRINT_LOOP:" << endl;
    asmFile << "    TEST ESI, ESI" << endl;
    asmFile << "    JZ   OUTDEC_NEWLINE" << endl;
    asmFile << "    MOV  EAX, 4" << endl;
    asmFile << "    MOV  ECX, ESP" << endl;
    asmFile << "    INT  0x80" << endl;
    asmFile << "    ADD  ESP, 4" << endl;
    asmFile << "    DEC  ESI" << endl;
    asmFile << "    JMP  OUTDEC_PRINT_LOOP" << endl;
    asmFile << "OUTDEC_NEWLINE:" << endl;
    asmFile << "    SUB  ESP, 4" << endl;
    asmFile << "    MOV  byte [ESP], 10" << endl;
    asmFile << "    MOV  EAX, 4" << endl;
    asmFile << "    MOV  ECX, ESP" << endl;
    asmFile << "    INT  0x80" << endl;
    asmFile << "    ADD  ESP, 4" << endl;
    asmFile << "    POP  ESI" << endl;
    asmFile << "    POP  EDX" << endl;
    asmFile << "    POP  ECX" << endl;
    asmFile << "    POP  EBX" << endl;
    asmFile << "    RET" << endl;
}

void CodeGenarator::generateFunctionPrologue(const string& funcName) {
    asmFile << endl;
    asmFile << funcName << ":" << endl;
    emit("PUSH EBP");
    emit("MOV  EBP, ESP");
}

void CodeGenarator::generateFunctionEpilogue(const string& funcName, int localSize, int paramSize) {
    string exitLabel = funcName + "_exit";
    asmFile << endl;
    asmFile << exitLabel << ":" << endl;
    if (localSize > 0) {
        emit("ADD  ESP, " + to_string(localSize));
    }
    emit("POP  EBP");
    if (paramSize > 0) {
        emit("RET " + to_string(paramSize));
    } else {
        emit("RET");
    }
}

void CodeGenarator::finalize() {
    asmFile.close();
}

// ==========================================
// Visitor Implementations
// ==========================================

antlrcpp::Any CodeGenarator::visitStart(C4Parser::StartContext* ctx) {
    return visit(ctx->program());
}

antlrcpp::Any CodeGenarator::visitProgram(C4Parser::ProgramContext* ctx) {
    // First pass: collect global variables by visiting var_declarations
    for (auto* unit : ctx->unit()) {
        if (unit->var_declaration()) {
            visit(unit->var_declaration());
        }
    }

    generateHeader();
    generateDataSegment();
    generateCodeSegmentStart();

    // Second pass: generate code for functions
    for (auto* unit : ctx->unit()) {
        if (unit->func_definition()) {
            visit(unit->func_definition());
        }
    }

    generateOutdecProcedure();
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitUnit(C4Parser::UnitContext* ctx) {
    if (ctx->var_declaration()) {
        visit(ctx->var_declaration());
    } else if (ctx->func_declaration()) {
        visit(ctx->func_declaration());
    } else if (ctx->func_definition()) {
        visit(ctx->func_definition());
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitFunc_declaration(C4Parser::Func_declarationContext* ctx) {
    // Count parameters and store function info in symbol table
    string funcName = ctx->ID()->getText();
    string returnType = ctx->type_specifier()->getText();
    int numParams = 0;
    if (ctx->parameter_list()) {
        numParams = (int)ctx->parameter_list()->ID().size();
    }
    symbolTable.insert(funcName, returnType);
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitFunc_definition(C4Parser::Func_definitionContext* ctx) {
    string funcName = ctx->ID()->getText();
    string returnType = ctx->type_specifier()->getText();
    currentFunctionName = funcName;

    // Count parameters
    paramCount = 0;
    if (ctx->parameter_list()) {
        paramCount = (int)ctx->parameter_list()->ID().size();
    }

    // Insert function into symbol table
    symbolTable.insert(funcName, returnType);

    // Generate function prologue
    generateFunctionPrologue(funcName);

    // Enter new scope for function body
    symbolTable.enterScope();

    // Add parameters to scope with [EBP+8], [EBP+12], etc.
    int paramOffset = 8;
    if (ctx->parameter_list()) {
        auto* paramList = ctx->parameter_list();
        // Collect parameters in order (left-to-right in grammar)
        vector<pair<string, string>> params;
        if (paramList->type_specifier().size() > 0) {
            // Recurse into parameter_list to collect params
            // The grammar is left-recursive: parameter_list COMMA type_specifier ID
            // We need to collect params in left-to-right order
            auto collectParams = [&](auto&& self, C4Parser::Parameter_listContext* pl) -> void {
                if (pl->parameter_list()) {
                    self(self, pl->parameter_list());
                }
                string pType = pl->type_specifier()->getText();
                if (!pl->ID().empty()) {
                    string pName = pl->ID().back()->getText();
                    params.push_back({pName, pType});
                }
            };
            collectParams(collectParams, paramList);
        }

        // Insert parameters into symbol table
        for (auto& p : params) {
            SymbolInfo* sym = new SymbolInfo(p.first, p.second);
            sym->setIdentityType("VAR");
            sym->setIsGlobal(false);
            sym->setStackOffset(paramOffset);
            symbolTable.insert(sym);
            paramOffset += 4;
        }
    }

    // Visit compound statement (body)
    visit(ctx->compound_statement());

    // Generate function epilogue
    ScopeTable* funcScope = symbolTable.getCurrentScope();
    int localSize = funcScope->getCurrentStackOffset();
    symbolTable.exitScope();

    generateFunctionEpilogue(funcName, localSize, paramCount * 4);

    return nullptr;
}

antlrcpp::Any CodeGenarator::visitParameter_list(C4Parser::Parameter_listContext* ctx) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitCompound_statement(C4Parser::Compound_statementContext* ctx) {
    if (ctx->statements()) {
        visit(ctx->statements());
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitVar_declaration(C4Parser::Var_declarationContext* ctx) {
    string type = ctx->type_specifier()->getText();
    if (type == "void") return nullptr;

    auto* declList = ctx->declaration_list();
    // Collect all declarations from the list
    vector<pair<string, int>> declarations; // (name, arraySize or 0)

    auto collectDecls = [&](auto&& self, C4Parser::Declaration_listContext* dl) -> void {
        if (dl->declaration_list()) {
            self(self, dl->declaration_list());
        }
        string name = dl->ID()->getText();
        int arrSize = 0;
        if (dl->CONST_INT().size() > 0) {
            arrSize = stoi(dl->CONST_INT().back()->getText());
        }
        declarations.push_back({name, arrSize});
    };
    collectDecls(collectDecls, declList);

    bool isGlobal = (symbolTable.getCurrentScope()->getId() == 1);

    for (auto& decl : declarations) {
        if (isGlobal) {
            dataSegmentGlobals.push_back(decl.first);
        } else {
            if (decl.second > 0) {
                symbolTable.allocateLocalArray(decl.first, type, decl.second);
            } else {
                symbolTable.allocateLocalVar(decl.first, type);
            }
        }
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitType_specifier(C4Parser::Type_specifierContext* ctx) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitDeclaration_list(C4Parser::Declaration_listContext* ctx) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitStatements(C4Parser::StatementsContext* ctx) {
    for (auto* stmt : ctx->statement()) {
        visit(stmt);
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitStatement(C4Parser::StatementContext* ctx) {
    if (ctx->var_declaration()) {
        visit(ctx->var_declaration());
    } else if (ctx->expression_statement()) {
        visit(ctx->expression_statement());
    } else if (ctx->compound_statement()) {
        symbolTable.enterScope();
        visit(ctx->compound_statement());
        symbolTable.exitScope();
    } else if (ctx->FOR()) {
        // FOR LPAREN expression_statement expression_statement expression RPAREN statement
        string startLabel = newLabel("for_start");
        string endLabel = newLabel("for_end");
        string bodyLabel = newLabel("for_body");

        symbolTable.enterScope();

        // Initializer
        if (ctx->expression_statement().size() > 0) {
            visit(ctx->expression_statement()[0]);
        }

        asmFile << startLabel << ":" << endl;

        // Condition
        if (ctx->expression_statement().size() > 1) {
            visit(ctx->expression_statement()[1]);
            // The condition result is in EAX; if 0, exit
            emit("TEST EAX, EAX");
            emit("JE   " + endLabel);
        }

        // Body
        visit(ctx->statement());

        // Increment (third expression in FOR header)
        if (ctx->expression()) {
            visit(ctx->expression());
        }

        emit("JMP  " + startLabel);
        asmFile << endLabel << ":" << endl;

        symbolTable.exitScope();
    } else if (ctx->IF() && !ctx->ELSE()) {
        // IF LPAREN expression RPAREN statement
        string endLabel = newLabel("if_end");
        visit(ctx->expression());
        emit("TEST EAX, EAX");
        emit("JE   " + endLabel);
        visit(ctx->statement()[0]);
        asmFile << endLabel << ":" << endl;
    } else if (ctx->IF() && ctx->ELSE()) {
        // IF LPAREN expression RPAREN statement ELSE statement
        string elseLabel = newLabel("if_else");
        string endLabel = newLabel("if_end");
        visit(ctx->expression());
        emit("TEST EAX, EAX");
        emit("JE   " + elseLabel);
        visit(ctx->statement()[0]);
        emit("JMP  " + endLabel);
        asmFile << elseLabel << ":" << endl;
        visit(ctx->statement()[1]);
        asmFile << endLabel << ":" << endl;
    } else if (ctx->WHILE()) {
        // WHILE LPAREN expression RPAREN statement
        string startLabel = newLabel("while_start");
        string endLabel = newLabel("while_end");

        symbolTable.enterScope();

        asmFile << startLabel << ":" << endl;
        visit(ctx->expression());
        emit("TEST EAX, EAX");
        emit("JE   " + endLabel);
        visit(ctx->statement());
        emit("JMP  " + startLabel);
        asmFile << endLabel << ":" << endl;

        symbolTable.exitScope();
    } else if (ctx->PRINTLN()) {
        // PRINTLN LPAREN ID RPAREN SEMICOLON
        string varName = ctx->ID()->getText();
        SymbolInfo* sym = symbolTable.lookUp(varName);
        if (sym) {
            emit("PUSH EAX");
            emit("MOV  EAX, " + getOperandAddr(sym));
            emit("CALL OUTDEC");
            emit("POP  EAX");
        }
    } else if (ctx->RETURN()) {
        // RETURN expression SEMICOLON
        visit(ctx->expression());
        string exitLabel = currentFunctionName + "_exit";
        emit("JMP  " + exitLabel);
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitExpression_statement(C4Parser::Expression_statementContext* ctx) {
    if (ctx->expression()) {
        visit(ctx->expression());
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitVariable(C4Parser::VariableContext* ctx) {
    string varName = ctx->ID()->getText();
    SymbolInfo* sym = symbolTable.lookUp(varName);
    if (!sym) return nullptr;

    if (ctx->expression()) {
        // Array access: ID LTHIRD expression RTHIRD
        // Compute byte offset = index * 4, result in EAX
        visit(ctx->expression());
        emit("MOV  EBX, 4");
        emit("MUL  EBX"); // EAX = byte offset

        if (sym->getIsGlobal()) {
            emit("MOV  EAX, [" + sym->getName() + " + EAX]");
        } else {
            // Local array: arr[i] at [EBP - baseOffset - i*4]
            int baseOffset = sym->getStackOffset();
            emit("ADD  EAX, " + to_string(baseOffset));
            emit("NEG  EAX");
            emit("MOV  EAX, [EBP + EAX]");
        }
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitExpression(C4Parser::ExpressionContext* ctx) {
    if (ctx->ASSIGNOP()) {
        string varName = ctx->variable()->ID()->getText();
        SymbolInfo* sym = symbolTable.lookUp(varName);
        if (!sym) return nullptr;

        if (sym->getIdentityType() == "ARRAY" && ctx->variable()->expression()) {
            // Array assignment: arr[expr] = expr
            // Step 1: Compute byte offset, push it
            visit(ctx->variable()->expression());
            emit("MOV  EBX, 4");
            emit("MUL  EBX");
            emit("PUSH EAX");

            // Step 2: Evaluate RHS expression
            visit(ctx->logic_expression());

            // Step 3: Pop byte offset into EBX
            emit("POP  EBX");

            // Step 4: Store value into array
            if (sym->getIsGlobal()) {
                emit("MOV  [" + sym->getName() + " + EBX], EAX");
            } else {
                int baseOffset = sym->getStackOffset();
                emit("ADD  EBX, " + to_string(baseOffset));
                emit("NEG  EBX");
                emit("MOV  [EBP + EBX], EAX");
            }
        } else {
            // Simple variable assignment: var = expr
            visit(ctx->logic_expression());
            if (sym->getIsGlobal()) {
                emit("MOV  [" + sym->getName() + "], EAX");
            } else {
                emit("MOV  [EBP-" + to_string(sym->getStackOffset()) + "], EAX");
            }
        }
    } else {
        visit(ctx->logic_expression());
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitLogic_expression(C4Parser::Logic_expressionContext* ctx) {
    if (ctx->LOGICOP()) {
        string op = ctx->LOGICOP()->getText();
        string endLabel = newLabel("logic_end");
        string falseLabel = newLabel("logic_false");

        visit(ctx->rel_expression()[0]); // left in EAX
        emit("TEST EAX, EAX");

        if (op == "&&") {
            emit("JE   " + falseLabel); // short-circuit: left false => result 0
            visit(ctx->rel_expression()[1]); // right in EAX
            emit("TEST EAX, EAX");
            emit("MOV  EAX, 0");
            emit("JE   " + endLabel);
            emit("MOV  EAX, 1");
            emit("JMP  " + endLabel);
            asmFile << falseLabel << ":" << endl;
            emit("MOV  EAX, 0");
        } else { // ||
            emit("JNE  " + falseLabel); // short-circuit: left true => result 1
            visit(ctx->rel_expression()[1]); // right in EAX
            emit("TEST EAX, EAX");
            emit("MOV  EAX, 0");
            emit("JE   " + endLabel);
            emit("MOV  EAX, 1");
            emit("JMP  " + endLabel);
            asmFile << falseLabel << ":" << endl;
            emit("MOV  EAX, 1");
        }
        asmFile << endLabel << ":" << endl;
    } else {
        visit(ctx->rel_expression()[0]);
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitRel_expression(C4Parser::Rel_expressionContext* ctx) {
    if (ctx->RELOP()) {
        string op = ctx->RELOP()->getText();
        string trueLabel = newLabel("relop_true");
        string endLabel = newLabel("relop_end");

        visit(ctx->simple_expression()[0]); // left in EAX
        emit("PUSH EAX");
        visit(ctx->simple_expression()[1]); // right in EAX
        emit("POP  EBX");
        // Compare: left (EBX) vs right (EAX)
        emit("CMP  EBX, EAX");
        emit(relOpToJump(op) + " " + trueLabel);
        emit("MOV  EAX, 0");
        emit("JMP  " + endLabel);
        asmFile << trueLabel << ":" << endl;
        emit("MOV  EAX, 1");
        asmFile << endLabel << ":" << endl;
    } else {
        visit(ctx->simple_expression()[0]);
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitSimple_expression(C4Parser::Simple_expressionContext* ctx) {
    if (ctx->ADDOP().size() > 0) {
        // Left-associative: simple_expression ADDOP term
        visit(ctx->term()[0]); // leftmost term in EAX
        for (size_t i = 0; i < ctx->ADDOP().size(); i++) {
            string op = ctx->ADDOP()[i]->getText();
            emit("PUSH EAX");
            visit(ctx->term()[i + 1]); // next term in EAX
            emit("POP  EBX");
            if (op == "+") {
                emit("ADD  EAX, EBX");
            } else {
                // EAX = right, EBX = left; we want left - right
                emit("XCHG EAX, EBX");
                emit("SUB  EAX, EBX");
            }
        }
    } else {
        visit(ctx->term()[0]);
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitTerm(C4Parser::TermContext* ctx) {
    if (ctx->MULOP().size() > 0) {
        visit(ctx->unary_expression()[0]);
        for (size_t i = 0; i < ctx->MULOP().size(); i++) {
            string op = ctx->MULOP()[i]->getText();
            emit("PUSH EAX");
            visit(ctx->unary_expression()[i + 1]);
            emit("POP  EBX");
            if (op == "*") {
                emit("MUL  EBX");
            } else if (op == "/") {
                emit("XCHG EAX, EBX");
                emit("XOR  EDX, EDX");
                emit("DIV  EBX");
            } else { // %
                emit("XCHG EAX, EBX");
                emit("XOR  EDX, EDX");
                emit("DIV  EBX");
                emit("MOV  EAX, EDX");
            }
        }
    } else {
        visit(ctx->unary_expression()[0]);
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitUnary_expression(C4Parser::Unary_expressionContext* ctx) {
    if (ctx->ADDOP()) {
        visit(ctx->unary_expression());
        string op = ctx->ADDOP()->getText();
        if (op == "-") {
            emit("NEG  EAX");
        }
        // "+" is a no-op
    } else if (ctx->NOT()) {
        visit(ctx->unary_expression());
        string trueLabel = newLabel("not_true");
        string endLabel = newLabel("not_end");
        emit("TEST EAX, EAX");
        emit("JNE  " + trueLabel);
        emit("MOV  EAX, 1");
        emit("JMP  " + endLabel);
        asmFile << trueLabel << ":" << endl;
        emit("MOV  EAX, 0");
        asmFile << endLabel << ":" << endl;
    } else {
        visit(ctx->factor());
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitFactor(C4Parser::FactorContext* ctx) {
    if (ctx->variable()) {
        string varName = ctx->variable()->ID()->getText();
        SymbolInfo* sym = symbolTable.lookUp(varName);

        if (ctx->INCOP() || ctx->DECOP()) {
            bool isInc = ctx->INCOP() != nullptr;
            if (sym && sym->getIdentityType() == "ARRAY" && ctx->variable()->expression()) {
                // Array INC/DEC: compute index, load old value, save, inc/dec, restore old value
                visit(ctx->variable()->expression());
                emit("MOV  EBX, 4");
                emit("MUL  EBX"); // byte offset in EAX
                emit("MOV  ECX, EAX"); // ECX = byte offset

                // Load current value
                if (sym->getIsGlobal()) {
                    emit("MOV  EAX, [" + sym->getName() + " + ECX]");
                } else {
                    int baseOffset = sym->getStackOffset();
                    emit("MOV  EDX, ECX");
                    emit("ADD  EDX, " + to_string(baseOffset));
                    emit("NEG  EDX");
                    emit("MOV  EAX, [EBP + EDX]");
                }
                emit("PUSH EAX"); // save old value

                // Increment/decrement in memory
                string op = isInc ? "INC" : "DEC";
                if (sym->getIsGlobal()) {
                    emit(op + "  dword [" + sym->getName() + " + ECX]");
                } else {
                    int baseOffset = sym->getStackOffset();
                    emit("MOV  EDX, ECX");
                    emit("ADD  EDX, " + to_string(baseOffset));
                    emit("NEG  EDX");
                    emit(op + "  dword [EBP + EDX]");
                }
                emit("POP  EAX"); // old value in EAX
            } else {
                // Simple variable INC/DEC: x++ or x--
                visit(ctx->variable()); // load variable (result unused for simple var, but sym lookup needed)
                emit("MOV  EAX, " + getOperandAddr(sym));
                emit("PUSH EAX"); // save old value
                string op = isInc ? "INC" : "DEC";
                emit(op + "  " + getOperandAddr(sym));
                emit("POP  EAX"); // old value in EAX
            }
        } else {
            // Plain variable read (no INC/DEC)
            visit(ctx->variable());
            if (sym) {
                emit("MOV  EAX, " + getOperandAddr(sym));
            }
        }
    } else if (ctx->ID() && ctx->argument_list()) {
        // Function call: ID LPAREN argument_list RPAREN
        string funcName = ctx->ID()->getText();

        // Evaluate and push arguments (right-to-left)
        int numArgs = 0;
        if (ctx->argument_list()->arguments()) {
            // Count arguments
            auto* args = ctx->argument_list()->arguments();
            auto countArgs = [&](auto&& self, C4Parser::ArgumentsContext* a) -> int {
                if (a->arguments()) {
                    return 1 + self(self, a->arguments());
                }
                return 1;
            };
            numArgs = countArgs(countArgs, args);

            // Collect all expressions, then push right-to-left
            vector<C4Parser::Logic_expressionContext*> argExprs;
            auto collectArgs = [&](auto&& self, C4Parser::ArgumentsContext* a) -> void {
                if (a->arguments()) {
                    self(self, a->arguments());
                }
                argExprs.push_back(a->logic_expression());
            };
            collectArgs(collectArgs, args);

            for (int i = (int)argExprs.size() - 1; i >= 0; i--) {
                visit(argExprs[i]);
                emit("PUSH EAX");
            }
        }

        emit("CALL " + funcName);

        if (numArgs > 0) {
            emit("ADD  ESP, " + to_string(numArgs * 4));
        }
    } else if (ctx->LPAREN()) {
        visit(ctx->expression());
    } else if (ctx->CONST_INT()) {
        emit("MOV  EAX, " + ctx->CONST_INT()->getText());
    } else if (ctx->CONST_FLOAT()) {
        emit("MOV  EAX, 0  ; float constant ignored");
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitArgument_list(C4Parser::Argument_listContext* ctx) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitArguments(C4Parser::ArgumentsContext* ctx) {
    return nullptr;
}
