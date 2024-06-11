//
// Apron.cpp
//

/// \file
/// \brief Defines Apron related declarations and definitions
///
#include <iostream>
#include <sstream>
#include <climits>
#include <cfloat>

#include <Apron.h>

/// \brief Apron manager
ap_manager_t* man;

/// \brief Apron environment
ap_environment_t* env;

/// \brief Pointer to structre to hold abstract value
AbsVal* AbsValPtr = NULL;

/// \brief Pointer to structure to store Abstract Memory
AbstractMemory* AbstractMemoryPtr = NULL;

/// \brief Counter for no. of temporary variables created
int mycount = 0;

/// \brief Apron type (int/real) of current expression
int expr_type = 0;

/// \brief Vector containg pointers to VariableCount Structure
VariableCountList globalVarList;

/// \brief MyApronExpr list
// initialize empty list
MyApronExpr* head = NULL;

/// \brief Pointer to CondExprAbsVal list
CondExprAbsVal* absval_head = NULL;

/// \brief Add apron type expression to MyApronExpr list
void addExpression(ap_texpr1_t* expression, int expr_type)
{
  MyApronExpr* t_node;

  t_node = (MyApronExpr*) new MyApronExpr;
  t_node->data = expression;
  t_node->expr_type = expr_type;
  t_node->next = head;
  head = t_node;
}

/// \brief Remove apron type expression from MyApronExpr list
MyApronExpr* removeExpression()
{
  MyApronExpr* t_node;

  t_node = (MyApronExpr*) new MyApronExpr;

  if (head != NULL)
  {
    // head is not empty, return head node and set head = head->next
    t_node->data = head->data;
    t_node->expr_type = head->expr_type;
    head = head->next;
    return t_node;
  }
  else
  {
    // Send underflow error
    std::cout << "Expression underflow! ";
    std::cout << "Attempt to extract expression from empty MyApronExpr!\n\n";
    return NULL;
  }
}

/// \brief Show apron type expressions of MyApronExpr list
void showExpression()
{
  MyApronExpr* t_node;
  ap_texpr1_t *t_tree_expr[1];

  t_node = (MyApronExpr*) new MyApronExpr;

  if (head != NULL)
  {
    // head is not empty, print list
    t_node = head;
    std::cout << " (";

    do
    {
      t_tree_expr[0] = t_node->data;
      ap_texpr1_print(t_tree_expr[0]);
      std::cout << " ,";
      t_node = t_node->next;
    } while (t_node != NULL);

    std::cout << " END)\n\n";
  }
  else
  {
    // empty list
    std::cout << "\n\nMyApronExpr Expression List is empty!\n\n";
  }
}

/// \brief Add condAbsVal and negCondAbsVal to CondExprAbsVal list
void addCondExprAbsVal(ap_abstract1_t absVal, ap_abstract1_t negAbsVal)
{
  CondExprAbsVal* t_CondExprAbsVal;

  t_CondExprAbsVal = (CondExprAbsVal*) new CondExprAbsVal;
  t_CondExprAbsVal->condAbsVal = absVal;
  t_CondExprAbsVal->negCondAbsVal = negAbsVal;
  t_CondExprAbsVal->next = absval_head;
  absval_head = t_CondExprAbsVal;
}

/// \brief Remove condAbsVal and negCondAbsVal from CondExprAbsVal list
CondExprAbsVal* removeCondExprAbsVal()
{
  CondExprAbsVal* t_CondExprAbsVal;

  t_CondExprAbsVal = (CondExprAbsVal*) new CondExprAbsVal;

  if (absval_head != NULL)
  {
    // absval_head is not empty, return head node and set head = head->next
    t_CondExprAbsVal->condAbsVal = absval_head->condAbsVal;
    t_CondExprAbsVal->negCondAbsVal = absval_head->negCondAbsVal;
    absval_head = absval_head->next;
    return t_CondExprAbsVal;
  }
  else
  {
    // Send underflow error
    std::cout << "CondExprAbsVal list underflow! ";
    std::cout << "Attempt to extract element from empty CondExprAbsVal!\n\n";
    return NULL;
  }
}

/// \brief Show condAbsVal and negCondAbsVal from CondExprAbsVal list
void showCondExprAbsVal()
{
  CondExprAbsVal* t_CondExprAbsVal;
  ap_abstract1_t t_absval;

  t_CondExprAbsVal = (CondExprAbsVal*) new CondExprAbsVal;

  if (absval_head != NULL)
  {
    // absval_head is not empty, print list
    t_CondExprAbsVal = absval_head;

    std::cerr << "\n  CondExprAbsVal:\n";

    do
    {
      t_absval = t_CondExprAbsVal->condAbsVal;
      ap_abstract1_fprint(stderr, man, &(t_absval));
      t_absval = t_CondExprAbsVal->negCondAbsVal;
      ap_abstract1_fprint(stderr, man, &(t_absval));

      t_CondExprAbsVal = t_CondExprAbsVal->next;
    } while (t_CondExprAbsVal != NULL);

    std::cout << "\n\n";
  }
  else
  {
    // empty list
    std::cout << "\n\nCondExprAbsVal list is empty!\n\n";
  }
}

/// \brief get abstract domain choice and set empty environment
void initApron()
{
  // First get abstract domain
  std::cout << "\nFollowing Numerical Abstract Domains are available:\n";
  std::cout << "\n\tBox (Interval) - 1\n\tOctagon - 2\n\tPolyhedra - 3\n";
  std::cout << "\n(These domains are provided by APRON library)\n";
start:
  std::cout << "\nPlease select a domain (1/2/3) to continue (or 0 to exit): ";

  int abstract_domain;
  std::cin >> abstract_domain;

  std::cout << "\tYou have entered domain: " << abstract_domain << "\n";

  if (abstract_domain == 0)
  {
    std::cout << "\n[Done]\n";
    return ;
  }

  if ((abstract_domain == 1) || (abstract_domain == 2) || (abstract_domain == 3))
  {
    // actually do nothing
  }
  else
  {
    // invalid domain, ask again
    std::cout << "\tInvalid value for domain\n";
    goto start;
  }

  switch (abstract_domain)
  {
    //Allocate appropriate manager for selected domain
    case 1:
      man = box_manager_alloc();
      break;
    case 2:
      man = oct_manager_alloc();
      break;
    case 3:
      man = pk_manager_alloc(true);
      break;
  }

  const char* library;
  const char* version;

  library = ap_manager_get_library(man);
  version = ap_manager_get_version(man);

  std::cout << "\n\tLibrary: " << library << "  Version: " << version << "\n";

  // Allocate environment - initially empty
  env = ap_environment_alloc_empty();

  AbsValPtr = (AbsVal *) new AbsVal;
  AbsValPtr->abstract_value = ap_abstract1_top(man, env);

  AbstractMemoryPtr = (AbstractMemory *) new AbstractMemory;
  AbstractMemoryPtr->blockAbsVal = ap_abstract1_top(man, env);

  std::cout << "initial abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
  std::cout << "\n";
}

