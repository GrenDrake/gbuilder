
#include "gbuilder.h"


void AsmOperandInteger::accept(AstWalker *walker) {
    walker->visit(this);
}

void AsmOperandStack::accept(AstWalker *walker) {
    walker->visit(this);
}

void AsmStatement::accept(AstWalker *walker) {
    walker->visit(this);
}

void CodeBlock::accept(AstWalker *walker) {
    walker->visit(this);
}

void FunctionDef::accept(AstWalker *walker) {
    walker->visit(this);
}

