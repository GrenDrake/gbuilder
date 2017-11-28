#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "gbuilder.h"

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
    GameData gamedata;

    std::string filename = "test.gb";
    std::string source = readFile(filename);
    ErrorLogger errors;
    Lexer lexer(errors, gamedata, filename, source);
    lexer.doLex();
    
    if (!errors.empty()) {
        showErrors(errors);
        return 1;
    }
    
    auto tokensList = lexer.getTokens();
    std::vector<Token> tokensVec(tokensList.begin(), tokensList.end());
    Parser parser(errors, gamedata, tokensVec);
    parser.doParse();
    if (!errors.empty()) {
        showErrors(errors);
        return 1;
    }

    /*
    for (Token &t : tokens) {
        std::cout << t.file << ":" << t.line << ":" << t.column << ":  " << tokenTypeName(t.type);
        switch (t.type) {
            case Identifier:
            case String:
            case Vocab:
                std::cout << " ~" << t.vText << "~\n";
                break;
            case Integer:
                std::cout << " ~" << t.vInteger << "~\n";
                break;
            case Float:
                std::cout << " ~" << t.vFloat << "~\n";
                break;
            default:
                std::cout << "\n";
        }
    }*/

    return 0;
}