/// \brief Get variable name and type for apron library and add to environment
void getVarNameAndType(const char* varName, const char* varType)
{
  const char* apronVarType = getApronDimType(varType);

  // Add int variable to apron environment
  if (strcmp(apronVarType, "int") == 0)
  {
    char** t_dim_int;
    t_dim_int = (char **)new char*;
    t_dim_int[0] = strdup((char* )varName);

    initCountForVar(varName);
    printGlobalVarList();

    env = ap_environment_add(env, (ap_var_t* )t_dim_int, 1, NULL, 0);

    std::cout << "abstract value:\n";
    AbsValPtr->abstract_value = ap_abstract1_change_environment(man, true,
      &(AbsValPtr->abstract_value), env, false);
    ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));

/*
    // while adding integers to environment, use MIN, MAX values from climits
    long int inf = LONG_MIN;
    long int sup = LONG_MAX;

    if ((strcmp(varType, "int") == 0) ||
        (strcmp(varType, "const int") == 0) )
    {
      inf = INT_MIN;
      sup = INT_MAX;
    }
    else
    if ((strcmp(varType, "long") == 0) ||
        (strcmp(varType, "long int") == 0) ||
        (strcmp(varType, "long long") == 0) ||
        (strcmp(varType, "long long int") == 0) )
    {
      inf = LONG_MIN;
      sup = LONG_MAX;
    }
    else
    if ((strcmp(varType, "unsigned") == 0) ||
        (strcmp(varType, "unsigned int") == 0) ||
        (strcmp(varType, "unsigned long") == 0) ||
        (strcmp(varType, "unsigned long int") == 0) ||
        (strcmp(varType, "unsigned long long") == 0) ||
        (strcmp(varType, "unsigned long long int") == 0) )
    {
      inf = 0;
      sup = UINT_MAX;
    }
    else
    if (strcmp(varType, "char") == 0)
    {
      inf = CHAR_MIN;
      sup = CHAR_MAX;
    }
    else
    if (strcmp(varType, "signed char") == 0)
    {
      inf = SCHAR_MIN;
      sup = SCHAR_MAX;
    }
    else
    if (strcmp(varType, "unsigned char") == 0)
    {
      inf = 0;
      sup = UCHAR_MAX;
    }

    //std::cerr << "\nUINT_MAX = " << UINT_MAX;
    //std::cerr << " ULONG_MAX = " << ULONG_MAX <<"\n";
    ap_texpr1_t* t_texpr = ap_texpr1_cst_interval_int(env, inf, sup);
    AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
      &(AbsValPtr->abstract_value), (char*) varName, t_texpr, NULL);
    ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
*/

    delete t_dim_int;
  }
  else
  // Add real variable to apron environment
  if (strcmp(apronVarType, "real") == 0)
  {
    char** t_dim_real;
    t_dim_real = (char **)new char*;
    t_dim_real[0] = strdup((char* )varName);

    initCountForVar(varName);
    printGlobalVarList();

    env = ap_environment_add(env, NULL, 0, (ap_var_t* )t_dim_real, 1);

    std::cout << "abstract value:\n";
    AbsValPtr->abstract_value = ap_abstract1_change_environment(man, true,
      &(AbsValPtr->abstract_value), env, false);
    ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));

/*
    // while adding real to environment, use MIN and MAX values from cfloat
    // - for double, long double use DBL_MIN and DBL_MAX
    // - for float use FLT_MIN and FLT_MAX
    double inf;
    double sup;

    if (strcmp(varType, "float") == 0)
    {
      inf = FLT_MIN;
      sup = FLT_MAX;
    }
    else
    if ((strcmp(varType, "double") == 0) ||
        (strcmp(varType, "long double") == 0) )
    {
      inf = DBL_MIN;
      sup = DBL_MAX;
    }

    std::cerr << "\tFLT_MIN = " << FLT_MIN;
    std::cerr << "\tFLT_MAX = " << FLT_MAX;
    std::cerr << "\tDBL_MIN = " << DBL_MIN;
    std::cerr << "\tDBL_MAX = " << DBL_MAX;
    std::cerr << "\tLDBL_MIN = " << LDBL_MIN;
    std::cerr << "\tLDBL_MAX = " << LDBL_MAX;


    ap_texpr1_t* t_texpr = ap_texpr1_cst_interval_double(env, inf, sup);
    AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
      &(AbsValPtr->abstract_value), (char*) varName, t_texpr, NULL);
    ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
*/

    delete t_dim_real;
  }

}

/// \brief Get Apron dimension type (int or real) from const char* type
const char* getApronDimType(const char* varType)
{
  const char* apronVarType;

  if ((strcmp(varType, "int") == 0) ||
      (strcmp(varType, "const int") == 0) ||
      (strcmp(varType, "signed") == 0) ||
      (strcmp(varType, "signed int") == 0) ||
      (strcmp(varType, "unsigned") == 0) ||
      (strcmp(varType, "unsigned int") == 0) ||
      (strcmp(varType, "short") == 0) ||
      (strcmp(varType, "short int") == 0) ||
      (strcmp(varType, "signed short") == 0) ||
      (strcmp(varType, "signed short int") == 0) ||
      (strcmp(varType, "unsigned short") == 0) ||
      (strcmp(varType, "unsigned short int") == 0) ||
      (strcmp(varType, "long") == 0) ||
      (strcmp(varType, "long int") == 0) ||
      (strcmp(varType, "signed long") == 0) ||
      (strcmp(varType, "signed long int") == 0) ||
      (strcmp(varType, "unsigned long") == 0) ||
      (strcmp(varType, "unsigned long int") == 0) ||
      (strcmp(varType, "long long") == 0) ||
      (strcmp(varType, "long long int") == 0) ||
      (strcmp(varType, "signed long long") == 0) ||
      (strcmp(varType, "signed long long int") == 0) ||
      (strcmp(varType, "unsigned long long") == 0) ||
      (strcmp(varType, "unsigned long long int") == 0) ||
      (strcmp(varType, "char") == 0) ||
      (strcmp(varType, "signed char") == 0) ||
      (strcmp(varType, "unsigned char") == 0) )
  {
    apronVarType = "int";
  }
  else
  if ((strcmp(varType, "float") == 0) ||
      (strcmp(varType, "double") == 0) ||
      (strcmp(varType, "long double") == 0) )
  {
    apronVarType = "real";
  }

  return apronVarType;
}

