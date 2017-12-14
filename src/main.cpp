#include <cstring>
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
void build_game(GameData &gamedata, std::vector<AsmLine*> lines, const ProjectFile *projectFile);

SymbolDef* SymbolTable::get(const std::string &name) {
    if (symbols.count(name) > 0) {
        return symbols.at(name);
        }

    if (parent) {
        return parent->get(name);
    } else {
        return nullptr;
    }
}

bool SymbolTable::exists(const std::string &name) const {
    if (symbols.count(name) > 0) {
        return true;
    }
    if (parent) {
        return parent->exists(name);
    } else {
        return false;
    }
}

void SymbolTable::add(SymbolDef *symbol, bool functionScope) {
    if (exists(symbol->name)) {
        return;
    }

    symbols.insert({symbol->name, symbol});
}

std::string GameData::addString(const std::string &text) {
    std::stringstream ss;
    ss << "__str_" << nextString;
    ++nextString;
    stringtable[ss.str()] = text;
    return ss.str();
}

int AsmOperand::getSize() {
    if (mySize >= 0) return mySize;

    switch (type) {
        case AsmOperand::Stack:
            mySize = 0;
            break;
        case AsmOperand::Constant:
            if (value == 0) {
                mySize = 0;
                break;
            }
            if (value >= -128 && value <= 127) {
                mySize = 1;
            } else if (value >= -32768 && value <= 32767) {
                mySize = 2;
            } else {
                mySize = 4;
            }
            break;
        case AsmOperand::Address:
        case AsmOperand::Local:
            if (value <= 0xFF) {
                mySize = 1;
            } else if (value <= 0xFFFF) {
                mySize = 2;
            } else {
                mySize = 4;
            }
            break;
        case AsmOperand::Identifier:
            mySize = 4;
            break;
    }
    return mySize;
}

int AsmOperand::getMode() {
    int sizeMode = 0;
    switch(getSize()) {
        case 0: sizeMode = 0;   break;
        case 1: sizeMode = 1;   break;
        case 2: sizeMode = 2;   break;
        case 4: sizeMode = 3;   break;
    }

    switch(type) {
        case AsmOperand::Stack:         return 8;
        case AsmOperand::Identifier:
        case AsmOperand::Constant:      return sizeMode;
        case AsmOperand::Address:       return 4 + sizeMode;
        case AsmOperand::Local:         return 8 + sizeMode;
    }

    return 0;
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

int main(int argc, char **argv) {
    ErrorLogger errors;
    GameData gamedata;

    if (argc != 2) {
        std::cerr << "USAGE: gbuilder <project-file>\n";
        return 1;
    }
    ProjectFile *pf = load_project(argv[1]);

    Lexer lexer(errors);
    for (const std::string &filename : pf->sourceFiles) {
        std::string source = readFile(filename);
        lexer.doLex(filename, source);

        if (!errors.empty()) {
            showErrors(errors);
            delete pf;
            return 1;
        }
    }

    Parser parser(errors, gamedata, lexer.getTokens());
    parser.doParse();
    if (!errors.empty()) {
        showErrors(errors);
        delete pf;
        return 1;
    }

    doFirstPass(gamedata);
//    printAST(gamedata);


    auto asmlist = buildAsm(gamedata);
//    dump_asm(asmlist);
    build_game(gamedata, asmlist, pf);

    delete pf;
    std::cout << "Success!\n";
    return 0;
}