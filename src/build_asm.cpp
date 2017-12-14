#include <iostream>
#include <vector>

#include <utf8.h>

#include "gbuilder.h"


class BuildAsm : public AstWalker {
public:
    BuildAsm(GameData &gamedata)
    : gamedata(gamedata) { }

    virtual void visit(Value *stmt) {
    }
    virtual void visit(AsmStatement *stmt) {
        AsmStatement *stmtCopy = new AsmStatement(*stmt);
        stmts.push_back(stmtCopy);
    }
    virtual void visit(AsmData *stmt) {
        AsmData *stmtCopy = new AsmData(*stmt);
        stmts.push_back(stmtCopy);
    }
    virtual void visit(CodeBlock *stmt) {
        for (StatementDef *s : stmt->statements) {
            s->accept(this);
        }
    }
    virtual void visit(FunctionDef *stmt) {
        LabelStmt *funcLabel = new LabelStmt(stmt->name);
        stmts.push_back(funcLabel);
        AsmData *funcHeader = new AsmData();
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
        AsmStatement *retStmt = new AsmStatement();
        retStmt->opname = "return";
        retStmt->opcode = 0x31;
        AsmOperand *retCode = new AsmOperand();
        retCode->value = new Value(0);
        retStmt->operands.push_back(retCode);
        stmts.push_back(retStmt);
    }
    virtual void visit(LabelStmt *stmt) {
        LabelStmt *stmtCopy = new LabelStmt(*stmt);
        stmts.push_back(stmtCopy);
    }

    void buildStrings() {
        for (const auto &strdef : gamedata.stringtable) {
            LabelStmt *strLabel = new LabelStmt(strdef.first);
            stmts.push_back(strLabel);

            bool isUnicode = false;
            for (unsigned char c : strdef.second) {
                if (c > 127) {
                    isUnicode = true;
                }
            }

            AsmData *strData = new AsmData;
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

    std::vector<AsmLine*> stmts;
private:
    GameData &gamedata;
};



std::vector<AsmLine*> buildAsm(GameData &gd) {

    BuildAsm buildAsmWalker(gd);

    buildAsmWalker.buildStrings();

    for (FunctionDef *f : gd.functions) {
        f->accept(&buildAsmWalker);
    }

    return buildAsmWalker.stmts;
}

