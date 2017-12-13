#include <iomanip>
#include <iostream>

#include "gbuilder.h"

class AsmPrinter : public AsmWalker {
public:
    virtual void visit(AsmStatement *stmt) {
        std::cout << "asm " << stmt->opname << " (" << std::hex << stmt->opcode << std::dec << ')';
        for (AsmOperand *op : stmt->operands) {
            switch(op->type) {
                case AsmOperand::Constant:
                    std::cout << " c:" << op->value;
                    break;
                case AsmOperand::Local:
                    std::cout << " l:" << op->value;
                    break;
                case AsmOperand::Identifier:
                    std::cout << " i:~" << op->text << '~';
                    break;
                default:
                    break;
            }
        }
        std::cout << '\n';
    }
    virtual void visit(AsmData *data) {
        std::cout << std::uppercase << std::hex << "DATA";
        for (unsigned char &ch : data->data) {
            std::cout << " 0x" << (int)ch;
        }
        std::cout << std::dec << '\n';
    }
    virtual void visit(LabelStmt *label) {
        std::cout << "\nLABEL " << label->name << "\n";
    }
};


void dump_asm(std::vector<AsmLine*> lines) {
    AsmPrinter asmPrinter;

    std::cout << "** Assembly Dump **\n";
    for (AsmLine *line : lines) {
        line->accept(&asmPrinter);
    }
}
