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
                std::shared_ptr<FunctionDef> newfunc(doFunction());
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

std::shared_ptr<FunctionDef> Parser::doFunction() {
    const Origin &origin = here()->origin;
    expect("function");
    expect(Identifier);

    std::shared_ptr<FunctionDef> newfunc(new FunctionDef);
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
        return nullptr;
    }
    gamedata.symbols.add(new SymbolDef(newfunc->name, SymbolDef::Function));

    std::shared_ptr<LiteralExpression> retValue(new LiteralExpression);
    retValue->litValue = 0;
    std::shared_ptr<ReturnDef> defaultReturn(new ReturnDef);
    defaultReturn->retValue = retValue;
    newfunc->code->statements.push_back(defaultReturn);
    return newfunc;
}


/* ************************************************************ *
 * STATEMENT PARSING                                            *
 * ************************************************************ */

 std::shared_ptr<StatementDef> Parser::doStatement() {
    std::shared_ptr<StatementDef> stmt;
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
        } else if (matches(Semicolon)) {
            next();
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

std::shared_ptr<CodeBlock> Parser::doCodeBlock() {
    const Origin &origin = here()->origin;
    expectAdv(OpenBrace);

    std::shared_ptr<CodeBlock> code(new CodeBlock);
    code->origin = origin;
    code->locals.parent = curTable;
    while (!matches(CloseBrace)) {
        if (here() == nullptr) {
            return nullptr;
        }

        curTable = &code->locals;
        std::shared_ptr<StatementDef> stmt(doStatement());
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

std::shared_ptr<LabelStmt> Parser::doLabel() {
    expect("label");
    expect(Identifier);
    const std::string &name = here()->vText;
    next();
    expectAdv(Semicolon);
    symbolExists(*curTable, name);
    SymbolDef *sym = new SymbolDef(name, SymbolDef::Label);
    curTable->add(sym, true);
    return std::shared_ptr<LabelStmt>(new LabelStmt(name));
}

std::shared_ptr<ReturnDef> Parser::doReturn() {
    expect("return");
    std::shared_ptr<ReturnDef> returnStmt(new ReturnDef);
    if (!matches(Semicolon)) {
        returnStmt->retValue = doExpression();
    } else {
        std::shared_ptr<LiteralExpression> retValue(new LiteralExpression);
        retValue->litValue = 0;
        returnStmt->retValue = retValue;
    }
    expectAdv(Semicolon);
    return returnStmt;
}

std::shared_ptr<ExpressionDef> Parser::doExpression() {
    std::shared_ptr<ExpressionDef> expr;
    if (matches(Integer)) {
        std::shared_ptr<LiteralExpression> realExpr(new LiteralExpression);
        realExpr->litValue = here()->vInteger;
        expr = realExpr;
        next();
    } else if (matches(Identifier)) {
        std::shared_ptr<NameExpression> realExpr(new NameExpression);
        realExpr->name = here()->vText;
        expr = realExpr;
        next();
    } else {
        std::stringstream ss;
        ss << "unexpected token ";
        ss << tokenTypeName(here()->type);
        ss << ".";
        errors.add(ErrorLogger::Error, here()->origin, ss.str());
        synchronize();
        return nullptr;
    }
    return expr;
}

std::shared_ptr<Value> Parser::doValue() {
    std::shared_ptr<Value> value(new Value);
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
            errors.add(ErrorLogger::Error, Origin("(unknown)",0,0), "expected value");
            next();
            return nullptr;
    }
}


/* ************************************************************ *
 * ASSEMBLY PARSING                                             *
 * ************************************************************ */

 std::shared_ptr<StatementDef> Parser::doAsmBlock() {
    const Origin &origin = here()->origin;
    expect("asm");

    if (!matches(OpenBrace)) {
        return doAsmStatement();
    }

    expectAdv(OpenBrace);
    std::shared_ptr<CodeBlock> code(new CodeBlock);
    code->origin = origin;
    code->locals.parent = curTable;


    while (!matches(CloseBrace)) {
        if (here() == nullptr) {
            return nullptr;
        }

        std::shared_ptr<StatementDef> stmt = doAsmStatement();
        if (stmt) {
            code->statements.push_back(stmt);
        }
    }
    next();
    return code;
}

std::shared_ptr<StatementDef> Parser::doAsmStatement() {
    if (matches("label")) return doLabel();

    if (!matches(Identifier) && !matches(ReservedWord)) {
        expect(Identifier);
    }
    std::shared_ptr<AsmStatement> stmt(new AsmStatement);
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
        std::shared_ptr<AsmOperand> op = doAsmOperand();
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

std::shared_ptr<AsmOperand> Parser::doAsmOperand() {
    std::shared_ptr<AsmOperand> op(new AsmOperand);

    if (matches(Identifier) && here()->vText == "sp") {
        op->isStack = true;
        next();
        return op;
    }

    op->value = doValue();

    if (!op->value) {
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
