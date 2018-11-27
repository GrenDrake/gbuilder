CXXFLAGS=-Wall -g -pedantic -std=c++11 -Isrc/utf8/source
OBJS=src/main.o src/lexer.o src/errorlogger.o src/parser.o src/dump_ast.o \
	 src/asm.o src/pass_1.o src/build_asm.o src/dump_asm.o src/build_game.o \
	 src/project.o src/dump_tokens.o src/symbols.o
TARGET=./gbuilder

$(TARGET): $(OBJS)
	g++ $(OBJS) -o $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: clean