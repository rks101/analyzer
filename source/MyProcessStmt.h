//
// MyProcessStmt.h
//

/// \file
/// \brief Defines MyProcessStmt inherited from clang::RecursiveASTVisitor to
/// handle and get information from different types of statements (clang::Stmt)
///
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/CompilerInstance.h"

#ifndef MYPROCESSSTMT_H
# define MYPROCESSSTMT_H

#include "MyASTVisitor.h"
#include "Apron.h"

using namespace clang;

/// \brief AST visitor class to process every statement
/// inherited from clang::RecursiveASTVisitor
class MyProcessStmt : public clang::RecursiveASTVisitor<MyProcessStmt>
{
  public:

    /// \brief Set MyProcessStmt members and call TraverseStmt()
    /// @param ci - compiler instance
    /// @param id - basic block id for current statement
    /// @param s - statement object (Stmt *s)
    /// @param isLastStatement - is current statement last statement of the block
    MyProcessStmt(clang::CompilerInstance* ci,
      unsigned id,
      Stmt* s,
      bool isLastStatement);

    // helper methods for MyProcessStmt

    /// \brief Get type as std::string from const ValueDecl*
    /// @param vd - ValueDecl object to get type
    /// @return std::string - type in string
    std::string getTypeAsString(const ValueDecl *vd);

    /// \brief Get operand for unary incr/decr operator
    /// @param E - unary operator
    /// @return std::string - operand of unary operator
    std::string getOperandForUnaryOp(UnaryOperator *E);

    // Visit methods for MyProcessStmt

    /// \brief Override visit method for declaration - DeclStmt
    /// @param s - declaration statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitDeclStmt(Stmt* s);

    /// \brief Override visit method for array subscript
    /// @param S - array subscript operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitArraySubscriptExpr(ArraySubscriptExpr* S);

    /// \brief Override visit method for assignment operator =
    /// @param E - binary assignment operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinAssign(BinaryOperator* E);

    /// \brief Common method for arithmetic operators +, -, *, /, %
    /// @param E - binary arithmetic operator
    /// @return void (nothing)
    void getLHSAndRHSForBO(BinaryOperator* E);

    // Override visit methods for arithmetic operators +, -, *, /, %
    /// \brief Override visit method for arithmetic operator +
    /// @param E - binary add operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinAdd(BinaryOperator* E);

    /// \brief Override visit method for arithmetic operator -
    /// @param E - binary subtract operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinSub(BinaryOperator* E);

    /// \brief Override visit method for arithmetic operator *
    /// @param E - binary multiply operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinMul(BinaryOperator* E);

    /// \brief Override visit method for arithmetic operator /
    /// @param E - binary divide operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinDiv(BinaryOperator* E);

    /// \brief Override visit method for arithmetic operator %
    /// @param E - binary modulo operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinRem(BinaryOperator* E);

    /// \brief Common method for shift operators <<, >>
    /// @param E - binary shift operator
    /// @return void (nothing)
    void getLHSAndRHSForShBO(BinaryOperator* E);

    // Override visit methods for shift operators
    /// \brief Override visit method for shift operator <<
    /// @param E - binary left shift operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinShl(BinaryOperator* E);

    /// \brief Override visit method for shift operator >>
    /// @param E - binary right shift operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinShr(BinaryOperator* E);

    /// \brief Common method for relational operators >, >=, <, <=, ==, !=
    /// @param E - binary relational operator
    /// @return void (nothing)
    void getLHSAndRHSForRelBO(BinaryOperator* E);

    // Override visit methods for relational operators >, >=, <, <=
    /// \brief Override visit method for relational operators >
    /// @param E - binary greater than operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinGT(BinaryOperator* E);

    /// \brief Override visit method for relational operators >=
    /// @param E - binary greater than or equal operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinGE(BinaryOperator* E);

    /// \brief Override visit method for relational operators <
    /// @param E - binary less than operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinLT(BinaryOperator* E);

    /// \brief Override visit method for relational operators <=
    /// @param E - binary less than or equal operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinLE(BinaryOperator* E);

    /// \brief Override visit method for equality operator ==
    /// @param E - binary equality operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinEQ(BinaryOperator* E);

    /// \brief Override visit method for equality operator !=
    /// @param E - binary in-equality operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinNE(BinaryOperator* E);

    // Override visit methods for bitwise operators
    /// \brief Override visit method for bitwise operator &
    /// @param E - binary bitwise and operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinAnd(BinaryOperator* E);

    /// \brief Override visit method for bitwise operator |
    /// @param E - binary bitwise or operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinOr(BinaryOperator* E);

    // Override visit methods for logical operators
    /// \brief Override visit method for logical operator &&
    /// @param E - binary logical and operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinLAnd(BinaryOperator* E);

    /// \brief Override visit method for logical operator ||
    /// @param E - binary logical or operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinLOr(BinaryOperator* E);

