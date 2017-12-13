#include <iostream>
#include <sstream>
#include <vector>

#include "gbuilder.h"

static int floatAsInt(float initial) {
    union {
        int a;
        float b;
    } typepun;
    typepun.b = initial;
    return typepun.a;
}

void Parser::doParse() {

    while (here()) {
        if (matches("constant")) {
            doConstant();
        } else if (matches("function")) {
            FunctionDef *newfunc = doFunction();
            if (newfunc) {
                gamedata.functions.push_back(newfunc);
            }
        } else {
            std::cout << "Unexpected token: ";
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

void Parser::doConstant() {
    if (!expect("constant")) return;

    if (!expect(Identifier)) return;
    const std::string &name = here()->vText;
    next();

    if (!expectAdv(OpAssign)) return;

    if (matches(Integer)) {
        SymbolDef symbol(name, SymbolDef::Constant);
        symbol.value = here()->vInteger;
        gamedata.symbols.symbols.push_back(std::move(symbol));
        next();
    } else if (matches(Float)) {
        SymbolDef symbol(name, SymbolDef::Constant);
        symbol.value = floatAsInt(here()->vFloat);
        gamedata.symbols.symbols.push_back(std::move(symbol));
        next();
    } else {
        expect(Integer);
        return;
    }

    if (!expectAdv(Semicolon)) return;
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
            SymbolDef sym(here()->vText, SymbolDef::Local);
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
    newfunc->code->statements.push_back(new ReturnDef);
    newfunc->code->locals.parent = &newfunc->args;
    newfunc->args.parent = &gamedata.symbols;
    return newfunc;
}

ReturnDef* Parser::doReturn() {
    if (!expect("return")) return nullptr;
    if (!expectAdv(Semicolon)) return nullptr;
    return new ReturnDef;
}
LabelStmt* Parser::doLabel() {
    if (!expect("label")) return nullptr;
    if (!expect(Identifier)) return nullptr;
    const std::string &name = here()->vText;
    next();
    if (!expectAdv(Semicolon)) return nullptr;
    return new LabelStmt(name);
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
        } else if (matches("return")) {
            stmt = doReturn();
        } else if (matches("label")) {
            stmt = doLabel();
        } else if (matches("asm")) {
            stmt = doAsmBlock();
            CodeBlock *cb = dynamic_cast<CodeBlock*>(stmt);
            if (cb) {
                cb->locals.parent = &code->locals;
            }
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
        SymbolDef sym(here()->vText, SymbolDef::Local);
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
    if (matches("label")) return doLabel();

    if (!matches(Identifier) && !matches(ReservedWord)) {
        expect(Identifier);
        return nullptr;
    }
    AsmStatement *stmt = new AsmStatement;
    stmt->opname = here()->vText;
    next();

    const AsmCode &ac = opcodeByName(stmt->opname);
    if (ac.name == nullptr) {
        addError(ErrorLogger::Error, "unknown assembly mnemonic");
    } else {
        stmt->opcode = ac.opcode;
        stmt->isRelative = ac.relative;
    }

    while (!matches(Semicolon)) {
        AsmOperand *op = doAsmOperand();
        if (op) {
            stmt->operands.push_back(op);
        }
    }

    if (ac.operands != stmt->operands.size()) {
        addError(ErrorLogger::Error, "bad operand count");
    }

    if (!expectAdv(Semicolon)) return nullptr;
    return stmt;
}

AsmOperand* Parser::doAsmOperand() {
    AsmOperand *op = new AsmOperand;
    switch(here()->type) {
        case Integer: {
            op->type = AsmOperand::Constant;
            op->value = here()->vInteger;
            next();
            return op;
        }
        case Float: {
            op->type = AsmOperand::Constant;
            op->value = floatAsInt(here()->vFloat);
            next();
            return op;
        }
        case String: {
            op->type = AsmOperand::Identifier;
//            const std::string &s = gamedata.addString(here()->vText);
            op->text = gamedata.addString(here()->vText);
            next();
            return op;
        }
        case Identifier: {
            if (here()->vText == "sp") {
                op->type = AsmOperand::Stack;
                next();
                return op;
            } else {
                op->type = AsmOperand::Identifier;
                op->text = here()->vText;
                next();
                return op;
            }
        }
        default:
            delete op;
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
