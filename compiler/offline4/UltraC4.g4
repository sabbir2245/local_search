grammar UltraC4;

// ============================================================
// COMBINED C4 GRAMMAR
//
// This file combines:
//   1. id2205040_CSubset(1).g4  -> parser rules
//   2. id2205040_Lexer(1).g4   -> lexer rules + lex logging
//   3. C4.g4                   -> single-file parser/lexer form
//
// The lexer rules below are kept in the same file so this grammar
// can be generated as one ANTLR grammar.
// ============================================================

// ============================================================
// PARSER RULES
// ============================================================

start
    : program
    ;

program
    : program unit
    | unit
    ;

unit
    : var_declaration
    | func_declaration
    | func_definition
    ;

func_declaration
    : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON
    | type_specifier ID LPAREN RPAREN SEMICOLON
    ;

func_definition
    : type_specifier ID LPAREN parameter_list RPAREN compound_statement
    | type_specifier ID LPAREN RPAREN compound_statement
    ;

parameter_list
    : parameter_list COMMA type_specifier ID
    | parameter_list COMMA type_specifier
    | type_specifier ID
    | type_specifier
    ;

compound_statement
    : LCURL statements RCURL
    | LCURL RCURL
    ;

var_declaration
    : type_specifier declaration_list SEMICOLON
    ;

type_specifier
    : INT
    | FLOAT
    | VOID
    ;

declaration_list
    : declaration_list COMMA ID
    | declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
    | ID
    | ID LTHIRD CONST_INT RTHIRD
    ;

statements
    : statement
    | statements statement
    ;

statement
    : var_declaration
    | expression_statement
    | compound_statement
    | FOR LPAREN expression_statement expression_statement expression RPAREN statement
    | IF LPAREN expression RPAREN statement
    | IF LPAREN expression RPAREN statement ELSE statement
    | WHILE LPAREN expression RPAREN statement
    | PRINTLN LPAREN ID RPAREN SEMICOLON
    | RETURN expression SEMICOLON
    ;

expression_statement
    : SEMICOLON
    | expression SEMICOLON
    ;

variable
    : ID
    | ID LTHIRD expression RTHIRD
    ;

expression
    : logic_expression
    | variable ASSIGNOP logic_expression
    ;

logic_expression
    : rel_expression
    | rel_expression LOGICOP rel_expression
    ;

rel_expression
    : simple_expression
    | simple_expression RELOP simple_expression
    ;

simple_expression
    : term
    | simple_expression ADDOP term
    ;

term
    : unary_expression
    | term MULOP unary_expression
    ;

unary_expression
    : ADDOP unary_expression
    | NOT unary_expression
    | factor
    ;

factor
    : variable
    | ID LPAREN argument_list RPAREN
    | LPAREN expression RPAREN
    | CONST_INT
    | CONST_FLOAT
    | variable INCOP
    | variable DECOP
    ;

argument_list
    : arguments
    |
    ;

arguments
    : arguments COMMA logic_expression
    | logic_expression
    ;


// ============================================================
// LEXER HEADER / MEMBERS
// ============================================================

@lexer::header {
    #pragma once
    #include <iostream>
    #include <fstream>
    #include <string>

    extern std::ofstream lexLogFile;
}

@lexer::members {
    void writeIntoLexLogFile(const std::string &message) {
        if (!lexLogFile.is_open()) {
            lexLogFile.open("lexLogFile.txt", std::ios::app);
            if (!lexLogFile) {
                std::cerr << "Error opening lexLogFile.txt" << std::endl;
                return;
            }
        }
        lexLogFile << message << std::endl;
        lexLogFile.flush();
    }
}


// ============================================================
// 1. COMMENTS
// ============================================================

// Original lexer rule: LINE_COMMENT
LINE_COMMENT
    : '//' ~[\r\n]* {
        writeIntoLexLogFile(
            "Line# " + std::to_string(getLine())
            + ": Token <SINGLE LINE COMMENT> Lexeme "
            + getText()
        );
    } -> skip
    ;

// Original lexer rule: BLOCK_COMMENT
BLOCK_COMMENT
    : '/*' ( . | '\r' | '\n' )*? '*/' {
        {
            std::string txt = getText();
            std::string content = txt.substr(2, txt.size() - 4);
            writeIntoLexLogFile(
                "Line# " + std::to_string(getLine())
                + ": Token <MULTI LINE COMMENT> Lexeme /*"
                + content + "*/"
            );
        }
    } -> skip
    ;

// ============================================================
// 2. STRING LITERALS
// ============================================================

