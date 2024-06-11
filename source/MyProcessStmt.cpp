//
// MyProcessStmt.cpp
//

/// \file
/// \brief Defines MyProcessStmt inherited from clang::RecursiveASTVisitor to
/// handle and get information from different types of statements (clang::Stmt)
///
#include <iostream>
#include <string>

#include "MyProcessStmt.h"

using namespace clang;

/// \brief Flag to indicate if current expression is compound expression
bool isCascadedAssign = false;

/// \brief Count number of assignments in a statement
/// (required for cascaded assignments)
int assignCount = 0;

/// \brief Set compiler instance, block id for a statement, statement object
/// (Stmt *s) and call TraverseStmt()
MyProcessStmt::MyProcessStmt(clang::CompilerInstance* ci,
  unsigned id,
  Stmt* s,
  bool isLastStatement)
{
  m_compilerInstance = ci;
  m_blockId = id;
  m_stmt = s;
  m_isLastStatement = isLastStatement;
  m_ignoreExpr = false;

  // call TraverseStmt(Stmt* ) will dispatch calls to Visit## methods
  TraverseStmt(m_stmt);
}

/// \brief Get type as std::string from const ValueDecl*
std::string MyProcessStmt::getTypeAsString(const ValueDecl* valueDecl)
{
  // Get QualType from ValueDecl
  clang::QualType declQT = valueDecl->getType();

  // Now call clang::QualType.getAsString(PrintingPolicy &Policy)
  clang::ASTContext &context = this->m_compilerInstance->getASTContext();

  return declQT.getAsString(context.getPrintingPolicy());
}

/// \brief Get operand for unary incr/decr operator
std::string MyProcessStmt::getOperandForUnaryOp(UnaryOperator *E)
{
  // get sub expression
  Expr* subexpr = E->getSubExpr();

  std::string varName;
  std::string varType;

  if (strcmp(subexpr->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *subexpr from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(subexpr);
    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      varName = valueDecl->getNameAsString();
      varType = getTypeAsString(valueDecl);
      std::cerr << "\tvarName = " << (char *)varName.c_str() << "\n";
      std::cerr << "\tvarType = " << varType << "\n";
    }
  }

  return varName;
}

/// \brief Override visit method for declaration - DeclStmt
/// - inside a function
/// - inside for loop initialization section
bool MyProcessStmt::VisitDeclStmt(Stmt *s)
{
  std::cerr << "\n   Found DeclStmt \n";

  // Get DeclStmt by dyn_cast s to DeclStmt
  DeclStmt *declStmt = dyn_cast<DeclStmt>(s);

  // Get DeclGroupRef (e.g. int x, y, z) and iterate over it
  // to get Decl (e.g. int x, int y, int z)
  // For DeclGroupRef int x, we get Decl int x
  DeclGroupRef declGroupRef = declStmt->getDeclGroup();

  clang::DeclGroupRef::iterator it;

  for (it = declGroupRef.begin(); it != declGroupRef.end(); it++)
  {
    clang::Decl* decl = *it;

    std::cerr << "\tdecl statement: ";
    decl->dump();
    std::cerr << "\n";

    std::string varName;
    std::string varType;

    // Get name for Decl
    const NamedDecl *namedDecl = dyn_cast<NamedDecl>(decl);

    if (namedDecl)
    {
      varName = namedDecl->getNameAsString();
      std::cerr << "\tidentifier name = " << varName << "\n";
    }

    // Get type for Decl
    const ValueDecl *valueDecl = dyn_cast<ValueDecl>(decl);

    if (valueDecl)
    {
      varType = getTypeAsString(valueDecl);
      std::cerr << "\ttype = " << varType << "\n";

      // Get variable name and type
      getVarNameAndType(varName.c_str(), varType.c_str());
    }

    // Get initial value for declaration and update abstract value
    const VarDecl *varDecl = dyn_cast<VarDecl>(decl);

    if (varDecl)
    {
      // Check if this declaration has initial value
      if (varDecl->hasInit())
      {
        // increment assignCount when DeclStmt has initial value
        assignCount++;

        const Expr *e = varDecl->getInit();
        e = e->IgnoreParenImpCasts();

        if (strcmp(e->getStmtClassName(), "CharacterLiteral") == 0)
        {
          std::cerr << "\tCharacterLiteral: \n";

          const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(e);

          // TODO : get correct character literal value
          unsigned ch = CL->getValue();
          std::cerr << "\tinitial value: " << ch << "\n";

          // Set initial value for character literal
          setValueForIntegerLiteral(varName.c_str(), (double) ch, assignCount);
          assignCount--;
        }
        else
        if (strcmp(e->getStmtClassName(), "IntegerLiteral") == 0)
        {
          std::cerr << "\tIntegerLiteral: " << "\n";

          const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(e);

          // TODO : get correct integer literal value
          double dval = IL->getValue().signedRoundToDouble();
          std::cerr << "\tinitial value: " << dval << "\n";

          // Set initial value for integer literal
          setValueForIntegerLiteral(varName.c_str(), dval, assignCount);
          assignCount--;
        }
        else
        if (strcmp(e->getStmtClassName(), "FloatingLiteral") == 0)
        {
          std::cerr << "\tFloatingLiteral: " << "\n";

          const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(e);

          // TODO : get correct floating literal value
          double dval = FL->getValue().convertToDouble();
          std::cerr << "\tinitial value: " << dval << "\n";

          // Set initial value for floating literal
          setValueForFloatingLiteral(varName.c_str(), dval, assignCount);
          assignCount--;
        }
        else
        if (strcmp(e->getStmtClassName(), "DeclRefExpr") == 0)
        {
          const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(e);

          if (declRefExpr)
          {
            const ValueDecl *valueDecl = declRefExpr->getDecl();

            if (valueDecl)
            {
              std::string rhsVar = valueDecl->getNameAsString();
              std::string varType = getTypeAsString(valueDecl);
              std::cerr << "\tidentifier = " << rhsVar << "\n";
              std::cerr << "\ttype: " << varType << "\n";

              // set value for variable on RHS (e.g. int x=10, w=x;)
              setValueForRHSVariable(varName.c_str(), rhsVar.c_str(),
                varType.c_str(), assignCount);
              assignCount--;
            }
          }
        }
        else
        if (strcmp(e->getStmtClassName(), "ImplicitCastExpr") == 0)
        {
          std::cerr << "\tImplicitCastExpr: \n";

          const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(e);

          const Expr *se = ICE->getSubExpr();

          // Get ImplicitCastExpr->LValueToRValue
          if (ICE->getCastKind() == CK_LValueToRValue)
          {
            std::cerr << "\tLValueToRValue " << "\n";

            const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

            if (declRefExpr)
            {
              const ValueDecl *valueDecl = declRefExpr->getDecl();

              if (valueDecl)
              {
                std::string rhsVar = valueDecl->getNameAsString();
                std::string varType = getTypeAsString(valueDecl);
                std::cerr << "\tidentifier = " << rhsVar << "\n";
                std::cerr << "\ttype: " << varType << "\n";

                // set value for variable on RHS (e.g. int x=10, w=x;)
                setValueForRHSVariable(varName.c_str(), rhsVar.c_str(),
                  varType.c_str(), assignCount);
                assignCount--;
              }
            }
          }
          else
          // Get ImplicitCastExpr->IntegralCast
          if (ICE->getCastKind() == CK_IntegralCast)
          {
            std::cerr << "\tIntegralCast \n";

            // IntegralCast can be CharacterLiteral or IntegerLiteral
            // but first check for CharacterLiteral
            if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
            {
              std::cerr << "\tCharacterLiteral: \n";

              const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

              // TODO : get correct character literal value
              unsigned ch = CL->getValue();
              std::cerr << "\tinitial value: " << ch << "\n";

              // Set initial value for character literal
              setValueForIntegerLiteral(varName.c_str(), (double) ch,
                assignCount);
              assignCount--;
            }
            else
            if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
            {
              std::cerr << "\tIntegerLiteral: \n";

              const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);
              double dval = IL->getValue().signedRoundToDouble();
              std::cerr << "\tinitial value: " << dval << "\n";

              // Set initial value for integer literal
              setValueForIntegerLiteral(varName.c_str(), dval, assignCount);
              assignCount--;
            }
            else
            if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
            {
              const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

              if (declRefExpr)
              {
                const ValueDecl *valueDecl = declRefExpr->getDecl();

                if (valueDecl)
                {
                  std::string rhsVar = valueDecl->getNameAsString();
                  std::string varType = getTypeAsString(valueDecl);
                  std::cerr << "\tidentifier = " << rhsVar << "\n";
                  std::cerr << "\ttype: " << varType << "\n";

                  // set value for variable on RHS (e.g. int x=10, w=x;)
                  setValueForRHSVariable(varName.c_str(), rhsVar.c_str(),
                    varType.c_str(), assignCount);
                  assignCount--;
                }
              }
            }
          }
          else
          // Get ImplicitCastExpr->FloatingCast
          if (ICE->getCastKind() == CK_FloatingCast)
          {
            std::cerr << "\tFloatingCast \n";

            // FloatingCast can be FloatingLiteral
            if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
            {
              std::cerr << "\tFloatingLiteral: \n";

              const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
              double dval = FL->getValue().convertToDouble();
              std::cerr << "\tinitial value: " << dval << "\n";

              // Set initial value for floating literal
              setValueForFloatingLiteral(varName.c_str(), dval, assignCount);
              assignCount--;
            }
          }

        }  // ImplicitCastExpr ends
        else
        if ((strcmp(e->getStmtClassName(), "BinaryOperator") == 0))
        {
          std::string opcode = ((BinaryOperator*)e)->getOpcodeStr();

          if (strcmp(opcode.c_str(), "=") == 0)
          {
            std::cout << "Found assignment on rhs, it is cascaded assignment.\n";

            isCascadedAssign = true;
          }

          std::cout << "\nFound an initialization with expression on the RHS\n";

          TraverseStmt((Stmt*) e);

          evaluateAssignment(varName.c_str(), assignCount, m_isLastStatement);
          assignCount--;

          removeTemporaryVariables();
        }
        if ((strcmp(e->getStmtClassName(), "UnaryOperator") == 0))
        {
          std::cout << "\nFound an initialization with Unary expression on RHS";

          TraverseStmt((Stmt*) e);

          evaluateAssignment(varName.c_str(), assignCount, m_isLastStatement);
          assignCount--;

          removeTemporaryVariables();
        }

      }  // VarDecl->hasInit ends
    }  // VarDecl ends

  }  // for loop of DeclGroupRef::iterator ends

  return false;
}

