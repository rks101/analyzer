#
# Makefile
#
#CXX := g++ -g
#CXX = g++ -g -fprofile-arcs -ftest-coverage
CXX := clang++ -g -std=c++11
LLVMCONFIG := llvm-config

D_FLAGS := -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
RTTI_FLAG := -fno-rtti
CXXFLAGS := $(shell $(LLVMCONFIG) --cxxflags) $(RTTI_FLAG)
LLVM_LDFLAGS := $(shell $(LLVMCONFIG) --ldflags )

# You need to edit these path (LLVM_HOME, INCLUDE_FLAGS and LIB_FLAGS) based on you installation
LLVM_HOME=/usr/llvm
INCLUDE_FLAGS := -I. -I$(LLVM_HOME)/llvm-3.1.src/include -I$(LLVM_HOME)/llvm-3.1.src/tools/clang/include
LIB_FLAGS := -L$(LLVM_HOME)/llvm-3.1.src/Release+Asserts/lib
APRON_INCLUDE_FLAGS := -I/usr/local/include
APRON_LIB_FLAGS := -L/usr/local/lib

SOURCES = CFGGenerator.cpp MyASTConsumer.cpp MyASTVisitor.cpp Apron.cpp MyProcessStmt.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXEC = CAnalyzer

CLANG_LIBS = \
	-lclangFrontendTool\
	-lclangFrontend\
	-lclangDriver\
	-lclangSerialization\
	-lclangCodeGen\
	-lclangParse\
	-lclangSema\
	-lclangStaticAnalyzerFrontend\
	-lclangStaticAnalyzerCheckers\
	-lclangStaticAnalyzerCore\
	-lclangAnalysis\
	-lclangARCMigrate\
	-lclangRewrite \
	-lclangEdit\
	-lclangAST\
	-lclangLex\
	-lclangBasic

LLVM_LIBS = \
	-lLLVMMC \
	-lLLVMSupport

APRON_LIBS = -lboxMPQ -loctMPQ -lpolkaMPQ -lapron -lgmp -lmpfr -lm

all: $(OBJECTS) $(EXEC) move_to_bin

%.o: %.cpp
	$(CXX) $(D_FLAGS) $(INCLUDE_FLAGS) $(RTTI_FLAG) $(APRON_INCLUDE_FLAGS) -c $< 

CAnalyzer: CFGGenerator.o MyASTConsumer.o MyASTVisitor.o Apron.o MyProcessStmt.o
	$(CXX) $(INCLUDE_FLAGS) $(LIB_FLAGS) $(APRON_INCLUDE_FLAGS) $(APRON_LIB_FLAGS) -o $@ $? $(CLANG_LIBS) $(LLVM_LIBS) $(APRON_LIBS) $(LLVM_LDFLAGS)

move_to_bin:
	mkdir -p ../bin
	mv *.o ../bin/
	mv $(EXEC) ../bin/

clean:
	-rm -f ../bin/$(EXEC) ../bin/*.o *.o *~
	rm -rf ../bin/