/// \brief Set value for integer literal for apron
void setValueForIntegerLiteral(const char* varName, double dval,
  int numAssignment)
{
  ap_texpr1_t* t_tree_expr[1];
  long int lival = (long int) dval;
  t_tree_expr[0] = ap_texpr1_cst_scalar_int(env, lival);

  ap_texpr1_t* t_expr_ca;
  t_expr_ca = ap_texpr1_var(env, (char *)varName);

  if (numAssignment > 1)
  {
    addExpression(t_expr_ca, 0);
  }

  std::cout << "ap_texpr1_print() : ";
  ap_texpr1_print(t_tree_expr[0]);
  std::cout << "\n";

  AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
    &(AbsValPtr->abstract_value), (char*) varName, t_tree_expr[0], NULL);
  std::cout << "abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
}

/// \brief Set value for floating literal for apron
void setValueForFloatingLiteral(const char* varName, double dval,
  int numAssignment)
{
  ap_texpr1_t* t_tree_expr[1];
  t_tree_expr[0] = ap_texpr1_cst_scalar_double(env, dval);

  ap_texpr1_t* t_expr_ca;
  t_expr_ca = ap_texpr1_var(env, (char *)varName);

  if (numAssignment > 1)
  {
    addExpression(t_expr_ca, 1);
  }

  std::cout << "ap_texpr1_print() : ";
  ap_texpr1_print(t_tree_expr[0]);
  std::cout << "\n";

  AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
    &(AbsValPtr->abstract_value), (char*) varName, t_tree_expr[0], NULL);
  std::cout << "abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
}

/// \brief Set value for variable on RHS for apron
void setValueForRHSVariable(const char* varName, const char* rhsVarName,
  const char* varType, int numAssignment)
{
  ap_texpr1_t* t_tree_expr[1];
  t_tree_expr[0] = ap_texpr1_var(env, (char *) rhsVarName);

  ap_texpr1_t* t_expr_ca;
  t_expr_ca = ap_texpr1_var(env, (char *)varName);

  const char* apronVarType = getApronDimType(varType);
  int exprType = 0;

  // Add int variable to apron environment
  if (strcmp(apronVarType, "int") == 0)
  {
    exprType = 0;
  }
  else
  if (strcmp(apronVarType, "real") == 0)
  {
    exprType = 1;
  }

  if (numAssignment > 1)
  {
    addExpression(t_expr_ca, exprType);
  }

  std::cout << "ap_texpr1_print() : ";
  ap_texpr1_print(t_tree_expr[0]);
  std::cout << "\n";

  AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
    &(AbsValPtr->abstract_value), (char*) varName, t_tree_expr[0], NULL);
  std::cout << "abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
}

/// \brief Evaluate assignment for apron
void evaluateAssignment(const char* lhsVarName, int numAssignment,
  bool isLastStatement)
{
  MyApronExpr* t_node;
  t_node = removeExpression();
  ap_texpr1_t* t_expr;
  t_expr = t_node->data;

  ap_texpr1_t* t_expr_ca;
  t_expr_ca = ap_texpr1_var(env, (ap_var_t)lhsVarName);

  if (numAssignment > 1)
  {
    addExpression(t_expr_ca, t_node->expr_type);
    showExpression();
    std::cout << "\n";
  }

  std::cout << "\n";
  ap_texpr1_print(t_expr);
  std::cout << "\n";

  AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
    &(AbsValPtr->abstract_value), (char*)lhsVarName, t_expr, NULL);
  std::cout << " abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));

  // if assignment has happened, reset count for variable on LHS
  resetCountForVarOnLHS(lhsVarName);

  updateAbsValFromGlobalVarList();

  // if this is last statement of block, add current block abstract value
  // in AbstractMemory->blockAbsVal to be added into MyCFGInfo->blockAbsVal
  std::cerr << "\tisLastStatement = " << isLastStatement << "\n";

  if (isLastStatement)
  {
    AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
      &(AbsValPtr->abstract_value));
  }
}

/// \brief Reset count for a variable on LHS
void resetCountForVarOnLHS(const char* lhsVarName)
{
  VariableCount * newVar;

  for (int i = 0; i < globalVarList.size(); i++)
  {
    newVar = globalVarList[i];

    if (strcmp(lhsVarName, newVar->varName) == 0)
    {
      newVar->varCount = 0;
    }
  }
}

/// \brief Get integer literal for apron
void getIntegerLiteral(double dval)
{
  long int lival = (long int) dval;
  ap_texpr1_t* t_tree_expr = ap_texpr1_cst_scalar_int(env, lival);

  addExpression(t_tree_expr, 0);

  showExpression();
  std::cout << "\n";
}

/// \brief Get floating literal for apron
void getFloatingLiteral(double dval)
{
  ap_texpr1_t* t_tree_expr = ap_texpr1_cst_scalar_double(env, dval);

  addExpression(t_tree_expr, 1);

  showExpression();
  std::cout << "\n";
}