/// \brief Override visit method for array subscript
bool MyProcessStmt::VisitArraySubscriptExpr(ArraySubscriptExpr* S)
{
  std::cerr <<"\n\tfound array subscript expr\n";

  m_ignoreExpr = true;

  Expr *base = S->getBase();

  if (strcmp(base->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(base);

    const Expr *se = ICE->getSubExpr();

    // Get ImplicitCastExpr->ArrayToPointerDecay
    if (ICE->getCastKind() == CK_ArrayToPointerDecay)
    {
      std::cerr << "\tArrayToPointerDecay " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::cerr << "\tidentifier = " << valueDecl->getNameAsString() << "\n";
      }
    }

  }  // ImplicitCastExpr ends

  Expr *idx = S->getIdx();

  if (strcmp(idx->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(idx);
    double val = IL->getValue().signedRoundToDouble();

    std::cerr << "\tindex value: " << val << "\n";
  }
  else
  {
    TraverseStmt((Stmt *)idx);
  }

  return false;
}

// Override visitor methods for binary operators

/// \brief Override visit method for assignment operator =
bool MyProcessStmt::VisitBinAssign(BinaryOperator *E)
{
  // BO_Assign =
  std::string opcode = E->getOpcodeStr();
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tAssignment Op " << opcode;
  std::cerr << "  in B" << this->m_blockId << "\n";

  // get operands of binary assignment operation
  Expr* lhs = E->getLHS();
  // take care of parenthesies on lhs
  lhs = lhs->IgnoreParens();

  Expr* rhs = E->getRHS();
  // take care of parenthesies and implicit cast expressions on rhs
  rhs = rhs->IgnoreParenImpCasts();

  std::string lhsVarName;
  const char* lhsVarType;

  bool rhs_found = false;

  // increment assignCount for every assignment
  assignCount++;

  // LHS of assignment

  if (strcmp(lhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *lhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(lhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      lhsVarName = valueDecl->getNameAsString();
      lhsVarType = getTypeAsString(valueDecl).c_str();
      std::cerr << "\tLHS identifier = " << lhsVarName << "\n";
      std::cerr << "\ttype: " << lhsVarType << "\n";
    }
  }

  // RHS of assignment

  if (strcmp(rhs->getStmtClassName(), "BinaryOperator") == 0)
  {
    std::string opcode = ((BinaryOperator*)rhs)->getOpcodeStr();

    // Important Note: - if rhs of assignment is = node, then
    // it is cascaded assignment, i.e. x = y = z = w = 10;
    // Hence if we get = operator, set isCascadedAssign to true
    if (strcmp(opcode.c_str(), "=") == 0)
    {
      std::cout << "Found assignment on rhs, it is cascaded assignment.\n";
      isCascadedAssign = true;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *rhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(rhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string rhsVar = valueDecl->getNameAsString();
      std::cerr << "\tRHS identifier = " << rhsVar << "\n";
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\ttype: " << varType << "\n";

      // set value for variable on RHS (e.g. y = x;)
      setValueForRHSVariable(lhsVarName.c_str(), rhsVar.c_str(),
        varType.c_str(), assignCount);
      assignCount--;
      rhs_found = true;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs);
    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    // Set value for integer literal
    setValueForIntegerLiteral(lhsVarName.c_str(), dval, assignCount);
    assignCount--;
    rhs_found = true;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "CharacterLiteral") == 0)
  {
    std::cerr << "\tCharacterLiteral:\n";

    const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(rhs);
    unsigned ch = CL->getValue();
    std::cerr << "\tRHS value: " << ch << "\n";

    // Set value for character literal
    setValueForIntegerLiteral(lhsVarName.c_str(), (double) ch, assignCount);
    assignCount--;
    rhs_found = true;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(rhs);
    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    // Set value for floating literal
    setValueForFloatingLiteral(lhsVarName.c_str(), dval, assignCount);
    assignCount--;
    rhs_found = true;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(rhs);

    const Expr *se = ICE->getSubExpr();
    std::cerr << "\tCastKind: " << ICE->getCastKind() << "\n";

    // Get ImplicitCastExpr->LValueToRValue
    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      if (declRefExpr)
      {
        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string rhsVar = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << rhsVar << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          // set value for variable on RHS (e.g. y = x;)
          setValueForRHSVariable(lhsVarName.c_str(), rhsVar.c_str(),
            varType.c_str(), assignCount);
          assignCount--;
          rhs_found = true;
        }
      }
    }
    else
    // Get ImplicitCastExpr->IntegralCast
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast \n";

      // IntegralCast can be CharacterLiteral or IntegerLiteral
      // i.e. unsigned int x = 0; or char c = 'a';
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\tCharacterLiteral:\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);
        unsigned ch = CL->getValue();
        std::cerr << "\tRHS value: " << ch << "\n";

        // Set value for character literal
        setValueForIntegerLiteral(lhsVarName.c_str(), (double) ch, assignCount);
        assignCount--;
        rhs_found = true;
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cerr << "\tIntegerLiteral:\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);
        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        // Set value for integer literal
        setValueForIntegerLiteral(lhsVarName.c_str(), dval, assignCount);
        assignCount--;
        rhs_found = true;
      }
    }
    else
    // Get ImplicitCastExpr->FloatingCast
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      // FloatingCast can be FloatingLiteral
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cerr << "\tFloatingLiteral: \n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tinitial value: " << dval << "\n";

        // Set value for floating literal
        setValueForFloatingLiteral(lhsVarName.c_str(), dval, assignCount);
        assignCount--;
        rhs_found = true;
      }
    }

  }  // ImplicitCastExpr ends

  // if assignment is not done => there is an expression node
  // call TraverseStmt() on rhs and get single expression
  //if (assignCount > 0)
  if (!rhs_found)
  {
    TraverseStmt((Stmt*) rhs);

    // now evaluate assignment by taking rhs expression from list
    // and updating abstract value for lhs identifier
    evaluateAssignment(lhsVarName.c_str(), assignCount, m_isLastStatement);

    assignCount--;

    if (isCascadedAssign == false)
    {
      removeTemporaryVariables();
    }

  }

  // remove temporary variables after all assignments are done
  if (assignCount == 0)
  {
    removeTemporaryVariables();
  }

  // set Ignore expression flag to true to ignore such cases (array,
  // struct, pointers, etc)
  if (this->m_ignoreExpr)
  {
    std::cerr << "\n found ignoreExpr! \n";
    lhs->dump();
  }

  // That's all! we are done, no need to go down further in AST, return false
  return false;
}

