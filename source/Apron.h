//
// Apron.h
//

/// \file
/// \brief Defines Apron related declarations and definitions
///
#include <iostream>
#include <vector>

#ifndef APRON_H
# define APRON_H

#include <ap_global0.h>
#include <ap_global1.h>
#include <box.h>
#include <oct.h>
#include <pk.h>

/// \brief Apron manager
extern ap_manager_t* man;

/// \brief Apron environment
extern ap_environment_t* env;

/// \brief Structure to store abstract value
typedef struct AbsVal
{
  /// \brief Current abstract value at any point in program
  ap_abstract1_t abstract_value;
} AbsVal;

/// \brief Pointer to structre to hold abstract value
extern AbsVal* AbsValPtr;

/// \brief Structure to store Abstract Memory
typedef struct AbstractMemory
{
  /// \brief Abstract value of a CFGBlock
  ap_abstract1_t blockAbsVal;

  /// \brief Abstract value of positive of condition
  ap_abstract1_t condAbsVal;

  /// \brief Abstract value of negative of condition
  ap_abstract1_t negCondAbsVal;
} AbstractMemory;

/// \brief Pointer to structure to store Abstract Memory
extern AbstractMemory* AbstractMemoryPtr;

/// \brief Apron type (int/real) of current expression
extern int expr_type;

/// \brief Structure to store apron type expressions (ap_texpr1_t*)
/// in the current clang statement (Stmt*) and expr_type can be
/// 1 - DOUBLE or 0 - INT
typedef struct MyApronExpr
{
  /// \brief Apron type tree expression for an expression
  ap_texpr1_t* data;

  /// \brief expression type 0-int, 1-real
  int expr_type;

  // \brief Pointer to next MyApronExpr
  struct MyApronExpr* next;
} MyApronExpr;

/// \brief MyApronExpr list
extern MyApronExpr* head;

/// \brief Structure to store abstract values - condAbsVal, negCondAbsVal
/// for conditional expressions
typedef struct CondExprAbsVal
{
  /// \brief Abstract value of positive of condition
  ap_abstract1_t condAbsVal;

  /// \brief Abstract value of negative of condition
  ap_abstract1_t negCondAbsVal;

  /// \brief Pointer to next CondExprAbsVal
  struct CondExprAbsVal* next;
} CondExprAbsVal;

/// \brief Pointer to CondExprAbsVal list
extern CondExprAbsVal* absval_head;

/// \brief Structure to hold the current count of a particular variable
/// to ensure the correct expression goes onto the stack in the case of
/// increment/decrement operators
typedef struct VariableCount
{
  /// brief varName
  const char * varName;

  /// \brief count of ??
  int varCount;
} VariableCount;

/// \brief Vector containg pointers to VariableCount Structure
typedef std::vector <VariableCount * > VariableCountList;

extern VariableCountList globalVarList;

/// \brief Add apron type expression to MyApronExpr list
/// @param expression - Apron type expression
/// @param expr_type - type of expression int(0) or real(1)
/// @return void (nothing)
void addExpression(ap_texpr1_t* expression,
  int expr_type);

/// \brief Remove apron type expression from MyApronExpr list
/// @param none
/// @return MyApronExpr* - Apron type expression with expression type
MyApronExpr* removeExpression();

/// \brief Show apron type expressions of MyApronExpr list
/// @param none
/// @return void (nothing)
void showExpression();

/// \brief Add condAbsVal and negCondAbsVal to CondExprAbsVal list
/// @param absVal - Abstract value of positive of condition
/// @param negAbsVal - Abstract value of negative of condition
/// @return void (nothing)
void addCondExprAbsVal(ap_abstract1_t absVal,
  ap_abstract1_t negAbsVal);

/// \brief Remove condAbsVal and negCondAbsVal from CondExprAbsVal list
/// @param none
/// @return CondExprAbsVal* - Abstract value of positive and
///         negative of condition
CondExprAbsVal* removeCondExprAbsVal();

/// \brief Show condAbsVal and negCondAbsVal from CondExprAbsVal list
/// @param none
/// @return void (nothing)
void showCondExprAbsVal();

/// \brief get abstract domain choice and set empty environment
/// @param none
/// @return void (nothing)
void initApron();

/// \brief Get variable name and type for apron library and add to environment
/// @param varName - variable name
/// @param varType - variable type
/// @return void (nothing)
void getVarNameAndType(const char* varName,
  const char* varType);

/// \brief Get Apron dimension type (int or real) from const char* type
/// @param varType - variable type
/// @return const char* - Apron dimension type - int / real
const char* getApronDimType(const char* varType);

/// \brief Set value for integer literal for apron
/// @param varName - variable name
/// @param dval - integer literal value
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void setValueForIntegerLiteral(const char* varName,
  double dval,
  int numAssignment);

/// \brief Set value for floating literal for apron
/// @param varName - variable name
/// @param dval - floating literal value
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void setValueForFloatingLiteral(const char* varName,
  double dval,
  int numAssignment);

