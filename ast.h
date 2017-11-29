#include <string>
#include <vector>

class AstWalker;

class StatementDef {
public:
    virtual ~StatementDef() {
    }
    virtual void accept(AstWalker *walker) = 0;
};

class AsmOperand {
public:
    virtual ~AsmOperand() {
    }
    virtual void accept(AstWalker *walker) = 0;
};

class AsmOperandInteger : public AsmOperand {
public:
    virtual void accept(AstWalker *walker);
    int value;
};

class AsmOperandStack : public AsmOperand {
public:
    virtual void accept(AstWalker *walker);
};

class AsmStatement : public StatementDef {
public:
    virtual ~AsmStatement() {
        for (AsmOperand *op : operands) {
            delete op;
        }
    }
    virtual void accept(AstWalker *walker);
    std::string opcode;
    std::vector<AsmOperand*> operands;
};

class LocalDef {
public:
    LocalDef(const std::string &name)
    : name(name) {
    }
    std::string name;
    int number;
};

class CodeBlock : public StatementDef {
public:
    ~CodeBlock() {
        for (StatementDef *stmt : statements) {
            delete stmt;
        }
    }
    virtual void accept(AstWalker *walker);
    std::vector<LocalDef> locals;
    std::vector<StatementDef*> statements;
};

class FunctionDef {
public:
    ~FunctionDef() {
        delete code;
    }
    void accept(AstWalker *walker);
    std::string name;
    int localCount;
    std::vector<LocalDef> locals;
    CodeBlock *code;
};

class AstWalker {
public:
    virtual void visit(AsmOperandInteger *stmt) = 0;
    virtual void visit(AsmOperandStack *stmt) = 0;
    virtual void visit(AsmStatement *stmt) = 0;
    virtual void visit(CodeBlock *stmt) = 0;
    virtual void visit(FunctionDef *stmt) = 0;
};