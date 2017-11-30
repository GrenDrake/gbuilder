#include <iostream>

#include "gbuilder.h"


class PrintAstWalker : public AstWalker {
public:
    virtual void visit(AsmOperandInteger *stmt) {
        std::cout << " i:" << stmt->value;
    }
    virtual void visit(AsmOperandIdentifier *stmt) {
        std::cout << " s:" << stmt->value;
    }
    virtual void visit(AsmOperandStack *stmt) {
        std::cout << " SP";
    }
    virtual void visit(AsmStatement *stmt) {
        spaces();
        std::cout << "ASM  " << stmt->opcode;
        for (AsmOperand *op : stmt->operands) {
            op->accept(this);
        }
        std::cout << '\n';
    }
    virtual void visit(CodeBlock *stmt) {
        spaces();
        std::cout << "BEGIN  ";
        printSymbols(stmt->locals);
        ++depth;
        for (StatementDef *s : stmt->statements) {
            s->accept(this);
        }
        --depth;
        spaces();
        std::cout << "END\n";
    }
    virtual void visit(FunctionDef *stmt) {
        depth = 0;
        std::cout << "\nFUNCTION " << stmt->name << ' ';
        printSymbols(stmt->args);
        if (stmt->code) {
            stmt->code->accept(this);
        } else {
            std::cout << "   (bad function body)\n";
        }
    }
    
private:
    void printSymbols(SymbolTable &symbols) {
        std::cout << "(" << symbols.symbols.size() << ":";
        for (SymbolDef &s : symbols.symbols) {
            std::cout << " ~" << s.name << '~';
        }
        std::cout << " )\n";
    }
    void spaces() const {
        for (int i = 0; i < depth; ++i) {
            std::cout << "   ";
        }
    }
    int depth;
};




void printAST(GameData &gd) {

    std::cout << "VOCABULARY: " << gd.vocabRaw.size() << " :";
    if (!gd.vocabRaw.empty()) {
        for (const std::string &s : gd.vocabRaw) {
            std::cout << ' ' << s;
        }
    }

    PrintAstWalker aw;
    std::cout << "\n\nFUNCTIONS: " << gd.functions.size() << '\n';
    for (FunctionDef *f : gd.functions) {
        f->accept(&aw);
//        std::cout << f->name << '\n';
    }

}