/// \brief Get variable for apron
void getVariable(const char* varName, const char* varType)
{
  int count;
  ap_texpr1_t * my_tree_expr[3];

  count = getCountForVar(varName);

  if (count != -1)
  {
    my_tree_expr[0] = ap_texpr1_cst_scalar_int(env, (long int)count);
    my_tree_expr[1] = ap_texpr1_var(env, (ap_var_t)varName);
    my_tree_expr[2] = ap_texpr1_binop(AP_TEXPR_ADD, my_tree_expr[1],
      my_tree_expr[0], AP_RTYPE_INT, AP_RDIR_ZERO);
  }

  ap_texpr1_t* t_tree_expr;
  t_tree_expr = ap_texpr1_copy(my_tree_expr[2]);

  const char* apronVarType = getApronDimType(varType);

  if (strcmp(apronVarType, "real") == 0)
  {
    addExpression(t_tree_expr, 1);
  }
  else
  {
    addExpression(t_tree_expr, 0);
  }

  showExpression();
  std::cout << "\n";
}

/// \brief Create single expression for binop for apron
void createSingleExpressionForBinOp(const char* opcode)
{
  // get both expressions

  MyApronExpr * t_node1, * t_node2;
  ap_texpr1_t * texpr1, * texpr2;
  int exprType1, exprType2;

  // first removeExpression() gives rhs (or second operand)
  // second removeExpression() gives lhs (or first operand)
  t_node1 = removeExpression();
  t_node2 = removeExpression();

  // texpr1 - first operand, texpr2 - second operand
  texpr1 = t_node2->data;
  texpr2 = t_node1->data;

  // For unary incr/decr operators
  int compare = ap_environment_compare(texpr1->env, texpr2->env);

  if (compare == -1)
  {
    //env of lhs is a subset of env of rhs
    texpr1 = ap_texpr1_extend_environment(texpr1, texpr2->env);
  }
  else
  if (compare == 1)
  {
    //env of rhs is a subset of env of lhs
    texpr2 = ap_texpr1_extend_environment(texpr2, texpr1->env);
  }

  // and find resultant expr_type
  exprType1 = t_node1->expr_type;
  exprType2 = t_node2->expr_type;

  if (exprType1 == 1 || exprType2 == 1)
    expr_type = 1;
  else
    expr_type = 0;

  // print opcode, lhs and rhs expr, expr_type
  std::cout << " opcode : " << opcode;
  std::cout << " \n";
  std::cout << " o ap_texpr1_print() : ";
  ap_texpr1_print(texpr1);
  std::cout << " \n";
  std::cout << " o ap_texpr1_print() : ";
  ap_texpr1_print(texpr2);
  std::cout << " \n";
  std::cout << " expr_type : " << expr_type;
  std::cout << " \n";

  // create a single expression from texpr1 and texpr2
  // for current opcode and expr_type
  // Note:- both operands of ap_texpr1_binop i.e. texpr1 and texpr2
  // are memory managed - dereferenced after the function call

  ap_texpr1_t* texpr_bo;

  if ((strcmp(opcode, "+") == 0) ||
      (strcmp(opcode, "+=") == 0) )
  {
    if (expr_type == 1)
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_ADD, texpr1, texpr2,
        AP_RTYPE_SINGLE, AP_RDIR_ZERO);
    }
    else
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_ADD, texpr1, texpr2,
        AP_RTYPE_INT, AP_RDIR_ZERO);
    }
  }
  else
  if ((strcmp(opcode, "-") == 0) ||
      (strcmp(opcode, "-=") == 0) )
  {
    if (expr_type == 1)
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_SUB, texpr1, texpr2,
        AP_RTYPE_SINGLE, AP_RDIR_ZERO);
    }
    else
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_SUB, texpr1, texpr2,
        AP_RTYPE_INT, AP_RDIR_ZERO);
    }
  }
  else
  if ((strcmp(opcode, "*") == 0) ||
      (strcmp(opcode, "*=") == 0) )
  {
    if (expr_type == 1)
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_MUL, texpr1, texpr2,
        AP_RTYPE_SINGLE, AP_RDIR_ZERO);
    }
    else
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_MUL, texpr1, texpr2,
        AP_RTYPE_INT, AP_RDIR_ZERO);
    }
  }
  else
  if ((strcmp(opcode, "/") == 0) ||
      (strcmp(opcode, "/=") == 0) )
  {
    if (expr_type == 1)
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_DIV, texpr1, texpr2,
        AP_RTYPE_SINGLE, AP_RDIR_ZERO);
    }
    else
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_DIV, texpr1, texpr2,
        AP_RTYPE_INT, AP_RDIR_ZERO);
    }
  }
  else
  if ((strcmp(opcode, "%") == 0) ||
      (strcmp(opcode, "%=") == 0) )
  {
    if (expr_type == 1)
    {
      std::cerr << "Error: real operand found for %, ";
      std::cerr << "binary operator mod cannot have real operands" << "\n";
    }
    else
    {
      texpr_bo = ap_texpr1_binop(AP_TEXPR_MOD, texpr1, texpr2,
        AP_RTYPE_INT, AP_RDIR_ZERO);
    }
  }

  std::cout << " +1 ap_texpr1_print() : ";
  ap_texpr1_print(texpr_bo);
  std::cout << "\n";

  // and now add this single apron expression to MyApronExpr list
  addExpression(texpr_bo, expr_type);

  showExpression();
  std::cout << "\n";
}

