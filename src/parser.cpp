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
            std::stringstream ss;
            ss << "unexpected token ";
            ss << tokenTypeName(here()->type);
            ss << ".";
            addError(ErrorLogger::Error, ss.str());
            next();
        }

        if (!errors.empty()) {
            return;
        }
    }
}


/* ************************************************************ *
 * TOP LEVEL CONSTRUCTS                                         *
 * ************************************************************ */

void Parser::doConstant() {
    if (!expect("constant")) return;

    if (!expect(Identifier)) return;
    const std::string &name = here()->vText;
    next();

    symbolExists(gamedata.symbols, name);

    if (!expectAdv(OpAssign)) return;

    if (matches(Integer)) {
        SymbolDef *symbol = new SymbolDef(name, SymbolDef::Constant);
        symbol->value = here()->vInteger;
        gamedata.symbols.add(symbol);
        next();
    } else if (matches(Float)) {
        SymbolDef *symbol = new SymbolDef(name, SymbolDef::Constant);
        symbol->value = floatAsInt(here()->vFloat);
        gamedata.symbols.add(symbol);
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
    newfunc->args.parent = &gamedata.symbols;
    next();
    expectAdv(OpenParan);
    if (matches(Identifier)) {
        while (true) {
            expect(Identifier);
            symbolExists(newfunc->args, here()->vText);
            SymbolDef *sym = new SymbolDef(here()->vText, SymbolDef::Local);
            newfunc->args.add(sym);
            next();
            if (matches(Comma)) {
                next();
            } else {
                break;
            }
        }
    }
    expectAdv(CloseParan);
    curTable = &newfunc->args;
    newfunc->code = doCodeBlock();
    if (!newfunc->code) {
        delete newfunc;
        return nullptr;
    }
    gamedata.symbols.add(new SymbolDef(newfunc->name, SymbolDef::Function));
    newfunc->code->statements.push_back(new ReturnDef);
    return newfunc;
}


/* ************************************************************ *
 * STATEMENT PARSING                                            *
 * ************************************************************ */

StatementDef* Parser::doStatement() {
    StatementDef *stmt = nullptr;
    if (here()->type == OpenBrace) {
        stmt = doCodeBlock();
    } else if (matches("local")) {
        if (!doLocalsStmt()) return nullptr;
    } else if (matches("return")) {
        stmt = doReturn();
    } else if (matches("label")) {
        stmt = doLabel();
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
    return stmt;
}

CodeBlock* Parser::doCodeBlock() {
    if (!expectAdv(OpenBrace)) return nullptr;

    CodeBlock *code = new CodeBlock;
    code->locals.parent = curTable;
    while (!matches(CloseBrace)) {
        if (here() == nullptr) {
            delete code;
            return nullptr;
        }

        curTable = &code->locals;
        StatementDef *stmt = doStatement();
        if (stmt) {
            code->statements.push_back(stmt);
        }
    }
    next();
    return code;
}

bool Parser::doLocalsStmt() {
    if (!expect("local")) return false;

    while (true) {
        if (!expect(Identifier)) return false;
        symbolExists(*curTable, here()->vText);
        SymbolDef *sym = new SymbolDef(here()->vText, SymbolDef::Local);
        curTable->add(sym);
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

LabelStmt* Parser::doLabel() {
    if (!expect("label")) return nullptr;
    if (!expect(Identifier)) return nullptr;
    const std::string &name = here()->vText;
    next();
    if (!expectAdv(Semicolon)) return nullptr;
    symbolExists(*curTable, name);
    SymbolDef *sym = new SymbolDef(name, SymbolDef::Label);
    curTable->add(sym, true);
    return new LabelStmt(name);
}

ReturnDef* Parser::doReturn() {
    if (!expect("return")) return nullptr;
    if (!expectAdv(Semicolon)) return nullptr;
    return new ReturnDef;
}


Value* Parser::doValue() {
    Value *value = new Value;
    switch(here()->type) {
        case Integer: {
            value->type = Value::Constant;
            value->value = here()->vInteger;
            next();
            return value;
        }
        case Float: {
            value->type = Value::Constant;
            value->value = floatAsInt(here()->vFloat);
            next();
            return value;
        }
        case String: {
            value->type = Value::Identifier;
            value->text = gamedata.addString(here()->vText);
            next();
            return value;
        }
        case Identifier: {
            value->type = Value::Identifier;
            value->text = here()->vText;
            next();
            return value;
        }
        default:
            delete value;
            addError(ErrorLogger::Error, "expected value");
            next();
            return nullptr;
    }
}


/* ************************************************************ *
 * ASSEMBLY PARSING                                             *
 * ************************************************************ */

StatementDef* Parser::doAsmBlock() {
    if (!expect("asm")) return nullptr;

    if (!matches(OpenBrace)) {
        StatementDef *stmt = doAsmStatement();
        return stmt;
    }

    if (!expectAdv(OpenBrace)) return nullptr;
    CodeBlock *code = new CodeBlock;
    code->locals.parent = curTable;


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

    if (matches(Identifier) && here()->vText == "sp") {
        op->isStack = true;
        next();
        return op;
    }

    op->value = doValue();

    if (!op->value) {
        delete op;
        return nullptr;
    }
    return op;
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

bool Parser::symbolExists(const SymbolTable &table, const std::string &name) {
    if (table.exists(name)) {
        std::stringstream ss;
        ss << "symbol "
           << name
           << " already declared.";
        addError(ErrorLogger::Error, ss.str());
        return true;
    }
    return false;
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
