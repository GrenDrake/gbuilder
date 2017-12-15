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
void dump_asm(std::vector<std::shared_ptr<AsmLine> > lines);
void doFirstPass(GameData &gd, ErrorLogger &errors);
std::vector<std::shared_ptr<AsmLine> > buildAsm(GameData &gd);
void build_game(GameData &gamedata, std::vector<std::shared_ptr<AsmLine> > lines, const ProjectFile *projectFile, bool dumpLabels);
void dump_tokens(const std::vector<Token> &tokens);

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

    if (functionScope && parent) {
        SymbolTable *cur = this;
        while (cur->parent->parent != nullptr) {
            cur = cur->parent;
        }
        cur->add(symbol, false);
    } else {
        symbols.insert({symbol->name, symbol});
    }
}

std::string GameData::addString(const std::string &text) {
    std::stringstream ss;
    ss << "__str_" << nextString;
    ++nextString;
    stringtable[ss.str()] = text;
    symbols.add(new SymbolDef(ss.str(), SymbolDef::String));
    return ss.str();
}

int AsmOperand::getSize() {
    if (mySize >= 0) return mySize;

    if (isStack) {
        mySize = 0;
        return 0;
    }

    if (value->type == Value::Identifier) {
        mySize = 4;
    } else if (value->type == Value::Constant && !isIndirect) {
        if (value == 0) {
            mySize = 0;
        } else if (value->value >= -128 && value->value <= 127) {
            mySize = 1;
        } else if (value->value >= -32768 && value->value <= 32767) {
            mySize = 2;
        } else {
            mySize = 4;
        }
    } else { // indirect access & locals
        if (value->value <= 0xFF) {
            mySize = 1;
        } else if (value->value <= 0xFFFF) {
            mySize = 2;
        } else {
            mySize = 4;
        }
    }

    return mySize;
}

int AsmOperand::getMode() {
    if (isStack) return 8;

    int sizeMode = 0;
    switch(getSize()) {
        case 0: sizeMode = 0;   break;
        case 1: sizeMode = 1;   break;
        case 2: sizeMode = 2;   break;
        case 4: sizeMode = 3;   break;
    }

    if (!isIndirect && value->type == Value::Constant && value->value == 0) {
        return 0;
    }

    if (isIndirect) {
        return 4 + sizeMode;
    } else if (value->type == Value::Local) {
        return 8 + sizeMode;
    } else { // Identifier & Constant
        return sizeMode;
    }
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
    for (auto op : operands) {
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
    bool showAST = false;
    bool showASM = false;
    bool showLabels = false;
    bool showTokens = false;

    if (argc < 2) {
        std::cerr << "USAGE: gbuilder <project-file> [-ast] [-asm]\n";
        return 1;
    }
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-ast") == 0) {
            showAST = true;
        } else if (strcmp(argv[i], "-asm") == 0) {
            showASM = true;
        } else if (strcmp(argv[i], "-labels") == 0) {
            showLabels = true;
        } else if (strcmp(argv[i], "-tokens") == 0) {
            showTokens = true;
        } else {
            std::cerr << "Unrecognized argument " << argv[i] << "\n";
            return 1;
        }
    }


    ProjectFile *pf = load_project(argv[1]);
    if (pf->sourceFiles.empty()) {
        std::cerr << "No source files specified!\n";
        delete pf;
        return 1;
    }
    std::cout << "Input files:";
    for (const std::string &file : pf->sourceFiles) {
        std::cout << ' ' << file;
    }
    std::cout << "\nTarget: " << pf->outputFile << "\n";


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

    if (showTokens) {
        dump_tokens(lexer.getTokens());
    }

    Parser parser(errors, gamedata, lexer.getTokens());
    parser.doParse();
    if (!errors.empty()) {
        showErrors(errors);
        delete pf;
        return 1;
    }

    doFirstPass(gamedata, errors);
    if (showAST) printAST(gamedata);
    if (!errors.empty()) {
        showErrors(errors);
        delete pf;
        return 1;
    }


    auto asmlist = buildAsm(gamedata);
    if (showASM) dump_asm(asmlist);
    build_game(gamedata, asmlist, pf, showLabels);

    delete pf;
    std::cout << "Success!\n";
    return 0;
}