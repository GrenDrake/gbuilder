#include <iostream>

#include "gbuilder.h"


class FirstPassWalker : public AstWalker {
public:

    virtual void visit(AsmStatement *stmt) {
        for (AsmOperand *op : stmt->operands) {
            if (op->type == AsmOperand::Identifier) {
                SymbolDef *s = codeBlock->locals.get(op->text);
                if (s) {
                    if (s->type == SymbolDef::Constant) {
                        op->value = s->value;
                        op->type = AsmOperand::Constant;
                    } else if (s->type == SymbolDef::Local) {
                        op->value = s->value;
                        op->type = AsmOperand::Local;
                    }
                }
            }
        }
    }
    virtual void visit(AsmData *stmt) {
    }
    virtual void visit(CodeBlock *stmt) {
        numberLocals(stmt->locals);
        int localCount = locals;
        int maxLocals = locals;
        for (StatementDef *s : stmt->statements) {
           locals = localCount;
           codeBlock = stmt;
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

    CodeBlock *codeBlock;
    int locals;
};




void doFirstPass(GameData &gd) {

    FirstPassWalker fpw;
    for (FunctionDef *f : gd.functions) {
        f->accept(&fpw);
    }

}