/// \brief Common method for arithmetic operators +, -, *, /, %
void MyProcessStmt::getLHSAndRHSForBO(BinaryOperator* E)
{
  // get arithmetic operator
  std::string opcode = E->getOpcodeStr();

  // get LHS and RHS of binary operator
  Expr* lhs = E->getLHS();
  // take care of parenthesies and implicit cast expressions on lhs
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  // take care of parenthesies and implicit cast expressions on rhs
  rhs = rhs->IgnoreParenImpCasts();

  std::string lhsVarName;
  std::string lhsVarType;

  // lhsFound (or rhsFound)
  // - a flag to denote lhs (or rhs) literal/identifier found
  int lhsFound = 0, rhsFound = 0;

  // LHS of binary operator

  if (strcmp(lhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *lhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(lhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tLHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      lhsFound = 1;
    }
  }
  else
  if (strcmp(lhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(lhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tLHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    lhsFound = 1;
  }
  else
  if (strcmp(lhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(lhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tLHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    lhsFound = 1;
  }
  else
  if (strcmp(lhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(lhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tLHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        lhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      se = se->IgnoreParenImpCasts();

      // Under ImplicitCastExpr - IntegralCast can be as follows:
      // int x,y; unsigned int ui;
      // x = ui + 1; <- ICE->IntegralCast->IntegerLiteral on rhs
      // x = 1 + ui; <- ICE->IntegralCast->IntegerLiteral on lhs
      // x = ui + y; <- ICE->IntegralCast->ICE->LValueToRValue->DeclRefExpr on rhs
      // x = y + ui; <- ICE->IntegralCast->ICE->LValueToRValue->DeclRefExpr on lhs
      // see AST dump for these examples
      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cerr << "\nICE->IntegralCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tLHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          lhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->IntegerLiteral\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tLHS value: " << dval << "\n";

        getIntegerLiteral(dval);

        lhsFound = 1;
      }
      else
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->CharacterLiteral\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

        unsigned ch = CL->getValue();
        std::cerr << "\tLHS value: " << ch << "\n";

        getIntegerLiteral((double)ch);

        lhsFound = 1;
      }

    } // IntegralCast ends
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->FloatingCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tLHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          lhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cout << "\nICE->FloatingCast->FloatingLiteral\n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tLHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        lhsFound = 1;
      }
    } // FloatingCast ends
    else
    {
      std::cout << "\nTough Luck in ImplicitCastExpr on lhs!\n";
      std::cout << "\ngot " << ICE->getCastKind() << "\n";
    }

  }  // ImplicitCastExpr ends

  // if lhs operand not found, recursively traverse AST for lhs
  if (!lhsFound)
  {
    std::cout << "\n!lhsFound\n";
    std::cout << "\nCannot determine lhs -> we are at operator node\n";

    // if lhs is again operator node in AST, get StmtClass of operation
    // cast it appropriately and print operator

    std::cout << lhs->getStmtClassName() << "\n";

    if (strcmp(lhs->getStmtClassName(), "BinaryOperator") == 0)
    {
      std::cout << ((BinaryOperator*)lhs)->getOpcodeStr() << "\n";
    }
    else
    if (strcmp(lhs->getStmtClassName(), "UnaryOperator") == 0)
    {
      std::cout << ((UnaryOperator*)lhs)->getOpcodeStr(
        ((UnaryOperator*)lhs)->getOpcode()) << "\n";
    }

    lhs = lhs->IgnoreParenImpCasts();
    TraverseStmt((Stmt*) lhs);
  }

  // RHS of binary operator

  if (strcmp(rhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *rhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(rhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tRHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      rhsFound = 1;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(rhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(rhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tRHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());
 
        rhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->IntegralCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          rhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cout << "\nICE->IntegralCast->IntegerLiteral\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getIntegerLiteral(dval);

        rhsFound = 1;
      }
      else
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->CharacterLiteral\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

        unsigned ch = CL->getValue();
        std::cerr << "\tRHS value: " << ch << "\n";

        getIntegerLiteral((double)ch);

        rhsFound = 1;
      }

    } // IntegralCast ends
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->FloatingCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          rhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cout << "\nICE->FloatingCast->FloatingLiteral\n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        rhsFound = 1;
      }
    } // FloatingCast ends
    else
    {
      std::cout << "\nTough Luck in ImplicitCastExpr on rhs!\n";
      std::cout << "\ngot " << ICE->getCastKind() << "\n";
    }

  }  // ImplicitCastExpr ends

  // if rhs operand not found, recursively traverse AST for rhs
  if (!rhsFound)
  {
    std::cout << "\n!rhsFound\n";
    std::cout << "\nCannot determine rhs -> we are at operator node\n";

    // if rhs is again operator node in AST, get StmtClass of operation
    // cast it appropriately and print operator

    std::cout << rhs->getStmtClassName() << "\n";

    if (strcmp(rhs->getStmtClassName(), "BinaryOperator") == 0)
    {
      std::cout << ((BinaryOperator*)rhs)->getOpcodeStr() << "\n";
    }
    else
    if (strcmp(rhs->getStmtClassName(), "UnaryOperator") == 0)
    {
      std::cout << ((UnaryOperator*)rhs)->getOpcodeStr(
        ((UnaryOperator*)rhs)->getOpcode()) << "\n";
    }

    rhs = rhs->IgnoreParenImpCasts();
    TraverseStmt((Stmt*) rhs);
  }

  // create a single expression for current opcode
  createSingleExpressionForBinOp(opcode.c_str());
}

// Override visit methods for arithmetic operators +, -, *, /, %

/// \brief Override visit method for arithmetic operator +
bool MyProcessStmt::VisitBinAdd(BinaryOperator* E)
{
  // BO_Add +
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tAdditive Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForBO(E);

  return false;
}

/// \brief Override visit method for arithmetic operator -
bool MyProcessStmt::VisitBinSub(BinaryOperator* E)
{
  // BO_Sub -
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tAdditive Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForBO(E);

  return false;
}

/// \brief Override visit method for arithmetic operator *
bool MyProcessStmt::VisitBinMul(BinaryOperator* E)
{
  // BO_Mul *
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tMultiplicative Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForBO(E);

  return false;
}

/// \brief Override visit method for arithmetic operator /
bool MyProcessStmt::VisitBinDiv(BinaryOperator* E)
{
  // BO_Div /
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tMultiplicative Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForBO(E);

  return false;
}

/// \brief Override visit method for arithmetic operator %
bool MyProcessStmt::VisitBinRem(BinaryOperator* E)
{
  // BO_Div %
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tMultiplicative Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForBO(E);

  return false;
}

