#include <iostream>
#include <vector>

#include <utf8.h>

#include "gbuilder.h"

class BuildExpr : public ExpressionWalker {
public:
    BuildExpr(std::vector<std::shared_ptr<AsmLine> > &stmts, GameData &gamedata)
    : stmts(stmts), gamedata(gamedata)
    { }

    void visit(NameExpression *expr) {

    }

    void visit(LiteralExpression *expr) {
        std::cout << "XXX\n";
        std::shared_ptr<AsmStatement> opCopy(new AsmStatement());
        opCopy->opname = "copy";
        opCopy->opcode = 0x40;

        std::shared_ptr<AsmOperand> litValue(new AsmOperand());
        litValue->value = std::shared_ptr<Value>(new Value(expr->litValue));
        opCopy->operands.push_back(litValue);

        std::shared_ptr<AsmOperand> destPos(new AsmOperand());
        destPos->isStack = true;
        opCopy->operands.push_back(destPos);

        stmts.push_back(opCopy);
    }

    void visit(PrefixOpExpression *expr) {

    }

    std::vector<std::shared_ptr<AsmLine> > &stmts;
private:
    GameData &gamedata;
};

class BuildAsm : public AstWalker {
public:
    BuildAsm(GameData &gamedata)
    : gamedata(gamedata) { }

    virtual void visit(Value *stmt) {
    }
    virtual void visit(AsmStatement *stmt) {
        std::shared_ptr<AsmStatement> stmtCopy(new AsmStatement(*stmt));
        stmts.push_back(stmtCopy);
    }
    virtual void visit(AsmData *stmt) {
        std::shared_ptr<AsmData> stmtCopy(new AsmData(*stmt));
        stmts.push_back(stmtCopy);
    }
    virtual void visit(CodeBlock *stmt) {
        for (auto s : stmt->statements) {
            s->accept(this);
        }
    }
    virtual void visit(FunctionDef *stmt) {
        std::shared_ptr<LabelStmt> funcLabel(new LabelStmt(stmt->name));
        stmts.push_back(funcLabel);
        std::shared_ptr<AsmData> funcHeader(new AsmData());
        funcHeader->data.push_back(0xC1);
        int locals = stmt->localCount;
        while (locals >= 255) {
            funcHeader->data.push_back(4);
            funcHeader->data.push_back(255);
            locals -= 255;
        }
        if (locals) {
            funcHeader->data.push_back(4);
            funcHeader->data.push_back(locals);
        }

        funcHeader->data.push_back(0);
        funcHeader->data.push_back(0);
        stmts.push_back(funcHeader);
        if (stmt->code) {
            stmt->code->accept(this);
        }
    }
    virtual void visit(ReturnDef *stmt) {
        BuildExpr bExpr(stmts, gamedata);
        stmt->retValue->accept(&bExpr);

        std::shared_ptr<AsmStatement> retStmt(new AsmStatement());
        retStmt->opname = "return";
        retStmt->opcode = 0x31;
        std::shared_ptr<AsmOperand> retCode(new AsmOperand());
        // retCode->value = std::shared_ptr<Value>(new Value(0));
        retCode->isStack = true;
        retStmt->operands.push_back(retCode);
        stmts.push_back(retStmt);
    }
    virtual void visit(LabelStmt *stmt) {
        std::shared_ptr<LabelStmt> stmtCopy(new LabelStmt(*stmt));
        stmts.push_back(stmtCopy);
    }

    void buildStrings() {
        for (const auto &strdef : gamedata.stringtable) {
            std::shared_ptr<LabelStmt> strLabel(new LabelStmt(strdef.first));
            stmts.push_back(strLabel);

            bool isUnicode = false;
            for (unsigned char c : strdef.second) {
                if (c > 127) {
                    isUnicode = true;
                }
            }

            std::shared_ptr<AsmData> strData(new AsmData);
            if (isUnicode) {
                strData->data.push_back(0xE2);
                strData->data.push_back(0);
                strData->data.push_back(0);
                strData->data.push_back(0);
                std::string::const_iterator cur = strdef.second.cbegin();
                while (cur != strdef.second.end()) {
                    int cp = utf8::next(cur, strdef.second.cend());
                    strData->pushWord(cp);
                }
                strData->pushWord(0);
            } else {
                strData->data.push_back(0xE0);
                for (char c : strdef.second) {
                    strData->data.push_back(c);
                }
                strData->data.push_back(0);
            }

            stmts.push_back(strData);
        }
    }

    std::vector<std::shared_ptr<AsmLine> > stmts;
private:
    GameData &gamedata;
};



std::vector<std::shared_ptr<AsmLine> > buildAsm(GameData &gd) {

    BuildAsm buildAsmWalker(gd);

    buildAsmWalker.buildStrings();

    for (auto f : gd.functions) {
        f->accept(&buildAsmWalker);
    }

    return buildAsmWalker.stmts;
}

