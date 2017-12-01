#include <string>
#include <vector>

class AsmOperandInteger;
class AsmOperandIdentifier;
class AsmOperandStack;
class AsmStatement;
class CodeBlock;
class FunctionDef;
class ReturnDef;
class LabelStmt;

class AstWalker {
public:
    virtual void visit(AsmOperandInteger *stmt) = 0;
    virtual void visit(AsmOperandIdentifier *stmt) = 0;
    virtual void visit(AsmOperandStack *stmt) = 0;
    virtual void visit(AsmStatement *stmt) = 0;
    virtual void visit(CodeBlock *stmt) = 0;
    virtual void visit(FunctionDef *stmt) = 0;
    virtual void visit(ReturnDef *stmt) = 0;
    virtual void visit(LabelStmt *stmt) = 0;
};

class SymbolDef {
public:
    SymbolDef(const std::string &name)
    : name(name) {
    }
    std::string name;
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
    virtual ~AsmOperand() {
    }
    virtual void accept(AstWalker *walker) = 0;
};

class AsmOperandInteger : public AsmOperand {
public:
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    int value;
};

class AsmOperandIdentifier : public AsmOperand {
public:
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    std::string value;
};

class AsmOperandStack : public AsmOperand {
public:
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
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

class ReturnDef : public StatementDef{
public:
    virtual ~ReturnDef() {
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
};

class LabelStmt : public StatementDef{
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


class SymbolTable {
public:
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
