#include <iomanip>
#include <iostream>

#include "gbuilder.h"

class BuildGame : public AsmWalker {
public:
    virtual void visit(AsmStatement *stmt) {
        std::cout << "STMT size:" << stmt->getSize() << " @" << stmt->pos << "\n";
    }
    virtual void visit(AsmData *data) {
        std::cout << "DATA size:" << data->getSize() << " @" << data->pos << "\n";
    }
    virtual void visit(LabelStmt *label) {
        std::cout << "LABL size:" << label->getSize() << " @" << label->pos << "\n";
    }
};


void build_game(std::vector<AsmLine*> lines) {
    BuildGame gameBuilder;
    int lastpos = 256;

    for (AsmLine *line : lines) {
        line->pos = lastpos;
        lastpos += line->getSize();

        line->accept(&gameBuilder);
    }
}
