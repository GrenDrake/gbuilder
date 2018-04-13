#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class Origin {
public:
    Origin(const std::string &file, int line, int column)
    : file(file), line(line), column(column)
    { }

    std::string file;
    int line, column;
};

#include "ast.h"

enum TokenType {
    Identifier,
    String,
    Integer,
    Float,
    Vocab,
    ReservedWord,
    EndOfFile,

    OpPlus,
    OpMinus,
    OpDivide,
    OpMultiply,
    OpPower,
    OpPlusEquals,
    OpMinusEquals,
    OpDivideEquals,
    OpMultiplyEquals,
    OpNot,
    OpModulus,
    OpAssign,
    OpProperty,
    OpIncrement,
    OpDecrement,

    LessThan,
    LessThanOrEquals,
    GreaterThan,
    GreaterThanOrEquals,
    NotEquals,
    Equals,
    LogicalAnd,
    LogicalOr,

    OpenBrace,
    CloseBrace,
    OpenParan,
    CloseParan,
    Semicolon,
    Comma,
    Question
};
const char* tokenTypeName(TokenType type);

class Token {
public:
    Token()
    : vInteger(0), vFloat(0.0), origin("(no-file)", 0, 0) {
    }
    Token(const std::string &file, int line, int column, TokenType type)
    : type(type), vInteger(0), vFloat(0.0), origin(file,line,column) {
    }

    TokenType type;
    std::string vText;
    int vInteger;
    double vFloat;

    Origin origin;
private:
};

class ErrorLogger {
public:
    enum Type {
        Error, Warning, Notice
    };

    class Message {
    public:
        Message(Type type, const Origin &origin, const std::string &message)
        : type(type), origin(origin), message(message) {
        }
        std::string format() const;
        const Type type;
        const Origin origin;
        const std::string message;
    };

    ErrorLogger()
    : theErrorCount(0), theWarningCount(0) {
    }

    void add(Type type, const Origin &origin, const std::string &message);
    std::list<Message>::iterator begin() {
        return errors.begin();
    }
    unsigned count() const {
        return errors.size();
    }
    unsigned errorCount() const {
        return theErrorCount;
    }
    unsigned warningCount() const {
        return theWarningCount;
    }
    std::list<Message>::iterator end() {
        return errors.end();
    }
    bool empty() const {
        return errors.empty();
    }
private:
    std::list<Message> errors;
    int theErrorCount, theWarningCount;
};

class GameData {
public:
    GameData()
    : nextString(0) {
    }
    ~GameData() {
    }
    std::string addString(const std::string &text);

    std::list<std::shared_ptr<FunctionDef> > functions;
    std::set<std::string> vocabRaw;
    std::map<std::string, std::string> stringtable;
    SymbolTable symbols;

private:
    int nextString;
};

class Lexer {
public:
    Lexer(ErrorLogger &errors)
    : errors(errors) {
    }

    void doLex(const std::string &sourceFile, const std::string &source_text);
    const std::vector<Token>& getTokens() const {
        return tokens;
    }
private:
    void doCharLiteral();
    void doIdentifier();
    void doHexNumber();
    void doNumber();
    void doSimpleToken(TokenType type);
    void doSimpleToken2(TokenType type);
    void doString();
    void doVocab();

    bool isIdentifier(int c, bool isInitial = false) const;
    void unescape(const Origin &origin, std::string &text);

    int here() const;
    int next();
    int peek() const;
    int prev() const;

    ErrorLogger &errors;
    std::set<std::string> vocab;
    std::string sourceFile;
    std::string source;
    std::vector<Token> tokens;
    int current;
    int cLine, cColumn;
};

class ParserError : public std::runtime_error {
public:
    ParserError()
    : std::runtime_error("Parser Error")
    { }
};
class Parser {
public:
    Parser(ErrorLogger &errors, GameData &gamedata, const std::vector<Token> &tokens)
    : current(0), errors(errors), gamedata(gamedata), tokens(tokens) {
    }

    void doParse();
private:
    void doConstant();
    std::shared_ptr<FunctionDef> doFunction();

    std::shared_ptr<StatementDef> doStatement();
    std::shared_ptr<CodeBlock> doCodeBlock();
    bool doLocalsStmt();
    std::shared_ptr<LabelStmt> doLabel();
    std::shared_ptr<ReturnDef> doReturn();
    std::shared_ptr<ExpressionDef> doExpression();
    std::shared_ptr<ExpressionStmt> doExpressionStmt();
    std::shared_ptr<Value> doValue();

    std::shared_ptr<StatementDef> doAsmBlock();
    std::shared_ptr<StatementDef> doAsmStatement();
    std::shared_ptr<AsmOperand> doAsmOperand();

    void synchronize();
    void expect(TokenType type);
    void expectAdv(TokenType type);
    void expect(const std::string &text);
    bool matches(TokenType type);
    bool matches(const std::string &text);
    bool symbolExists(const SymbolTable &table, const std::string &name);
    const Token* here();
    const Token* next();

    int current;
    ErrorLogger &errors;
    GameData &gamedata;
    const std::vector<Token> &tokens;
    SymbolTable *curTable;
};

class AsmCode {
public:
    AsmCode(const char *name, int opcode, int operands, bool relative = false)
    : name(name), opcode(opcode), operands(operands), relative(relative) {
    }

    const char *name;
    int opcode;
    int operands;
    bool relative;
};

const AsmCode& opcodeByName(const std::string &name);

#include "project.h"