/// \brief Create constraint for binary relational operator for apron
void createConstraintForBinRelOp(const char* opcode)
{
  // create expression of form (lhs - rhs) for (lhs relop rhs)
  // e.g. for condition (a > b), create (a - b)
  createSingleExpressionForBinOp("-");

  // get expressions: (lhs - rhs) and -(lhs - rhs)
  // e.g. for condition (a > b), t_texpr = (a - b) and t_texpr_neg = -(a - b)
  MyApronExpr * t_last_expr;
  t_last_expr = removeExpression();

  ap_texpr1_t * t_texpr, * t_texpr_neg;
  t_texpr = t_last_expr->data;

  // copy texpr and apply unary negative to negate the expression
  t_texpr_neg = ap_texpr1_copy(t_texpr);
  t_texpr_neg = ap_texpr1_unop(AP_TEXPR_NEG, t_texpr_neg, AP_RTYPE_INT,
    AP_RDIR_ZERO);

  //if (int isScalar = ap_texpr1_is_scalar(t_texpr))
  if (int isScalar = ap_texpr1_is_interval_cst(t_texpr))
  {
    std::cout << "\nconditional expression is scalar, treated as true\n";
  }

  // let's see what we've got so far
  std::cout << " opcode : " << opcode << "\n";

  std::cout << " o ap_texpr1_print() : ";
  ap_texpr1_print(t_texpr);
  std::cout << " \n";

  std::cout << " o ap_texpr1_print() : ";
  ap_texpr1_print(t_texpr_neg);
  std::cout << " \n";

  std::cout << " abstract value: before " << opcode << "\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
  std::cout << " \n";

  // create tree constraints for t_texpr and t_texpr_neg
  ap_tcons1_t t_tcons, t_tcons_neg;
  int satisfy_texpr = 0;
  int satisfy_texpr_neg = 0;

  /* Note:-
   * ap_tconst1_t ap_tcons1_make(ap_constyp_t constyp, ap_texpr1_t* expr,
   *      ap_scalar_t* scalar)
   * - creates a constraint of given type with the given expression.
   * enum ap_constyp_t    : type of constraints
   *      AP_CONS_SUPEQ   : >= constraint
   *      AP_CONS_SUP     : > constraint
   *      AP_CONS_EQ      : equality constraint
   *      AP_CONS_DISEQ   : disequality constraint
   */

  if (strcmp(opcode, ">") == 0)
  {
    t_tcons = ap_tcons1_make(AP_CONS_SUP, t_texpr, NULL);

    t_tcons_neg = ap_tcons1_make(AP_CONS_SUPEQ, t_texpr_neg, NULL);
  }
  else
  if (strcmp(opcode, ">=") == 0)
  {
    t_tcons = ap_tcons1_make(AP_CONS_SUPEQ, t_texpr, NULL);

    t_tcons_neg = ap_tcons1_make(AP_CONS_SUP, t_texpr_neg, NULL);
  }
  else
  if (strcmp(opcode, "<") == 0)
  {
    t_tcons = ap_tcons1_make(AP_CONS_SUP, t_texpr_neg, NULL);

    t_tcons_neg = ap_tcons1_make(AP_CONS_SUPEQ, t_texpr, NULL);
  }
  else
  if (strcmp(opcode, "<=") == 0)
  {
    t_tcons = ap_tcons1_make(AP_CONS_SUPEQ, t_texpr_neg, NULL);

    t_tcons_neg = ap_tcons1_make(AP_CONS_SUP, t_texpr, NULL);
  }
  else
  if (strcmp(opcode, "==") == 0)
  {
    t_tcons = ap_tcons1_make(AP_CONS_EQ, t_texpr, NULL);

    t_tcons_neg = ap_tcons1_make(AP_CONS_DISEQ, t_texpr_neg, NULL);
  }
  else
  if (strcmp(opcode, "!=") == 0)
  {
    t_tcons = ap_tcons1_make(AP_CONS_DISEQ, t_texpr, NULL);

    t_tcons_neg = ap_tcons1_make(AP_CONS_EQ, t_texpr_neg, NULL);
  }
  else
  {
    fprintf(stderr, "Unknown operator %s", opcode);
  }

  // what are t_tcons and t_tcons_neg?
  // condition  : t_tcons       : t_tcons_neg
  // -----------:---------------:---------------
  // a >  b     : a - b >  0    : -(a - b) >= 0
  // a >= b     : a - b >= 0    : -(a - b) >  0
  // a <  b     : -(a - b) >  0 : a - b >= 0
  // a <= b     : -(a - b) >= 0 : a - b >  0
  // a == b     : a - b  = 0    : a - b != 0
  // a != b     : a - b != 0    : a - b  = 0
  
  std::cout << "\nt_tcons\n";
  ap_tcons1_fprint(stdout, &t_tcons);
  std::cout << "\nt_tcons_neg\n";
  ap_tcons1_fprint(stdout, &t_tcons_neg);
  std::cout << "\n";

  updateAbsValFromGlobalVarList();

  // does the abstract value (before relational op) satisfy this constraint
  satisfy_texpr = ap_abstract1_sat_tcons(man, &(AbsValPtr->abstract_value),
    &t_tcons);
  std::cout << "\nsatisfy_texpr = " << satisfy_texpr << "\n";

  satisfy_texpr_neg = ap_abstract1_sat_tcons(man, &(AbsValPtr->abstract_value),
    &t_tcons_neg);
  std::cout << "\nsatisfy_texpr_neg = " << satisfy_texpr_neg << "\n";
 
  // fill tree constraints array with tree constraints
  // in order to generate abstract value for debug purpose
  // and deallocate tree constraints array
  ap_tcons1_array_t t_consarray;
  ap_abstract1_t t_abs_value;
  ap_abstract1_t t_abs_value_neg;

  // e.g. for condition (a > b), create tconsarray (a - b) == 0
  t_consarray = ap_tcons1_array_make(env, 1);
  ap_tcons1_array_set(&t_consarray, 0, &t_tcons);

  t_abs_value = ap_abstract1_of_tcons_array(man, env, &t_consarray);
  std::cout << " abstract value: t_abs_value\n";
  ap_abstract1_fprint(stdout, man, &t_abs_value);
  std::cout << " \n";

  ap_tcons1_array_clear(&t_consarray);

  // e.g. for condition (a > b), create tconsarray -(a - b) == 0
  t_consarray = ap_tcons1_array_make(env, 1);
  ap_tcons1_array_set(&t_consarray, 0, &t_tcons_neg);

  t_abs_value_neg = ap_abstract1_of_tcons_array(man, env, &t_consarray);
  std::cout << " abstract value: t_abs_value_neg\n";
  ap_abstract1_fprint(stdout, man, &t_abs_value_neg);
  std::cout << " \n";

  ap_tcons1_array_clear(&t_consarray);

  // set AbstractMemory - current abstract value for block, condition
  // negation of condition

  addCondExprAbsVal(t_abs_value, t_abs_value_neg);
  showCondExprAbsVal();
}

