//
// MyASTConsumer.cpp
//

/// \file
/// \brief Defines AST reader class MyASTConsumer inherited from
/// clang::ASTConsumer
///
#include <iostream>

#include "MyASTConsumer.h"

/// \brief Set compiler instance for MyASTConsumer and MyASTVisitor
MyASTConsumer::MyASTConsumer(clang::CompilerInstance* ci)
{
  m_complierInstance = ci;

  m_astVisitor = MyASTVisitor();

  m_astVisitor.setCompilerInstance(ci);
}

/// \brief Implement HandleTopLevelDecl and call TraverseDecl()
bool MyASTConsumer::HandleTopLevelDecl(clang::DeclGroupRef declGroupRef)
{
  clang::DeclGroupRef::iterator it;

  for (it = declGroupRef.begin(); it != declGroupRef.end(); it++)
  {
/*
    // Print top-level declarations

    clang::Decl* decl = *it;
    std::cerr << " top-level-decl: \n";

    if (const clang::NamedDecl *ND = clang::dyn_cast<clang::NamedDecl>(decl))
    {
      std::cerr << ND->getNameAsString() << "\n";
    }
*/
    // Call TraverseDecl(clang::Decl* ) on instance of MyASTVisitor
    this->m_astVisitor.TraverseDecl(*it);
  }

  return true;
}
