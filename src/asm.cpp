
#include "gbuilder.h"

static AsmCode codes[] = {
    AsmCode("nop",           0x00,  0),
    AsmCode("quit",          0x120, 0),
    AsmCode("glk",           0x130, 3),
    AsmCode("getiosys",      0x148, 2),
    AsmCode("setiosys",      0x149, 2),
    AsmCode("gestalt",       0x100, 3),

    AsmCode("debugtrap",     0x101, 1),
    AsmCode("getmemsize",    0x102, 1),
    AsmCode("setmemsize",    0x103, 2),
    AsmCode("random",        0x110, 2),
    AsmCode("setrandom",     0x111, 1),
    AsmCode("verify",        0x121, 1),

    AsmCode("restart",       0x122, 0),
    AsmCode("save",          0x123, 2),
    AsmCode("restore",       0x124, 2),
    AsmCode("saveundo",      0x125, 1),
    AsmCode("restoreundo",   0x126, 1),
    AsmCode("protect",       0x127, 2),
    AsmCode("getstringtbl",  0x140, 1),
    AsmCode("setstringtbl",  0x141, 1),
    
    AsmCode("linearsearch",  0x150, 8),
    AsmCode("binarysearch",  0x151, 8),
    AsmCode("linkedsearch",  0x152, 7),
    AsmCode("mzero",         0x170, 2),
    AsmCode("mcopy",         0x171, 3),
    AsmCode("malloc",        0x178, 2),
    AsmCode("mfree",         0x179, 1),
    AsmCode("accelfunc",     0x180, 2),
    AsmCode("accelparam",    0x181, 2),

    // integer math
    AsmCode("add",           0x10,  3),
    AsmCode("sub",           0x11,  3),
    AsmCode("mul",           0x12,  3),
    AsmCode("div",           0x13,  3),
    AsmCode("mod",           0x14,  3),
    AsmCode("neg",           0x15,  2),

    // bitwise operations
    AsmCode("bitand",        0x18,  3),
    AsmCode("bitor",         0x19,  3),
    AsmCode("bitxor",        0x1A,  3),
    AsmCode("bitnot",        0x1B,  3),
    AsmCode("shiftl",        0x1C,  3),
    AsmCode("sshiftr",       0x1D,  3),
    AsmCode("ushiftr",       0x1E,  3),

    // floating conversions
    AsmCode("numtof",        0x190, 2),
    AsmCode("ftonumz",       0x191, 2),
    AsmCode("ftonumn",       0x192, 2),

    // floating point math
    AsmCode("ceil",          0x198, 2),
    AsmCode("floor",         0x199, 2),
    AsmCode("fadd",          0x1A0, 3),
    AsmCode("fsub",          0x1A1, 3),
    AsmCode("fmul",          0x1A2, 3),
    AsmCode("fdiv",          0x1A3, 3),
    AsmCode("fmod",          0x1A4, 4),
    AsmCode("sqrt",          0x1A8, 2),
    AsmCode("exp",           0x1A9, 2),
    AsmCode("log",           0x1AA, 2),
    AsmCode("pow",           0x1AB, 3),
    AsmCode("sin",           0x1B0, 2),
    AsmCode("cos",           0x1B1, 2),
    AsmCode("tan",           0x1B2, 2),
    AsmCode("asin",          0x1B3, 2),
    AsmCode("acos",          0x1B4, 2),
    AsmCode("atan",          0x1B5, 2),
    AsmCode("atan2",         0x1B6, 3),

    // floating point branching
    AsmCode("jfeq",          0x1C0, 4, true),
    AsmCode("jfne",          0x1C1, 4, true),
    AsmCode("jflt",          0x1C2, 3, true),
    AsmCode("jfle",          0x1C3, 3, true),
    AsmCode("jfgt",          0x1C4, 3, true),
    AsmCode("jfge",          0x1C5, 3, true),
    AsmCode("jisnan",        0x1C8, 2, true),
    AsmCode("jisinf",        0x1C9, 2, true),

    // jumps
    AsmCode("jump",          0x20,  1, true),
    AsmCode("jz",            0x22,  2, true),
    AsmCode("jnz",           0x23,  2, true),
    AsmCode("jeq",           0x24,  3, true),
    AsmCode("jne",           0x25,  3, true),
    AsmCode("jlt",           0x26,  3, true),
    AsmCode("jge",           0x27,  3, true),
    AsmCode("jgt",           0x28,  3, true),
    AsmCode("jle",           0x29,  3, true),
    AsmCode("jltu",          0x2A,  3, true),
    AsmCode("jgeu",          0x2B,  3, true),
    AsmCode("jgtu",          0x2C,  3, true),
    AsmCode("jleu",          0x2D,  3, true),
    AsmCode("jumpabs",       0x2D,  1),

    // function calls
    AsmCode("call",          0x30,  3),
    AsmCode("return",        0x31,  1),
    AsmCode("catch",         0x32,  2),
    AsmCode("throw",         0x33,  2),
    AsmCode("tailcall",      0x30,  2),
    AsmCode("callf",         0x160, 2),
    AsmCode("callfi",        0x161, 3),
    AsmCode("callfii",       0x162, 4),
    AsmCode("callfiii",      0x163, 5),

    // moving data
    AsmCode("copy",          0x40,  2),
    AsmCode("copys",         0x41,  2),
    AsmCode("copyb",         0x42,  2),
    AsmCode("sexs",          0x44,  2),
    AsmCode("sexb",          0x45,  2),
    AsmCode("aload",         0x48,  3),
    AsmCode("aloads",        0x49,  3),
    AsmCode("aloadb",        0x4A,  3),
    AsmCode("aloadbit",      0x4B,  3),
    AsmCode("astore",        0x4C,  3),
    AsmCode("astores",       0x4D,  3),
    AsmCode("astoreb",       0x4E,  3),
    AsmCode("astorebit",     0x4F,  3),

    // output operations
    AsmCode("streamchar",    0x70,  1),
    AsmCode("streamnum",     0x71,  1),
    AsmCode("streamstr",     0x72,  1),
    AsmCode("streamunichar", 0x73,  1),

    // stack operations
    AsmCode("stkcount",      0x50,  1),
    AsmCode("stkpeek",       0x51,  2),
    AsmCode("stkswap",       0x52,  0),
    AsmCode("stkroll",       0x53,  2),
    AsmCode("stkcopy",       0x54,  1),

    AsmCode(nullptr,         0,     0)
};

const AsmCode& opcodeByName(const std::string &name) {
    for (int i = 0; ; ++i) {
        if (codes[i].name == nullptr || name == codes[i].name) {
            return codes[i];
        }
    }
    return codes[0];
}
