# GBuilder

GBuilder is an alternative compiler used for creating [Glulx](http://www.eblong.com/zarf/glulx/) game files. In its current state it is extremely minimalistic and little more than a awkward assembler, but it will gradually be expanded to include a wide range of syntaxes.

## Usage

```
./gbuilder <project-file>
```

Each line of the project file begin with the name of an option. This is followed by a whitespace delimited list of values for that option. The currently available options are:

- **files** A list of source files to include in the compilation
- **output** The name of the glulx game file to create (defaults to "output.ulx")

At a minimum, the project file must have at least one files directive with at least one source file listed. An example project file is shown below:

```
files source1.gc source2.gc
output mygame.ulx
```

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

return-statement -> "return" expression-def ";"

expression-def -> IDENTIFIER | INTEGER

asm-block -> "asm" "{" asm-statement* "}"
           | "asm" asm-statement
asm-statement -> IDENTIFIER asm-operand* ";"
asm-operand -> NUMBER | IDENTIFIER
```
