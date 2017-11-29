#include <iostream>
#include <sstream>
#include <vector>

#include "gbuilder.h"

void Parser::doParse() {

    while (here()) {
        if (matches("function")) {
            FunctionDef *newfunc = doFunction();
            if (newfunc) {
                gamedata.functions.push_back(newfunc);
            }
        } else {
            std::cout << here()->file
                      << ":" 
                      << here()->line 
                      << ":" 
                      << here()->column 
                      << ":  " 
                      << tokenTypeName(here()->type) << '\n';
            next();
        }
    }
}

FunctionDef* Parser::doFunction() {
    if (!expect("function")) return nullptr;
    if (!expect(Identifier)) return nullptr;
    
    FunctionDef *newfunc = new FunctionDef;
    newfunc->name = here()->vText;
    next();
    expectAdv(OpenParan);
    if (matches(Identifier)) {
        while (true) {
            expect(Identifier);
            SymbolDef sym(here()->vText);
            newfunc->args.symbols.push_back(sym);
            next();
            if (matches(Comma)) {
                next();
            } else {
                break;
            }
        }
    }
    expectAdv(CloseParan);
    newfunc->code = doCodeBlock();
    if (!newfunc->code) {
        delete newfunc;
        return nullptr;
    }
    newfunc->code->locals.parent = &newfunc->args;
    return newfunc;
}

CodeBlock* Parser::doCodeBlock() {
    if (!expectAdv(OpenBrace)) return nullptr;
    
    CodeBlock *code = new CodeBlock;
    while (!matches(CloseBrace)) {
        if (here() == nullptr) {
            delete code;
            return nullptr;
        }
        
        StatementDef *stmt = nullptr;
        if (here()->type == OpenBrace) {
            stmt = doCodeBlock();
            ((CodeBlock*)stmt)->locals.parent = &code->locals;
        } else if (matches("local")) {
            if (!doLocalsStmt(code)) return nullptr;
        } else if (matches("asm")) {
            stmt = doAsmBlock();
        } else {
            std::stringstream ss;
            ss << "unexpected token ";
            ss << tokenTypeName(here()->type);
            ss << ".";
            addError(ErrorLogger::Error, ss.str());
            next();
        }
        if (stmt) {
            code->statements.push_back(stmt);
        }
    }
    next();
    return code;
}

bool Parser::doLocalsStmt(CodeBlock *code) {
    if (!expect("local")) return false;

    while (true) {
        expect(Identifier);
        SymbolDef sym(here()->vText);
        code->locals.symbols.push_back(sym);
        next();
        if (matches(Comma)) {
            next();
        } else {
            break;
        }
    }
    if (!expectAdv(Semicolon)) return false;
    return true;
}

StatementDef* Parser::doAsmBlock() {
    if (!expect("asm")) return nullptr;
    
    if (!matches(OpenBrace)) {
        StatementDef *stmt = doAsmStatement();
        return stmt;
    }

    if (!expectAdv(OpenBrace)) return nullptr;
    CodeBlock *code = new CodeBlock;
    
    
    while (!matches(CloseBrace)) {
        if (here() == nullptr) {
            delete code;
            return nullptr;
        }
        
        StatementDef *stmt = doAsmStatement();
        if (stmt) {
            code->statements.push_back(stmt);
        }
    }
    next();
    return code;
}

StatementDef* Parser::doAsmStatement() {
    if (!expect(Identifier)) return nullptr;
    AsmStatement *stmt = new AsmStatement;
    stmt->opcode = here()->vText;
    next();

    while (!matches(Semicolon)) {
        AsmOperand *op = doAsmOperand();
        if (op) {
            stmt->operands.push_back(op);
        }
    }

    if (!expectAdv(Semicolon)) return nullptr;
    return stmt;
}

AsmOperand* Parser::doAsmOperand() {
    switch(here()->type) {
        case Integer: {
            AsmOperandInteger *op = new AsmOperandInteger;
            op->value = here()->vInteger;
            next();
            return op;
        }
        case Identifier: {
            if (here()->vText == "sp") {
                AsmOperandStack *op = new AsmOperandStack;
                next();
                return op;
            }
        }
        default:
            addError(ErrorLogger::Error, "bad operand type");
            next();
            return nullptr;
    }
}


bool Parser::expect(TokenType type) {
    if (matches(type)) {
        return true;
    }

    if (!here()) {
        addError(ErrorLogger::Error, "Unexpected EOF");
        return false;
    }

    std::stringstream ss;
    ss << "expected "
       << tokenTypeName(type)
       << " but found "
       << tokenTypeName(here()->type)
       << ".";
    addError(ErrorLogger::Error, ss.str());

    return false;
}

bool Parser::expectAdv(TokenType type) {
    if (expect(type)) {
        next();
        return true;
    }
    return false;
}

bool Parser::matches(TokenType type) {
    if (!here() || here()->type != type) {
        return false;
    }
    return true;
}

bool Parser::expect(const std::string &text) {
    if (matches(text)) {
        next();
        return true;
    }

    if (!here()) {
        addError(ErrorLogger::Error, "Unexpected EOF");
        return false;
    }

    std::stringstream ss;
    ss << "expected keyword \""
       << text
       << "\".";
    addError(ErrorLogger::Error, ss.str());

    return false;
}

bool Parser::matches(const std::string &text) {
    if (!here()) {
        return false;
    }
    
    if (!here() || here()->type != ReservedWord || here()->vText != text) {
        return false;
    }
    return true;
}

const Token* Parser::here() {
    if (current < tokens.size()) {
        return &tokens[current];
    } else {
        return nullptr;
    }
}

const Token* Parser::next() {
    ++current;
    return here();
}

void Parser::addError(ErrorLogger::Type type, const std::string &text) {
    if (!here()) {
        errors.add(type, "<unknown>", 0, 0, text);
    } else {
        errors.add(type, here()->file, here()->line, here()->column, text);
    }
}
