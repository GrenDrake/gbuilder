#include <sstream>
#include <string>

#include "gbuilder.h"

const char* operatorName(OperatorType type) {
    switch(type) {
        case OperatorType::Plus:                return "OpPlus";
        case OperatorType::Minus:               return "OpMinus";
        case OperatorType::Divide:              return "OpDivide";
        case OperatorType::Multiply:            return "OpMultiply";
        case OperatorType::Power:               return "OpPower";
        case OperatorType::Not:                 return "OpNot";
        case OperatorType::Modulus:             return "OpModulus";
        case OperatorType::Property:            return "OpProperty";
        case OperatorType::PlusEquals:          return "OpPlusEquals";
        case OperatorType::MinusEquals:         return "OpMinusEquals";
        case OperatorType::DivideEquals:        return "OpDivideEquals";
        case OperatorType::MultiplyEquals:      return "OpMultiplyEquals";
        case OperatorType::Increment:           return "OpIncrement";
        case OperatorType::Decrement:           return "OpDecrement";

        case OperatorType::LessThan:            return "LessThan";
        case OperatorType::LessThanOrEquals:    return "LessThanOrEquals";
        case OperatorType::GreaterThan:         return "GreaterThan";
        case OperatorType::GreaterThanOrEquals: return "GreaterThanOrEquals";
        case OperatorType::NotEquals:           return "NotEquals";
        case OperatorType::Equals:              return "Equals";

        case OperatorType::LogicalAnd:          return "LogicalAnd";
        case OperatorType::LogicalOr:           return "LogicalOr";
    }
}

const char* tokenTypeName(TokenType type) {
    switch(type) {
        case Identifier:            return "Identifier";
        case String:                return "String";
        case Integer:               return "Integer";
        case Float:                 return "Float";
        case Vocab:                 return "Vocab";
        case ReservedWord:          return "ReservedWord";
        case EndOfFile:             return "EndOfFile";

        case Operator:              return "OpAssign";
        case Assignment:            return "OpAssign";

        case OpenBrace:             return "OpenBrace";
        case CloseBrace:            return "CloseBrace";
        case OpenParan:             return "OpenParan";
        case CloseParan:            return "CloseParan";
        case Semicolon:             return "Semicolon";
        case Comma:                 return "Comma";
        case Question:              return "Question";

        default:                    return "unnamed token";
    }
}

static const char *reservedWords[] = {
    "asm",
    "constant",
    "function",
    "label",
    "local",
    "return"
};
static bool isReservedWord(const std::string &word) {
    for (const char *test : reservedWords) {
        if (test && word == test) {
            return true;
        }
    }
    return false;
}

void Lexer::doLex(const std::string &sourceFile, const std::string &source_text) {
    this->sourceFile = sourceFile;
    source = source_text;
    Token t;

    cLine = cColumn = 1;
    current = 0;

    while (here()) {
        if (isspace(here())) {
            next();

        } else if (here() == '/' && peek() == '/') {
            while (here() != '\n' && here() != 0) next();
        } else if (here() == '/' && peek() == '*') {
            int sline = cLine, scol = cColumn;
            while (here() != '/' || prev() != '*') {
                next();
                if (here() == 0) {
                    errors.add(ErrorLogger::Error, Origin(sourceFile, sline, scol), "unterminated block comment /* */");
                    break;
                }
            }
            next();

        } else if (here() == '{') {
            doSimpleToken(OpenBrace);
        } else if (here() == '}') {
            doSimpleToken(CloseBrace);
        } else if (here() == '(') {
            doSimpleToken(OpenParan);
        } else if (here() == ')') {
            doSimpleToken(CloseParan);
        } else if (here() == ';') {
            doSimpleToken(Semicolon);

        } else if (here() == '+') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::PlusEquals, 2);
            } else if (peek() == '+') {
                doOperatorToken(OperatorType::Increment, 2);
            } else {
                doOperatorToken(OperatorType::Plus, 1);
            }
        } else if (here() == '-') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::MinusEquals, 2);
            } else if (peek() == '-') {
                doOperatorToken(OperatorType::Decrement, 2);
            } else {
                doOperatorToken(OperatorType::Minus, 1);
            }
        } else if (here() == '/') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::DivideEquals, 2);
            } else {
                doOperatorToken(OperatorType::Divide, 1);
            }
        } else if (here() == '*') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::MultiplyEquals, 2);
            } else {
                doOperatorToken(OperatorType::Multiply, 1);
            }
        } else if (here() == '=') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::Equals, 2);
            } else {
                doSimpleToken(Assignment);
            }
        } else if (here() == '%') {
                doOperatorToken(OperatorType::Modulus, 1);
        } else if (here() == '!') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::NotEquals, 2);
            } else {
                doOperatorToken(OperatorType::Not, 1);
            }
        } else if (here() == '^') {
            doOperatorToken(OperatorType::Power, 1);
        } else if (here() == '.') {
                doOperatorToken(OperatorType::Property, 1);

        } else if (here() == '<') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::LessThanOrEquals, 2);
            } else {
                doOperatorToken(OperatorType::LessThan, 1);
            }
        } else if (here() == '>') {
            if (peek() == '=') {
                doOperatorToken(OperatorType::GreaterThanOrEquals, 2);
            } else {
                doOperatorToken(OperatorType::GreaterThan, 1);
            }
        } else if (here() == ',') {
            doSimpleToken(Comma);
        } else if (here() == '?') {
            doSimpleToken(Question);

        } else if (here() == '&' && peek() == '&') {
            doOperatorToken(OperatorType::LogicalAnd, 2);
        } else if (here() == '|' && peek() == '|') {
                doOperatorToken(OperatorType::LogicalOr, 2);

        } else if (isIdentifier(here(), true)) {
            doIdentifier();
        } else if (here() == '0' && (peek() == 'x' || peek() == 'X')) {
            doHexNumber();
        } else if (isdigit(here())) {
            doNumber();
        } else if (here() == '"') {
            doString();
        } else if (here() == '\'') {
            doCharLiteral();
        } else if (here() == '$') {
            doVocab();
        } else {
            std::stringstream ss;
            ss << "Unexpected character '" << (char)here() << "' (" << here() << ").";
            errors.add(ErrorLogger::Error, Origin(sourceFile, cLine, cColumn), ss.str());
            next();
        }
    }
    doSimpleToken(EndOfFile);
}