/// \brief Set abstract values for while(1) and while(0) cases
void setAbstractMemory(int positive)
{
  // set AbstractMemory - current abstract value for block, condition,
  // negation of condition

  if (positive == 1)
  {
    // condition is: while (1)
    AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
      &(AbsValPtr->abstract_value));
    
    addCondExprAbsVal(ap_abstract1_top(man, env),
      ap_abstract1_bottom(man, env));
  }
  else
  if (positive == 0)
  {
    // condition is: while (0)
    AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
      &(AbsValPtr->abstract_value));

    addCondExprAbsVal(ap_abstract1_bottom(man, env),
      ap_abstract1_top(man, env));
  }
  else
  if (positive == 2)
  {
    // condition is: while (x)
    AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
      &(AbsValPtr->abstract_value));

    addCondExprAbsVal(ap_abstract1_top(man, env),
      ap_abstract1_top(man, env));
  }

  std::cerr << "\nAbstractMemory\n";
  ap_abstract1_fprint(stderr, man, &(AbstractMemoryPtr->blockAbsVal));
  showCondExprAbsVal();
  std::cout << "\n";
}

/// \brief Update abstract values for logical not
void updateAbstractValuesForLNot()
{
  CondExprAbsVal* t_CondExprAbsVal;
  t_CondExprAbsVal = removeCondExprAbsVal();

  ap_abstract1_t t_absVal, t_negAbsVal;
  t_absVal = t_CondExprAbsVal->condAbsVal;
  t_negAbsVal = t_CondExprAbsVal->negCondAbsVal;

  addCondExprAbsVal(t_negAbsVal, t_absVal);

  std::cerr << "\n  after swapping condAbsVal and negCondAbsval for LNot !\n";
  showCondExprAbsVal();
  std::cout << "\n";
}

/// \brief Get integer literal (unary op) for apron
void getIntegerLiteralUnaryOp(double dval, const char* opcode)
{
  long int lival = (long int) dval;

  ap_texpr1_t* t_tree_expr[1];
  t_tree_expr[0] = ap_texpr1_cst_scalar_int(env, lival);

  ap_texpr1_t* t_texpr1;

  // add expression with integer expr_type 0
  if (strcmp(opcode, "-") == 0)
  {
    t_texpr1 = ap_texpr1_unop(AP_TEXPR_NEG, t_tree_expr[0],
      AP_RTYPE_INT, AP_RDIR_ZERO);
    addExpression(t_texpr1, 0);
  }
  else
  if (strcmp(opcode, "+") == 0)
  {
    addExpression(t_tree_expr[0], 0);
  }

}

/// \brief Get floating literal (unary op) for apron
void getFloatingLiteralUnaryOp(double dval, const char* opcode)
{
  ap_texpr1_t* t_tree_expr[1];
  t_tree_expr[0] = ap_texpr1_cst_scalar_double(env, dval);

  ap_texpr1_t* t_texpr1;

  // add expression with real expr_type 1
  if (strcmp(opcode, "-") == 0)
  {
    t_texpr1 = ap_texpr1_unop(AP_TEXPR_NEG, t_tree_expr[0],
      AP_RTYPE_SINGLE, AP_RDIR_ZERO);
    addExpression(t_texpr1, 1);
  }
  else
  if (strcmp(opcode, "+") == 0)
  {
    addExpression(t_tree_expr[0], 1);
  }

}

/// \brief Get variable of unary op for apron
void getVariableUnaryOp(const char* varName, const char* varType,
  const char* opcode)
{
  ap_texpr1_t* t_tree_expr[1];
  t_tree_expr[0] = ap_texpr1_var(env, (char *) varName);

  ap_texpr1_t* t_texpr1;
  int exprType = 0;
  const char* apronVarType = getApronDimType(varType);

  // add expression with appropriate expr_type
  if (strcmp(opcode, "-") == 0)
  {
    if (strcmp(apronVarType, "real") == 0)
    {
      t_texpr1 = ap_texpr1_unop(AP_TEXPR_NEG, t_tree_expr[0],
        AP_RTYPE_SINGLE, AP_RDIR_ZERO);
      exprType = 1;
    }
    else if (strcmp(apronVarType, "int") == 0)
    {
      t_texpr1 = ap_texpr1_unop(AP_TEXPR_NEG, t_tree_expr[0],
        AP_RTYPE_INT, AP_RDIR_ZERO);
      exprType = 0;
    }

    addExpression(t_texpr1, exprType);
  }
  else
  if (strcmp(opcode, "+") == 0)
  {
    if (strcmp(apronVarType, "real") == 0)
    {
      exprType = 1;
    }
    else if (strcmp(apronVarType, "int") == 0)
    {
      exprType = 0;
    }

    addExpression(t_tree_expr[0], exprType);
  }

}

/// \brief Create single expression for unary op for apron
void createSingleExpressionForUnaryOp(const char* opcode)
{
  // unary - : get the expression, negate it and add to MyApronExpr list
  // e.g -(x + 1)
  // unary + : get the expression and add to MyApronExpr list
  // e.g +(x + 1)

  MyApronExpr* t_node;
  t_node = removeExpression();

  ap_texpr1_t* t_tree_expr[1];
  t_tree_expr[0] = t_node->data;

  ap_texpr1_t* t_texpr1;
  int exprType = 0;

  // add expression with appropriate expr_type
  if (strcmp(opcode, "-") == 0)
  {
    if (t_node->expr_type == 1)
    {
      t_texpr1 = ap_texpr1_unop(AP_TEXPR_NEG, t_tree_expr[0],
        AP_RTYPE_SINGLE, AP_RDIR_ZERO);
      exprType = 1;
    }
    else
    {
      t_texpr1 = ap_texpr1_unop(AP_TEXPR_NEG, t_tree_expr[0],
        AP_RTYPE_INT, AP_RDIR_ZERO);
      exprType = 0;
    }

    addExpression(t_texpr1, exprType);

  }
  else
  if (strcmp(opcode, "+") == 0)
  {
    if (t_node->expr_type == 1)
    {
      exprType = 1;
    }
    else if (t_node->expr_type == 0)
    {
      exprType = 0;
    }

    addExpression(t_tree_expr[0], exprType);

  }

  std::cerr << "\n unary expression:\n";
  showExpression();
  std::cerr << "\n";

}

