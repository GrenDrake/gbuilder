
#include "gbuilder.h"

static AsmCode codes[] = {
    AsmCode("nop", 0x00, 0),
    AsmCode("quit", 0x120, 0),
    AsmCode("glk", 0x130, 3),
    AsmCode("getiosys", 0x148, 2),
    AsmCode("setiosys", 0x149, 2),
    AsmCode("gestalt", 0x100, 3),

    // integer math
    AsmCode("add", 0x10, 3),
    AsmCode("sub", 0x11, 3),
    AsmCode("mul", 0x12, 3),
    AsmCode("div", 0x13, 3),
    AsmCode("mod", 0x14, 3),
    AsmCode("neg", 0x15, 2),

    // jumps
    AsmCode("jump", 0x20, 1, true),
    AsmCode("jz", 0x22, 2, true),
    AsmCode("jnz", 0x23, 2, true),
    AsmCode("jeq", 0x24, 3, true),
    AsmCode("jne", 0x25, 3, true),
    AsmCode("jlt", 0x26, 3, true),
    AsmCode("jge", 0x27, 3, true),
    AsmCode("jgt", 0x28, 3, true),
    AsmCode("jle", 0x29, 3, true),
    AsmCode("jltu", 0x2A, 3, true),
    AsmCode("jgeu", 0x2B, 3, true),
    AsmCode("jgtu", 0x2C, 3, true),
    AsmCode("jleu", 0x2D, 3, true),
    AsmCode("jumpabs", 0x2D, 1),

    // function calls
    AsmCode("call", 0x30, 3),
    AsmCode("return", 0x31, 1),
    AsmCode("catch", 0x32, 2),
    AsmCode("throw", 0x33, 2),
    AsmCode("tailcall", 0x30, 2),
    AsmCode("callf", 0x160, 2),
    AsmCode("callfi", 0x161, 3),
    AsmCode("callfii", 0x162, 4),
    AsmCode("callfiii", 0x163, 5),

    // moving data
    AsmCode("copy", 0x40, 2),
    AsmCode("copys", 0x41, 2),
    AsmCode("copyb", 0x42, 2),
    AsmCode("sexs", 0x44, 2),
    AsmCode("sexb", 0x45, 2),
    AsmCode("aload", 0x48, 3),
    AsmCode("aloads", 0x49, 3),
    AsmCode("aloadb", 0x4A, 3),
    AsmCode("aloadbit", 0x4B, 3),
    AsmCode("astore", 0x4C, 3),
    AsmCode("astores", 0x4D, 3),
    AsmCode("astoreb", 0x4E, 3),
    AsmCode("astorebit", 0x4F, 3),

    // output operations
    AsmCode("streamchar", 0x70, 1),
    AsmCode("streamnum", 0x71, 1),
    AsmCode("streamstr", 0x72, 1),
    AsmCode("streamunichar", 0x73, 1),


    AsmCode(nullptr, 0, 0),
};

const AsmCode& opcodeByName(const std::string &name) {
    for (int i = 0; ; ++i) {
        if (codes[i].name == nullptr || name == codes[i].name) {
            return codes[i];
        }
    }
    return codes[0];
}


