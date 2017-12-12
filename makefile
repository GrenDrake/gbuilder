CXXFLAGS=-Wall -g -pedantic -std=c++11 -Iutf8/source
OBJS=main.o lexer.o errorlogger.o parser.o dump_ast.o asm.o pass_1.o build_asm.o dump_asm.o build_game.o
TARGET=~/gbuilder

$(TARGET): $(OBJS)
	g++ $(OBJS) -o $(TARGET)

clean:
	$(RM) *.o $(TARGET)

.PHONY: clean