/// \brief Common method for shift operators <<, >>
void MyProcessStmt::getLHSAndRHSForShBO(BinaryOperator* E)
{
  // get arithmetic operator
  std::string opcode = E->getOpcodeStr();

  // get LHS and RHS of binary operator
  Expr* lhs = E->getLHS();
  // take care of parenthesies and implicit cast expressions on lhs
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  // take care of parenthesies and implicit cast expressions on rhs
  rhs = rhs->IgnoreParenImpCasts();

  std::string lhsVarName;
  std::string lhsVarType;

  // lhsFound (or rhsFound)
  // - a flag to denote lhs (or rhs) literal/identifier found
  int lhsFound = 0, rhsFound = 0;

  // LHS of binary operator

  if (strcmp(lhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *lhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(lhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tLHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      lhsFound = 1;
    }
  }
  else
  if (strcmp(lhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(lhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tLHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    lhsFound = 1;
  }
  else
  if (strcmp(lhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(lhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tLHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    lhsFound = 1;
  }
  else
  if (strcmp(lhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(lhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tLHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        lhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      se = se->IgnoreParenImpCasts();

      // Under ImplicitCastExpr - IntegralCast can be as follows:
      // int x,y; unsigned int ui;
      // x = ui + 1; <- ICE->IntegralCast->IntegerLiteral on rhs
      // x = 1 + ui; <- ICE->IntegralCast->IntegerLiteral on lhs
      // x = ui + y; <- ICE->IntegralCast->ICE->LValueToRValue->DeclRefExpr on rhs
      // x = y + ui; <- ICE->IntegralCast->ICE->LValueToRValue->DeclRefExpr on lhs
      // see AST dump for these examples
      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cerr << "\nICE->IntegralCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tLHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          lhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->IntegerLiteral\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tLHS value: " << dval << "\n";

        getIntegerLiteral(dval);

        lhsFound = 1;
      }
      else
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->CharacterLiteral\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

        unsigned ch = CL->getValue();
        std::cerr << "\tLHS value: " << ch << "\n";

        getIntegerLiteral((double)ch);

        lhsFound = 1;
      }

    } // IntegralCast ends
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->FloatingCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tLHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          lhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cout << "\nICE->FloatingCast->FloatingLiteral\n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tLHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        lhsFound = 1;
      }
    } // FloatingCast ends
    else
    {
      std::cout << "\nTough Luck in ImplicitCastExpr on lhs!\n";
      std::cout << "\ngot " << ICE->getCastKind() << "\n";
    }

  }  // ImplicitCastExpr ends

  // if lhs operand not found, recursively traverse AST for lhs
  if (!lhsFound)
  {
    std::cout << "\n!lhsFound\n";
    std::cout << "\nCannot determine lhs -> we are at operator node\n";

    // if lhs is again operator node in AST, get StmtClass of operation
    // cast it appropriately and print operator

    std::cout << lhs->getStmtClassName() << "\n";

    if (strcmp(lhs->getStmtClassName(), "BinaryOperator") == 0)
    {
      std::cout << ((BinaryOperator*)lhs)->getOpcodeStr() << "\n";
    }
    else
    if (strcmp(lhs->getStmtClassName(), "UnaryOperator") == 0)
    {
      std::cout << ((UnaryOperator*)lhs)->getOpcodeStr(
        ((UnaryOperator*)lhs)->getOpcode()) << "\n";
    }

    lhs = lhs->IgnoreParenImpCasts();
    TraverseStmt((Stmt*) lhs);
  }

  // RHS of binary operator

  if (strcmp(rhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *rhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(rhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tRHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      rhsFound = 1;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(rhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(rhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tRHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        rhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->IntegralCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          rhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cout << "\nICE->IntegralCast->IntegerLiteral\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getIntegerLiteral(dval);

        rhsFound = 1;
      }
      else
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->CharacterLiteral\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

        unsigned ch = CL->getValue();
        std::cerr << "\tRHS value: " << ch << "\n";

        getIntegerLiteral((double)ch);

        rhsFound = 1;
      }

    } // IntegralCast ends
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->FloatingCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          rhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cout << "\nICE->FloatingCast->FloatingLiteral\n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        rhsFound = 1;
      }
    } // FloatingCast ends
    else
    {
      std::cout << "\nTough Luck in ImplicitCastExpr on rhs!\n";
      std::cout << "\ngot " << ICE->getCastKind() << "\n";
    }

  }  // ImplicitCastExpr ends

  // if rhs operand not found, recursively traverse AST for rhs
  if (!rhsFound)
  {
    std::cout << "\n!rhsFound\n";
    std::cout << "\nCannot determine rhs -> we are at operator node\n";

    // if rhs is again operator node in AST, get StmtClass of operation
    // cast it appropriately and print operator

    std::cout << rhs->getStmtClassName() << "\n";

    if (strcmp(rhs->getStmtClassName(), "BinaryOperator") == 0)
    {
      std::cout << ((BinaryOperator*)rhs)->getOpcodeStr() << "\n";
    }
    else
    if (strcmp(rhs->getStmtClassName(), "UnaryOperator") == 0)
    {
      std::cout << ((UnaryOperator*)rhs)->getOpcodeStr(
        ((UnaryOperator*)rhs)->getOpcode()) << "\n";
    }

    rhs = rhs->IgnoreParenImpCasts();
    TraverseStmt((Stmt*) rhs);
  }

  createSingleExpressionForBinShiftOp(opcode.c_str(), assignCount);

  //Check if the relational operator's result is part of an assignment.
  //If yes, then create a temporary variable and put it on the stack.
}

// Override visit methods for shift operators

/// \brief Override visit method for shift operator <<
bool MyProcessStmt::VisitBinShl(BinaryOperator* E)
{
  // BO_Shl <<
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tShift Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForShBO(E);

  // we must return false, else for -1 << 4, it will visit unary operator again
  // for UO_Plus, UO_Minus
  return false;
}

/// \brief Override visit method for shift operator >>
bool MyProcessStmt::VisitBinShr(BinaryOperator* E)
{
  // BO_Shr >>
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tShift Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForShBO(E);

  return false;
}

/// \brief Common method for relational operators >, >=, <, <=, ==, !=
void MyProcessStmt::getLHSAndRHSForRelBO(BinaryOperator* E)
{
  // get relational operator
  std::string opcode = E->getOpcodeStr();

  // get LHS and RHS of binary operator
  Expr* lhs = E->getLHS();
  // take care of parenthesies and implicit cast expressions on lhs
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  // take care of parenthesies and implicit cast expressions on rhs
  rhs = rhs->IgnoreParenImpCasts();

  assignCount++;

  // lhsFound (or rhsFound)
  // - a flag to denote lhs (or rhs) literal/identifier found
  int lhsFound = 0, rhsFound = 0;

  // LHS of relational operator

  if (strcmp(lhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *lhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(lhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tLHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      lhsFound = 1;
    }
  }
  else
  if (strcmp(lhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(lhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tLHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    lhsFound = 1;
  }
  else
  if (strcmp(lhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(lhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tLHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    lhsFound = 1;
  }
  else
  if (strcmp(lhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(lhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tLHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        lhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cerr << "\nICE->IntegralCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tLHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          lhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->IntegerLiteral\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tLHS value: " << dval << "\n";

        getIntegerLiteral(dval);

        lhsFound = 1;
      }
      else
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->CharacterLiteral\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

        unsigned ch = CL->getValue();
        std::cerr << "\tLHS value: " << ch << "\n";

        getIntegerLiteral((double) ch);

        lhsFound = 1;
      }

    } // IntegralCast ends
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->FloatingCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tLHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          lhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cout << "\nICE->FloatingCast->FloatingLiteral\n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);

        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tLHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        lhsFound = 1;
      }
    } // FloatingCast ends

  }  // ImplicitCastExpr ends

  // if lhs operand not found, recursively traverse AST for lhs
  if (!lhsFound)
  {
    std::cout << "\n!lhsFound\n";

    std::cout << lhs->getStmtClassName() << "\n";

    lhs = lhs->IgnoreParenImpCasts();
    TraverseStmt((Stmt*) lhs);
  }

  // RHS of relational operator

  if (strcmp(rhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *rhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(rhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tRHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      rhsFound = 1;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(rhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(rhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tRHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        rhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      std::cerr << "\tIntegralCast " << "\n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->IntegralCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          rhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "IntegerLiteral") == 0)
      {
        std::cout << "\nICE->IntegralCast->IntegerLiteral\n";

        const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

        double dval = IL->getValue().signedRoundToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";
 
        getIntegerLiteral(dval);

        rhsFound = 1;
      }
      else
      if (strcmp(se->getStmtClassName(), "CharacterLiteral") == 0)
      {
        std::cerr << "\nICE->IntegralCast->CharacterLiteral\n";

        const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(se);

        unsigned ch = CL->getValue();
        std::cerr << "\tRHS value: " << ch << "\n";

        getIntegerLiteral((double) ch);

        rhsFound = 1;
      }

    } // IntegralCast ends
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      se = se->IgnoreParenImpCasts();

      if (strcmp(se->getStmtClassName(), "DeclRefExpr") == 0)
      {
        std::cout << "\nICE->FloatingCast->DeclRefExpr\n";

        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string varName = valueDecl->getNameAsString();
          std::string varType = getTypeAsString(valueDecl);
          std::cerr << "\tRHS identifier = " << varName << "\n";
          std::cerr << "\ttype: " << varType << "\n";

          getVariable(varName.c_str(), varType.c_str());

          rhsFound = 1;
        }
      }
      else
      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cout << "\nICE->FloatingCast->FloatingLiteral\n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);
        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        rhsFound = 1;
      }
    } // FloatingCast ends

  }  // ImplicitCastExpr ends

  // if lhs operand not found, recursively traverse AST for lhs
  if (!rhsFound)
  {
    std::cout << "\n!rhsFound\n";

    std::cout << rhs->getStmtClassName() << "\n";

    rhs = rhs->IgnoreParenImpCasts();
    TraverseStmt((Stmt*) rhs);
  }

  // create a single expression for current opcode >, >=, <, <=
  createConstraintForBinRelOp(opcode.c_str());

  assignCount--;

}

// Override visit methods for relational operators >, >=, <, <=

/// \brief Override visit method for relational operators >
bool MyProcessStmt::VisitBinGT(BinaryOperator* E)
{
  // BO_GT >
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tRelational Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForRelBO(E);

  return false;
}

/// \brief Override visit method for relational operators >=
bool MyProcessStmt::VisitBinGE(BinaryOperator* E)
{
  // BO_GE >=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tRelational Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForRelBO(E);

  return false;
}

/// \brief Override visit method for relational operators <
bool MyProcessStmt::VisitBinLT(BinaryOperator* E)
{
  // BO_LT <
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tRelational Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForRelBO(E);

  return false;
}

/// \brief Override visit method for relational operators <=
bool MyProcessStmt::VisitBinLE(BinaryOperator* E)
{
  // BO_LE <=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tRelational Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForRelBO(E);

  return false;
}

/// \brief Override visit method for equality operator ==
bool MyProcessStmt::VisitBinEQ(BinaryOperator* E)
{
  // BO_EQ ==
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tEquality Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForRelBO(E);

  return false;
}

/// \brief Override visit method for equality operator !=
bool MyProcessStmt::VisitBinNE(BinaryOperator* E)
{
  // BO_EQ !=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tEquality Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForRelBO(E);

  return false;
}

// Override visit methods for bitwise operators

/// \brief Override visit method for bitwise operator &
bool MyProcessStmt::VisitBinAnd(BinaryOperator* E)
{
  // BO_And &
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tBitwise Op " << E->getOpcodeStr() << "\n";

  Expr* lhs = E->getLHS();
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  rhs = rhs->IgnoreParenImpCasts();

  return false;
}

/// \brief Override visit method for bitwise operator |
bool MyProcessStmt::VisitBinOr(BinaryOperator* E)
{
  // BO_Or |
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tBitwise Op " << E->getOpcodeStr() << "\n";

  Expr* lhs = E->getLHS();
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  rhs = rhs->IgnoreParenImpCasts();

  return false;
}

// Override visit methods for logical operators

/// \brief Override visit method for logical operator &&
bool MyProcessStmt::VisitBinLAnd(BinaryOperator* E)
{
  // BO_LAnd &&
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tLogical Op " << E->getOpcodeStr() << "\n";

  Expr* lhs = E->getLHS();
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  rhs = rhs->IgnoreParenImpCasts();

  TraverseStmt((Stmt*) lhs);
  TraverseStmt((Stmt*) rhs);

  return false;
}

/// \brief Override visit method for logical operator ||
bool MyProcessStmt::VisitBinLOr(BinaryOperator* E)
{
  // BO_LOr ||
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tLogical Op " << E->getOpcodeStr() << "\n";

  Expr* lhs = E->getLHS();
  lhs = lhs->IgnoreParenImpCasts();

  Expr* rhs = E->getRHS();
  rhs = rhs->IgnoreParenImpCasts();

  TraverseStmt((Stmt*) lhs);
  TraverseStmt((Stmt*) rhs);

  return false;
}

/// \brief Common method for compound assignment +=, -=, *=, /=, %=
void MyProcessStmt::getLHSAndRHSForCAO(CompoundAssignOperator* E)
{
  // get compound assignment operator
  std::string opcode = E->getOpcodeStr();

  // get LHS and RHS of compound assignment operator
  Expr* lhs = E->getLHS();

  Expr* rhs = E->getRHS();
  // take care of parenthesies and implicit cast expressions on rhs
  rhs = rhs->IgnoreParenImpCasts();

  std::string lhsVarName;
  std::string lhsVarType;

  // increment assignCount for every compound assignment
  assignCount++;

  // rhsFound - a flag to denote rhs literal/identifier found
  int rhsFound = 0;

  // LHS of compound assignment operator

  if (strcmp(lhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *lhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(lhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      lhsVarName = valueDecl->getNameAsString();
      lhsVarType = getTypeAsString(valueDecl);
      std::cerr << "\tLHS identifier = " << lhsVarName << "\n";
      std::cerr << "\ttype: " << lhsVarType << "\n";

      getVariable(lhsVarName.c_str(), lhsVarType.c_str());
    }
  }

  // RHS of compound assignment operator

  if (strcmp(rhs->getStmtClassName(), "CompoundAssignOperator") == 0)
  {
    std::string opcode = ((BinaryOperator*)rhs)->getOpcodeStr();

    if ((strcmp(opcode.c_str(), "+=") == 0) ||
        (strcmp(opcode.c_str(), "-=") == 0) ||
        (strcmp(opcode.c_str(), "*=") == 0) ||
        (strcmp(opcode.c_str(), "/=") == 0) ||
        (strcmp(opcode.c_str(), "%=") == 0) )
    {
      std::cout << "\nFound compound assignment on rhs,"
        " it is cascaded compound assignment.\n";
      isCascadedAssign = true;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *rhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(rhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tRHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      rhsFound = 1;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(rhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(rhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tRHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        rhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

      double dval = IL->getValue().signedRoundToDouble();
      std::cerr << "\tRHS value: " << dval << "\n";

      getIntegerLiteral(dval);

      rhsFound = 1;
    }
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cerr << "\tFloatingLiteral: \n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);

        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        rhsFound = 1;
      }
    }

  }  // ImplicitCastExpr ends

  if (!rhsFound)
  {
    std::cout << "\n!rhsFound\n";
    std::cout << "\nCannot determine rhs -> we are at operator node\n";

    TraverseStmt((Stmt*) rhs);
  }

  // create a single expression for current opcode
  createSingleExpressionForBinOp(opcode.c_str());

  // now evaluate assignment by taking rhs expression from list
  // and updating abstract value for lhs identifier
  evaluateAssignment(lhsVarName.c_str(), assignCount, m_isLastStatement);
  assignCount--;

  if (isCascadedAssign == false)
  {
    removeTemporaryVariables();
  }

  if (assignCount == 0)
  {
    removeTemporaryVariables();
  }
}

// Override visit methods for compound assignment operators

/// \brief Override visit method for compound assignment operator +=
bool MyProcessStmt::VisitBinAddAssign(CompoundAssignOperator* E)
{
  // BO_AddAssign +=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tCompound Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForCAO(E);

  return false;
}

/// \brief Override visit method for compound assignment operator -=
bool MyProcessStmt::VisitBinSubAssign(CompoundAssignOperator* E)
{
  // BO_SubAssign -=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tCompound Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForCAO(E);

  return false;
}

/// \brief Override visit method for compound assignment operator *=
bool MyProcessStmt::VisitBinMulAssign(CompoundAssignOperator* E)
{
  // BO_MulAssign *=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tCompound Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForCAO(E);

  return false;
}

/// \brief Override visit method for compound assignment operator /=
bool MyProcessStmt::VisitBinDivAssign(CompoundAssignOperator* E)
{
  // BO_DivAssign /=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tCompound Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForCAO(E);

  return false;
}

/// \brief Override visit method for compound assignment operator %=
bool MyProcessStmt::VisitBinRemAssign(CompoundAssignOperator* E)
{
  // BO_RemAssign %=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tCompound Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForCAO(E);

  return false;
}

/// \brief Common method for compound shift operators <<=, >>=
void MyProcessStmt::getLHSAndRHSForShCAO(CompoundAssignOperator* E)
{
  std::string opcode = E->getOpcodeStr();
  // get LHS side
  Expr* lhs = E->getLHS();
  Expr* rhs = E->getRHS();

  std::string lhsVarName;
  std::string lhsVarType;

  assignCount++;

  if (strcmp(lhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *lhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(lhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      lhsVarName = valueDecl->getNameAsString();
      lhsVarType = getTypeAsString(valueDecl);
      std::cerr << "\tLHS identifier = " << valueDecl->getNameAsString() << "\n";
      std::cerr << "\ttype: " << getTypeAsString(valueDecl) << "\n";

      getVariable(lhsVarName.c_str(), lhsVarType.c_str());
    }

  }

  // get RHS side

  int rhsFound = 0;

  if (strcmp(rhs->getStmtClassName(), "CompoundAssignOperator") == 0)
  {
    std::string opcode = ((BinaryOperator*)rhs)->getOpcodeStr();

    if ((strcmp(opcode.c_str(), "+=") == 0) ||
        (strcmp(opcode.c_str(), "-=") == 0) ||
        (strcmp(opcode.c_str(), "*=") == 0) ||
        (strcmp(opcode.c_str(), "/=") == 0) ||
        (strcmp(opcode.c_str(), "%=") == 0) )
    {
      std::cout << "\nFound compound assignment on rhs,"
        " it is cascaded compound assignment.\n";
      isCascadedAssign = true;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "DeclRefExpr") == 0)
  {
    // get name and type for Expr *rhs from DeclRefExpr
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(rhs);

    const ValueDecl *valueDecl = declRefExpr->getDecl();

    if (valueDecl)
    {
      std::string varName = valueDecl->getNameAsString();
      std::string varType = getTypeAsString(valueDecl);
      std::cerr << "\tRHS identifier = " << varName << "\n";
      std::cerr << "\ttype: " << varType << "\n";

      getVariable(varName.c_str(), varType.c_str());

      rhsFound = 1;
    }
  }
  else
  if (strcmp(rhs->getStmtClassName(), "IntegerLiteral") == 0)
  {
    std::cerr << "\tIntegerLiteral: " << "\n";

    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs);

    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getIntegerLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "FloatingLiteral") == 0)
  {
    std::cerr << "\tFloatingLiteral: " << "\n";

    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(rhs);

    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tRHS value: " << dval << "\n";

    getFloatingLiteral(dval);

    rhsFound = 1;
  }
  else
  if (strcmp(rhs->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    std::cerr << "\tImplicitCastExpr: " << "\n";

    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(rhs);

    const Expr *se = ICE->getSubExpr();

    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      std::cerr << "\tLValueToRValue " << "\n";

      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tRHS identifier = " << varName << "\n";
        std::cerr << "\ttype: " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());

        rhsFound = 1;
      }
    }
    else
    if (ICE->getCastKind() == CK_IntegralCast)
    {
      std::cerr << "\tIntegralCast " << "\n";

      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(se);

      double dval = IL->getValue().signedRoundToDouble();
      std::cerr << "\tRHS value: " << dval << "\n";

      getIntegerLiteral(dval);

      rhsFound = 1;
    }
    else
    if (ICE->getCastKind() == CK_FloatingCast)
    {
      std::cerr << "\tFloatingCast \n";

      if (strcmp(se->getStmtClassName(), "FloatingLiteral") == 0)
      {
        std::cerr << "\tFloatingLiteral: \n";

        const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(se);

        double dval = FL->getValue().convertToDouble();
        std::cerr << "\tRHS value: " << dval << "\n";

        getFloatingLiteral(dval);

        rhsFound = 1;
      }
    }

  }  // ImplicitCastExpr ends

  if (!rhsFound)
  {
    std::cout << "\n!rhsFound\n";
    std::cout << "\nCannot determine rhs -> we are at operator node\n";

    TraverseStmt((Stmt*) rhs);
  }

  //Create a new temporary variable.
  createSingleExpressionForBinShiftOp(opcode.c_str(),assignCount);
  //Set value of lhs to the new temp variable.
  evaluateAssignment(lhsVarName.c_str(), assignCount, m_isLastStatement);

  assignCount--;

  if (isCascadedAssign == false)
  {
    removeTemporaryVariables();
  }

  if (assignCount == 0)
  {
    removeTemporaryVariables();
  }

}

// Override visit methods for shift assignment operators

/// \brief Override visit method for shift assignment operator <<=
bool MyProcessStmt::VisitBinShlAssign(CompoundAssignOperator* E)
{
  // BO_ShlAssign <<=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tShift Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForShCAO(E);

  return false;
}

/// \brief Override visit method for shift assignment operator >>=
bool MyProcessStmt::VisitBinShrAssign(CompoundAssignOperator* E)
{
  // BO_ShrAssign >>=
  std::cerr << "\n    Found BinaryOperator \n";
  std::cerr << "\tShift Assignment Op " << E->getOpcodeStr() << "\n";

  getLHSAndRHSForShCAO(E);

  return false;
}

// Override visitor methods for unary operators

/// \brief Override visit method for unary operator -
bool MyProcessStmt::VisitUnaryMinus(UnaryOperator *E)
{
  std::string opcode = E->getOpcodeStr(E->getOpcode());
  std::cerr << "\tArithmetic Unary Op " << opcode << "\n";

  // get sub expression
  Expr* subexpr = E->getSubExpr();

  // take care of parenthesies and implicit cast expressions on subexpr
  subexpr = subexpr->IgnoreParenCasts();

  // flag to indicate unary subexpression received
  int got_unary_subexpr = 0;

  if (strcmp(subexpr->getStmtClassName(), "IntegerLiteral") == 0)
  {
    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(subexpr);
    double dval = IL->getValue().signedRoundToDouble();

    getIntegerLiteralUnaryOp(dval, opcode.c_str());
    got_unary_subexpr = 1;
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "FloatingLiteral") == 0)
  {
    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(subexpr);
    double dval = FL->getValue().convertToDouble();

    getFloatingLiteralUnaryOp(dval, opcode.c_str());
    got_unary_subexpr = 1;
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "DeclRefExpr") == 0)
  {
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(subexpr);

    if (declRefExpr)
    {
      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string uVarName = valueDecl->getNameAsString();
        std::string uVarType = getTypeAsString(valueDecl);
        std::cerr << "\tunary op identifier = " << uVarName << "\n";
        std::cerr << "\ttype: " << uVarType << "\n";

        getVariableUnaryOp(uVarName.c_str(), uVarType.c_str(),
          opcode.c_str());
        got_unary_subexpr = 1;
      }
    }
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(subexpr);
    const Expr *se = ICE->getSubExpr();

    // Get ImplicitCastExpr->LValueToRValue
    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      if (declRefExpr)
      {
        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string uVarName = valueDecl->getNameAsString();
          std::string uVarType = getTypeAsString(valueDecl);
          std::cerr << "\tunary op identifier = " << uVarName << "\n";
          std::cerr << "\ttype: " << uVarType << "\n";

          getVariableUnaryOp(uVarName.c_str(), uVarType.c_str(),
            opcode.c_str());
          got_unary_subexpr = 1;
        }
      }
    }
  } // ImplicitCastExpr ends

  // if unary operator is not of this kind x = -1 or x = -y
  // then unary opertaor is not received and we need to invoke
  // TraverseStmt() with this subexpression
  if (!got_unary_subexpr)
  {
    TraverseStmt((Stmt*) subexpr);

    // get the expression, negate it and add to MyApronExpr list
    // e.g -(x + 1)
    createSingleExpressionForUnaryOp(opcode.c_str());
  }

  return false;
}

