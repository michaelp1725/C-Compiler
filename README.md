# Hi, I built this for CS136, a course taken at the University of Waterloo. Enjoy!!

Overview
This project implements a custom compiler in C for a simple programming language. It features a complete compilation pipeline, including lexical analysis, parsing, and basic code generation, demonstrating how source code is processed into executable instructions.

Features
Tokenizer / lexical analyzer that breaks source code into tokens

Recursive-descent parser that constructs an abstract syntax tree (AST)

Basic semantic checks and error handling

Code generation producing intermediate or target code

How It Works
The compiler processes source code in multiple stages:

Lexing — Tokenizes the input source into meaningful symbols

Parsing — Analyzes tokens and builds an AST according to language grammar

Semantic Analysis — (Optional) Checks for correctness like type validation

Code Generation — Translates the AST into intermediate or executable code

Getting Started
Prerequisites
GCC or another standard C compiler

make utility (Optional)