STRING
    : '"' ( '\\' . | ~["\\\r\n] )* '"' {
        writeIntoLexLogFile(
            "Line# " + std::to_string(getLine())
            + ": Token <STRING> Lexeme " + getText()
        );
    } -> skip
    ;

// ============================================================
// 3. WHITESPACE
// ============================================================

WS
    : [ \t\f\r\n]+ -> skip
    ;

// ============================================================
// 4. KEYWORDS
// ============================================================

IF
    : 'if' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <IF> Lexeme " + getText());
    }
    ;

ELSE
    : 'else' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <ELSE> Lexeme " + getText());
    }
    ;

FOR
    : 'for' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <FOR> Lexeme " + getText());
    }
    ;

WHILE
    : 'while' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <WHILE> Lexeme " + getText());
    }
    ;

// Both spellings are supported because the supplied files differ:
//   original lexer -> printf
//   C4.g4          -> println
PRINTLN
    : 'printf'
    | 'println'
    {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <PRINTLN> Lexeme " + getText());
    }
    ;

RETURN
    : 'return' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <RETURN> Lexeme " + getText());
    }
    ;

INT
    : 'int' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <INT> Lexeme " + getText());
    }
    ;

FLOAT
    : 'float' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <FLOAT> Lexeme " + getText());
    }
    ;

VOID
    : 'void' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <VOID> Lexeme " + getText());
    }
    ;

// ============================================================
// 5. PUNCTUATION / DELIMITERS
// ============================================================

LPAREN
    : '(' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <LPAREN> Lexeme " + getText());
    }
    ;

RPAREN
    : ')' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <RPAREN> Lexeme " + getText());
    }
    ;

LCURL
    : '{' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <LCURL> Lexeme " + getText());
    }
    ;

RCURL
    : '}' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <RCURL> Lexeme " + getText());
    }
    ;

LTHIRD
    : '[' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <LTHIRD> Lexeme " + getText());
    }
    ;

RTHIRD
    : ']' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <RTHIRD> Lexeme " + getText());
    }
    ;

SEMICOLON
    : ';' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <SEMICOLON> Lexeme " + getText());
    }
    ;

COMMA
    : ',' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <COMMA> Lexeme " + getText());
    }
    ;

// ============================================================
// 6. OPERATORS
// ============================================================

// Longest-match behavior ensures ++/-- are recognized before +/-. 
INCOP
    : '++' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <INCOP> Lexeme " + getText());
    }
    ;

DECOP
    : '--' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <DECOP> Lexeme " + getText());
    }
    ;

LOGICOP
    : '&&' | '||' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <LOGICOP> Lexeme " + getText());
    }
    ;

RELOP
    : '<=' | '==' | '>=' | '>' | '<' | '!=' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <RELOP> Lexeme " + getText());
    }
    ;

ASSIGNOP
    : '=' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <ASSIGNOP> Lexeme " + getText());
    }
    ;

ADDOP
    : [+\-] {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <ADDOP> Lexeme " + getText());
    }
    ;

MULOP
    : [*/%] {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <MULOP> Lexeme " + getText());
    }
    ;

NOT
    : '!' {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <NOT> Lexeme " + getText());
    }
    ;

// ============================================================
// 7. IDENTIFIERS
// ============================================================

ID
    : [A-Za-z_] [A-Za-z0-9_]* {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <ID> Lexeme " + getText());
    }
    ;

// ============================================================
// 8. INTEGER CONSTANTS
// ============================================================

CONST_INT
    : [0-9]+ {
        writeIntoLexLogFile("Line# " + std::to_string(getLine()) + ": Token <CONST_INT> Lexeme " + getText());
    }
    ;

// Complete floating-point rule from the supplied lexer.
// Supports forms such as:
//   123.45
//   123.
//   .45
//   123e10
//   123.45e-2
CONST_FLOAT
    : [0-9]+ ('.' [0-9]*)? ([Ee][+\-]? [0-9]+)? {
        writeIntoLexLogFile(
            "Line# " + std::to_string(getLine())
            + ": Token <CONST_FLOAT> Lexeme " + getText()
        );
    }
    | '.' [0-9]+ {
        writeIntoLexLogFile(
            "Line# " + std::to_string(getLine())
            + ": Token <CONST_FLOAT> Lexeme " + getText()
        );
    }
    | [0-9]+ '.' {
        writeIntoLexLogFile(
            "Line# " + std::to_string(getLine())
            + ": Token <CONST_FLOAT> Lexeme " + getText()
        );
    }
    ;
