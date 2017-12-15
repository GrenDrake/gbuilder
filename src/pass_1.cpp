#include <iostream>
#include <sstream>

#include "gbuilder.h"


class FirstPassWalker : public AstWalker {
public:
    FirstPassWalker(ErrorLogger &errors)
    : errors(errors) {
    }

    virtual void visit(Value *stmt) {
        if (stmt->type == Value::Identifier) {
            SymbolDef *s = codeBlock->locals.get(stmt->text);
            if (s) {
                if (s->type == SymbolDef::Constant) {
                    stmt->value = s->value;
                    stmt->type = Value::Constant;
                } else if (s->type == SymbolDef::Local) {
                    stmt->value = s->value;
                    stmt->type = Value::Local;
                } else if (s->type == SymbolDef::Label) {
                    stmt->text = "__" + function->name + "__" + stmt->text;
                }
            } else {
                std::stringstream ss;
                ss << "Undefined symbol " << stmt->text << ".";
                errors.add(ErrorLogger::Error, Origin("(1st-pass)", 0, 0), ss.str());
            }
        }
    }
    virtual void visit(AsmStatement *stmt) {
        for (AsmOperand *op : stmt->operands) {
            if (!op->isStack) {
                op->value->accept(this);
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