/*
        // floating point math and conversions
        aList.put("numtof",        new Mnemonic("numtof",        0x190,  2));
        aList.put("ftonumz",       new Mnemonic("ftonumz",       0x191,  2));
        aList.put("ftonumn",       new Mnemonic("ftonumn",       0x192,  2));
        aList.put("ceil",          new Mnemonic("ceil",          0x198,  2));
        aList.put("floor",         new Mnemonic("floor",         0x199,  2));
        aList.put("fadd",          new Mnemonic("fadd",          0x1A0,  3));
        aList.put("fsub",          new Mnemonic("fsub",          0x1A1,  3));
        aList.put("fmul",          new Mnemonic("fmul",          0x1A2,  3));
        aList.put("fdiv",          new Mnemonic("fdiv",          0x1A3,  3));
        aList.put("fmod",          new Mnemonic("fmod",          0x1A4,  4));
        aList.put("sqrt",          new Mnemonic("sqrt",          0x1A8,  2));
        aList.put("exp",           new Mnemonic("exp",           0x1A9,  2));
        aList.put("log",           new Mnemonic("log",           0x1AA,  2));
        aList.put("pow",           new Mnemonic("pow",           0x1AB,  3));
        aList.put("sin",           new Mnemonic("sin",           0x1B0,  2));
        aList.put("cos",           new Mnemonic("cos",           0x1B1,  2));
        aList.put("tan",           new Mnemonic("tan",           0x1B2,  2));
        aList.put("asin",          new Mnemonic("asin",          0x1B3,  2));
        aList.put("acos",          new Mnemonic("acos",          0x1B4,  2));
        aList.put("atan",          new Mnemonic("atan",          0x1B5,  2));
        aList.put("atan2",         new Mnemonic("atan2",         0x1B6,  3));
        // bitwise operations
        aList.put("bitand",        new Mnemonic("bitand",        0x18,  3));
        aList.put("bitor",         new Mnemonic("bitor",         0x19,  3));
        aList.put("bitxor",        new Mnemonic("bitxor",        0x1A,  3));
        aList.put("bitnot",        new Mnemonic("bitnot",        0x1B,  3));
        aList.put("shiftl",        new Mnemonic("shiftl",        0x1C,  3));
        aList.put("sshiftr",       new Mnemonic("sshiftr",       0x1D,  3));
        aList.put("ushiftr",       new Mnemonic("ushiftr",       0x1E,  3));
        // jumps (most take relative addresses)
        aList.put("jfeq",          new Mnemonic("jfeq",          0x1C0, 4, true));
        aList.put("jfne",          new Mnemonic("jfne",          0x1C1, 4, true));
        aList.put("jflt",          new Mnemonic("jflt",          0x1C2, 3, true));
        aList.put("jfle",          new Mnemonic("jfle",          0x1C3, 3, true));
        aList.put("jfgt",          new Mnemonic("jfgt",          0x1C4, 3, true));
        aList.put("jfge",          new Mnemonic("jfge",          0x1C5, 3, true));
        aList.put("jisnan",        new Mnemonic("jisnan",        0x1C8, 2, true));
        aList.put("jisinf",        new Mnemonic("jisinf",        0x1C9, 2, true));
        // stack operations
        aList.put("stkcount",      new Mnemonic("stkcount",      0x50,  1));
        aList.put("stkpeek",       new Mnemonic("stkpeek",       0x51,  2));
        aList.put("stkswap",       new Mnemonic("stkswap",       0x52,  0));
        aList.put("stkroll",       new Mnemonic("stkroll",       0x53,  2));
        aList.put("stkcopy",       new Mnemonic("stkcopy",       0x54,  1));
        aList.put("debugtrap",     new Mnemonic("debugtrap",     0x101, 1));
        aList.put("getmemsize",    new Mnemonic("getmemsize",    0x102, 1));
        aList.put("setmemsize",    new Mnemonic("setmemsize",    0x103, 2));
        aList.put("random",        new Mnemonic("random",        0x110, 2));
        aList.put("setrandom",     new Mnemonic("setrandom",     0x111, 1));
        aList.put("verify",        new Mnemonic("verify",        0x121, 1));

        aList.put("restart",       new Mnemonic("restart",       0x122, 0));
        aList.put("save",          new Mnemonic("save",          0x123, 2));
        aList.put("restore",       new Mnemonic("restore",       0x124, 2));
        aList.put("saveundo",      new Mnemonic("saveundo",      0x125, 1));
        aList.put("restoreundo",   new Mnemonic("restoreundo",   0x126, 1));
        aList.put("protect",       new Mnemonic("protect",       0x127, 2));
        aList.put("getstringtbl",  new Mnemonic("getstringtbl",  0x140, 1));
        aList.put("setstringtbl",  new Mnemonic("setstringtbl",  0x141, 1));
        
        aList.put("linearsearch",  new Mnemonic("linearsearch",  0x150, 8));
        aList.put("binarysearch",  new Mnemonic("binarysearch",  0x151, 8));
        aList.put("linkedsearch",  new Mnemonic("linkedsearch",  0x152, 7));
        aList.put("mzero",         new Mnemonic("mzero",         0x170, 2));
        aList.put("mcopy",         new Mnemonic("mcopy",         0x171, 3));
        aList.put("malloc",        new Mnemonic("malloc",        0x178, 2));
        aList.put("mfree",         new Mnemonic("mfree",         0x179, 1));
        aList.put("accelfunc",     new Mnemonic("accelfunc",     0x180, 2));
        aList.put("accelparam",    new Mnemonic("accelparam",    0x181, 2));
*/