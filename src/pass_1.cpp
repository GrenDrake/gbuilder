#include <iostream>
#include <sstream>

#include "gbuilder.h"


class FirstPassWalker : public AstWalker {
public:
    FirstPassWalker(ErrorLogger &errors)
    : errors(errors) {
    }

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
                    } else if (s->type == SymbolDef::Label) {
                        op->text = "__" + function->name + "__" + s->name;
                    }
                } else {
                    std::stringstream ss;
                    ss << "Undefined symbol " << op->text << ".";
                    errors.add(ErrorLogger::Error, "(1st-pass)", 0, 0, ss.str());
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
        function = stmt;
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
        stmt->name = "__" + function->name + "__" + stmt->name;
    }

private:
    void numberLocals(SymbolTable &symbols) {
        int cLocal = locals;
        for (auto &s : symbols.symbols) {
            s.second->value = cLocal;
            ++cLocal;
        }
        locals = cLocal;
    }

    FunctionDef *function;
    CodeBlock *codeBlock;
    int locals;
    ErrorLogger &errors;
};




void doFirstPass(GameData &gd, ErrorLogger &errors) {

    FirstPassWalker fpw(errors);
    for (FunctionDef *f : gd.functions) {
        f->accept(&fpw);
    }

}

