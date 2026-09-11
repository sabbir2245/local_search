#include "CodeGenarator.h"
#include <cstdlib>





CodeGenarator::CodeGenarator(const string& outputFileName)
    : labelCount(0), currentLine(1), paramCount(0) {
    asmFile.open(outputFileName);
    if (!asmFile.is_open()) {
        cerr << "Error: Cannot open output file " << outputFileName << endl;
    }
    out = &asmFile;
}

CodeGenarator::~CodeGenarator() {
    if (asmFile.is_open()) asmFile.close();
}





string CodeGenarator::newLabel(const string& prefix) {
    return prefix + "_" + to_string(labelCount++);
}

void CodeGenarator::emit(const string& instruction) {
    (*out) << "    " << instruction << endl;
}

void CodeGenarator::emitWithComment(const string& instruction, const string& comment) {
    (*out) << "    " << instruction << "    ; " << comment << endl;
}

string CodeGenarator::getOperandAddr(SymbolInfo* sym) {
    if (sym->getIsGlobal()) {
        return "[" + sym->getName() + "]";
    }
    int offset = sym->getStackOffset();
    if (offset > 0) {
        return "[EBP+" + to_string(offset) + "]";
    } else {
        return "[EBP-" + to_string(-offset) + "]";
    }
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
        if (op == "==") return "JE";
        if (op == "!=") return "JNE";
        if (op == "<")  return "JL";
        if (op == ">")  return "JG";
        if (op == "<=") return "JLE";
        if (op == ">=") return "JGE";
    } else {
        if (op == "==") return "JNE";
        if (op == "!=") return "JE";
        if (op == "<")  return "JGE";
        if (op == ">")  return "JLE";
        if (op == "<=") return "JG";
        if (op == ">=") return "JL";
    }
    return "JMP";
}





void CodeGenarator::generateHeader() {
    asmFile << "format ELF executable" << endl;
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
    if (localSize != 0) {
        emit("ADD  ESP, " + to_string(abs(localSize)));
    }
    emit("POP  EBP");
    if (funcName == "main") {
        emit("MOV  EBX, EAX");
        emit("MOV  EAX, 1");
        emit("INT  0x80");
    } else if (paramSize > 0) {
        emit("RET " + to_string(paramSize));
    } else {
        emit("RET");
    }
}

void CodeGenarator::finalize() {
    asmFile.close();
}





antlrcpp::Any CodeGenarator::visitStart(C4Parser::StartContext* ctx) {
    return visit(ctx->program());
}

