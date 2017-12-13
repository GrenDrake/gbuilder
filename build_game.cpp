#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

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

static void writeWord(std::ostream &out, int word) {
    out.put((word >> 24) & 0xFF);
    out.put((word >> 16) & 0xFF);
    out.put((word >>  8) & 0xFF);
    out.put((word      ) & 0xFF);
}

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
static void writeHeader(GameData &gamedata, std::ostream &out) {
    writeWord(out, 0x476C756C); // magic number
    writeWord(out, 0x00030102); // glulx version
    writeWord(out, 0x00000100); // ramstart
    writeWord(out, 0x00000000); // extstart
    writeWord(out, 0x00000000); // endmem
    writeWord(out, 0x00000800); // stack size
    writeWord(out, 0x00000000); // start func
    writeWord(out, 0x00000000); // decoding table
    writeWord(out, 0x00000000); // checksum
    
    writeWord(out, 0x47424C44); // gbuilder id

    while (out.tellp() % 256) {
        out.put(0);
    }
}

void build_game(GameData &gamedata, std::vector<AsmLine*> lines) {
    int lastpos = 256;
    std::unordered_map<std::string, int> labels;

    for (AsmLine *line : lines) {
        line->pos = lastpos;
        LabelStmt *label = dynamic_cast<LabelStmt*>(line);
        if (label) {
            labels[label->name] = label->pos;
        }
        lastpos += line->getSize();
    }


    std::cout << "lastpos  " << lastpos << "\n";
    for (auto i : labels) {
        std::cout << i.first << " -> " << i.second << '\n';
    }

    BuildGame gameBuilder;
    std::ofstream out("output.ulx");
    writeHeader(gamedata, out);
    for (AsmLine *line : lines) {
        line->accept(&gameBuilder);
    }
}