/// \brief Override visit method for unary operator +
bool MyProcessStmt::VisitUnaryPlus(UnaryOperator *E)
{
  std::string opcode = E->getOpcodeStr(E->getOpcode());
  std::cerr << "\tArithmetic Unary Op " << opcode << "\n";

  // get sub expression
  Expr* subexpr = E->getSubExpr();
  // take care of parenthesies and implicit cast expressions on subexpr
  subexpr = subexpr->IgnoreParenCasts();

  // flag to indicate unary subexpression received
  int got_unary_subexpr = 0;

  if (strcmp(subexpr->getStmtClassName(), "IntegerLiteral") == 0)
  {
    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(subexpr);
    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tvalue: " << dval << "\n";

    getIntegerLiteralUnaryOp(dval, opcode.c_str());
    got_unary_subexpr = 1;
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "FloatingLiteral") == 0)
  {
    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(subexpr);
    double dval = FL->getValue().convertToDouble();
    std::cerr << "\tvalue: " << dval << "\n";

    getFloatingLiteralUnaryOp(dval, opcode.c_str());
    got_unary_subexpr = 1;
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "DeclRefExpr") == 0)
  {
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(subexpr);

    if (declRefExpr)
    {
      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string uVarName = valueDecl->getNameAsString();
        std::string uVarType = getTypeAsString(valueDecl);
        std::cerr << "\tunary op identifier = " << uVarName << "\n";
        std::cerr << "\ttype: " << uVarType << "\n";

        getVariableUnaryOp(uVarName.c_str(), uVarType.c_str(),
          opcode.c_str());
        got_unary_subexpr = 1;
      }
    }
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(subexpr);
    const Expr *se = ICE->getSubExpr();

    // Get ImplicitCastExpr->LValueToRValue
    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      if (declRefExpr)
      {
        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string uVarName = valueDecl->getNameAsString();
          std::string uVarType = getTypeAsString(valueDecl);
          std::cerr << "\tunary op identifier = " << uVarName << "\n";
          std::cerr << "\ttype: " << uVarType << "\n";

          getVariableUnaryOp(uVarName.c_str(), uVarType.c_str(),
            opcode.c_str());
          got_unary_subexpr = 1;
        }
      }
    }
  } // ImplicitCastExpr ends

  // if unary operator is not of this kind x = +1 or x = +y
  // then unary operator is not received and we need to invoke
  // TraverseStmt() with this subexpression
  if (!got_unary_subexpr)
  {
    TraverseStmt((Stmt*) subexpr);

    // get the expression and add to MyApronExpr list
    // e.g +(x + 1)
    createSingleExpressionForUnaryOp(opcode.c_str());
  }

  return false;
}

