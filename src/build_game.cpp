#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include "gbuilder.h"

static void writeByte(std::ostream &out, int word) {
    out.put((word      ) & 0xFF);
}

static void writeShort(std::ostream &out, int word) {
    out.put((word >>  8) & 0xFF);
    out.put((word      ) & 0xFF);
}

static void writeWord(std::ostream &out, int word) {
    out.put((word >> 24) & 0xFF);
    out.put((word >> 16) & 0xFF);
    out.put((word >>  8) & 0xFF);
    out.put((word      ) & 0xFF);
}

static int readWord(std::istream &in) {
    int result = 0;
    result |= in.get() << 24;
    result |= in.get() << 16;
    result |= in.get() << 8;
    result |= in.get();
    return result;
}

class GlulxGame : public AsmWalker {
public:
    virtual void visit(Value *stmt) {
    }
    virtual void visit(AsmStatement *stmt) {
        if (stmt->opcode > 0x3FFF) {
            writeWord(out, stmt->opcode + 0xC0000000);
        } else if (stmt->opcode > 0x7F) {
            writeShort(out, stmt->opcode + 0x8000);
        } else {
            writeByte(out, stmt->opcode);
        }

        int curMode = -1;
        for (AsmOperand *op : stmt->operands) {
            int mode = op->getMode();
            if (curMode < 0) {
                curMode = mode;
            } else {
//                curMode <<= 4;
                curMode |= (mode << 4);
                writeByte(out, curMode);
                curMode = -1;
            }
        }
        if (curMode >= 0) {
            writeByte(out, curMode);
        }

        for (int i = 0; i < stmt->operands.size(); ++i) {
            AsmOperand *op = stmt->operands[i];
            if (op->isStack) continue;
            int value = op->value->value;
            if (op->value->type == Value::Identifier) {
                value = labels[op->value->text];
            }
            if (stmt->isRelative && i == stmt->operands.size() - 1) {
                value = value - (stmt->pos + stmt->getSize()) + 2;
            }

            switch(op->getSize()) {
                case 1: writeByte(out, value);  break;
                case 2: writeShort(out, value); break;
                case 4: writeWord(out, value);  break;
            }
        }
    }
    virtual void visit(AsmData *data) {
        for (unsigned char c : data->data) {
            out.put(c);
        }
    }
    virtual void visit(LabelStmt *label) {
        // do nothing
    }

    GlulxGame(std::ostream &out, std::vector<AsmLine*> &lines)
    : lines(lines), out(out)
    { }

    int firstRam;
    int endOfRam;
    int endOfExtended;
    int stackSize;
    std::unordered_map<std::string, int> labels;
    std::vector<AsmLine*> &lines;
    std::ostream &out;

};



/*
 0 | Magic Number  | 47 6C 75 6C | (4 bytes)
 4 | Glulx Version | 00030102    | (4 bytes)
 8 | RAMSTART      | (4 bytes)
12 | EXTSTART      | (4 bytes)
16 | ENDMEM        | (4 bytes)
20 | Stack Size    | (4 bytes)
24 | Start Func    | (4 bytes)
28 | Decoding Tbl  | (4 bytes)
32 | Checksum      | (4 bytes)
*/
static void writeHeader(GlulxGame &glulx, std::ostream &out) {
    writeWord(out, 0x476C756C); // magic number
    writeWord(out, 0x00030102); // glulx version
    writeWord(out, glulx.firstRam); // ramstart
    writeWord(out, glulx.endOfRam); // extstart
    writeWord(out, glulx.endOfExtended); // endmem
    writeWord(out, glulx.stackSize); // stack size
    if (glulx.labels.count("main") > 0) {
         // start func
        writeWord(out, glulx.labels["main"]);
    } else {
        writeWord(out, 0x00000000);
    }
    writeWord(out, 0x00000000); // decoding table
    writeWord(out, 0x00000000); // checksum


    writeWord(out, 0x47424C44); // gbuilder id
    writeWord(out, 0x00010000); // gbuilder version

    // pad out header
    while (out.tellp() % 256) {
        out.put(0);
    }
}

void build_game(GameData &gamedata, std::vector<AsmLine*> lines, const ProjectFile *projectFile, bool dumpLabels) {
    std::fstream out(projectFile->outputFile, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    GlulxGame gameBuilder(out, lines);
    int lastpos = 256;

    for (AsmLine *line : lines) {
        line->pos = lastpos;
        LabelStmt *label = dynamic_cast<LabelStmt*>(line);
        if (label) {
            gameBuilder.labels[label->name] = label->pos;
        }
        lastpos += line->getSize();
    }
    while (lastpos % 256) {
        ++lastpos;
    }

    gameBuilder.stackSize = 2048;
    gameBuilder.firstRam = 256;
    gameBuilder.endOfRam = lastpos;
    gameBuilder.endOfExtended = lastpos;


    if (dumpLabels) {
        std::cout << std::hex << std::setfill('0');
        for (auto i : gameBuilder.labels) {
            std::cout << std::setw(8) << i.second << ": " << i.first << '\n';
        }
        std::cout << std::dec << std::setfill(' ');
    }

    writeHeader(gameBuilder, out);
    for (AsmLine *line : lines) {
        line->accept(&gameBuilder);
    }

    for (int i = out.tellp(); i < gameBuilder.endOfRam; ++i) {
        out.put(0);
    }

    out.seekg(0);
    unsigned checksum = 0, next = 0;
    do {
        checksum += next;
        next = readWord(out);
    } while (out.good());
    out.clear();
    out.seekp(32);
    writeWord(out, checksum);
}