/// \brief For PostIncrement, Increment the Value of a variable and create
/// appropriate tree expression
void doPostIncrement(const char* varName, int numAssignment)
{
  ap_texpr1_t* my_tree_expr[3];
  int count;
  long int lival = 1;
  ap_texpr1_t* texpr1_uo = NULL;

  // count for variable in global var list
  count = getCountForVar(varName);

  my_tree_expr[0] = ap_texpr1_cst_scalar_int(env, (long int)count);
  my_tree_expr[1] = ap_texpr1_var(env, (ap_var_t)varName);
  my_tree_expr[2] = ap_texpr1_binop(AP_TEXPR_ADD, my_tree_expr[1],
    my_tree_expr[0], AP_RTYPE_INT, AP_RDIR_ZERO);

  // push onto stack varName+count
  texpr1_uo = ap_texpr1_copy(my_tree_expr[2]);

  count = count +1;

  // set count for variable
  if (setCountForVar(varName, count) != -1)
  {
    printGlobalVarList();
  }


  // checking if the unary operator is a part of the binary assignment
  if (numAssignment > 0)
  {
    addExpression(texpr1_uo, 0);
    showExpression();
  }
}

/// \brief For PreIncrement, Increment the Value of a variable and create
/// appropriate tree expression
void doPreIncrement(const char* varName, int numAssignment)
{
  ap_texpr1_t* my_tree_expr[3];
  int count;
  long int lival = 1;
  ap_texpr1_t* texpr1_uo = NULL;

  count = getCountForVar(varName);

  my_tree_expr[0] = ap_texpr1_cst_scalar_int(env, (long int)(count+1));
  my_tree_expr[1] = ap_texpr1_var(env, (ap_var_t)varName);
  my_tree_expr[2] = ap_texpr1_binop(AP_TEXPR_ADD, my_tree_expr[1],
    my_tree_expr[0], AP_RTYPE_INT, AP_RDIR_ZERO);

  texpr1_uo = ap_texpr1_copy(my_tree_expr[2]);

  count = count +1;

  if (setCountForVar(varName, count) != -1)
  {
    printGlobalVarList();
  }

  // checking if the unary operator is a part of the binary assignment
  if (numAssignment > 0)
  {
    addExpression(texpr1_uo, 0);
    showExpression();
  }
}

/// \brief PreDecrement Expression creation and value Update function
void doPreDecrement(const char* varName, int numAssignment)
{
  ap_texpr1_t* my_tree_expr[3];
  int count;
  long int lival = 1;
  ap_texpr1_t* texpr1_uo = NULL;

  count = getCountForVar(varName);

  my_tree_expr[0] = ap_texpr1_cst_scalar_int(env, (long int)(count-1));
  my_tree_expr[1] = ap_texpr1_var(env, (ap_var_t)varName);
  my_tree_expr[2] = ap_texpr1_binop(AP_TEXPR_ADD, my_tree_expr[1],
    my_tree_expr[0], AP_RTYPE_INT, AP_RDIR_ZERO);

  texpr1_uo = ap_texpr1_copy(my_tree_expr[2]);

  count = count -1;

  if (setCountForVar(varName, count) != -1)
  {
    printGlobalVarList();
  }

  // checking if the unary operator is a part of the binary assignment
  if (numAssignment > 0)
  {
    addExpression(texpr1_uo, 0);
    showExpression();
  }
}

/// \brief PostDecrement Expression creation and value Update function
void doPostDecrement(const char* varName, int numAssignment)
{
  ap_texpr1_t* my_tree_expr[3];
  int count;
  long int lival = 1;
  ap_texpr1_t* texpr1_uo = NULL;

  count = getCountForVar(varName);

  my_tree_expr[0] = ap_texpr1_cst_scalar_int(env, (long int)(count));
  my_tree_expr[1] = ap_texpr1_var(env, (ap_var_t)varName);
  my_tree_expr[2] = ap_texpr1_binop(AP_TEXPR_ADD, my_tree_expr[1],
    my_tree_expr[0], AP_RTYPE_INT, AP_RDIR_ZERO);

  texpr1_uo = ap_texpr1_copy(my_tree_expr[2]);

  count = count -1;

  if (setCountForVar(varName, count) != -1)
  {
    printGlobalVarList();
  }

  // checking if the unary operator is a part of the binary assignment
  if (numAssignment > 0)
  {
    addExpression(texpr1_uo, 0);
    showExpression();
  }
}

/// \brief Function to add a new temporary variable, assign abstract value
///  and return name of temporary variable added
char * addTemporaryVariables(const char* varName, int a)
{
  // varName is the variable for which a temporary variable is being created
  // and new temporary variable name is stored in newVarName as __tmp_<mycount>

  // construct temporary variable name
  std::stringstream ss;
  ss  << "__tmp_" << mycount;

  mycount++;

  std::string s = ss.str();
  const char* newVarName = s.c_str();

  char** unary_dummy;
  unary_dummy = (char**) new char*;
  unary_dummy[0] = strdup((char*) newVarName);

  // add temporary variable to env
  initCountForVar(*(unary_dummy));
  printGlobalVarList();

  env = ap_environment_add(env, (ap_var_t*) unary_dummy, 1, NULL, 0);

  AbsValPtr->abstract_value = ap_abstract1_change_environment(man, true,
    &(AbsValPtr->abstract_value), env, false);

  ap_texpr1_t* t_tree_expr;
  t_tree_expr = ap_texpr1_var(env, ((char *) varName));
  ap_texpr1_print(t_tree_expr);
  std::cout<<"\n";

  // update abstract value
  AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
    &(AbsValPtr->abstract_value), (char*)s.c_str(), t_tree_expr, NULL);

  std::cout << "abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));

  // return name of temporary variable added
  return (char*)newVarName;
}

/// \brief Function to remove temp variables
void removeTemporaryVariables()
{
  std::string str1 ("__tmp_");

  // get (initially) number of variables in env
  int total_var = (env->intdim + env->realdim);
  int i = 0;

  ap_var_t my_var;

  char* charptr;

  while (i < total_var)
  {
    // get temporary variable name
    my_var =  ap_environment_var_of_dim(env, i);

    std::string str2;
    charptr = (char *)my_var;

    if (memcmp(charptr, str1.data(), 6) == 0)
    {
      // remove variable from env and update abstract value
      env = ap_environment_remove(env, &(my_var), 1);

      AbsValPtr->abstract_value = ap_abstract1_change_environment(man, true,
        &(AbsValPtr->abstract_value), env, false);

      i =-1;

      // reset number of variables in env
      total_var = (env->intdim + env->realdim);
    }

    i++;
  }

  // re-initialize mycount - number of temporary variables
  mycount = 0;

  std::cout << "abstract value:\n";
  ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));
}

