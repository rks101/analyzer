//
// CFGGenerator.cpp
//

/// \mainpage C Analyzer Documentation
/// \section intro_sec Introduction
/// CAnalyzer is a tool for static analysis of C programs using theory of
/// abstract interpretation.
///
/// CAnalyzer uses LLVM frontend Clang v3.1 for CFG generation and traversal.

/// \file
/// \brief Driver program to get input C program, set compiler instance options
///   and invoke parser by calling ParseAST()
///
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "llvm/Support/Host.h"

#include "clang/Basic/FileManager.h"
#include "clang/Parse/ParseAST.h"

#include "MyASTConsumer.h"

#define HEADER_SEARCH_PATH "HEADER_SEARCH_PATH"

/// \brief Split a string for a given delimiter character
/// and return a vector of split-strings
/// @param str - string to split
/// @param c - character to split with
/// @return std::vector<std::string> - vector of strings containing paths
std::vector<std::string> split(const char *str, char c = ':')
{
  std::vector<std::string> result;

  do
  {
    const char *begin = str;

    while (*str != c && *str)
    {
      str++;
    }

    result.push_back(std::string(begin, str));

  } while (0 != *str++);

  return result;
}

/// \brief main program that invokes parser by calling ParseAST()
/// @param argc - number of arguments
/// @param argv - command line arguments to main()
/// @return int - return value
int main(int argc, const char** argv)
{
  // Check for arguments and print usage if arguments not provided
  if (argc < 3)
  {
    std::cout << "\n\nUsage:\n\n"
      << "./CAnalyzer <.c_file_to_analyze> <dump_file_to_be_created>\n\n"
      << "e.g.\n" << "./CAnalyzer prg1.c prg1.txt\n\n"
      << "./CAnalyzer testdata/prg1.c output/prg1.txt\n\n";

    return 1;
  }

  // Dump CFG to a file instead of console
  // Get dump file name from command line argument
  // - this file will be created in path specified
  const char* DumpFile = argv[2];

  freopen(DumpFile, "w", stderr);

  // CompilerIntance manages various objects - preprocessor, target information,
  // ASTContext, etc. and provides utility function to manage clang objects
  clang::CompilerInstance ci;

  // Create Diagnostics - compiler instance must have Diagnostics created
  ci.createDiagnostics(0, NULL);

  // Get default target triple
  // (Target triple is a string in the format of:
  // CPU_TYPE-VENDOR-KERNEL-OPERATING_SYSTEM
  // e.g. "i386-pc-linux-gnu")
  clang::TargetOptions to;

  to.Triple = llvm::sys::getDefaultTargetTriple();

  // Set Target - compiler instance must have a Target set
  clang::TargetInfo *ti;

  ti = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), to);
  ci.setTarget(ti);

  // Set language default options
  ci.getInvocation().setLangDefaults(clang::IK_C);

  // Add header search options
  // - get environment variable HEADER_SEARCH_PATH
  // - split HEADER_SEARCH_PATH into paths and add to header search path
  const char* header_path = getenv(HEADER_SEARCH_PATH);
  
  if (header_path == NULL)
  {
    std::cout << "\n HEADER_SEARCH_PATH is invalid! \n\n";

    return 1;
  }
  else
  {
    std::cout << "\n HEADER_SEARCH_PATH = " << header_path << "\n";
  }

  std::vector <std::string> paths = split(header_path);

  llvm::StringRef headerLoc;

  for (int i = 0; i < paths.size(); i++)
  {
    headerLoc = paths[i];

    ci.getHeaderSearchOpts().AddPath(headerLoc, clang::frontend::System,
      false, false, false);
  }
  
  // Get input file to be parsed from command line argument
  // and add to path
  const char* InputFile = argv[1];

  llvm::StringRef FileToParse = InputFile;

  ci.getHeaderSearchOpts().AddPath(FileToParse, clang::frontend::System,
    false, false, false);

  // Create file manager and source manager - for file management
  ci.createFileManager();
  ci.createSourceManager(ci.getFileManager());

  // Create preprocessor
  ci.createPreprocessor();

  // Create ASTContext - holds AST node types and declarations
  clang::ASTContext *astContext;

  ci.createASTContext();
  astContext = & ci.getASTContext();

  // Create AST reader
  MyASTConsumer *astConsumer;

  astConsumer = new MyASTConsumer(&ci);
  ci.setASTConsumer(astConsumer);

  // FileEntry - holds information about a file including file descriptor
  const clang::FileEntry *pFile;

  pFile = ci.getFileManager().getFile(FileToParse);
  ci.getSourceManager().createMainFileID(pFile);
  ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
     &ci.getPreprocessor());

  /// Invoke parser by calling ParseAST() which will call HandleTopLevelDecl()
  ///
  /// See llvm/llvm-3.1.src/tools/clang/lib/Parse/ParseAST.cpp
  clang::ParseAST(ci.getPreprocessor(), astConsumer, ci.getASTContext());

  ci.getDiagnosticClient().EndSourceFile();

  fclose(stderr);

  return 0;
}

