#include <iostream>
#include <sstream>
#include <vector>

#include "gbuilder.h"

void Parser::doParse() {

    doFunction();
    while (here()) {
        if (matches("function")) {
            doFunction();
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

void Parser::doFunction() {
    if (!expect("function")) return;
    if (!expect(Identifier)) return;
    const std::string &funcname = here()->vText;
    next();
    expectAdv(OpenParan);
    expectAdv(CloseParan);
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