antlrcpp::Any CodeGenarator::visitProgram(C4Parser::ProgramContext* ctx) {
    
    
    auto collectUnits = [&](auto&& self, C4Parser::ProgramContext* p) -> void {
        if (p->program()) {
            self(self, p->program());
        }
        C4Parser::UnitContext* unit = p->unit();
        if (unit->var_declaration()) {
            visit(unit->var_declaration());
        }
    };
    collectUnits(collectUnits, ctx);

    generateHeader();
    generateDataSegment();
    generateCodeSegmentStart();

    auto generateFuncs = [&](auto&& self, C4Parser::ProgramContext* p) -> void {
        if (p->program()) {
            self(self, p->program());
        }
        C4Parser::UnitContext* unit = p->unit();
        if (unit->func_definition()) {
            visit(unit->func_definition());
        }
    };
    generateFuncs(generateFuncs, ctx);

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
    string funcName = ctx->ID()->getText();
    string returnType = ctx->type_specifier()->getText();
    symbolTable.insert(funcName, returnType);
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitFunc_definition(C4Parser::Func_definitionContext* ctx) {
    string funcName = ctx->ID()->getText();
    string returnType = ctx->type_specifier()->getText();
    currentFunctionName = funcName;

    
    paramCount = 0;
    if (ctx->parameter_list()) {
        auto countParams = [&](auto&& self, C4Parser::Parameter_listContext* pl) -> int {
            int c = 0;
            if (pl->parameter_list()) {
                c += self(self, pl->parameter_list());
            }
            if (pl->ID()) c++;
            return c;
        };
        paramCount = countParams(countParams, ctx->parameter_list());
    }

    symbolTable.insert(funcName, returnType);
    symbolTable.enterScope();

    
    int paramOffset = 8;
    if (ctx->parameter_list()) {
        vector<pair<string, string>> params;
        auto collectParams = [&](auto&& self, C4Parser::Parameter_listContext* pl) -> void {
            if (pl->parameter_list()) {
                self(self, pl->parameter_list());
            }
            string pType = pl->type_specifier()->getText();
            if (pl->ID()) {
                string pName = pl->ID()->getText();
                params.push_back({pName, pType});
            }
        };
        collectParams(collectParams, ctx->parameter_list());

        for (auto& p : params) {
            SymbolInfo* sym = new SymbolInfo(p.first, p.second);
            sym->setIdentityType("VAR");
            sym->setIsGlobal(false);
            sym->setStackOffset(paramOffset);
            symbolTable.insert(sym);
            paramOffset += 4;
        }
    }

    
    
    
    
    
    ostringstream bodyBuffer;
    ostream* savedOut = out;
    out = &bodyBuffer;

    visit(ctx->compound_statement());

    ScopeTable* funcScope = symbolTable.getCurrentScope();
    int localSize = funcScope->getCurrentStackOffset();
    symbolTable.exitScope();

    out = savedOut;

    generateFunctionPrologue(funcName);
    if (localSize > 0) {
        emit("SUB  ESP, " + to_string(localSize));
    }
    (*out) << bodyBuffer.str();

    generateFunctionEpilogue(funcName, localSize, paramCount * 4);

    return nullptr;
}

antlrcpp::Any CodeGenarator::visitParameter_list(C4Parser::Parameter_listContext*) {
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
    vector<pair<string, int>> declarations;

    auto collectDecls = [&](auto&& self, C4Parser::Declaration_listContext* dl) -> void {
        if (dl->declaration_list()) {
            self(self, dl->declaration_list());
        }
        string name = dl->ID()->getText();
        int arrSize = 0;
        if (dl->CONST_INT()) {
            arrSize = stoi(dl->CONST_INT()->getText());
        }
        declarations.push_back({name, arrSize});
    };
    collectDecls(collectDecls, declList);

    bool isGlobal = (symbolTable.getCurrentScope()->getId() == 1);

    for (auto& decl : declarations) {
        if (isGlobal) {
            dataSegmentGlobals.push_back(decl.first);
            if (decl.second > 0) {
                SymbolInfo* sym = new SymbolInfo(decl.first, type);
                sym->setIdentityType("ARRAY");
                sym->setArraySize(decl.second);
                sym->setIsGlobal(true);
                sym->setStackOffset(0);
                symbolTable.insert(sym);
            } else {
                SymbolInfo* sym = new SymbolInfo(decl.first, type);
                sym->setIdentityType("VAR");
                sym->setIsGlobal(true);
                sym->setStackOffset(0);
                symbolTable.insert(sym);
            }
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

antlrcpp::Any CodeGenarator::visitType_specifier(C4Parser::Type_specifierContext*) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitDeclaration_list(C4Parser::Declaration_listContext*) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitStatements(C4Parser::StatementsContext* ctx) {
    
    
    if (ctx->statements()) {
        visit(ctx->statements());
    }
    visit(ctx->statement());
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitStatement(C4Parser::StatementContext* ctx) {
    if (ctx->var_declaration()) {
        visit(ctx->var_declaration());
    } else if (ctx->FOR()) {
        
        
        
        
        
        string startLabel = newLabel("for_start");
        string endLabel = newLabel("for_end");
        string bodyLabel = newLabel("for_body");

        symbolTable.enterScope();

        auto exprStmts = ctx->expression_statement();
        
        if (exprStmts.size() > 0) {
            visit(exprStmts[0]);
        }

        (*out) << startLabel << ":" << endl;

        
        if (exprStmts.size() > 1) {
            visit(exprStmts[1]);
            emit("TEST EAX, EAX");
            emit("JE   " + endLabel);
        }

        
        visit(ctx->statement(0));

        
        if (ctx->expression()) {
            visit(ctx->expression());
        }

        emit("JMP  " + startLabel);
        (*out) << endLabel << ":" << endl;

        symbolTable.exitScope();
    } else if (!ctx->expression_statement().empty()) {
        visit(ctx->expression_statement(0));
    } else if (ctx->compound_statement()) {
        symbolTable.enterScope();
        visit(ctx->compound_statement());
        symbolTable.exitScope();
    } else if (ctx->IF() && !ctx->ELSE()) {
        string endLabel = newLabel("if_end");
        visit(ctx->expression());
        emit("TEST EAX, EAX");
        emit("JE   " + endLabel);
        visit(ctx->statement(0));
        (*out) << endLabel << ":" << endl;
    } else if (ctx->IF() && ctx->ELSE()) {
        string elseLabel = newLabel("if_else");
        string endLabel = newLabel("if_end");
        visit(ctx->expression());
        emit("TEST EAX, EAX");
        emit("JE   " + elseLabel);
        visit(ctx->statement(0));
        emit("JMP  " + endLabel);
        (*out) << elseLabel << ":" << endl;
        visit(ctx->statement(1));
        (*out) << endLabel << ":" << endl;
    } else if (ctx->WHILE()) {
        string startLabel = newLabel("while_start");
        string endLabel = newLabel("while_end");

        symbolTable.enterScope();

        (*out) << startLabel << ":" << endl;
        visit(ctx->expression());
        emit("TEST EAX, EAX");
        emit("JE   " + endLabel);
        visit(ctx->statement(0));
        emit("JMP  " + startLabel);
        (*out) << endLabel << ":" << endl;

        symbolTable.exitScope();
    } else if (ctx->PRINTLN()) {
        string varName = ctx->ID()->getText();
        SymbolInfo* sym = symbolTable.lookUp(varName);
        if (sym) {
            emit("PUSH EAX");
            emit("MOV  EAX, " + getOperandAddr(sym));
            emit("CALL OUTDEC");
            emit("POP  EAX");
        }
    } else if (ctx->RETURN()) {
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
        visit(ctx->expression());
        emit("MOV  EBX, 4");
        emit("MUL  EBX");

        if (sym->getIsGlobal()) {
            emit("MOV  EAX, [" + sym->getName() + " + EAX]");
        } else {
            int baseOffset = sym->getStackOffset();
            emit("SUB  EAX, " + to_string(baseOffset));
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
            visit(ctx->variable()->expression());
            emit("MOV  EBX, 4");
            emit("MUL  EBX");
            emit("PUSH EAX");

            visit(ctx->logic_expression());

            emit("POP  EBX");

            if (sym->getIsGlobal()) {
                emit("MOV  [" + sym->getName() + " + EBX], EAX");
            } else {
                int baseOffset = sym->getStackOffset();
                emit("SUB  EBX, " + to_string(baseOffset));
                emit("NEG  EBX");
                emit("MOV  [EBP + EBX], EAX");
            }
        } else {
            visit(ctx->logic_expression());
            if (sym->getIsGlobal()) {
                emit("MOV  [" + sym->getName() + "], EAX");
            } else {
                emit("MOV  " + getOperandAddr(sym) + ", EAX");
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

        visit(ctx->rel_expression(0));
        emit("TEST EAX, EAX");

        if (op == "&&") {
            emit("JE   " + falseLabel);
            visit(ctx->rel_expression(1));
            emit("TEST EAX, EAX");
            emit("MOV  EAX, 0");
            emit("JE   " + endLabel);
            emit("MOV  EAX, 1");
            emit("JMP  " + endLabel);
            (*out) << falseLabel << ":" << endl;
            emit("MOV  EAX, 0");
        } else {
            emit("JNE  " + falseLabel);
            visit(ctx->rel_expression(1));
            emit("TEST EAX, EAX");
            emit("MOV  EAX, 0");
            emit("JE   " + endLabel);
            emit("MOV  EAX, 1");
            emit("JMP  " + endLabel);
            (*out) << falseLabel << ":" << endl;
            emit("MOV  EAX, 1");
        }
        (*out) << endLabel << ":" << endl;
    } else {
        visit(ctx->rel_expression(0));
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitRel_expression(C4Parser::Rel_expressionContext* ctx) {
    if (ctx->RELOP()) {
        string op = ctx->RELOP()->getText();
        string trueLabel = newLabel("relop_true");
        string endLabel = newLabel("relop_end");

        visit(ctx->simple_expression(0));
        emit("PUSH EAX");
        visit(ctx->simple_expression(1));
        emit("POP  EBX");
        emit("CMP  EBX, EAX");
        emit(relOpToJump(op) + " " + trueLabel);
        emit("MOV  EAX, 0");
        emit("JMP  " + endLabel);
        (*out) << trueLabel << ":" << endl;
        emit("MOV  EAX, 1");
        (*out) << endLabel << ":" << endl;
    } else {
        visit(ctx->simple_expression(0));
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitSimple_expression(C4Parser::Simple_expressionContext* ctx) {
    if (ctx->simple_expression()) {
        
        visit(ctx->simple_expression()); 
        emit("PUSH EAX");
        visit(ctx->term()); 
        emit("POP  EBX");
        string op = ctx->ADDOP()->getText();
        if (op == "+") {
            emit("ADD  EAX, EBX");
        } else {
            
            emit("XCHG EAX, EBX");
            emit("SUB  EAX, EBX");
        }
    } else {
        visit(ctx->term());
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitTerm(C4Parser::TermContext* ctx) {
    if (ctx->term()) {
        
        visit(ctx->term()); 
        emit("PUSH EAX");
        visit(ctx->unary_expression()); 
        emit("POP  EBX");
        string op = ctx->MULOP()->getText();
        if (op == "*") {
            emit("MUL  EBX");
        } else if (op == "/") {
            emit("XCHG EAX, EBX");
            emit("XOR  EDX, EDX");
            emit("DIV  EBX");
        } else {
            emit("XCHG EAX, EBX");
            emit("XOR  EDX, EDX");
            emit("DIV  EBX");
            emit("MOV  EAX, EDX");
        }
    } else {
        visit(ctx->unary_expression());
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
    } else if (ctx->NOT()) {
        visit(ctx->unary_expression());
        string trueLabel = newLabel("not_true");
        string endLabel = newLabel("not_end");
        emit("TEST EAX, EAX");
        emit("JNE  " + trueLabel);
        emit("MOV  EAX, 1");
        emit("JMP  " + endLabel);
        (*out) << trueLabel << ":" << endl;
        emit("MOV  EAX, 0");
        (*out) << endLabel << ":" << endl;
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
                visit(ctx->variable()->expression());
                emit("MOV  EBX, 4");
                emit("MUL  EBX");
                emit("MOV  ECX, EAX");

                if (sym->getIsGlobal()) {
                    emit("MOV  EAX, [" + sym->getName() + " + ECX]");
                } else {
                    int baseOffset = sym->getStackOffset();
                    emit("MOV  EDX, ECX");
                    emit("SUB  EDX, " + to_string(baseOffset));
                    emit("NEG  EDX");
                    emit("MOV  EAX, [EBP + EDX]");
                }
                emit("PUSH EAX");

                string op = isInc ? "INC" : "DEC";
                if (sym->getIsGlobal()) {
                    emit(op + "  dword [" + sym->getName() + " + ECX]");
                } else {
                    int baseOffset = sym->getStackOffset();
                    emit("MOV  EDX, ECX");
                    emit("SUB  EDX, " + to_string(baseOffset));
                    emit("NEG  EDX");
                    emit(op + "  dword [EBP + EDX]");
                }
                emit("POP  EAX");
            } else {
                emit("MOV  EAX, " + getOperandAddr(sym));
                emit("PUSH EAX");
                string op = isInc ? "INC" : "DEC";
                emit(op + "  dword " + getOperandAddr(sym));
                emit("POP  EAX");
            }
        } else {
            visit(ctx->variable());
            if (sym) {
                emit("MOV  EAX, " + getOperandAddr(sym));
            }
        }
    } else if (ctx->ID() && ctx->argument_list()) {
        string funcName = ctx->ID()->getText();

        if (ctx->argument_list()->arguments()) {

            vector<C4Parser::Logic_expressionContext*> argExprs;
            auto collectArgs = [&](auto&& self, C4Parser::ArgumentsContext* a) -> void {
                if (a->arguments()) {
                    self(self, a->arguments());
                }
                argExprs.push_back(a->logic_expression());
            };
            collectArgs(collectArgs, ctx->argument_list()->arguments());

            for (int i = (int)argExprs.size() - 1; i >= 0; i--) {
                visit(argExprs[i]);
                emit("PUSH EAX");
            }
        }

        emit("CALL " + funcName);
        
        
        
    } else if (ctx->LPAREN()) {
        visit(ctx->expression());
    } else if (ctx->CONST_INT()) {
        emit("MOV  EAX, " + ctx->CONST_INT()->getText());
    } else if (ctx->CONST_FLOAT()) {
        emit("MOV  EAX, 0  ; float constant ignored");
    }
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitArgument_list(C4Parser::Argument_listContext*) {
    return nullptr;
}

antlrcpp::Any CodeGenarator::visitArguments(C4Parser::ArgumentsContext*) {
    return nullptr;
}
