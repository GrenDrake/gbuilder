#include <iostream>

#include "gbuilder.h"


class FirstPassWalker : public AstWalker {
public:
    FirstPassWalker(GameData &gd)
    : gamedata(gd) {

    }

    virtual void visit(AsmOperandInteger *stmt) {}
    virtual void visit(AsmOperandIdentifier *stmt) {}
    virtual void visit(AsmOperandStack *stmt) {}
    virtual void visit(AsmStatement *stmt) {
        auto op = stmt->operands.begin();
        while (op != stmt->operands.end()) {
            AsmOperandIdentifier *ident = dynamic_cast<AsmOperandIdentifier*>(*op);
            if (ident) {
                SymbolDef *s = gamedata.symbols.get(ident->value);
                if (s) {
                    if (s->type == SymbolDef::Constant) {
                        AsmOperandInteger *newInt = new AsmOperandInteger;
                        newInt->value = s->value;
                        delete *op;
                        op = stmt->operands.erase(op);
                        op = stmt->operands.insert(op, newInt);
                        std::cout << ident->value << " = " << newInt->value << '\n';
                    }
                }
            }
            op = ++op;
        }
    }
    virtual void visit(CodeBlock *stmt) {
        numberLocals(stmt->locals);
        int localCount = locals;
        int maxLocals = locals;
        for (StatementDef *s : stmt->statements) {
           locals = localCount;
            s->accept(this);
            if (locals > maxLocals) {
               maxLocals = locals;
            }
        }
       locals = maxLocals;
    }
    virtual void visit(FunctionDef *stmt) {
        locals = 0;
        numberLocals(stmt->args);
        int localCount = locals;
        int maxLocals = locals;
        if (stmt->code) {
            locals = localCount;
            stmt->code->accept(this);
            if (locals > maxLocals) {
                maxLocals = locals;
            }
        }
        stmt->localCount = maxLocals;
    }
    virtual void visit(ReturnDef *stmt) {
    }
    virtual void visit(LabelStmt *stmt) {
    }

private:
    void numberLocals(SymbolTable &symbols) {
        int cLocal = locals;
        for (SymbolDef &s : symbols.symbols) {
            s.value = cLocal;
            ++cLocal;
        }
        locals = cLocal;
    }

    GameData &gamedata;
    int locals;
};




void doFirstPass(GameData &gd) {

    FirstPassWalker fpw(gd);
    for (FunctionDef *f : gd.functions) {
        f->accept(&fpw);
    }

}

