#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "gbuilder.h"

std::string escapeString(const std::string &text) {
    std::stringstream ss;
    for (char c : text) {
        switch(c) {
            case '\t':
                ss << "\\t";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '\r':
                ss << "\\r";
                break;
            default:
                ss << c;
        }
    }
    return ss.str();
}


void dump_tokens(const std::vector<Token> &tokens) {
    const int maxStringSize = 20;

    for (const auto &token : tokens) {
        std::cout << std::setw(3) << std::right << token.type << ": ";
        std::cout << std::setw(15) << std::left << tokenTypeName(token.type);
        if (token.type == Identifier || token.type == String || token.type == ReservedWord) {
            const std::string &text = escapeString(token.vText);
            if (text.size() > maxStringSize) {
                std::cout << text.substr(0,maxStringSize - 3) << "...";
            } else {
                std::cout << std::setw(maxStringSize) << text;
            }
        } else if (token.type == Integer) {
            std::cout << std::setw(maxStringSize) << token.vInteger;
        } else if (token.type == Float) {
            std::cout << std::setw(maxStringSize) << token.vFloat;
        } else {
            std::cout << "                    ";
        }
        std::cout << ' ' << token.file << ':' << token.line << ':' << token.column;
        std::cout << "\n";
    }
    std::cout << std::right;
}