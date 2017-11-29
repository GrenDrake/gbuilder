#include <iostream>

#include "gbuilder.h"


class PrintAstWalker : public AstWalker {
public:
    virtual void visit(AsmOperandInteger *stmt) {
        std::cout << " i:" << stmt->value;
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
        std::cout << "BEGIN\n";
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
        std::cout << "FUNCTION " << stmt->name << "\n";
        if (stmt->code) {
            stmt->code->accept(this);
        } else {
            std::cout << "   (bad function body)\n";
        }
    }
    
private:
    void spaces() const {
        for (int i = 0; i < depth; ++i) {
            std::cout << "   ";
        }
    }
    int depth;
};




void printAST(GameData &gd) {

    PrintAstWalker aw;
    std::cout << "FUNCTIONS: " << gd.functions.size() << '\n';
    for (FunctionDef *f : gd.functions) {
        f->accept(&aw);
//        std::cout << f->name << '\n';
    }

}

