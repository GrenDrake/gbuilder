#include <string>
#include <vector>

class AsmOperand;
class AsmStatement;
class CodeBlock;
class FunctionDef;
class ReturnDef;
class LabelStmt;

class AstWalker {
public:
    virtual void visit(AsmStatement *stmt) = 0;
    virtual void visit(CodeBlock *stmt) = 0;
    virtual void visit(FunctionDef *stmt) = 0;
    virtual void visit(ReturnDef *stmt) = 0;
    virtual void visit(LabelStmt *stmt) = 0;
};

class SymbolDef {
public:
    enum Type {
        Constant, Local, RAM
    };

    SymbolDef(const std::string &name, Type type)
    : name(name), type(type), value(0) {
    }
    std::string name;
    Type type;
    int value;
};

class StatementDef {
public:
    virtual ~StatementDef() {
    }
    virtual void accept(AstWalker *walker) = 0;
};

class AsmOperand {
public:
    enum Type {
        Constant, Local, Address, Stack, Identifier
    };

    Type type;
    int value;
    std::string text;
};

class AsmStatement : public StatementDef {
public:
    virtual ~AsmStatement() {
        for (AsmOperand *op : operands) {
            delete op;
        }
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    std::string opname;
    int opcode;
    bool isRelative;
    std::vector<AsmOperand*> operands;
};

class LabelStmt : public AsmStatement{
public:
    LabelStmt(const std::string &name)
    : name(name) {
    }
    virtual ~LabelStmt() {
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }

    std::string name;
};


class ReturnDef : public StatementDef{
public:
    virtual ~ReturnDef() {
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
};

class SymbolTable {
public:
    SymbolDef* get(const std::string &name);

    SymbolTable *parent;
    std::vector<SymbolDef> symbols;
};

class CodeBlock : public StatementDef {
public:
    ~CodeBlock() {
        for (StatementDef *stmt : statements) {
            delete stmt;
        }
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    SymbolTable locals;
    std::vector<StatementDef*> statements;
};

class FunctionDef {
public:
    ~FunctionDef() {
        delete code;
    }
    void accept(AstWalker *walker) {
        walker->visit(this);
    }
    SymbolTable args;
    std::string name;
    int localCount;
    CodeBlock *code;
};
