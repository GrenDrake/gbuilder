#include <list>
#include <string>
#include <vector>

enum TokenType {
    Identifier,
    String,
    Integer,
    Float,
    Vocab,
    ReservedWord,

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
    : vInteger(0), vFloat(0.0) {
    }
    Token(const std::string &file, int line, int column, TokenType type) 
    : type(type), vInteger(0), vFloat(0.0), file(file), line(line), column(column) {
    }

    TokenType type;
    std::string vText;
    int vInteger;
    double vFloat;
    
    std::string file;
    int line, column;
private:
};

class ErrorLogger {
public:
    class Message {
    public:
        Message(const std::string &file, int line, int column, const std::string &message)
        : file(file), line(line), column(column), message(message) {
        }
        std::string format() const;
        const std::string file;
        const int line, column;
        const std::string message;
    };
    
    void add(const std::string &file, int line, int column, const std::string &message);
    std::list<Message>::iterator begin() {
        return errors.begin();
    }
    unsigned count() const {
        return errors.size();
    }
    std::list<Message>::iterator end() {
        return errors.end();
    }
    bool empty() const {
        return errors.empty();
    }
private:
    std::list<Message> errors;
};

class GameData {
public:
private:
};

class Lexer {
public:
    Lexer(ErrorLogger &errors, GameData &gamedata, const std::string &sourceFile, const std::string &source_text)
    : errors(errors), gamedata(gamedata), sourceFile(sourceFile), source(source_text) {
    }
    
    void doLex();
    const std::list<Token>& getTokens() const {
        return tokens;
    }
private:
    void doIdentifier();
    void doNumber();
    void doSimpleToken(TokenType type);
    void doSimpleToken2(TokenType type);
    void doString();
    void doVocab();

    bool isIdentifier(int c, bool isInitial = false) const;

    int here() const;
    int next();
    int peek() const;
    int prev() const;

    ErrorLogger &errors;
    GameData &gamedata;
    const std::string &sourceFile;
    const std::string &source;
    std::list<Token> tokens;
    int current;
    int cLine, cColumn;
};

class Parser {
public:
    Parser(ErrorLogger &errors, GameData &gamedata, const std::vector<Token> &tokens)
    : current(0), errors(errors), gamedata(gamedata), tokens(tokens) {
    }
    
    void doParse();
private:
    void doFunction();

    bool expect(TokenType type);
    bool expectAdv(TokenType type);
    bool expect(const std::string &text);
    bool matches(TokenType type);
    bool matches(const std::string &text);
    const Token* here();
    const Token* next();
    
    void addError(const std::string &text);

    int current;
    ErrorLogger &errors;
    GameData &gamedata;
    const std::vector<Token> &tokens;
};
