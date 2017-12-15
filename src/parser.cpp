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
        try {
            if (matches(EndOfFile)) {
                // do nothing
                next();
            } else if (matches("constant")) {
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
                errors.add(ErrorLogger::Error, here()->origin, ss.str());
                next();
            }
        } catch (ParserError &e) {
            std::cerr << "Fatal error occured.\n";
            return;
        }
    }
}


/* ************************************************************ *
 * TOP LEVEL CONSTRUCTS                                         *
 * ************************************************************ */

void Parser::doConstant() {
    expect("constant");

    expect(Identifier);
    const std::string &name = here()->vText;
    next();

    symbolExists(gamedata.symbols, name);

    expectAdv(OpAssign);

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
    }

    expectAdv(Semicolon);
}

FunctionDef* Parser::doFunction() {
    const Origin &origin = here()->origin;
    expect("function");
    expect(Identifier);

    FunctionDef *newfunc = new FunctionDef;
    newfunc->name = here()->vText;
    newfunc->args.parent = &gamedata.symbols;
    newfunc->origin = origin;
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
    try {
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
            errors.add(ErrorLogger::Error, here()->origin, ss.str());
            synchronize();
        }
    } catch (ParserError &e) {
        synchronize();
    }
    return stmt;
}

CodeBlock* Parser::doCodeBlock() {
    const Origin &origin = here()->origin;
    expectAdv(OpenBrace);

    CodeBlock *code = new CodeBlock;
    code->origin = origin;
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
    expect("local");

    while (true) {
        expect(Identifier);
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
    expectAdv(Semicolon);
    return true;
}

LabelStmt* Parser::doLabel() {
    expect("label");
    expect(Identifier);
    const std::string &name = here()->vText;
    next();
    expectAdv(Semicolon);
    symbolExists(*curTable, name);
    SymbolDef *sym = new SymbolDef(name, SymbolDef::Label);
    curTable->add(sym, true);
    return new LabelStmt(name);
}

ReturnDef* Parser::doReturn() {
    expect("return");
    expectAdv(Semicolon);
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
            errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), "expected value");
            next();
            return nullptr;
    }
}


/* ************************************************************ *
 * ASSEMBLY PARSING                                             *
 * ************************************************************ */

StatementDef* Parser::doAsmBlock() {
    const Origin &origin = here()->origin;
    expect("asm");

    if (!matches(OpenBrace)) {
        StatementDef *stmt = doAsmStatement();
        return stmt;
    }

    expectAdv(OpenBrace);
    CodeBlock *code = new CodeBlock;
    code->origin = origin;
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
    }
    AsmStatement *stmt = new AsmStatement;
    stmt->opname = here()->vText;
    next();

    const AsmCode &ac = opcodeByName(stmt->opname);
    if (ac.name == nullptr) {
        errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), "unknown assembly mnemonic");
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
        errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), "bad operand count");
    }

    expectAdv(Semicolon);
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


void Parser::synchronize() {
    while(here()->type != Semicolon && here()->type != CloseBrace && here()->type != EndOfFile) {
        next();
    }
    next();
}

void Parser::expect(TokenType type) {
    if (matches(type)) {
        return;
    }

    if (!here()) {
        errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), "Unexpected EOF");
        throw ParserError();
    }

    std::stringstream ss;
    ss << "expected "
       << tokenTypeName(type)
       << " but found "
       << tokenTypeName(here()->type)
       << ".";
    errors.add(ErrorLogger::Error, here()->origin, ss.str());
    throw ParserError();
}

void Parser::expectAdv(TokenType type) {
    expect(type);
    next();
    return;
}

bool Parser::matches(TokenType type) {
    if (!here() || here()->type != type) {
        return false;
    }
    return true;
}

void Parser::expect(const std::string &text) {
    if (matches(text)) {
        next();
        return;
    }

    if (!here()) {
        errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), "Unexpected EOF");
        throw ParserError();
    }

    std::stringstream ss;
    ss << "expected keyword \""
       << text
       << "\".";
    errors.add(ErrorLogger::Error, here()->origin, ss.str());

    throw ParserError();
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
        errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), ss.str());
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
