# C-- Lexer Project

This project implements a lexer for the C-- programming language. The lexer is responsible for converting raw source code into a sequence of tokens that can be further processed by a compiler or interpreter.

## Overview

The C-- lexer supports the following token types:

- **Keywords**: `int`, `void`, `if`, `else`, `while`, `return`, `input`, `output`
- **Identifiers**: Variable names and function names
- **Literals**: Numeric values
- **Operators**: Arithmetic and comparison operators
- **Delimiters**: Braces, parentheses, semicolons, and commas

## Project Structure

```
c-minus-compiler
├── src
│   ├── lexer.cpp      # Implementation of the lexer and token classes
│   ├── lexer.h        # Header file defining token types and classes
│   └── main.cpp       # Entry point for the application
├── tests
│   └── lexer_tests.cpp # Unit tests for the lexer using Google Test
├── Makefile           # Build instructions for the project
└── README.md          # Project documentation
```

## C-- Grammar

The formal grammar for C-- is provided in [grammar.bnf](grammar.bnf) using BNF notation.  
Use this file as a reference for language structure and parser development.

## Building the Project

To build the project, navigate to the project directory and run:

```
make
```

This will compile the source files and create an executable named `c_minus_compiler`.

## Running Tests

To run the unit tests, use the following command:

```
make test
```

This will execute the tests defined in `tests/lexer_tests.cpp` using the Google Test framework.

## Cleaning Up

To remove the compiled object files and the executable, run:

```
make clean
```

## Usage

After building the project, you can run the lexer on a source file by providing the file name as an argument:

```
./c_minus_compiler <source-file>
```

This will tokenize the input source file and print the tokens along with their line and column positions.

## Example

Given a source file containing:

```
int x = 42;
```

The lexer will output:

```
Token: KEYWORD(int) at line 1, col 1
Token: IDENTIFIER(main) at line 1, col 5
Token: LPAREN(() at line 1, col 9
Token: KEYWORD(void) at line 1, col 10
Token: RPAREN()) at line 1, col 14
Token: LBRACE({) at line 1, col 16
Token: KEYWORD(int) at line 2, col 5
Token: IDENTIFIER(x) at line 2, col 9
Token: EQUAL(=) at line 2, col 11
Token: NUMBER(42) at line 2, col 13
Token: SEMICOLON(;) at line 2, col 15
Token: RBRACE(}) at line 3, col 1
``` 

This indicates the line and column of each token along with its type.
