#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include "gbuilder.h"

void printAST(GameData &gd);
void dump_asm(std::vector<AsmLine*> lines);
void doFirstPass(GameData &gd);
std::vector<AsmLine*> buildAsm(GameData &gd);

SymbolDef* SymbolTable::get(const std::string &name) {
    for (SymbolDef &s : symbols) {
        if (s.name == name) {
            return &s;
        }
    }
    if (parent) {
        return parent->get(name);
    } else {
        return nullptr;
    }
}

std::string GameData::addString(const std::string &text) {
    std::stringstream ss;
    ss << "__str_" << nextString;
    ++nextString;
    stringtable[ss.str()] = text;
    return ss.str();
}

void showErrors(ErrorLogger &errors) {
    for (auto m : errors) {
        std::cerr << m.format() << "\n";
    }
    std::cout << errors.count() << " error(s) occured.\n";
}

std::string readFile(const std::string &file) {
    std::ifstream inf(file);
    std::string content( (std::istreambuf_iterator<char>(inf)),
                         std::istreambuf_iterator<char>() );
    return content;
}

int main() {
    std::array<const char *, 2> sourceFiles = { {
        "test.gb",
        "glk.gc"
    } };
    ErrorLogger errors;
    GameData gamedata;

    std::vector<Token> tokenList;
    for (const char *filename : sourceFiles) {
        std::string source = readFile(filename);
        Lexer lexer(errors, gamedata, filename, source);
        lexer.doLex();

        if (!errors.empty()) {
            showErrors(errors);
            return 1;
        }

        auto& newTokens = lexer.getTokens();
        tokenList.insert(tokenList.cend(), newTokens.begin(), newTokens.end());
    }

    Parser parser(errors, gamedata, tokenList);
    parser.doParse();
    if (!errors.empty()) {
        showErrors(errors);
        return 1;
    }

    doFirstPass(gamedata);
    printAST(gamedata);


    auto asmlist = buildAsm(gamedata);
    dump_asm(asmlist);

    return 0;
}