/// \brief Set value for variable on RHS for apron
/// @param varName - variable name
/// @param rhsVarName - variable name on rhs
/// @param varType - variable type
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void setValueForRHSVariable(const char* varName,
  const char* rhsVarName,
  const char* varType,
  int numAssignment);

/// \brief Evaluate assignment for apron
/// @param lhsVarName - variable name on lhs
/// @param numAssignment - reference count for number of assignments
/// @param isLastStatement - is current statement last statement of the block
/// @return void (nothing)
void evaluateAssignment(const char* lhsVarName,
  int numAssignment,
  bool isLastStatement);

/// \brief Reset count for a variable on LHS
void resetCountForVarOnLHS(const char* lhsVarName);

/// \brief Get integer literal for apron
/// @param dval - integer literal value
/// @return void (nothing)
void getIntegerLiteral(double dval);

/// \brief Get floating literal for apron
/// @param dval - floating literal value
/// @return void (nothing)
void getFloatingLiteral(double dval);

/// \brief Get variable for apron
/// @param varName - variable name
/// @param varType - variable type
/// @return void (nothing)
void getVariable(const char* varName,
  const char* varType);

/// \brief Create single expression for binop for apron
/// @param opcode - binary arithmetic opcode
/// @return void (nothing)
void createSingleExpressionForBinOp(const char* opcode);

/// \brief Create constraint for binary relational operator for apron
/// @param opcode - binary relational opcode
/// @return void (nothing)
void createConstraintForBinRelOp(const char* opcode);

/// \brief Set abstract values for while(1) and while(0) cases
/// @param positive - condition is non-zero scalar (1) or zero (0)
/// @return void (nothing)
void setAbstractMemory(int positive);

/// \brief Update abstract values for logical not
/// @param none
/// @return void (nothing)
void updateAbstractValuesForLNot();

/// \brief Get integer literal (unary op) for apron
/// @param dval - integer literal value
/// @param opcode - unary opcode
/// @return void (nothing)
void getIntegerLiteralUnaryOp(double dval,
  const char* opcode);

/// \brief Get floating literal (unary op) for apron
/// @param dval - floating literal value
/// @param opcode - unary opcode
/// @return void (nothing)
void getFloatingLiteralUnaryOp(double dval,
  const char* opcode);

/// \brief Get variable of unary op for apron
/// @param varName - variable name
/// @param varType - variable type
/// @param opcode - unary opcode
/// @return void (nothing)
void getVariableUnaryOp(const char* varName,
  const char* varType,
  const char* opcode);

/// \brief Create single expression for unary op for apron
/// @param opcode - unary opcode
/// @return void (nothing)
void createSingleExpressionForUnaryOp(const char* opcode);

/// \brief PostIncrement Expression creation and value Update function
/// @param varName - variable name
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void doPostIncrement(const char* varName,
  int numAssignment);

/// \brief PreIncrement Expression creation and value Update function
/// @param varName - variable name
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void doPreIncrement(const char* varName,
  int numAssignment);

/// \brief PostDecrement Expression creation and value Update function
/// @param varName - variable name
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void doPostDecrement(const char* varName,
  int numAssignment);

/// \brief PreDecrement Expression creation and value Update function
/// @param varName - variable name
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void doPreDecrement(const char* varName,
  int numAssignment);

/// \brief Function to add a new temporary variable, assign abstract value
///  and return name of temporary variable added
/// @param varName - variable name
/// @param a - 
/// @return char* - 
char* addTemporaryVariables(const char* varName,
  int a);

/// \brief Function to remove temp variables
/// @param none
/// @return void (nothing)
void removeTemporaryVariables();

/// \brief Create constraint for binary shift operator for apron
/// @param opcode - binary shift opcode
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void createSingleExpressionForBinShiftOp(const char* opcode,
  int numAssignment);

/// \brief Creates a pseudo variable and puts this expression onto the stack
/// depending on the value of numAssignment
/// @param numAssignment - reference count for number of assignments
/// @return void (nothing)
void addPseudoVariableRel(int numAssignment);

/// \brief Evaluate return statement for apron - just remove expression
/// @param none
/// @return void (nothing)
void evaluateReturn();

/// \brief Get the count value for a particular variable
/// @param varName - variable name
/// @return int - 
int getCountForVar(const char * varName);

/// \brief Set the count value for a particular variable
/// Return -1 if variable not found
/// @param varName - variable name
/// @param varCount - 
/// @return int - 
int setCountForVar(const char * varName,
  int varCount);

/// \brief Initialize the count value for a particular variable
/// Return -1 if variable already present
/// @param varName - variable name
/// @return int - 
int initCountForVar(const char * varName);

/// \brief Prints the Global Var List
/// @param none
/// @return int - 
int printGlobalVarList();

/// \brief Updates all Variables in the environment
///  according to the Global VarList
/// @param none
/// @return int - 
int updateAbsValFromGlobalVarList();

#endif // APRON_H

