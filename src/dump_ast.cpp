#include <iostream>

#include "gbuilder.h"


class PrintAstWalker : public AstWalker {
public:
    virtual void visit(Value *stmt) {
        switch(stmt->type) {
            case Value::Constant:
                std::cout << " c:" << stmt->value;
                break;
            case Value::Local:
                std::cout << " l:" << stmt->value;
                break;
            case Value::Identifier:
                std::cout << " i:~" << stmt->text << '~';
                break;
            default:
                break;
        }
    }
    virtual void visit(AsmStatement *stmt) {
        spaces();
        std::cout << "ASM  " << stmt->opname << " (" << stmt->opcode << ')';
        for (AsmOperand *op : stmt->operands) {
            if (op->isStack) {
                std::cout << " sp";
            } else {
                op->value->accept(this);
            }
        }
        std::cout << '\n';
    }
    virtual void visit(AsmData *stmt) {
    }
    virtual void visit(CodeBlock *stmt) {
        spaces();
        std::cout << "BEGIN  ";
        printOrigin(stmt->origin);
        std::cout << ' ';
        printSymbols(stmt->locals);
        ++depth;
        for (StatementDef *s : stmt->statements) {
            curBlock = stmt;
            s->accept(this);
        }
        --depth;
        spaces();
        std::cout << "END\n";
    }
    virtual void visit(FunctionDef *stmt) {
        depth = 0;
        std::cout << "\nFUNCTION " << stmt->name;
        std::cout << " (locals: " << stmt->localCount << ") ";
        printOrigin(stmt->origin);
        std::cout << ' ';
        printSymbols(stmt->args);
        if (stmt->code) {
            stmt->code->accept(this);
        } else {
            std::cout << "   (bad function body)\n";
        }
    }
    virtual void visit(ReturnDef *stmt) {
        spaces();
        std::cout << "RETURN 0\n";
    }
    virtual void visit(LabelStmt *stmt) {
        spaces();
        std::cout << "LABEL ~" << stmt->name << "~\n";
    }

private:
    void printOrigin(const Origin &origin) {
        std::cout << "[" << origin.file << ":" << origin.line << ":" << origin.column << "]";
    }
    void printSymbols(SymbolTable &symbols) {
        std::cout << "(" << symbols.symbols.size() << ":";
        for (auto &s : symbols.symbols) {
            std::cout << "  (" << s.second->value << ") ~" << s.second->name << '~';
        }
        std::cout << " )\n";
    }
    void spaces() const {
        for (int i = 0; i < depth; ++i) {
            std::cout << "   ";
        }
    }
    int depth;
    CodeBlock *curBlock;
};




void printAST(GameData &gd) {

    std::cout << "VOCABULARY: " << gd.vocabRaw.size() << " :";
    if (!gd.vocabRaw.empty()) {
        for (const std::string &s : gd.vocabRaw) {
            std::cout << ' ' << s;
        }
    }

    std::cout << "\n\nSTRINGS: " << gd.stringtable.size() << '\n';
    if (!gd.stringtable.empty()) {
        for (auto &s : gd.stringtable) {
            std::cout << "   " << s.first << ": ~" << s.second << "~\n";
        }
    }

    std::cout << "\nGLOBALS (" << gd.symbols.symbols.size() << "):\n";
    for (auto &s : gd.symbols.symbols) {
        std::cout << "   " << s.second->name << " (" << s.second->type << ") = " << s.second->value << '\n';
    }

    PrintAstWalker aw;
    std::cout << "\nFUNCTIONS: " << gd.functions.size() << '\n';
    for (FunctionDef *f : gd.functions) {
        f->accept(&aw);
    }

}

