//
// MyASTConsumer.h
//

/// \file
/// \brief Defines AST reader class MyASTConsumer inherited from
/// clang::ASTConsumer
///
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

#ifndef MYASTCONSUMER_H
# define MYASTCONSUMER_H

#include "MyASTVisitor.h"

/// \brief AST reader class inherited from clang::ASTConsumer
class MyASTConsumer : public clang::ASTConsumer
{
  public:

    /// \brief Set compiler instance for MyASTConsumer and MyASTVisitor
    /// @param ci - compiler instance
    MyASTConsumer(clang::CompilerInstance* ci);

    /// \brief Implement HandleTopLevelDecl and call TraverseDecl()
    /// @param declGroupRef - declarations
    /// @return bool - return value true
    virtual bool HandleTopLevelDecl(clang::DeclGroupRef declGroupRef);

  private:

    /// \brief CompilerInstance handle
    clang::CompilerInstance* m_complierInstance;

    /// \brief MyASTVisitor handle
    MyASTVisitor m_astVisitor;
};

#endif // MYASTCONSUMER_H

