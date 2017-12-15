#include <string>
#include <map>
#include <vector>

class AsmOperand;
class AsmStatement;
class AsmData;
class CodeBlock;
class FunctionDef;
class ReturnDef;
class LabelStmt;
class Value;

class AstWalker {
public:
    virtual void visit(Value *stmt) = 0;
    virtual void visit(AsmStatement *stmt) = 0;
    virtual void visit(AsmData *stmt) = 0;
    virtual void visit(CodeBlock *stmt) = 0;
    virtual void visit(FunctionDef *stmt) = 0;
    virtual void visit(ReturnDef *stmt) = 0;
    virtual void visit(LabelStmt *stmt) = 0;
};

class AsmWalker {
public:
    virtual void visit(Value *stmt) = 0;
    virtual void visit(AsmStatement *stmt) = 0;
    virtual void visit(AsmData *stmt) = 0;
    virtual void visit(LabelStmt *stmt) = 0;
};

class StatementDef {
public:
    virtual ~StatementDef() {
    }
    virtual void accept(AstWalker *walker) = 0;
};

class AsmLine : public StatementDef {
public:
    virtual ~AsmLine() { };
    virtual void accept(AstWalker *walker) = 0;
    virtual void accept(AsmWalker *walker) = 0;

    virtual int getSize() const = 0;
    int pos;
};
class AsmOperand {
public:
    enum Type {
        Constant, Local, Address, Stack, Identifier
    };

    AsmOperand()
    : value(nullptr), isStack(false), isIndirect(false), mySize(-1)
    { }

    int getSize();
    int getMode();

    Value *value;
    bool isStack;
    bool isIndirect;
    int mySize;
};
class AsmData : public AsmLine {
public:
    virtual ~AsmData() { };
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    virtual void accept(AsmWalker *walker) {
        walker->visit(this);
    }

    void pushByte(unsigned word) {
        data.push_back(word & 0xFF);
    }
    void pushShort(unsigned word) {
        data.push_back( (word >>  8) & 0xFF );
        data.push_back( (word      ) & 0xFF );
    }
    void pushWord(unsigned word) {
        data.push_back( (word >> 24) & 0xFF );
        data.push_back( (word >> 16) & 0xFF );
        data.push_back( (word >>  8) & 0xFF );
        data.push_back( (word      ) & 0xFF );
    }

    virtual int getSize() const {
        return data.size();
    }

    std::vector<unsigned char> data;
};

class AsmStatement : public AsmLine {
public:
    virtual ~AsmStatement() {
        for (AsmOperand *op : operands) {
            delete op;
        }
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    virtual void accept(AsmWalker *walker) {
        walker->visit(this);
    }

    virtual int getSize() const;

    std::string opname;
    int opcode;
    bool isRelative;
    std::vector<AsmOperand*> operands;
};

class LabelStmt : public AsmLine {
public:
    LabelStmt(const std::string &name)
    : name(name) {
    }
    virtual ~LabelStmt() {
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
    virtual void accept(AsmWalker *walker) {
        walker->visit(this);
    }

    virtual int getSize() const {
        return 0;
    }

    std::string name;
};


class ReturnDef : public StatementDef {
public:
    virtual ~ReturnDef() {
    }
    virtual void accept(AstWalker *walker) {
        walker->visit(this);
    }
};

class Value {
public:
    enum Type {
        Constant, Identifier, Local
    };

    Value()
    : type(Constant), value(0)
    { }
    Value(int value)
    : type(Constant),  value(value)
    { }
    Value(const std::string &text)
    : type(Identifier),  text(text)
    { }

    void accept(AstWalker *walker) {
        walker->visit(this);
    }
    void accept(AsmWalker *walker) {
        walker->visit(this);
    }

    Type type;
    int value;
    std::string text;
};

class SymbolDef {
public:
    enum Type {
        Constant, Local, RAM, Label, Function, String
    };

    SymbolDef(const std::string &name, Type type)
    : name(name), type(type), value(0) {
    }
    std::string name;
    Type type;
    int value;
};

class SymbolTable {
public:
    SymbolTable()
    : parent(nullptr) {
    }
    ~SymbolTable() {
        for (auto i : symbols) {
            delete i.second;
        }
    }

    SymbolDef* get(const std::string &name);
    bool exists(const std::string &name) const;
    void add(SymbolDef *name, bool functionScope = false);

    SymbolTable *parent;
    std::map<std::string, SymbolDef*> symbols;
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
    FunctionDef()
    : localCount(0), origin("(unknown)", 0, 0)
    { }
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
    Origin origin;
};
