# GBuilder

GBuilder is an alternative compiler used for creating [Glulx](http://www.eblong.com/zarf/glulx/) game files. In its current state it is extremely minimalistic and little more than a awkward assembler, but it will gradually be expanded to include a wide range of syntaxes.

## Usage

```
./gbuilder <project-file>
```

The project file contains two lines. The first is a whitespace separated list of all the files to compile and the second is the name of the output file.


## Language Grammar

```
IDENTIFIER -> [a-zA-Z_][a-zA-Z0-9_]*
INTEGER -> [0-9]* 
         | 0[xX][0-9a-fA-F] 
         | '.'
FLOAT -> [0-9]+\.[0-9]*
NUMBER -> INTEGER 
        | FLOAT

program -> (top-level)*
top-level -> function-def | constant-def

constant-def -> "constant" IDENTIFIER "=" INTEGER ";"

function-def -> "function" [IDENTIFIER] "(" (IDENTIFIER ("," IDENTIFIER)*)? ")" code-block
code-block -> "{" statement* "}"
statement -> asm-block 
           | return-statement

return-statement -> "return" ";"

asm-block -> "asm" "{" asm-statement* "}" 
           | "asm" asm-statement
asm-statement -> IDENTIFIER asm-operand* ";"
asm-operand -> NUMBER | IDENTIFIER
```