    /// \brief Common method for compound assignment +=, -=, *=, /=, %=
    /// @param E - compound assignment operator
    /// @return void (nothing)
    void getLHSAndRHSForCAO(CompoundAssignOperator* E);

    // Override visit methods for compound assignment operators
    /// \brief Override visit method for compound assignment operator +=
    /// @param E - binary compound add assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinAddAssign(CompoundAssignOperator* E);

    /// \brief Override visit method for compound assignment operator -=
    /// @param E - binary compound subtract assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinSubAssign(CompoundAssignOperator* E);

    /// \brief Override visit method for compound assignment operator *=
    /// @param E - binary compound multiply assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinMulAssign(CompoundAssignOperator* E);

    /// \brief Override visit method for compound assignment operator /=
    /// @param E - binary compound divide assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinDivAssign(CompoundAssignOperator* E);

    /// \brief Override visit method for compound assignment operator %=
    /// @param E - binary compound modulo assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinRemAssign(CompoundAssignOperator* E);

    /// \brief Common method for compound shift operators <<=, >>=
    /// @param E - compound assignment operator
    /// @return void (nothing)
    void getLHSAndRHSForShCAO(CompoundAssignOperator* E);

    // Override visit methods for shift assignment operators
    /// \brief Override visit method for shift assignment operator <<=
    /// @param E - binary left shift assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinShlAssign(CompoundAssignOperator* E);

    /// \brief Override visit method for shift assignment operator >>=
    /// @param E - binary right shift assign operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitBinShrAssign(CompoundAssignOperator* E);

    // Override visit methods for unary operators
    /// \brief Override visit method for unary operator -
    /// @param E - unary minus operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryMinus(UnaryOperator* E);

    /// \brief Override visit method for unary operator +
    /// @param E - unary plus operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryPlus(UnaryOperator* E);

    /// \brief Override visit method for unary operator ++ (a++)
    /// @param E - unary post-increment operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryPostInc(UnaryOperator *E);

    /// \brief Override visit method for unary operator -- (a--)
    /// @param E - unary post-decrement operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryPostDec(UnaryOperator *E);
    
    /// \brief Override visit method for unary operator ++ (++a)
    /// @param E - unary pre-increment operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryPreInc(UnaryOperator *E);

    /// \brief Override visit method for unary operator -- (--a)
    /// @param E - unary pre-decrement operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryPreDec(UnaryOperator *E);

    /// \brief Override visit method for UO_AddrOf operator &
    /// @param S - unary address of operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryAddrOf(UnaryOperator *S);

    /// \brief Override visit method for UO_Deref operator *
    /// @param S - unary de-referencing operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryDeref(UnaryOperator *S);

    /// \brief Override visit method for UO_LNot operator !
    /// @param S - unary logical not operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryLNot(UnaryOperator *S);

    /// \brief Override visit method for UO_Not operator ~
    /// @param S - unary not operator
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitUnaryNot(UnaryOperator *S);

    /// \brief Override VisitIfStmt to get condition, then clause and else clause
    /// @param s - if statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitIfStmt(Stmt *s);

    /// \brief Override VisitWhileStmt to get condition and compound stmt block
    /// @param s - while loop statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitWhileStmt(Stmt *s);

    /// \brief Override VisitDoStmt to get condition and compound stmt block
    /// @param s - do while loop statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitDoStmt(Stmt *s);

    /// \brief Override VisitForStmt to get initialization, condition,
    /// increment section and compound stmt body
    /// @param s - for loop statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitForStmt(Stmt *s);

    /// \brief Override visit method for conditional operator ?:
    /// @param S - conditional operator ?:
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitConditionalOperator(ConditionalOperator* S);
    
    /// \brief Override VisitReturnStmt to get expression of return statement
    /// @param s - return statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitReturnStmt(Stmt *s);

    /// \brief Override VisitNullStmt to traverse null Stmt i.e. ;
    /// @param s - null statement
    /// @return bool - false to stop AST traversal further down in AST
    bool VisitNullStmt(Stmt *s);

    /// \brief Override VisitStmt to traverse every Stmt
    /// @param s - any statement of type clang::Stmt
    /// @return bool - true to continue AST traversal further down in AST
    bool VisitStmt(Stmt *s);

private:

    /// \brief CompilerInstance handle
    CompilerInstance* m_compilerInstance;

    /// \brief Block Id to which this statement belongs
    unsigned m_blockId;

    /// \brief Current Statement
    Stmt* m_stmt;

    /// \brief is this statement last statement of the basic block
    bool m_isLastStatement;

    /// \brief Ignore expression flag to ignore an expression, e.g. array,
    /// struct, pointer, etc.
    bool m_ignoreExpr;
};

#endif // MYPROCESSSTMT_H