/// \brief Override visit method for unary operator ++ (a++)
bool MyProcessStmt::VisitUnaryPostInc(UnaryOperator *E)
{
  std::cerr << "\n    Found UnaryOperator \n";
  std::cerr << "\tUnary post Increment Op " << E->getOpcodeStr(E->getOpcode());
  std::cerr << "\n";

  std::string varName;
  varName = getOperandForUnaryOp(E);

  doPostIncrement(varName.c_str(), assignCount);

  if (assignCount == 0)
  {
    removeTemporaryVariables();
  }

  return false;
}

/// \brief Override visit method for unary operator -- (a--)
bool MyProcessStmt::VisitUnaryPostDec(UnaryOperator *E)
{
  std::cerr << "\n    Found UnaryOperator \n";
  std::cerr << "\tUnary pre Increment Op " << E->getOpcodeStr(E->getOpcode());
  std::cerr << "\n";

  std::string varName;
  varName = getOperandForUnaryOp(E);

  doPostDecrement(varName.c_str(), assignCount);

  if (assignCount == 0)
  {
    removeTemporaryVariables();
  }

  return false;
}

/// \brief Override visit method for unary operator ++ (++a)
bool MyProcessStmt::VisitUnaryPreInc(UnaryOperator *E)
{
  std::cerr << "\n    Found UnaryOperator \n";
  std::cerr << "\tUnary pre Increment Op " << E->getOpcodeStr(E->getOpcode());
  std::cerr << "\n";

  std::string varName;
  varName = getOperandForUnaryOp(E);

  doPreIncrement(varName.c_str(), assignCount);

  if (assignCount == 0)
  {
    removeTemporaryVariables();
  }

  return false;
}

