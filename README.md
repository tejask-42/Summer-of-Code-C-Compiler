# C-- Compiler Project

This repository contains a modular C-- compiler, supporting all fundamental compilation stages:  
**Lexical analysis, parsing, semantic analysis, IR generation, code optimization, and code generation (x86-64 assembly output).**  

---

## Inspiration

This project was inspired by [SoC-Now-You-C-Me](https://github.com/adityasanapala/SoC-Now-You-C-Me), the official repository for "Now You C Me," a Summer of Code 2025 project by my mentor Aditya Sanapala.  
Check out their work for foundational compiler-building learning material and weekly task progression!

---

## Features

- **Lexical Analysis:**  
  Tokenizes source into keywords, identifiers, literals, operators, and delimiters.
- **Parsing:**  
  Builds an Abstract Syntax Tree (AST) from C-- source, supporting functions, declarations, expressions, and control flow.
- **Semantic Analysis:**  
  Checks for undeclared/void variables, incorrect uses, and type errors.
- **Intermediate Representation (IR) & Optimization:**  
  Generates IR for further passes, applies optimizations (constant folding, dead code, etc.).
- **Assembly Code Generation:**  
  Generates x86-64 assembly output.
- **Extensive Modular Unit Tests:**  
  Comprehensive GoogleTest-based tests for all modules, each with their own target for focused, green builds.
---

## Project Layout

```

c-minus-compiler/
├── src/
│   ├── lexer.cpp, lexer.h
│   ├── parser.cpp, parser.h
│   ├── ast.h              \# AST node definitions and Visitor
│   ├── semantic-analyzer.cpp, .h
│   ├── ir-generator.cpp, .h
│   ├── ir-optimizer.cpp, .h
│   ├── assembly-generator.cpp, .h
│   ├── advanced-optimizer.cpp, .h
│   └── cmmc.cpp           \# Command line driver
├── tests/
│   ├── lexer_tests.cpp
│   ├── parser_tests.cpp
│   ├── semantic_tests.cpp
│   ├── ir_tests.cpp
│   ├── assembly_tests.cpp
│   ├── ast_tests.cpp
│   └── test_main.cpp       \# (optional, not run by default)
├── grammar.bnf            \# Full C-- grammar specification
├── Makefile
└── README.md

```

---

## C-- Grammar

See [grammar.bnf](grammar.bnf) for the formal grammar in BNF notation used by the parser and referenced when extending language features.

---

## Building the Compiler and Tests

Compile everything with:
```

make all

```

**Build or run individual tests (examples):**
```

make lexer-tests
make parser-tests
make semantic-tests
make ir-tests
make assembly-tests
make ast-tests

```

---

## Running the Compiler

Compile your C-- source file (e.g., `hello.cmm`):
```

./bin/cmmc hello.cmm

```
You will see passes for lexing, parsing, semantic analysis, etc., and output (stdout or files) per chosen options.

---

## Running the Tests

**Run all GoogleTest module tests (default targets):**
```

make test

```
This will sequentially run all individual test binaries, and all output will show passing (green) if only passing tests are enabled.

Run a specific test target:
```

make parser-tests
make semantic-tests

# etc.

```

---

## Cleaning Up

```

make clean

```

Removes all compiled code, objects, binaries, and test artifacts.

---

## Extending/Contributing

- New tokens? Extend `lexer.h/cpp`.
- New statements or nodes? Update `ast.h`, parser, and semantic analysis.
- Want to target another architecture or add backend features? Extend codegen modules.
- All new features must be accompanied by new targeted unit tests in the `tests/` folder.

---

## Questions or issues?

Open a GitHub issue or pull request. Contributions are welcome!

---