void Lexer::doHexNumber() {
    Token t(sourceFile, cLine, cColumn, Integer);
    int start = current;
    next(); next();

    while (isxdigit(here())) {
        next();
    }

    const std::string &text = source.substr(start, current-start);
    t.vInteger = std::stoi(text, nullptr, 16);
    tokens.push_back(std::move(t));
}

void Lexer::doNumber() {
    Token t(sourceFile, cLine, cColumn, Integer);

    bool isFloat = false;
    int start = current;
    while (isdigit(here())) {
        next();
    }
    if (here() == '.') {
        next();
        isFloat = true;
        while (isdigit(here())) {
            next();
        }
    }

    const std::string &text = source.substr(start, current-start);
    if (isFloat) {
        t.type = Float;
        t.vFloat = std::stod(text);
    } else {
        t.vInteger = std::stoi(text, nullptr, 10);
    }
    tokens.push_back(std::move(t));
}

void Lexer::doIdentifier() {
    Token t(sourceFile, cLine, cColumn, Identifier);

    int start = current;
    while (isIdentifier(here())) {
        next();
    }

    t.vText = source.substr(start, current-start);
    if (isReservedWord(t.vText)) {
        t.type = ReservedWord;
    }
    tokens.push_back(std::move(t));
}

void Lexer::doOperatorToken(OperatorType type, int length) {
    tokens.push_back(Token(sourceFile, cLine, cColumn, type));
    while (length > 0) --length;
}
void Lexer::doSimpleToken(TokenType type) {
    tokens.push_back(Token(sourceFile, cLine, cColumn, type));
    next();
}

void Lexer::doSimpleToken2(TokenType type) {
    tokens.push_back(Token(sourceFile, cLine, cColumn, type));
    next();
    next();
}

void Lexer::doCharLiteral() {
    Token t(sourceFile, cLine, cColumn, Integer);

    next();
    int start = current;
    while (here() != 0 && (here() != '\'' || prev() == '\\')) {
        next();
        if (here() == 0) {
            errors.add(ErrorLogger::Error, t.origin, "unterminated character literal");
        }
    }

    std::string rawText = source.substr(start, current-start);
    unescape(t.origin, rawText);
    if (rawText.size() == 0) {
        errors.add(ErrorLogger::Error, t.origin, "empty character literal");
    } else {
        if (rawText.size() > 1) {
            errors.add(ErrorLogger::Error, t.origin, "character literal too long");
        }
        t.vInteger = rawText[0];
    }
    tokens.push_back(std::move(t));
    next();
}

void Lexer::doString() {
    Token t(sourceFile, cLine, cColumn, String);

    next();
    int start = current;
    while (here() != 0 && (here() != '"' || prev() == '\\')) {
        next();
        if (here() == 0) {
            errors.add(ErrorLogger::Error, t.origin, "unterminated string");
        }
    }

    t.vText = source.substr(start, current-start);
    unescape(t.origin, t.vText);
    tokens.push_back(std::move(t));
    next();
}

void Lexer::doVocab() {
    Token t(sourceFile, cLine, cColumn, Vocab);

    next();
    int start = current;
    while (here() != 0 && here() != '$') {
        next();
        if (here() == 0) {
            errors.add(ErrorLogger::Error, t.origin, "unterminated vocab word");
        }
    }

    t.vText = source.substr(start, current-start);
    vocab.insert(t.vText);
    tokens.push_back(std::move(t));
    next();
}

bool Lexer::isIdentifier(int c, bool isInitial) const {
    if (isalpha(c) || c == '_') {
        return true;
    }
    if (!isInitial && isdigit(c)) {
        return true;
    }
    return false;
}

void Lexer::unescape(const Origin &origin, std::string &text) {
    for (unsigned i = 0; i < text.size(); ++i) {
        if (text[i] != '\\') {
            continue;
        }

        if (i + 1 >= text.size()) {
            errors.add(ErrorLogger::Error, origin, "incomplete escape at end of string");
            continue;
        }

        switch(text[i+1]) {
            case '\\':
            case '\'':
            case '"':
                text[i] = text[i+1];
                break;
            case 'n':
                text[i] = '\n';
                break;
            case 't':
                text[i] = '\t';
                break;
            default:
                errors.add(ErrorLogger::Error, origin, "unknown string escape");
                text[i] = '?';
                break;
        }
        text.erase(i+1, 1);
    }
}

int Lexer::here() const {
    if (current < source.size()) {
        return source[current];
    } else {
        return 0;
    }
}

int Lexer::next() {
    if (current < source.size()) {
        ++current;
        ++cColumn;
    }
    int c = here();
    if (c == '\n') {
        cColumn = 0;
        ++cLine;
    }
    return c;
}

int Lexer::peek() const {
    if (current + 1 < source.size()) {
        return source[current + 1];
    } else {
        return 0;
    }
}

int Lexer::prev() const {
    if (current - 1 > 0) {
        return source[current - 1];
    } else {
        return 0;
    }
}