/// \brief Override visit method for unary operator -- (--a)
bool MyProcessStmt::VisitUnaryPreDec(UnaryOperator *E)
{
  std::cerr << "\n    Found UnaryOperator \n";
  std::cerr << "\tUnary pre Decrement Op " << E->getOpcodeStr(E->getOpcode());
  std::cerr << "\n";

  std::string varName;
  varName = getOperandForUnaryOp(E);

  doPreDecrement(varName.c_str(), assignCount);

  if (assignCount)
  {
    removeTemporaryVariables();
  }

  return false;
}

/// \brief Override visit method for UO_AddrOf operator &
bool MyProcessStmt::VisitUnaryAddrOf(UnaryOperator *S)
{
  std::cerr <<"\n\tfound UO_AddrOf & \n";

  return false;
}

/// \brief Override visit method for UO_Deref operator *
bool MyProcessStmt::VisitUnaryDeref(UnaryOperator *S)
{
  std::cerr <<"\n\tfound UO_Deref * \n";

  return false;
}

/// \brief Override visit method for UO_LNot operator !
bool MyProcessStmt::VisitUnaryLNot(UnaryOperator *E)
{
  std::string opcode = E->getOpcodeStr(E->getOpcode());
  std::cerr << "\tLogical Unary Op " << opcode << "\n";

  // get sub expression
  Expr* subexpr = E->getSubExpr();
  // take care of parenthesies and implicit cast expressions on subexpr
  subexpr = subexpr->IgnoreParenCasts();

  // flag to indicate unary subexpression received
  int got_unary_subexpr = 0;

  if (strcmp(subexpr->getStmtClassName(), "IntegerLiteral") == 0)
  {
    const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(subexpr);
    double dval = IL->getValue().signedRoundToDouble();
    std::cerr << "\tvalue: " << dval << "\n";

    if (dval == 0)
    {
      std::cerr << "\t\tIntegerLiteral is zero! \n";
      setAbstractMemory(0);
    }
    else
    {
      std::cerr << "\t\tIntegerLiteral is non-zero! \n";
      setAbstractMemory(1);
    }

    got_unary_subexpr = 1;
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "FloatingLiteral") == 0)
  {
    const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(subexpr);
    double dval = FL->getValue().convertToDouble();

    // do nothing for now

    got_unary_subexpr = 1;
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "DeclRefExpr") == 0)
  {
    const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(subexpr);

    if (declRefExpr)
    {
      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string uVarName = valueDecl->getNameAsString();
        std::string uVarType = getTypeAsString(valueDecl);
        std::cerr << "\tunary op identifier = " << uVarName << "\n";
        std::cerr << "\ttype: " << uVarType << "\n";

        setAbstractMemory(2);
        got_unary_subexpr = 1;
      }
    }
  }
  else
  if (strcmp(subexpr->getStmtClassName(), "ImplicitCastExpr") == 0)
  {
    const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(subexpr);
    const Expr *se = ICE->getSubExpr();

    // Get ImplicitCastExpr->LValueToRValue
    if (ICE->getCastKind() == CK_LValueToRValue)
    {
      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

      if (declRefExpr)
      {
        const ValueDecl *valueDecl = declRefExpr->getDecl();

        if (valueDecl)
        {
          std::string uVarName = valueDecl->getNameAsString();
          std::string uVarType = getTypeAsString(valueDecl);
          std::cerr << "\tunary op identifier = " << uVarName << "\n";
          std::cerr << "\ttype: " << uVarType << "\n";

          setAbstractMemory(2);
          got_unary_subexpr = 1;
        }
      }
    }
  } // ImplicitCastExpr ends

  // if unary operator is not of this kind !1 or !1.1 or !x
  // then unary operator is not received and we need to invoke
  // TraverseStmt() with this subexpression
  if (!got_unary_subexpr)
  {
    TraverseStmt((Stmt*) subexpr);
  }

  // update abstract values for UO_LNot
  updateAbstractValuesForLNot();

  return false;
}

/// \brief Override visit method for UO_Not operator ~
bool MyProcessStmt::VisitUnaryNot(UnaryOperator *E)
{
  std::string opcode = E->getOpcodeStr(E->getOpcode());
  std::cerr << "\tUnary Op " << opcode << "\n";

  return false;
}

