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


std::string GameData::addString(const std::string &text) {
    std::stringstream ss;
    ss << "__str_" << nextString;
    ++nextString;
    stringtable[ss.str()] = text;
    symbols.add(new SymbolDef(ss.str(), SymbolDef::String));
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
    if (!inf) {
        throw std::runtime_error("Could not open file.");
    }
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
        std::string source;
        try {
            source = readFile(filename);
        } catch (std::runtime_error &e) {
            std::cerr << "An error occured while trying to read the file \"" << filename << "\".\n";
            delete pf;
            return 1;
        }
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