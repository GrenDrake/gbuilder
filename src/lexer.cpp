#include <sstream>
#include <string>

#include "gbuilder.h"

const char* tokenTypeName(TokenType type) {
    switch(type) {
        case Identifier:            return "Identifier";
        case String:                return "String";
        case Integer:               return "Integer";
        case Float:                 return "Float";
        case Vocab:                 return "Vocab";
        case ReservedWord:          return "ReservedWord";

        case OpPlus:                return "OpPlus";
        case OpMinus:               return "OpMinus";
        case OpDivide:              return "OpDivide";
        case OpMultiply:            return "OpMultiply";
        case OpPower:               return "OpPower";
        case OpNot:                 return "OpNot";
        case OpModulus:             return "OpModulus";
        case OpAssign:              return "OpAssign";
        case OpProperty:            return "OpProperty";
        case OpPlusEquals:          return "OpPlusEquals";
        case OpMinusEquals:         return "OpMinusEquals";
        case OpDivideEquals:        return "OpDivideEquals";
        case OpMultiplyEquals:      return "OpMultiplyEquals";
        case OpIncrement:           return "OpIncrement";
        case OpDecrement:           return "OpDecrement";

        case LessThan:              return "LessThan";
        case LessThanOrEquals:      return "LessThanOrEquals";
        case GreaterThan:           return "GreaterThan";
        case GreaterThanOrEquals:   return "GreaterThanOrEquals";
        case NotEquals:             return "NotEquals";
        case Equals:                return "Equals";

        case OpenBrace:             return "OpenBrace";
        case CloseBrace:            return "CloseBrace";
        case OpenParan:             return "OpenParan";
        case CloseParan:            return "CloseParan";
        case Semicolon:             return "Semicolon";
        case Comma:                 return "Comma";
        case Question:              return "Question";

        case LogicalAnd:            return "LogicalAnd";
        case LogicalOr:             return "LogicalOr";

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

void Lexer::doLex() {
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
                    errors.add(ErrorLogger::Error, sourceFile, sline, scol, "unterminated block comment /* */");
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
                doSimpleToken2(OpPlusEquals);
            } else if (peek() == '+') {
                doSimpleToken2(OpIncrement);
            } else {
                doSimpleToken(OpPlus);
            }
        } else if (here() == '-') {
            if (peek() == '=') {
                doSimpleToken2(OpMinusEquals);
            } else if (peek() == '+') {
                doSimpleToken2(OpDecrement);
            } else {
                doSimpleToken(OpMinus);
            }
        } else if (here() == '/') {
            if (peek() == '=') {
                doSimpleToken2(OpDivideEquals);
            } else {
                doSimpleToken(OpDivide);
            }
        } else if (here() == '*') {
            if (peek() == '=') {
                doSimpleToken2(OpDivideEquals);
            } else {
                doSimpleToken(OpMultiply);
            }
        } else if (here() == '=') {
            if (peek() == '=') {
                doSimpleToken2(Equals);
            } else {
                doSimpleToken(OpAssign);
            }
        } else if (here() == '%') {
            doSimpleToken(OpModulus);
        } else if (here() == '!') {
            if (peek() == '=') {
                doSimpleToken2(NotEquals);
            } else {
                doSimpleToken(OpNot);
            }
        } else if (here() == '^') {
            doSimpleToken(OpPower);
        } else if (here() == '.') {
            doSimpleToken(OpProperty);

        } else if (here() == '<') {
            if (peek() == '=') {
                doSimpleToken2(LessThanOrEquals);
            } else {
                doSimpleToken(LessThan);
            }
        } else if (here() == '>') {
            if (peek() == '=') {
                doSimpleToken2(GreaterThanOrEquals);
            } else {
                doSimpleToken(GreaterThan);
            }
        } else if (here() == ',') {
            doSimpleToken(Comma);
        } else if (here() == '?') {
            doSimpleToken(Question);

        } else if (here() == '&' && peek() == '&') {
            doSimpleToken2(LogicalAnd);
        } else if (here() == '|' && peek() == '|') {
            doSimpleToken2(LogicalAnd);

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
            errors.add(ErrorLogger::Error, sourceFile, cLine, cColumn, ss.str());
            next();
        }
    }

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
            errors.add(ErrorLogger::Error, sourceFile, t.line, t.column, "unterminated character literal");
        }
    }

    std::string rawText = source.substr(start, current-start);
    unescape(t.line, t.column, rawText);
    if (rawText.size() == 0) {
        errors.add(ErrorLogger::Error, sourceFile, t.line, t.column, "empty character literal");
    } else {
        if (rawText.size() > 1) {
            errors.add(ErrorLogger::Error, sourceFile, t.line, t.column, "character literal too long");
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
            errors.add(ErrorLogger::Error, sourceFile, t.line, t.column, "unterminated string");
        }
    }

    t.vText = source.substr(start, current-start);
    unescape(t.line, t.column, t.vText);
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
            errors.add(ErrorLogger::Error, sourceFile, t.line, t.column, "unterminated vocab word");
        }
    }

    t.vText = source.substr(start, current-start);
    gamedata.vocabRaw.insert(t.vText);
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

void Lexer::unescape(int line, int column, std::string &text) {
    for (unsigned i = 0; i < text.size(); ++i) {
        if (text[i] != '\\') {
            continue;
        }

        if (i + 1 >= text.size()) {
            errors.add(ErrorLogger::Error, sourceFile, line, column, "incomplete escape at end of string");
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
                errors.add(ErrorLogger::Error, sourceFile, line, column, "unknown string escape");
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
