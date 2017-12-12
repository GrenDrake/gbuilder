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
void build_game(std::vector<AsmLine*> lines);

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

int AsmOperand::getSize() const {
    ;
    return 2;
}

int AsmStatement::getSize() const {
    int size = 0;

    // size of opcode
    if (opcode > 0x3FFF) {
        size += 4;
    } else if (opcode > 0x7F) {
        size += 2;
    } else {
        size += 1;
    }

    // size of addressing modes
    size += operands.size() / 2;
    if (operands.size() % 2) {
        ++size;
    }

    // size of operands
    for (AsmOperand *op : operands) {
        size += op->getSize();
    }

    return size;
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
        "test.gc",
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
//    printAST(gamedata);


    auto asmlist = buildAsm(gamedata);
    dump_asm(asmlist);
    build_game(asmlist);

    return 0;
}