/// \brief Create constraint for binary shift operator for apron
void createSingleExpressionForBinShiftOp(const char* opcode,
  int numAssignment)
{
  // get both expressions

  MyApronExpr * t_node1, * t_node2;
  ap_texpr1_t * texpr1, * texpr2;
  int exprType1, exprType2;

  // first removeExpression() gives rhs (or second operand)
  // second removeExpression() gives lhs (or first operand)
  t_node1 = removeExpression();
  t_node2 = removeExpression();

  // texpr1 - first operand, texpr2 - second operand
  texpr1 = t_node2->data;
  texpr2 = t_node1->data;

  // For unary incr/decr operators
  int compare = ap_environment_compare(texpr1->env, texpr2->env);

  if (compare == -1)
  {
    //env of lhs is a subset of env of rhs
    texpr1 = ap_texpr1_extend_environment(texpr1, texpr2->env);
  }
  else
  if (compare == 1)
  {
    //env of rhs is a subset of env of lhs
    texpr2 = ap_texpr1_extend_environment(texpr2, texpr1->env);
  }

  // and find resultant expr_type
  exprType1 = t_node1->expr_type;
  exprType2 = t_node2->expr_type;

  if (exprType1 == 1 || exprType2 == 1)
    expr_type = 1;
  else
    expr_type = 0;

  // print opcode, lhs and rhs expr, expr_type
  std::cout << " opcode : " << opcode;
  std::cout << " \n";
  std::cout << " o ap_texpr1_print() : ";
  ap_texpr1_print(texpr1);
  std::cout << " \n";
  std::cout << " o ap_texpr1_print() : ";
  ap_texpr1_print(texpr2);
  std::cout << " \n";
  std::cout << " expr_type : " << expr_type;
  std::cout << " \n";

  // call to create temporary variable
  addPseudoVariableRel(numAssignment);
}

/// \brief Creates a pseudo variable and puts this expression onto the stack
void addPseudoVariableRel(int numAssignment)
{
  // construct temporary variable name
  std::stringstream ss;
  ss  << "__tmp_" << mycount;

  mycount++;

  std::string s = ss.str();
  const char* newVarName = s.c_str();

  char** unary_dummy;
  unary_dummy = (char**) new char*;
  unary_dummy[0] = strdup((char*) newVarName);

  // add temporary variable to env
  env = ap_environment_add(env, (ap_var_t*) unary_dummy, 1, NULL, 0);

  AbsValPtr->abstract_value = ap_abstract1_change_environment(man, true,
    &(AbsValPtr->abstract_value), env, false);

  ap_texpr1_t* t_tree_expr;
  t_tree_expr = ap_texpr1_var(env, ((char *) newVarName));

  // The expression is added onto the stack only when there is
  // a pending assignment to be done
  if (numAssignment != 0)
  {
    addExpression(t_tree_expr, 1);
  }
}

/// \brief Evaluate return statement for apron - just remove expression
void evaluateReturn()
{
  MyApronExpr* t_node;
  t_node = removeExpression();

  //showExpression();
  //std::cout << "\n";

  // as this is last statement of block, add current block abstract value
  // in AbstractMemory->blockAbsVal to be added into MyCFGInfo->blockAbsVal
  AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
    &(AbsValPtr->abstract_value));
}

/// \brief Get the count value for a particular variable
int getCountForVar(const char * varName)
{
  int count = -1;

  VariableCount * newVar;

  for (int i = 0; i < globalVarList.size(); i++)
  {
    newVar = globalVarList[i];

    if (strcmp(varName, newVar->varName) == 0)
    {
      return newVar->varCount;
    }
  }

  return count;
}

/// \brief Set the count value for a particular variable
/// Return -1 if variable not found
int setCountForVar(const char * varName, int varCount)
{
  int count = -1;

  VariableCount * newVar;

  for (int i = 0; i < globalVarList.size(); i++)
  {
    newVar = globalVarList[i];

    if (strcmp(varName, newVar->varName) == 0)
    {
      newVar->varCount = varCount;
      return 0;
    }
  }

  return count;
}

/// \brief Initialize the count value for a particular variable
/// Return -1 if variable already present
int initCountForVar(const char* varName)
{
  VariableCount * newVar = (VariableCount *) new VariableCount;

  newVar->varName = strdup(varName);
  newVar->varCount = 0;
  globalVarList.push_back(newVar);

  return 0;
}

/// \brief Prints the Global Var List
int printGlobalVarList()
{
  std::cout << "\nPrinting globalVarList : \n";

  VariableCount * newVar;

  for (int i = 0; i < globalVarList.size(); i++)
  {
    newVar = globalVarList[i];
    std::cout << "Varname = " << newVar->varName;
    std::cout << "\t\t Count = " << newVar->varCount << "\n";
  }

  std::cout << "\n\n";
  return 0;
}

/// \brief Updates all Variables in the environment
///  according to the Global VarList
int updateAbsValFromGlobalVarList()
{
  VariableCount * newVar;
  ap_texpr1_t* my_tree_expr[3];
  int count;
  char * varName;

  //std::cout << "\nUpdating Abstract Values : \n";

  for (int i = 0; i < globalVarList.size(); i++)
  {
    newVar = globalVarList[i];
    varName = strdup(newVar->varName);
    count = newVar->varCount;

    if (count != 0)
    {
      std::cout << "Updating Variable = " << newVar->varName;
      std::cout << "\t\t with Count = " << newVar->varCount << "\n";

      my_tree_expr[0] = ap_texpr1_cst_scalar_int(env, (long int)(count));
      my_tree_expr[1] = ap_texpr1_var(env, (ap_var_t)varName);
      my_tree_expr[2] = ap_texpr1_binop(AP_TEXPR_ADD, my_tree_expr[1],
        my_tree_expr[0], AP_RTYPE_INT, AP_RDIR_ZERO);

      AbsValPtr->abstract_value = ap_abstract1_assign_texpr(man, true,
        &(AbsValPtr->abstract_value), (char*) varName, my_tree_expr[2], NULL);
      std::cout << "abstract value:\n";
      ap_abstract1_fprint(stdout, man, &(AbsValPtr->abstract_value));

      newVar->varCount = 0;
    }
  }

  std::cout << "\n\n";
  return 0;
}