/// \brief Override VisitIfStmt to get condition, then clause and else clause
bool MyProcessStmt::VisitIfStmt(Stmt *s)
{
  std::cerr << "\tfound if stmt in B" << this->m_blockId << "\n";

  // Cast s to IfStmt to access then and else clauses
  IfStmt *If = cast<IfStmt>(s);

  const Expr* cond = If->getCond();
  std::cerr << "\t=> condition: \n";
  std::cerr << "\tStatement class: " << cond->getStmtClassName();

  if (cond)
  {
    // Find if (1) or if (0)
    if (strcmp(cond->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // condition is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(cond);
      double val = IL->getValue().signedRoundToDouble();
      std::cerr << "\t\tvalue: " << val << "\n";

      if (val == 0)
      {
        std::cerr << "\t\tIntegerLiteral is zero! \n";
        setAbstractMemory(0);
      }
      else
      {
        std::cerr << "\t\tIntegerLiteral is non-zero! \n";
        setAbstractMemory(1);
      }
    }
    else
    // Find if (x)
    if (strcmp(cond->getStmtClassName(), "ImplicitCastExpr") == 0)
    {
      const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(cond);

      const Expr *se = ICE->getSubExpr();

      if (ICE->getCastKind() == CK_LValueToRValue)
      {
        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        if (declRefExpr)
        {
          const ValueDecl *valueDecl = declRefExpr->getDecl();

          if (valueDecl)
          {
            std::string varName = valueDecl->getNameAsString();
            std::string varType = getTypeAsString(valueDecl);
            std::cerr << "\tidentifier = " << varName << "\n";
            std::cerr << "\ttype: " << varType << "\n";

            setAbstractMemory(2);
          }
        }
      }
    }
    else
    // Find if (x = y)
    if ((strcmp(cond->getStmtClassName(), "BinaryOperator") == 0) &&
        (strcmp(((BinaryOperator*)cond)->getOpcodeStr(), "=") == 0) )
    {
      std::cerr << "Assignment found in condition!\n";
    }
    else
    {
      TraverseStmt((Stmt *)cond);
    }
  }

  return false;
}

/// \brief Override VisitWhileStmt to get condition and compound stmt block
bool MyProcessStmt::VisitWhileStmt(Stmt *s)
{
  std::cerr << "\n\tfound while in B" << this->m_blockId << "\n";

  // Cast s to WhileStmt to access cond and body of while
  WhileStmt* While = cast<WhileStmt>(s);

  const Expr* cond = While->getCond();
  std::cerr << "\t=> condition: \n";
  std::cerr << "\tStatement class: " << cond->getStmtClassName();

  if (cond)
  {
    // Find while (1) or while (0)
    if (strcmp(cond->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // condition is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(cond);
      double dval = IL->getValue().signedRoundToDouble();
      std::cerr << "\t\tvalue: " << dval << "\n";

      if (dval == 0)
      {
        std::cerr << "\t\tIntegerLiteral is zero! \n";
        setAbstractMemory(0);
      }
      else
      {
        std::cerr << "\t\tIntegerLiteral is non-zero! \n";
        setAbstractMemory(1);
      }
    }
    else
    // Find while (x)
    if (strcmp(cond->getStmtClassName(), "ImplicitCastExpr") == 0)
    {
      const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(cond);

      const Expr *se = ICE->getSubExpr();

      if (ICE->getCastKind() == CK_LValueToRValue)
      {
        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        if (declRefExpr)
        {
          const ValueDecl *valueDecl = declRefExpr->getDecl();

          if (valueDecl)
          {
            std::string varName = valueDecl->getNameAsString();
            std::string varType = getTypeAsString(valueDecl);
            std::cerr << "\tidentifier = " << varName << "\n";
            std::cerr << "\ttype: " << varType << "\n";
        
            setAbstractMemory(2);
          }
        }
      }
    }
    else
    // Find while (x = y)
    if ((strcmp(cond->getStmtClassName(), "BinaryOperator") == 0) &&
        (strcmp(((BinaryOperator*)cond)->getOpcodeStr(), "=") == 0) )
    {
      std::cerr << "Assignment found in condition!\n";
    }
    else
    {
      TraverseStmt((Stmt *)cond);
    }
  }

  return false;
}

/// \brief Override VisitDoStmt to get condition and compound stmt block
bool MyProcessStmt::VisitDoStmt(Stmt *s)
{
  std::cerr << "\n\tfound do-while in B" << this->m_blockId << "\n";

  // Cast s to DoStmt to access cond and body of do-while
  DoStmt* do_while = cast<DoStmt>(s);

  const Expr* cond = do_while->getCond();
  std::cerr << "\t=> condition: \n";
  std::cerr << "\tStatement class: " << cond->getStmtClassName();

  if (cond)
  {
    // Find while (1) or while (0)
    if (strcmp(cond->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // condition is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(cond);
      double val = IL->getValue().signedRoundToDouble();
      std::cerr << "\t\tvalue: " << val << "\n";

      if (val == 0)
      {
        std::cerr << "\t\tIntegerLiteral is zero! \n";
        setAbstractMemory(0);
      }
      else
      {
        std::cerr << "\t\tIntegerLiteral is non-zero! \n";
        setAbstractMemory(1);
      }
    }
    else
    // Find while (x)
    if (strcmp(cond->getStmtClassName(), "ImplicitCastExpr") == 0)
    {
      const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(cond);

      const Expr *se = ICE->getSubExpr();

      if (ICE->getCastKind() == CK_LValueToRValue)
      {
        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        if (declRefExpr)
        {
          const ValueDecl *valueDecl = declRefExpr->getDecl();

          if (valueDecl)
          {
            std::string varName = valueDecl->getNameAsString();
            std::string varType = getTypeAsString(valueDecl);
            std::cerr << "\tidentifier = " << varName << "\n";
            std::cerr << "\ttype: " << varType << "\n";
          }
        }
      }
    }
    else
    // Find while (x = y)
    if ((strcmp(cond->getStmtClassName(), "BinaryOperator") == 0) &&
        (strcmp(((BinaryOperator*)cond)->getOpcodeStr(), "=") == 0) )
    {
      std::cerr << "Assignment found in condition!\n";
    }
    else
    {
      TraverseStmt((Stmt *)cond);
    }
  }

  return false;
}

/// \brief Override VisitForStmt to get initialization, condition, increment
/// section and compound stmt body
bool MyProcessStmt::VisitForStmt(Stmt *s)
{
  std::cerr << "\n\tfound for stmt in B" << this->m_blockId << "\n";

  // Cast s to ForStmt to access init, cond, inc, body of for
  ForStmt* For = cast<ForStmt>(s);

  const Expr* cond = For->getCond();
  std::cerr << "\n\t=> condition: \n";
  std::cerr << "\tStatement class: " << cond->getStmtClassName();

  if (cond)
  {
    // Find for (1) or (0)
    if (strcmp(cond->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // condition is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(cond);
      double val = IL->getValue().signedRoundToDouble();
      std::cerr << "\t\tvalue: " << val << "\n";

      if (val == 0)
      {
        std::cerr << "\t\tIntegerLiteral is zero! \n";
        setAbstractMemory(0);
      }
      else
      {
        std::cerr << "\t\tIntegerLiteral is non-zero! \n";
        setAbstractMemory(1);
      }
    }
    else
    // Find for (x)
    if (strcmp(cond->getStmtClassName(), "ImplicitCastExpr") == 0)
    {
      const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(cond);

      const Expr *se = ICE->getSubExpr();

      if (ICE->getCastKind() == CK_LValueToRValue)
      {
        const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(se);

        if (declRefExpr)
        {
          const ValueDecl *valueDecl = declRefExpr->getDecl();

          if (valueDecl)
          {
            std::string varName = valueDecl->getNameAsString();
            std::string varType = getTypeAsString(valueDecl);
            std::cerr << "\tidentifier = " << varName << "\n";
            std::cerr << "\ttype: " << varType << "\n";
          }
        }
      }
    }
    else
    // Find for (x = y)
    if ((strcmp(cond->getStmtClassName(), "BinaryOperator") == 0) &&
        (strcmp(((BinaryOperator*)cond)->getOpcodeStr(), "=") == 0) )
    {
      std::cerr << "Assignment found in condition!\n";
    }
    else
    {
      TraverseStmt((Stmt *)cond);
    }
  }

  return false;
}

/// \brief Override visit method for conditional operator ?:
bool MyProcessStmt::VisitConditionalOperator(ConditionalOperator* s)
{
  std::cerr << "\tfound conditional operator in B" << this->m_blockId << "\n";

  // Cast s to ConditionalOperator to access condition, true and false expression
  ConditionalOperator *CO = cast<ConditionalOperator>(s);

  std::cerr << "\t=> condition: \n";
  const Expr* cond = CO->getCond();

  if (cond)
  {
    if (strcmp(cond->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // condition is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(cond);
      double val = IL->getValue().signedRoundToDouble();
      std::cerr << "\tvalue: " << val << "\n";
    }
    else
    {
      TraverseStmt((Stmt *)cond);
    }
  }

  std::cerr << "\n\t=> true expression: \n";
  const Expr* TrueExpr = CO->getTrueExpr();

  if (TrueExpr)
  {
    if (strcmp(TrueExpr->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // TrueExpr is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(TrueExpr);
      double val = IL->getValue().signedRoundToDouble();
      std::cerr << "\tvalue: " << val << "\n";
    }
    else
    {
      TraverseStmt((Stmt *)TrueExpr);
    }
  }

  std::cerr << "\n\t=> false expression: \n";
  const Expr* FalseExpr = CO->getFalseExpr();

  if (FalseExpr)
  {
    if (strcmp(FalseExpr->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // FalseExpr is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(FalseExpr);
      double val = IL->getValue().signedRoundToDouble();
      std::cerr << "\tvalue: " << val << "\n";
    }
    else
    {
      TraverseStmt((Stmt *)FalseExpr);
    }
  }
  
  return false;
}

/// \brief Override VisitReturnStmt to get expression of return statement
bool MyProcessStmt::VisitReturnStmt(Stmt *s)
{
  std::cerr << "\treturn stmt: in B" << this->m_blockId << "\n";

  // Cast s to ReturnStmt to get return value
  ReturnStmt* Return = cast<ReturnStmt>(s);

  const Expr *ret_value = Return->getRetValue();

  // take care of parenthesies and implicit cast expressions on ret_value
  ret_value = ret_value->IgnoreParenImpCasts();

  if (ret_value)
  {
    if (strcmp(ret_value->getStmtClassName(), "IntegerLiteral") == 0)
    {
      // return value is integer literal, no need to traverse further
      const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(ret_value);

      double dval = IL->getValue().signedRoundToDouble();
      std::cerr << "\tvalue: " << dval << "\n";

      getIntegerLiteral(dval);
    }
    else
    if (strcmp(ret_value->getStmtClassName(), "CharacterLiteral") == 0)
    {
      const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(ret_value);

      unsigned ch = CL->getValue();
      std::cerr << "\tvalue: " << ch << "\n";

      getIntegerLiteral((double) ch);
    }
    else
    if (strcmp(ret_value->getStmtClassName(), "FloatingLiteral") == 0)
    {
      const FloatingLiteral *FL = dyn_cast<FloatingLiteral>(ret_value);

      double dval = FL->getValue().convertToDouble();
      std::cerr << "\tvalue: " << dval << "\n";

      getFloatingLiteral(dval);
    }
    else
    if (strcmp(ret_value->getStmtClassName(), "DeclRefExpr") == 0)
    {
      const DeclRefExpr *declRefExpr = dyn_cast<DeclRefExpr>(ret_value);
      const ValueDecl *valueDecl = declRefExpr->getDecl();

      if (valueDecl)
      {
        std::string varName = valueDecl->getNameAsString();
        std::string varType = getTypeAsString(valueDecl);
        std::cerr << "\tvarName = " << (char *)varName.c_str() << "\n";
        std::cerr << "\tvarType = " << varType << "\n";

        getVariable(varName.c_str(), varType.c_str());
      }
    }
    else
    {
      // this return is an expression, call TraverseStmt() on ret_value
      TraverseStmt((Stmt *)ret_value);
    }
  }
  else
  {
    std::cout << "\nEmpty return statement found!\n";
  }

  evaluateReturn();

  return false;
}

/// \brief Override VisitNullStmt to traverse null Stmt i.e. ;
bool MyProcessStmt::VisitNullStmt(Stmt *s)
{
  std::cerr << "\t\tNull stmt ;\n";
  // Nothing to be done for null statement
  return false;
}

/// \brief Override VisitStmt to traverse every Stmt
bool MyProcessStmt::VisitStmt(Stmt *s)
{
  // Returning false from one of the overridden visitor functions
  // will abort the entire traversal.
  return true;
}

