
//============================================================================//
// CAnalyzer
//============================================================================//

Functionality

 - take C program as input
 - generate in-memory CFG
 - traverse CFG for a function by iterating over basic blocks
 - traverse a basic block by iterating over statements
 - implement visitor methods for different AST nodes to get info on every
    integer literal, floating literal, variable, declarations, expressions
    (binary and unary), operands for every expression, assignments,
    if-else, while, do-while, for, null statement, conditional operator,
    return statement inside a basic block
 - create required data structures for analysis
 - do abstract analysis for:
   . single basic block (statements and expressions)
   . conditions (if-else)
   . loops (while, do-while, for)
   . nested loops (nested while)


//============================================================================//
// Directories
//============================================================================//

source - CAnalyzer source files

testdata - examples, C program as input testdata

scripts - scripts being used

docs - documentation config file


//============================================================================//
// Code files
//============================================================================//

source/
CFGGenerator.cpp        : Driver program to get input C program, set compiler
                          instance options and call parseAST()

MyASTConsumer.h         : AST reader class MyASTConsumer inherited from class
                          clang::ASTConsumer

MyASTConsumer.cpp       : MyASTConsumer definitions

MyASTVisitor.h          : AST node visitor class MyASTVisitor inherited from
                          clang::RecursiveASTVisitor

MyASTVisitor.cpp        : MyASTVisitor definitions

MyProcessStmt.h         : MyProcessStmt inherited from RecursiveASTVisitor to
                          handle different type of statements (Stmt)

MyProcessStmt.cpp	: MyProcessStmt definitions

Apron.h                 : Apron related declarations

Apron.cpp               : Apron related definitions

Makefile                : make file to build the code, do not alter the order
                          of clang libraries and option for target CFG

testdata/
*.c                     : C programs for testing purpose

scripts/
setHeaderSearchPath.sh  : script to set HEADER_SEARCH_PATH environment variable

run_examples.sh         : script to execute command to test examples

run_examples_CIL.sh     : script to execute command to test examples using CIL

myscript		: script to run example using CIL

get_ast-dump.sh         : script to get AST dump for input C source file

docs/
Doxyfile                : configuration file for Doxygen documentation,
                          generate/update documentation using: doxygen Doxyfile
                          Generated html dir contains - Doxygen documentation


//============================================================================//
// To build code
//============================================================================//

Set HEADER_SEARCH_PATH, edit setHeaderSearchPath.sh for required paths
. ./setHeaderSearchPath.sh

Edit Makefile for llvm and clang path for include and lib directories
(macro INCLUDE_FLAGS and LIB_FLAGS) and then run make inside source dir

make clean; make


//============================================================================//
// To test code with sample testdata
//============================================================================//

from inside source dir

./CAnalyzer ../testdata/sample.c output/sample.txt

or from CAnalyzer directory itself
./scripts/run_examples.sh

or from CAnalyzer directory itself using CIL
./scripts/run_examples_CIL.sh

CFG takes first input argument a C program and second argument a filename to
put output into. This output will contain generated CFG details and debug print
statements as well. You may give fully qualified path for these two arguments.


//============================================================================//
// Clang commands
//============================================================================//

To build C program (sample.c) using clang:
# clang sample.c -o sample

To dump AST:
# clang -cc1 -ast-dump sample.c

To run static analyzer:
# clang -cc1 -analyze -analyzer-checker=debug testdata/prg1.c

To get pretty-print CFG on screen:
# clang -cc1 -analyze -analyzer-checker=debug.DumpCFG testdata/prg1.c

To list available analyzers:
# clang -cc1 -analyzer-checker-help

