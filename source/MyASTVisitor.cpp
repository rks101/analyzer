//
// MyASTVisitor.cpp
//

/// \file
/// \brief Defines AST node visitor class MyASTVisitor inherited from
/// clang::RecursiveASTVisitor
///
#include <iostream>
#include <string>
#include <iomanip>

#include "MyASTVisitor.h"
#include "MyProcessStmt.h"

/// \brief Vector of MyCFGInfo records
MyCFGInfoList myCFGInfoList;

/// \brief List of edges in the top-down flow graph
edgeStructList myEdgeStructList;

/// \brief source and destination block pointers to keep track of callee
/// and called root nodes in the DFS
CFGBlock * currentSourceBlock;
CFGBlock * currentDestBlock;

/// \brief 2d array to hold the adjacency matrix
std::vector <std::vector <int> > adjacencyMatrix;
std::vector <std::vector <edgeStruct*> > edgeMatrix;

/// \brief Keep track of blocks to visit
int* blocksToVisit;

/// \brief Number of times loop unrolling to be done to delay widening
int NumUnrollings = 5;

// add brief description for all declarations
int myNumBlocks;
t_blockStructList globalBlockList;
t_loopStructList loopStructList;

/// \brief CFG block list to traverse
std::vector<clang::CFGBlock *> blockList;
std::vector<clang::CFGBlock *> tempblockList;
std::vector<clang::CFGBlock *> myTempBlockList;

t_loopStruct * currentLoop;
clang::CFGBlock* exit_block;
int * myBlocksToVisit;

std::vector<int > myindex;
std::vector<int > lowlink;
std::vector<clang::CFGBlock *> S;
int indexcount;
std::vector<bool > blocked;
int se;

/// \brief Set compiler instance
void MyASTVisitor::setCompilerInstance(clang::CompilerInstance* ci)
{
  m_compilerInstance = ci;

  // Pointer to list of structure MySuccOfPredInfo
  SuccOfPredInfo = NULL;
}

/// \brief Get terminator type (If/While/Do/For/None) for basic block
std::string MyASTVisitor::getTerminatorType(clang::CFGBlock* cfg_block)
{
  // Get CFGTerminator object
  clang::CFGTerminator terminator = cfg_block->getTerminator();

  // Get statement of Terminator
  clang::Stmt* s = terminator.getStmt();

  // Note:- ENTRY and EXIT block of CFG will have terminator type as "Empty"

  std::string terminator_type = "None";

  if (s)
  {
    std::string termStmtClass = s->getStmtClassName();

    if (strcmp(termStmtClass.c_str(), "IfStmt") == 0)
    {
      terminator_type = "If";
    }
    else
    if (strcmp(termStmtClass.c_str(), "WhileStmt") == 0)
    {
      terminator_type = "While";
    }
    else
    if (strcmp(termStmtClass.c_str(), "DoStmt") == 0)
    {
      terminator_type = "DoWhile";
    }
    else
    if (strcmp(termStmtClass.c_str(), "ForStmt") == 0)
    {
      terminator_type = "For";
    }
    else
    if (strcmp(termStmtClass.c_str(), "GotoStmt") == 0)
    {
      terminator_type = "Goto";
    }
  }

  return terminator_type;
}

/// \brief Override VisitDecl (for declarations outside FunctionDecl)
bool MyASTVisitor::VisitDecl(clang::Decl *decl)
{
  clang::SourceLocation sLoc = decl->getLocation();
  clang::SourceManager & sm = m_compilerInstance->getSourceManager();

  clang::FileID fId= sm.getFileID(sLoc);
  const clang::FileEntry *fe = sm.getFileEntryForID(fId);

  if(fe)
  {
    const char* fileName = fe->getName();
    bool isSystemHeader = sm.isInSystemHeader(sLoc);

    // Do not dump system header files.
    if(!isSystemHeader)
    {
      //std::cout << "\n" << decl->getDeclKindName() << "\n";

      // Take care of my global Variables
      if (strcmp(decl->getDeclKindName(), "Var") == 0)
      {
        decl->dump();
        std::cerr << "\n";

        const NamedDecl *namedDecl = dyn_cast<NamedDecl>(decl);

        if (namedDecl)
        {
          std::string varName = namedDecl->getNameAsString();
          std::cerr << "\tidentifier name = " << varName << "\n";
        }

        const ValueDecl *valueDecl = dyn_cast<ValueDecl>(decl);

        if (valueDecl)
        {
          // Get QualType from ValueDecl
          clang::QualType declQT = valueDecl->getType();

          // Now call clang::QualType.getAsString(PrintingPolicy &Policy)
          clang::ASTContext &context = this->m_compilerInstance->getASTContext();

          std::string varType = declQT.getAsString(context.getPrintingPolicy());
          std::cerr << "\ttype = " << varType << "\n";
        }

      }  // Var ends

      return true;
    }
  }

  return false;
}

/// \brief Override VisitFunctionDecl() to traverse every FunctionDecl
bool MyASTVisitor::VisitFunctionDecl(clang::FunctionDecl *functionDecl)
{
  // Print function first
  std::cerr << "\n------------------------------------------------------------";
  std::cerr << "--------------------\n\n Print function:\n\n";
  functionDecl->dump();

  // Check storage class specifier - only extern and static allowed in C
  switch (functionDecl->getStorageClassAsWritten())
  {
    case SC_None:
      // no storage class specifier
      break;
    case SC_Extern:
      std::cerr << "extern ";
      break;
    case SC_Static:
      std::cerr << "static ";
      break;
    case SC_Auto:
    case SC_Register:
    // added SC_PrivateExtern, SC_OpenCLWorkGroupLocal
    // to avoid warning (enumeration not handled in switch) from clang++
    case SC_PrivateExtern:
    case SC_OpenCLWorkGroupLocal:
      // not needed for functions in C language
      std::cerr << "Invalid storage class for functions";
      return false;
  }

  // initialize apron manager and environment (see Apron.cpp)
  initApron();

  // Get function name
  std::string functionName = functionDecl->getNameInfo().getAsString();
  std::cerr << "\n Function Name: " << functionName << "\n";

  // Get return type of function
  clang::QualType QT = functionDecl->getResultType();
  std::string returnType = QT.getAsString();
  std::cerr << "\n Function Return Type: " << returnType << "\n";

  // Get function arguments
  clang::FunctionDecl::param_iterator pit;
  std::cerr << "\n Function Arguments: \n";

  for (pit  = functionDecl->param_begin();
       pit != functionDecl->param_end();
       pit++)
  {
    clang::Decl *param = *pit;
    std::string varName;
    std::string varType;

    const NamedDecl *namedDecl = dyn_cast<NamedDecl>(param);

    if (namedDecl)
    {
      varName = namedDecl->getNameAsString();
      std::cerr << "\tparam name = " << varName << "\n";
    }

    const ValueDecl *valueDecl = dyn_cast<ValueDecl>(param);

    if (valueDecl)
    {
      QualType declQT = valueDecl->getType();
      clang::ASTContext &context = param->getASTContext();

      varType = declQT.getAsString(context.getPrintingPolicy());
      std::cerr << "\ttype = " << varType << "\n";
    }

    getVarNameAndType(varName.c_str(), varType.c_str());

  } // for pit ends

  // For a function definition
  // - generate CFG and get pointer to CFG object
  // - find entry and exit blocks
  // - find blockList for list of blocks to be visited
  // - get pointer to each statement in a basic block of CFG
  // - call MyProcessStmt.getStmtToProcess() to invoke TraverseStmt() further

  if (llvm::isa<clang::FunctionDecl>(functionDecl))
  {
    clang::FunctionDecl* FD = llvm::cast<clang::FunctionDecl>(functionDecl);

    // if this is a function definition (body)
    if (FD->isThisDeclarationADefinition())
    {
      // get function body and generate the control flow graph (CFG)
      clang::CFG::BuildOptions buildOpts;
      clang::LangOptions languageOptions;

      clang::Stmt *body = functionDecl->getBody();

      MyCFG my_cfg;

      my_cfg.setCompilerInstance(this->m_compilerInstance);
      my_cfg.cfg = my_cfg.getCFG(functionDecl, body,
        &(this->m_compilerInstance->getASTContext()), buildOpts);

      std::cerr << "\n--------------------------------------------------------";
      std::cerr << "------------------------\n\n Print Control Flow Graph:\n\n";

      unsigned NumberOfBlocks = my_cfg.cfg->getNumBlockIDs();
      std::cerr << " Number of blocks in CFG: " << NumberOfBlocks << "\n";

      // get Entry block (topmost block)
      clang::CFGBlock* cfg_entry_block = *(my_cfg.cfg->rbegin());

      // get Exit block (last block)
      clang::CFGBlock* cfg_exit_block = *(--(my_cfg.cfg->rend()));

      // do pre-processing before we start analysis
      // and prepare list of blocks to be visited in order
      doPreprocessingBeforeAnalysis(my_cfg);
      getListBreak(my_cfg);

      bool isCurrentBlockASourceOfBackEdge = false;
      bool isThisFixPoint = false;
      bool isReachableFromEntryBlock = false;
      int numWideningDone = 0;
      unsigned blockId = 0;

      clang::CFGBlock* cfg_block;
      clang::CFGReverseBlockReachabilityAnalysis blockReahabilityCheck(*(my_cfg.cfg));

      // now, traverse CFG blocks using blockList

      for (int i = 0; i < blockList.size(); i++)
      {
        cfg_block = blockList[i];

        blockId = cfg_block->getBlockID();

        isReachableFromEntryBlock = blockReahabilityCheck.isReachable(
          cfg_entry_block, cfg_block);

        std::cerr << "\n--------------------------------------------------\n";
        std::cerr << "\n Current CFGBlock \n - blockId = " << blockId;

        edgeStruct* backEdge = isASourceOfBackEdge(cfg_block);

        if (backEdge == NULL)
        {
          isCurrentBlockASourceOfBackEdge = false;
          std::cerr << "\n - is not the source of a back edge ";
        }
        else
        {
          isCurrentBlockASourceOfBackEdge = true;
          std::cerr << "\n - is the source of a back edge ";
        }

        std::cerr << "\n - is reachable from Entry Block ";
        std::cerr << "(true-1/false-0): ";
        std::cerr << isReachableFromEntryBlock << "\n";

        // Now lets print current basic block first
        cfg_block->dump(my_cfg.cfg, languageOptions);

        // get number of predecessors and successors
        int numPredecessors = getNumPredecessors(cfg_block);
        int numSuccessors = getNumSuccessors(cfg_block);

        std::cerr << "\n  numPredecessors = " << numPredecessors << "\n";
        std::cerr << "\n  numSuccessors = " << numSuccessors << "\n";

        // get info about successors of predecessors
        clang::CFGBlock::pred_iterator pred_it;

        for (pred_it  = cfg_block->pred_begin();
             pred_it != cfg_block->pred_end();
             pred_it++)
        {
          clang::CFGBlock* pred_block = *pred_it;

          std::cerr << "\n  cfg_block = " << cfg_block->getBlockID();
          std::cerr << " \tpredBlock = " << pred_block->getBlockID() << " \n";

          addSuccOfPredInfo(cfg_block, pred_block);
        }

        showSuccOfPredInfo();

        // get current basic block's terminator type
        std::string termType = getTerminatorType(cfg_block);

        // update block traversal list for while and for loop
        if ((strcmp(termType.c_str(), "While") == 0) ||
            (strcmp(termType.c_str(), "For") == 0))
        {
          processListWhileFor(cfg_block);
        }

        // update block traversal list for do-while loop
        if (strcmp(termType.c_str(), "DoWhile") == 0)
        {
          processListDoWhile(cfg_block);
        }

        if (cfg_block == cfg_entry_block)
        {
          termType = "Empty";

          // set abstract value at entry block
          AbstractMemoryPtr->blockAbsVal = ap_abstract1_top(man, env);
        }
        else
        if (cfg_block == cfg_exit_block)
        {
          termType = "Empty";
        }

        // flag to denote is current block a unique successor
        // i.e. if current block has one predecessor and that predecessor has
        // only one successor
        bool isUniqueSucc = isUniqueSuccessor(cfg_block);
        std::cerr << "\n  isUniqueSucc = " << isUniqueSucc << "\n";

        // this view is of current basic block

        MyCFGInfo* currentCFGInfo;

        // get predecessors of current basic block
        std::vector<clang::CFGBlock *> preds = getPredecessors(cfg_block);

        if (numPredecessors > 1)
        {
          // current basic block has more than one predecessor blocks

          // abstract value of current block will be join of abstract values
          // coming from its predecessors
          //    B4                        B6
          //     |                         |  B4     B3
          //    B3                         \   |    / |
          //   /  \                         \  \  B2  |
          //  B2   |                         \  \  |  |
          //   \  /                           \  \ | /
          //    B1   B1 - two preds               B1    B1 - 4 preds
          //

          std::cerr << "\n came inside numPredecessors > 1\n";
          ap_abstract1_t joined_abs_val;
          edgeStruct * edge;
          int count = 0;

          for (int j = 0; j < preds.size(); j++)
          {
            std::cerr << "\n came inside numPredecessors > 1 -- first for loop\n";

            // get edge b/w predecessor and cfg_block
            edge = getEdge(preds[j], cfg_block);

            std::cerr << "\n edge->isABackEdge = " << edge->isABackEdge;
            std::cerr << " edge->hasBeenVisited = " << edge->hasBeenVisited << "\n";

            isReachableFromEntryBlock = blockReahabilityCheck.isReachable(
              cfg_entry_block, preds[j]);

            // get initial joined_abs_val when following condition is true
            if (!(edge->isABackEdge && !(edge->hasBeenVisited)) &&
                 isReachableFromEntryBlock)
            {
              count = j;

              std::cerr << "\n-- got initial joined_abs_val\n";

              currentCFGInfo = getRowOfMyCFGInfo(preds[j]);

              joined_abs_val = ap_abstract1_copy(man,
                &(currentCFGInfo->blockAbsVal));

              break;
            }

            markVisited(preds[j], cfg_block);

          }

          for (int j = count +  1; j < preds.size(); j++)
          {
            std::cerr << "\n came inside numPredecessors > 1 -- second for loop\n";
            edge = getEdge(preds[j], cfg_block);

            isReachableFromEntryBlock = blockReahabilityCheck.isReachable(
              cfg_entry_block, preds[j]);

            // skip this edge if following condition is true
            if (!(edge->isABackEdge && !(edge->hasBeenVisited)) &&
                 isReachableFromEntryBlock)
            {
              std::cerr << "\n-- got next joined_abs_val\n";

              // get pointer to predecessor's block info (2nd, 3rd, ...)
              currentCFGInfo = getRowOfMyCFGInfo(preds[j]);

              // join abstract values from predecessors
              joined_abs_val = ap_abstract1_join(man, false, &joined_abs_val,
                &(currentCFGInfo->blockAbsVal));
            }

              markVisited(preds[j], cfg_block);
          }

          std::cerr << "\n  @begin of block " << cfg_block->getBlockID();
          std::cerr << "  abstract value after join\n";
          ap_abstract1_fprint(stderr, man, &joined_abs_val);

          // set abstract value @ begin of this  block = joined abstract values
          // from its predecessors
          AbsValPtr->abstract_value = ap_abstract1_copy(man,
            &joined_abs_val);

        } // if numPredecessors > 1 ends
        else
        if (numPredecessors == 1)
        {
          // current basic block has only one predecessor block

          // get pointer to single predecessor's block info
          clang::CFGBlock* singlePred = preds[0];

          // saveCount is index of current cfg_block in blockList
          // i+1 points to next cfg_block to be visited in blockList
          markVisited(singlePred, cfg_block);

          std::cerr << "\n  predSingle blockid " << singlePred->getBlockID() << "\n";

          // given this single predecessor find row of MyCFGInfo for abs val
          currentCFGInfo = getRowOfMyCFGInfo(singlePred);

          if ((strcmp(getTerminatorType(singlePred).c_str(), "If") == 0) ||
              (strcmp(getTerminatorType(singlePred).c_str(), "While") == 0) ||
              (strcmp(getTerminatorType(singlePred).c_str(), "DoWhile") == 0) ||
              (strcmp(getTerminatorType(singlePred).c_str(), "For") == 0))
          {
            //    B3                      B4
            //   /  \                    /  \
            //  B2   |  B2 first_succ   B3  B2  B3 first_succ, B2 second_succ
            //   \  /                    \  /
            //    B1                      B1
            //
            std::cerr << "\n  I am block " << cfg_block->getBlockID();
            std::cerr << " with single pred and term type If\n";

            // find if the current block is first or second successor
            // of its predecessor
            bool isFirstSucc = isFirstSuccOfPred(cfg_block);
            std::cerr << "\n  isFirstSucc = " << isFirstSucc << "\n";

            if (isFirstSucc)
            {
              // first successor of its predecessor
              ap_abstract1_t block_abs_val = ap_abstract1_copy(man,
                &(currentCFGInfo->blockAbsVal));
              ap_abstract1_t cond_abs_val = ap_abstract1_copy(man,
                &(currentCFGInfo->condAbsVal));

              // set abstract value @ begin of this  block = meet of abstract
              // value of predecessor block and abstract value of positive of
              // condition
              AbsValPtr->abstract_value = ap_abstract1_meet(man, false,
                &block_abs_val, &cond_abs_val);

              std::cerr << "\n  @begin of block " << cfg_block->getBlockID();
              std::cerr << "  abstract value after meet\n";
              ap_abstract1_fprint(stderr, man, &(AbsValPtr->abstract_value));
            }
            else
            {
              // second successor of its predecessor
              ap_abstract1_t block_abs_val = ap_abstract1_copy(man,
                &(currentCFGInfo->blockAbsVal));
              ap_abstract1_t neg_cond_abs_val = ap_abstract1_copy(man,
                &(currentCFGInfo->negCondAbsVal));

              // set abstract value @ begin of this  block = meet of abstract
              // value of predecessor block and abstract value of negative of
              // condition
              AbsValPtr->abstract_value = ap_abstract1_meet(man, false,
                &block_abs_val, &neg_cond_abs_val);

              std::cerr << "\n  @begin of block " << cfg_block->getBlockID();
              std::cerr << "  abstract value after meet\n";
              ap_abstract1_fprint(stderr, man, &(AbsValPtr->abstract_value));
            }
          }
          else
          if (isUniqueSucc)
          {
            // current basic block is unique successor

            //    B4  <- predecessor block (singlePred)
            //    |
            //    B3  <- current block (cfg_block)
            //
            // get abstract value of predecessor block and copy
            // as abstract value for this unique successor
            std::cerr << "\n  found unique succ\n";

            // set abstract value @ begin of this  block = abstract value
            // of single predecessor block
            AbsValPtr->abstract_value = ap_abstract1_copy(man,
              &(currentCFGInfo->blockAbsVal));

            std::cerr << "  abstract value after copy for isUniqueSucc\n";
            ap_abstract1_fprint(stderr, man, &(AbsValPtr->abstract_value));
          }

        }

        // START: Iterate over statements in a basic block

        // Iterate over a CFGBlock to get every element in it
        clang::CFGBlock::iterator cfg_block_it;

        // flag to denote is current statement last statement of basic block
        bool isLastStatementOfBlock = false;

        for (cfg_block_it  = cfg_block->begin();
             cfg_block_it != cfg_block->end();
             cfg_block_it++)
        {
          // Get CFGElement
          clang::CFGElement element = *cfg_block_it;

          // Ignore everything except clang::CFGElement::Statement for now
          if (element.getKind() == clang::CFGElement::Statement)
          {
            // Get Stmt objects from CFGElement
            const clang::CFGStmt* cfg_stmt = element.getAs<clang::CFGStmt>();

            const clang::Stmt *stmt = cfg_stmt->getStmt();

            isLastStatementOfBlock = false;

            if (stmt)
            {
              // find if current statement is last statement of the block
              clang::CFGElement last_element = *(--(cfg_block->end()));

              const clang::CFGStmt* last_cfg_stmt;
              last_cfg_stmt = last_element.getAs<clang::CFGStmt>();

              const clang::Stmt* last_stmt = last_cfg_stmt->getStmt();

              if (stmt == last_stmt)
              {
                isLastStatementOfBlock = true;
              }

              std::cerr << "\n found stmt \n";
              std::cerr << "  isLastStatementOfBlock = ";
              std::cerr << isLastStatementOfBlock << "\n";

              // if this statement is last statement and this block has a
              // terminator If, While, DoWhile, For - skip this conditional
              // statement - will be taken care of inside Terminator's Visit
              // method
              if (isLastStatementOfBlock &&
                 ((strcmp(termType.c_str(), "If") == 0) ||
                  (strcmp(termType.c_str(), "While") == 0) ||
                  (strcmp(termType.c_str(), "DoWhile") == 0) ||
                  (strcmp(termType.c_str(), "For") == 0) ) )
              {
                // do nothing for this conditional statement
              }
              else
              {
                // Constructor of MyProcessStmt will invoke TraverseStmt(Stmt *)
                MyProcessStmt myProcessStmt(this->m_compilerInstance,
                  cfg_block->getBlockID(),
                  (clang::Stmt *)stmt,
                  isLastStatementOfBlock);

                updateAbsValFromGlobalVarList();
              }

            } // if stmt ends

          } // if element.getKind() ends

        } // for cfg_block_it ends

        // END: Iterate over statements in a basic block

        std::cerr << "\n  abstract value after this block is processed\n";
        ap_abstract1_fprint(stderr, man, &(AbsValPtr->abstract_value));

        // update AbstractMemoryPtr now for current block
        // thus we store abstract value after current basic block is processed
        //if (strcmp(termType.c_str(), "None") == 0)
        if ((strcmp(termType.c_str(), "None") == 0) ||
            (strcmp(termType.c_str(), "Goto") == 0) )
        {
          AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
            &(AbsValPtr->abstract_value));
        }

        // Get CFGTerminator object
        clang::CFGTerminator terminator = cfg_block->getTerminator();

        // Get statement of Terminator
        clang::Stmt* s = terminator.getStmt();

        if (s)
        {
          std::cerr << "\n found terminator stmt \n";
          std::cerr << "\tStmtClassName: ";
          std::cerr << s->getStmtClassName() << "\n\n";

          // We want to know what is inside CFGTerminator
          // - control flow statements (if-else, while, etc.)
          MyProcessStmt myProcessStmt(this->m_compilerInstance,
            cfg_block->getBlockID(),
            (clang::Stmt *)s,
            true);

          updateAbsValFromGlobalVarList();
        }

        // set abstract values for blocks with terminator
        // If / While / DoWhile / For
        if ((strcmp(termType.c_str(), "If") == 0) ||
            (strcmp(termType.c_str(), "While") == 0) ||
            (strcmp(termType.c_str(), "DoWhile") == 0) ||
            (strcmp(termType.c_str(), "For") == 0) )
        {
          AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
            &(AbsValPtr->abstract_value));

          CondExprAbsVal* t_CondExprAbsVal = removeCondExprAbsVal();
          AbstractMemoryPtr->condAbsVal = ap_abstract1_copy(man,
            &(t_CondExprAbsVal->condAbsVal));
          AbstractMemoryPtr->negCondAbsVal = ap_abstract1_copy(man,
            &(t_CondExprAbsVal->negCondAbsVal));

          std::cerr << "\n  abstract value after block terminator is processed\n";
          ap_abstract1_fprint(stderr, man, &(AbstractMemoryPtr->blockAbsVal));
          ap_abstract1_fprint(stderr, man, &(AbstractMemoryPtr->condAbsVal));
          ap_abstract1_fprint(stderr, man, &(AbstractMemoryPtr->negCondAbsVal));
        }

        // START: widening logic

        ap_abstract1_t loopExitAbsValCurrent;
        ap_abstract1_t loopExitAbsValNew;
        ap_abstract1_t loopExitAbsValOld;

        if (isCurrentBlockASourceOfBackEdge)
        {
          // See if this is first visit to block leading to back edge
          if (isFirstTime(cfg_block))
          {
            loopExitAbsValOld = ap_abstract1_bottom(man, env);
          }
          else
          {
            // if not, get loopExitAbsValOld
            loopExitAbsValOld = getLoopExitAbsValOld(cfg_block);
          }

          // that is we are at exit block of loop
          // - from current block there is back edge to block with T:While
          loopExitAbsValCurrent = ap_abstract1_copy(man,
            &(AbstractMemoryPtr->blockAbsVal));

          std::cerr << "\n  loopExitAbsValOld before widening:\n";
          ap_abstract1_fprint(stderr, man, &loopExitAbsValOld);

          //if (! ap_abstract1_is_bottom(man, &loopExitAbsValOld))
          if (backEdge->timesVisited > 1)
          {
            // this is the case other than first iteration of the loop

            // Condition for loop unrolling
            if (backEdge->timesVisited % NumUnrollings == 0)
            {
              loopExitAbsValNew = ap_abstract1_widening(man, &loopExitAbsValOld,
                &loopExitAbsValCurrent);
              numWideningDone++;
              std::cerr << "widening done.\n";
            }
            else
            {
              loopExitAbsValNew = ap_abstract1_copy(man, &loopExitAbsValCurrent);
            }

            if (ap_abstract1_is_eq(man, &loopExitAbsValNew, &loopExitAbsValOld))
            {
              // stop
              isThisFixPoint = true;
              std::cerr << "\n  We have reached at fixed point! ";
              std::cerr << "after widening " << numWideningDone << " times\n";

              // Reset timesVisited counter of back edge if fix point is reached
              resetTimesVisited(cfg_block);
            }
            else
            {
              // do again and call modifyListWiden()
              // copy widened new value into old value
              loopExitAbsValOld = ap_abstract1_copy(man, &loopExitAbsValNew);
              isThisFixPoint = false;
              std::cerr << "\n  not yet fixed point ";
            }
          }
          else
          {
            // in first iteration of loop, loopExitAbsValOld is bottom
            // so, copy loopExitAbsValCurrent into loopExitAbsValOld
            loopExitAbsValOld = ap_abstract1_copy(man, &loopExitAbsValCurrent);
            loopExitAbsValNew = ap_abstract1_copy(man, &loopExitAbsValCurrent);
          }

          // don't forget to update blockAbsVal in MyCFGInfo for this block
          AbstractMemoryPtr->blockAbsVal = ap_abstract1_copy(man,
            &loopExitAbsValNew);

          std::cerr << "\n  inside isCurrentBlockASourceOfBackEdge:";
          std::cerr << "\n  loopExitAbsValOld:\n";
          ap_abstract1_fprint(stderr, man, &loopExitAbsValOld);
          std::cerr << "\n  loopExitAbsValCurrent:\n";
          ap_abstract1_fprint(stderr, man, &loopExitAbsValCurrent);
          std::cerr << "\n  loopExitAbsValNew:\n";
          ap_abstract1_fprint(stderr, man, &loopExitAbsValNew);
          std::cerr << "\n  numWideningDone = " << numWideningDone << "\n";
        }

        // END: widening logic

        // add this basic block info in CFGInfo
        my_cfg.addCFGInfo(blockId,
          cfg_block,
          termType,
          isCurrentBlockASourceOfBackEdge,
          AbstractMemoryPtr,
          loopExitAbsValNew);

        my_cfg.printCFGInfo();

        // ok, if this block is leading to back edge and this is not fix point
        // call modifyListWiden()
        // but this should happen after MyCFGInfo entry of this block is updated
        if (isCurrentBlockASourceOfBackEdge && (!isThisFixPoint))
        {
          i = modifyListWiden(cfg_block, i);
        }

        // reset fix point flag
        isThisFixPoint = false;
      } // for CFG::iterator ends

    } // if isThisDeclarationADefinition ends
  } //if isa<FunctionDecl> ends

  // Return false to stop at function declaration level,
  // (see ENTRY block with return true)
  return false;
}

/// \brief Do pre-processing before analysis begins on CFG block list
void MyASTVisitor::doPreprocessingBeforeAnalysis(MyCFG my_cfg)
{
  unsigned NumberOfBlocks = my_cfg.cfg->getNumBlockIDs();

  createBlockList(my_cfg);

  printBlockStruct();

  // get Entry block (topmost block)
  clang::CFGBlock* cfg_entry_block = *(my_cfg.cfg->rbegin());

  // get Exit block (last block)
  clang::CFGBlock* cfg_exit_block = *(--(my_cfg.cfg->rend()));

  // get vector<clang::CFGBlock *> blockList containing list of blocks
  // to be visited in order

  // initially mark all blocks as not visited or to be visited
  blocksToVisit = (int *) new int [NumberOfBlocks];

  for (int i = 0; i < NumberOfBlocks; i++)
  {
    blocksToVisit[i] = WHITE;
  }

  // creating and initializing adjacency matrix to 0
  for (int i = 0; i < NumberOfBlocks; i++)
  {
    std::vector <int> row;

    for (int j = 0; j < NumberOfBlocks; j++)
    {
      row.push_back(0);
    }
    adjacencyMatrix.push_back(row);
  }

  
  for (int i = 0; i < NumberOfBlocks; i++)
  {
    std::vector <edgeStruct *> row;

    for (int j = 0; j < NumberOfBlocks; j++)
    {
      row.push_back(NULL);
    }
    edgeMatrix.push_back(row);
  }

  // creating edges of the original cfg top down
  clang::CFG::reverse_iterator cfg_rit;
  int edgesCreated[NumberOfBlocks];

  for (int i = 0; i < NumberOfBlocks; i++)
  {
    edgesCreated[i] = 0;
  }

  for (cfg_rit  = my_cfg.cfg->rbegin();
       cfg_rit != my_cfg.cfg->rend();
       cfg_rit++)
  {
    clang::CFGBlock * myBlock = *(cfg_rit);
    unsigned idOfBlock = myBlock->getBlockID();

    if (edgesCreated[idOfBlock]== 0)
    {
      clang::CFGBlock::succ_iterator mySucc_it;

      for (mySucc_it  = myBlock->succ_begin();
           mySucc_it != myBlock->succ_end();
           mySucc_it++)
      {

        if (mySucc_it != NULL)
        {
          // pass cfg_block and its successor to create forward edge
          createEdges(*(cfg_rit), *(mySucc_it));
        }
      }

      edgesCreated[idOfBlock] = 1;
    }
  }

  std::cout <<"\nEdges in the forward direction are: \n";
  displayEdges();

  // Detecting the back edges
  findBackEdges(cfg_entry_block, blocksToVisit);

  std::cout << "\nThe back edges are: \n";
  displayBackEdges();

  // creating edges of the original cfg bottom up
  clang::CFG::iterator cfg_it;

  for (int i = 0; i < NumberOfBlocks; i++)
  {
    edgesCreated[i] = 0;
  }

  // Display the Adjacency Matrix
  std::cout << "\nAdjacency Matrix is: \n";

  for (int i = 0; i < NumberOfBlocks; i++)
  {
    std::cout << "\n|";

    for (int j = 0; j < NumberOfBlocks; j++)
    {
      std::cout << " " << adjacencyMatrix[i][j];
    }
    std::cout <<"|";
  }

  std::cout << "\n\n";

  
  std::cerr << "\nEdge Matrix is: \n";

  for (int i = 0; i < NumberOfBlocks; i++)
  {
    std::cerr << "\n|";

    for (int j = 0; j < NumberOfBlocks; j++)
    {
      if (edgeMatrix[i][j])
      {
        std::cerr << " " << std::setw(3)
          << edgeMatrix[i][j]->sourceBlock->getBlockID() << "->"
          << std::setw(3) <<edgeMatrix[i][j]->destBlock->getBlockID();
      }
      else
      {
        std::cerr << " " << std::setw(3)
          << "xx" << "->" << std::setw(3) << "xx";
      }
    }

    std::cerr <<"|";
  }
  
  
  // creating the block_traversal_list
  for (int i = 0; i < NumberOfBlocks; i++)
  {
    blocksToVisit[i] = WHITE;
  }

  getList(cfg_entry_block, blocksToVisit);

  //removeUnreachableBlocks(my_cfg);
  tarzanAlgo();
  printLoops();

/*
  // displaying the block_traversal_list
  std::cout << "\nBlock traversal list\n";

  for (int i = 0; i < blockList.size(); i++)
  {
    clang::CFGBlock* block = blockList[i];
    std::cout << block->getBlockID() << " ";
  }

  std::cout << "\n\n";
*/
}

/// \brief Remove Unreachable Blocks from traversal List
void MyASTVisitor::removeUnreachableBlocks(MyCFG my_cfg)
{
  bool isReachableFromEntryBlock = false;
  clang::CFGBlock * cfg_entry_block;
  std::vector<clang::CFGBlock *> tempList;

  clang::CFGReverseBlockReachabilityAnalysis blockReahabilityCheck(*(my_cfg.cfg));
  cfg_entry_block = *(my_cfg.cfg->rbegin());

  tempList.push_back(blockList[0]);

  for (int i = 1; i < blockList.size(); i++)
  {
    isReachableFromEntryBlock = blockReahabilityCheck.isReachable(
      cfg_entry_block, blockList[i]);

    if (isReachableFromEntryBlock)
    {
      tempList.push_back(blockList[i]);
    }
  }

  blockList = tempList;
}

/// \brief Return edge structure given source and destination blocks
edgeStruct* MyASTVisitor::getEdge(clang::CFGBlock* sourceBlock,
  clang::CFGBlock* destBlock)
{
  edgeStruct * myEdgeStruct;
  myEdgeStruct = edgeMatrix[sourceBlock->getBlockID()][destBlock->getBlockID()];

  if (myEdgeStruct != NULL)
    return myEdgeStruct;
  else
    return NULL;
}

/// \brief Mark an edge visited based on given in indices in blockList
void MyASTVisitor::markVisited(clang::CFGBlock* thisBlock,
  clang::CFGBlock* nextBlock)
{
   edgeStruct* myEdgeStruct;

  myEdgeStruct = edgeMatrix[thisBlock->getBlockID()][nextBlock->getBlockID()];

  // update flags for marked edge
  myEdgeStruct->hasBeenVisited = true;
}

/// \brief Get blockList - list of blocks to be visited in order
void MyASTVisitor::getList(clang::CFGBlock* cfg_block, int* blocksToVisit)
{
  clang::CFGBlock::pred_iterator pred_it;
  clang::CFGBlock::succ_iterator succ_it;

  if (blocksToVisit[cfg_block->getBlockID()] != BLACK)
  {
    if (blocksToVisit[cfg_block->getBlockID()] != RED)
    {
      for (pred_it  = cfg_block->pred_begin();
           pred_it != cfg_block->pred_end();
           pred_it++)
      {
        clang::CFGBlock * pred_block = *(pred_it);

        if (!isPresentInBackEdgeList(pred_block, cfg_block))
        {
          if ((blocksToVisit[pred_block->getBlockID()] != RED) &&
              (blocksToVisit[pred_block->getBlockID()] != BLACK))
          {
            getList(pred_block, blocksToVisit);
            //std::cout << "\nReturn from block " << cfg_block->getBlockID();
            //std::cout << " since its pred " << pred_block->getBlockID();
            //std::cout << " is not visited.";
            //return;
          }
        }
      }
    }

    if ((blocksToVisit[cfg_block->getBlockID()] != RED) &&
        (blocksToVisit[cfg_block->getBlockID()] != BLACK))
    {
      blockList.push_back(cfg_block);
      blocksToVisit[cfg_block->getBlockID()] = RED;
    }

    for (succ_it  = cfg_block->succ_begin();
         succ_it != cfg_block->succ_end();
         succ_it++)
    {
      clang::CFGBlock* succ_block = *succ_it;
      if(succ_block != NULL)
      {
        if ((blocksToVisit[succ_block->getBlockID()] != RED) &&
            (blocksToVisit[succ_block->getBlockID()] != BLACK))
        {
          getList(succ_block, blocksToVisit);
        }
      }
    }

    blocksToVisit[cfg_block->getBlockID()] = BLACK;
  }
}

/// \brief Does a depth first search top-down to detect the back edges
void MyASTVisitor::findBackEdges(clang::CFGBlock* cfg_block, int* blocksToVisit)
{
  if (blocksToVisit[cfg_block->getBlockID()] == BLACK)
  {
    return;
  }
  else
  if (blocksToVisit[cfg_block->getBlockID()] == GRAY)
  {
    // all successors have not been taken into consideration
    createBackEdge(currentSourceBlock, currentDestBlock);
    return;
  }

  // color this block GRAY as we call get list on its successors
  blocksToVisit[cfg_block->getBlockID()] = GRAY;

  clang::CFGBlock::pred_iterator pred_it;
  clang::CFGBlock::succ_iterator succ_it;

  for (succ_it  = cfg_block->succ_begin();
       succ_it != cfg_block->succ_end();
       succ_it++)
  {
    // call findBackEdges() for every successor

    clang::CFGBlock* succ_block = *succ_it;

    // handle cases where the successor is NULL
    if (succ_block != NULL)
    {
      currentSourceBlock = cfg_block;
      currentDestBlock = succ_block;

      findBackEdges(succ_block, blocksToVisit);
    }
  }

  // color this block BLACK as all successors have been considered.
  currentSourceBlock = cfg_block;
  blocksToVisit[cfg_block->getBlockID()] = BLACK;
}

/// \brief Creates edges between two connected nodes in the cfg
/// in the forward direction
void MyASTVisitor::createEdges(clang::CFGBlock* source, clang::CFGBlock* dest)
{
  if ((dest != NULL) && (source != NULL))
  {
    edgeStruct * myEdgeStruct = (edgeStruct *) new edgeStruct;
    myEdgeStruct->sourceBlock = source;
    myEdgeStruct->destBlock = dest;
    myEdgeStruct->hasBeenAddedWhile = false;
    myEdgeStruct->isABackEdge = false;
    myEdgeStruct->hasBeenVisited = false;
    myEdgeStruct->timesVisited = 0;
    myEdgeStruct->edgeColor = WHITE;

    myEdgeStructList.push_back(myEdgeStruct);

    edgeMatrix[source->getBlockID()][dest->getBlockID()] = myEdgeStruct;
    adjacencyMatrix[source->getBlockID()][dest->getBlockID()] = 1;
  }
}

/// \brief If an edge is found to be a back edge in the DFS,
/// set the corresponding flag
void MyASTVisitor::createBackEdge(clang::CFGBlock* source,
  clang::CFGBlock* dest)
{
   edgeStruct * myEdgeStruct;
  myEdgeStruct = edgeMatrix[source->getBlockID()][dest->getBlockID()];
  myEdgeStruct->isABackEdge = true;
  myEdgeStruct->edgeColor = BLACK;

  t_loopStruct * myLoop;
  myLoop = (t_loopStruct *) new (t_loopStruct);
  myLoop->loopStart = dest;
  myLoop->loopEnd = source;
  myLoop->loop_done = false;

  loopStructList.push_back(myLoop);
}

/// \brief Displays all edges in forward direction in no particular order
void MyASTVisitor::displayEdges()
{
   int numBlocks;
  numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    for (int j = 0; j < numBlocks; j++)
    {
      if (edgeMatrix[i][j] != NULL)
      {
        std::cerr << " " << edgeMatrix[i][j]->sourceBlock->getBlockID()
          << " " << edgeMatrix[i][j]->destBlock->getBlockID();
      }
    }

    std::cerr << "\n";
  }
}


/// \brief Displays all the back edges
void MyASTVisitor::displayBackEdges()
{
  int numBlocks;
  numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    for (int j = 0; j < numBlocks; j++)
    {
      if (edgeMatrix[i][j] != NULL)
      {
	if (edgeMatrix[i][j]->isABackEdge)
	{
          std::cerr << " " << edgeMatrix[i][j]->sourceBlock->getBlockID()
            << " " << edgeMatrix[i][j]->destBlock->getBlockID();
	}
      }
    }

    std::cerr << "\n";
  }
}

/// \brief Find if this edge is present in the list of back edges
bool MyASTVisitor::isPresentInBackEdgeList(clang::CFGBlock * sourceBlock,
  clang::CFGBlock * destBlock)
{
   if (edgeMatrix[sourceBlock->getBlockID()][destBlock->getBlockID()] != NULL)
  {
    return (edgeMatrix[sourceBlock->getBlockID()][destBlock->getBlockID()]->isABackEdge);
  }

  return false;
}

/// \brief Get number of predecessors
int MyASTVisitor::getNumPredecessors(clang::CFGBlock* block)
{
  clang::CFGBlock::pred_iterator pred_it;
  int numPredecessors = 0;

  for (pred_it  = block->pred_begin();
       pred_it != block->pred_end();
       pred_it++)
  {
    numPredecessors++;
  }

  return numPredecessors;
}

/// \brief Get number of successors
int MyASTVisitor::getNumSuccessors(clang::CFGBlock* block)
{
  clang::CFGBlock::succ_iterator succ_it;
  int numSuccessors = 0;

  for (succ_it  = block->succ_begin();
       succ_it != block->succ_end();
       succ_it++)
  {
    numSuccessors++;
  }

  return numSuccessors;
}

/// \brief Get list of predecessors
std::vector<clang::CFGBlock *>
MyASTVisitor::getPredecessors(clang::CFGBlock* block)
{
  clang::CFGBlock::pred_iterator pred_it;
  std::vector<clang::CFGBlock *> predList;

  for (pred_it  = block->pred_begin();
       pred_it != block->pred_end();
       pred_it++)
  {
    clang::CFGBlock* currSucc = *pred_it;
    predList.push_back(currSucc);
  }

  return predList;
}

/// \brief Get list of successors
std::vector<clang::CFGBlock *>
MyASTVisitor::getSuccessors(clang::CFGBlock* block)
{
  clang::CFGBlock::succ_iterator succ_it;
  std::vector<clang::CFGBlock *> succList;

  for (succ_it  = block->succ_begin();
       succ_it != block->succ_end();
       succ_it++)
  {
    clang::CFGBlock* currSucc = *succ_it;
    if (currSucc)
    succList.push_back(currSucc);
  }

  std::cerr << "\n  predecessor's successor list\n";

  for (int i = 0; i < succList.size(); i++)
  {
    clang::CFGBlock* blk = succList[i];
    std::cerr << " " << blk->getBlockID();
  }
  std::cerr << "\n";

  return succList;
}

/// \brief Add an entry for successors of a predecessor of a basic block
void MyASTVisitor::addSuccOfPredInfo(clang::CFGBlock* cfg_block,
  clang::CFGBlock* pred_block)
{
  // create a t_MySuccOfPredInfo
  MySuccOfPredInfo* t_MySuccOfPredInfo;
  t_MySuccOfPredInfo = new MySuccOfPredInfo;

  t_MySuccOfPredInfo->cfgBlock = cfg_block;
  t_MySuccOfPredInfo->predBlock = pred_block;
  t_MySuccOfPredInfo->numSuccOfPred = getNumSuccessors(pred_block);
  t_MySuccOfPredInfo->succOfPredList = getSuccessors(pred_block);
  edgeStruct * myEdgeStruct;

  if (edgeMatrix[pred_block->getBlockID()][cfg_block->getBlockID()] != NULL)
  {
    myEdgeStruct = edgeMatrix[pred_block->getBlockID()][cfg_block->getBlockID()];
    t_MySuccOfPredInfo-> pointerToEdge = myEdgeStruct;

    if ((strcmp(getTerminatorType(pred_block).c_str(), "If") == 0) ||
        (strcmp(getTerminatorType(pred_block).c_str(), "While") == 0) ||
        (strcmp(getTerminatorType(pred_block).c_str(), "DoWhile") == 0) ||
        (strcmp(getTerminatorType(pred_block).c_str(), "For") == 0))
    {
      if (cfg_block == t_MySuccOfPredInfo->succOfPredList[0])
      {
        // edge for then clause
        myEdgeStruct->edgeColor = GRAY;
      }
      else
      if (cfg_block == t_MySuccOfPredInfo->succOfPredList[1])
      {
        // edge for else clause
        myEdgeStruct->edgeColor = RED;
      }
    }
  }

  // check head of MySuccOfPredInfo
  if (!SuccOfPredInfo)
  {
    // empty MySuccOfPredInfo, add first entry
    t_MySuccOfPredInfo->next = SuccOfPredInfo;

    SuccOfPredInfo = t_MySuccOfPredInfo;
  }
  else
  {
    // add one more entry in MySuccOfPredInfo at the end
    t_MySuccOfPredInfo->next = NULL;

    MySuccOfPredInfo* l = SuccOfPredInfo;

    while (l->next)
    {
      l = l->next;
    }

    l->next = t_MySuccOfPredInfo;
  }

}

/// \brief Show successors of predecessors
void MyASTVisitor::showSuccOfPredInfo()
{
  if (!SuccOfPredInfo)
  {
    std::cerr << "\n  Empty SuccOfPredInfo!\n";
  }
  else
  {
    MySuccOfPredInfo* t_MySuccOfPredInfo;
    t_MySuccOfPredInfo = SuccOfPredInfo;

    std::cerr << "\ncfgBlock \tpredBlock numSuccOfPred Edge EdgeColor ";
    std::cerr << "succOfPredList\n";
    std::cerr << "------------------------------------------------------------\n";

    do
    {
      std::cerr << t_MySuccOfPredInfo->cfgBlock->getBlockID() << " \t\t";
      std::cerr << t_MySuccOfPredInfo->predBlock->getBlockID() << " \t\t";
      std::cerr << t_MySuccOfPredInfo->numSuccOfPred << " \t";
      std::cerr << t_MySuccOfPredInfo->pointerToEdge->sourceBlock->getBlockID();
      std::cerr << "->";
      std::cerr << t_MySuccOfPredInfo->pointerToEdge->destBlock->getBlockID();
      std::cerr << " \t";
      std::cerr << t_MySuccOfPredInfo->pointerToEdge->edgeColor << "\t";

      for (int i = 0; i < t_MySuccOfPredInfo->succOfPredList.size(); i++)
      {
        clang::CFGBlock* currentBlock = t_MySuccOfPredInfo->succOfPredList[i];
        std::cerr << currentBlock->getBlockID() << ", ";
      }
      std::cerr << "END\n";

      t_MySuccOfPredInfo = t_MySuccOfPredInfo->next;
    }
    while (t_MySuccOfPredInfo);

    std::cerr << "\n";
  }
}

/// \brief Find if this block is unique successor -
/// block with one predecessor and predecessor has only one successor
bool MyASTVisitor::isUniqueSuccessor(clang::CFGBlock* cfg_block)
{
  // Get predecessor and predecessor's successor info
  clang::CFGBlock::pred_iterator pred_it;
  clang::CFGBlock::succ_iterator succOfPred_it;
  int numPredecessors = 0;
  int numSuccOfPred = 0;
  bool isUniqueSucc = false;

  std::cerr << "\n  predecessor list: \n";

  for (pred_it  = cfg_block->pred_begin();
       pred_it != cfg_block->pred_end();
       pred_it++)
  {
    clang::CFGBlock* pred_block = *pred_it;

    if (pred_block)
    {
      numPredecessors++;
      std::cerr << pred_block->getBlockID() << "  ";

      // get successors of predecessor block
      for (succOfPred_it  = pred_block->succ_begin();
           succOfPred_it != pred_block->succ_end();
           succOfPred_it++)
      {
        clang::CFGBlock* succOfPred_block = *succOfPred_it;

        if (succOfPred_block)
        {
          numSuccOfPred++;
          //store numSuccOfPred->getBlockID()
        }

      } // for succOfPred_it ends

    } // if pred_block ends

  } // for pred_it ends

  if ((numPredecessors == 1) && (numSuccOfPred == 1))
  {
    isUniqueSucc = true;
  }

  std::cerr << "\n  Block " << cfg_block->getBlockID() << " has ";
  std::cerr << numPredecessors << " predecessors\n";

  return isUniqueSucc;
}

/// \brief Get a row of structure MyCFGInfo
MyCFGInfo* MyASTVisitor::getRowOfMyCFGInfo(clang::CFGBlock* block)
{
  MyCFGInfoList::iterator it;
  MyCFGInfo* t_MyCFGInfo = NULL;

  for (it  = myCFGInfoList.begin();
       it != myCFGInfoList.end();
       it++)
  {
    t_MyCFGInfo = *it;

    if (t_MyCFGInfo->cfg_block == block)
    {
      return t_MyCFGInfo;
    }
  }

  return NULL;
}

/// \brief Find if this block is first successor of its predecessor -
/// return true for first successor, false otherwise
bool MyASTVisitor::isFirstSuccOfPred(clang::CFGBlock* cfg_block)
{
  std::vector<clang::CFGBlock *> t_succOfPredList;
  bool isFirstSucc = false;

  while (SuccOfPredInfo)
  {
    // if this block is found in SuccOfPredList
    if (SuccOfPredInfo->cfgBlock == cfg_block)
    {
      std::cerr << "\n  this block " << cfg_block->getBlockID();
      std::cerr << " found in SuccOfPredList \n";

      // get successor list of its predecessor
      t_succOfPredList = SuccOfPredInfo->succOfPredList;

      // is this block first succ
      if (cfg_block == t_succOfPredList[0])
      {
        isFirstSucc = true;
      }
      else
      if (cfg_block == t_succOfPredList[1])
      {
        isFirstSucc = false;
      }

      break;
    }

    SuccOfPredInfo = SuccOfPredInfo->next;
  }

  return isFirstSucc;
}

/// \brief In case of for loops and while loops, we add the loop head
/// at the end of the loop tail in the traversal list
void MyASTVisitor::processListWhileFor(clang::CFGBlock * cfg_block)
{
  clang::CFGBlock * source;
  edgeStruct * currentEdge;
  edgeStruct * myEdgeStruct;
  int count = 0;

  int numBlocks;
  numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[i][cfg_block->getBlockID()] != NULL)
    {
      myEdgeStruct = edgeMatrix[i][cfg_block->getBlockID()];

      if (myEdgeStruct->isABackEdge == true)
      {
        source = myEdgeStruct->sourceBlock;
        currentEdge = myEdgeStruct;
        break;
      }
    }
  }


  if (currentEdge->hasBeenAddedWhile == false)
  {
    for (int i = 0; i < blockList.size(); i++)
    {
      if (blockList[i] == source)
      {
        count = i;
        break;
      }
    }

    std::vector<clang::CFGBlock *> tempList;

    for (int i = 0; i <= count; i++)
    {
      tempList.push_back(blockList[i]);
    }

    tempList.push_back(cfg_block);

    for (int i = count + 1; i < blockList.size(); i++)
    {
      tempList.push_back(blockList[i]);
    }

    blockList = tempList;
    currentEdge->hasBeenAddedWhile = true;
  }
}

/// \brief In case of Dowhile loops, we add the loop head to
/// the block before loop tail at the end of the loop tail
/// in the traversal list
void MyASTVisitor::processListDoWhile(clang::CFGBlock * cfg_block)
{
  clang::CFGBlock * source;
  clang::CFGBlock * dest;
  dest = cfg_block;

  clang::CFGBlock::succ_iterator succ_it;
  succ_it = cfg_block->succ_begin();
  source = *(succ_it);

  bool found = false;
  edgeStruct * currentEdge = NULL;
  edgeStruct * myEdgeStruct = NULL;
  int s_count = 0, d_count =0;

  int numBlocks;
  numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[source->getBlockID()][i] != NULL)
    {
      myEdgeStruct = edgeMatrix[source->getBlockID()][i];

      if (myEdgeStruct->isABackEdge == true)
      {
        dest = myEdgeStruct->destBlock;
        currentEdge = myEdgeStruct;
	found = true;
        break;
      }
    }
  }

  if (found && !currentEdge->hasBeenAddedWhile)
  {
    for (int i = 0; i < blockList.size(); i++)
    {
      if (blockList[i] == source)
      {
        s_count = i;
        break;
      }
    }

    for (int i = 0; i < blockList.size(); i++)
    {
      if (blockList[i] == dest)
      {
        d_count = i;
        break;
      }
    }

    std::vector<clang::CFGBlock *> tempList;

    for (int i = 0; i <= s_count; i++)
    {
      tempList.push_back(blockList[i]);
    }

    for(int i = d_count; i<s_count; i++)
    {
      tempList.push_back(blockList[i]);
    }

    for (int i = s_count + 1; i < blockList.size(); i++)
    {
      tempList.push_back(blockList[i]);
    }

    blockList = tempList;
    currentEdge->hasBeenAddedWhile = true;
  }
}

/// \brief When widening needs to be done, the pointer of the blocklist
/// is shifted back to the loop head. Should be called from the source
/// of a back edge
int MyASTVisitor::modifyListWiden(clang::CFGBlock * cfg_block, int original)
{
   int count = 0;
  clang::CFGBlock * dest;
  edgeStruct * currentEdge;
  edgeStruct * myEdgeStruct;

  int numBlocks;
  numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[cfg_block->getBlockID()][i] != NULL)
    {
      myEdgeStruct = edgeMatrix[cfg_block->getBlockID()][i];

      if (myEdgeStruct->isABackEdge == true)
      {
        currentEdge = myEdgeStruct;
        currentEdge->timesVisited++;
        break;
      }
    }
  }

  if (currentEdge->isABackEdge == true)
  {
    for (int k = 0; k < blockList.size(); k++)
    {
      if (blockList[k] == currentEdge->destBlock)
      {
        return (k-1);
      }
    }
  }

  return original;
}

/// \brief Find if this block is the source of a back edge
edgeStruct* MyASTVisitor::isASourceOfBackEdge(clang::CFGBlock * cfg_block)
{
 edgeStruct * myEdgeStruct;
  int numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[cfg_block->getBlockID()][i] != NULL)
    {
      myEdgeStruct = edgeMatrix[cfg_block->getBlockID()][i];

      if (myEdgeStruct->isABackEdge)
      {
        return myEdgeStruct;
      }
    }
  }

  return NULL;
}

/// \brief Find if this block is the destination of a back edge
edgeStruct* MyASTVisitor::isADestOfBackEdge(clang::CFGBlock * cfg_block)
{
  edgeStruct * myEdgeStruct;
  int numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[i][cfg_block->getBlockID()] != NULL)
    {
      myEdgeStruct = edgeMatrix[i][cfg_block->getBlockID()];

      if (myEdgeStruct->isABackEdge)
      {
        return myEdgeStruct;
      }
    }
  }

  return NULL;
}

/// \brief See if this is first visit to block leading to back edge
bool MyASTVisitor::isFirstTime(clang::CFGBlock* cfg_block)
{
  edgeStruct * myEdgeStruct;
  int numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[cfg_block->getBlockID()][i] != NULL)
    {
      myEdgeStruct = edgeMatrix[cfg_block->getBlockID()][i];

      if (myEdgeStruct->isABackEdge)
      {
         if (myEdgeStruct->timesVisited == 0)
          return true;
      }
    }
  }

  return false;
}

/// \brief Reset timesVisited counter for back edge if fix point is reached
void MyASTVisitor::resetTimesVisited(clang::CFGBlock* cfg_block)
{
   edgeStruct * myEdgeStruct;
  int numBlocks = edgeMatrix.size();

  for (int i = 0; i < numBlocks; i++)
  {
    if (edgeMatrix[cfg_block->getBlockID()][i] != NULL)
    {
      myEdgeStruct = edgeMatrix[cfg_block->getBlockID()][i];

      if (myEdgeStruct->isABackEdge)
      {
        myEdgeStruct->timesVisited = 0;
      }
    }
  }
}

/// \brief Return loopExitAbsValOld for given cfg_block
ap_abstract1_t MyASTVisitor::getLoopExitAbsValOld(clang::CFGBlock* cfg_block)
{
  MyCFGInfoList::iterator it;
  MyCFGInfo* t_MyCFGInfo;
  ap_abstract1_t loopExitAbsValOld;

  for (it  = myCFGInfoList.begin();
       it != myCFGInfoList.end();
       it++)
  {
    t_MyCFGInfo = *it;

    if (t_MyCFGInfo->cfg_block == cfg_block)
    {
      std::cerr << "\n  cfg_block = " << cfg_block->getBlockID();
      loopExitAbsValOld = t_MyCFGInfo->loopExitAbsValOld;
      break;
    }
  }

  return loopExitAbsValOld;
}

/// \brief Use Tarzan's Algorithm to find SCC in the CFG
void MyASTVisitor::tarzanAlgo()
{
  // myEdgeStructList : Edge Struct List
  // blockList : List of Vertices
  // numBlocks : Number of Blocks
  // Each vertex has associated index and lowlink.
  // S : Stack, implemented as vector.

  indexcount = 0;

  int numBlocks;
  numBlocks = blockList.size();

  // initialize everything to undefined : -100
  for (int i = 0; i < blockList.size(); i++)
  {
      myindex.push_back(-100);
      lowlink.push_back(-100);
  }

  // iterating over all vertices
  for (int i = 0; i < blockList.size(); i++)
  {
    clang::CFGBlock* cfg_block;
    cfg_block = blockList[i];

    if (myindex[cfg_block->getBlockID()] == -100)
    {
      stronglyConnect(cfg_block);
    }
  }
}

/// \brief Support member for Tajan's Algorithm
void MyASTVisitor::stronglyConnect(clang::CFGBlock* cfg_block)
{
  myindex[cfg_block->getBlockID()] = indexcount;
  lowlink[cfg_block->getBlockID()] = indexcount;
  indexcount = indexcount + 1;
  S.push_back(cfg_block);

  edgeStructList::iterator edge_it;
  edgeStruct * myEdgeStruct;
  clang::CFGBlock * destBlock;

  for (edge_it  = myEdgeStructList.begin();
       edge_it != myEdgeStructList.end();
       edge_it++)
  {
    myEdgeStruct = *(edge_it);

    if (myEdgeStruct->sourceBlock == cfg_block)
    {
      destBlock = myEdgeStruct->destBlock;

      if (myindex[destBlock->getBlockID()] == -100)
      {
        stronglyConnect(destBlock);
        lowlink[cfg_block->getBlockID()] =
          (lowlink[cfg_block->getBlockID()] > lowlink[destBlock->getBlockID()])
           ? lowlink[destBlock->getBlockID()]:lowlink[cfg_block->getBlockID()];
      }
      else if(isPresentinS(destBlock))
      {
        lowlink[cfg_block->getBlockID()] =
          (lowlink[cfg_block->getBlockID()] > myindex[destBlock->getBlockID()])
           ? myindex[destBlock->getBlockID()]:lowlink[cfg_block->getBlockID()];
      }
    }
  }

  std::vector<clang::CFGBlock * > tempSCC;

  if (lowlink[cfg_block->getBlockID()] == myindex[cfg_block->getBlockID()])
  {
    int k;

    while (1)
    {
      k = S.size();
      destBlock = S[k-1];
      S.pop_back();
      tempSCC.push_back(destBlock);

      if (destBlock == cfg_block)
        break;
    }

    std::cout <<"\nPrinting SCC \n";

    while (tempSCC.size() > 0)
    {
        std::cout << (tempSCC[tempSCC.size()-1])->getBlockID() << " ";
        tempSCC.pop_back();
    }
  }
}

/// \brief Support member for Tajan's Algorithm
bool MyASTVisitor::isPresentinS(clang::CFGBlock * cfg_block)
{
  std::vector<clang::CFGBlock *>::iterator it;
  it = std::find(S.begin(),S.end(),cfg_block);

  if (it != S.end())
      return true;
  else
      return false;
}

/// \brief Populate the blockStructList containing the block pointer and the
//  block terminator type.
void MyASTVisitor::createBlockList(MyCFG my_cfg)
{
  t_blockStructList blockStructList(my_cfg.cfg->getNumBlockIDs(),NULL);

  clang::CFG::iterator block_it;
  clang::CFGBlock* cfg_block;

  for (block_it  = my_cfg.cfg->begin();
       block_it != my_cfg.cfg->end();
       block_it++)
  {
    cfg_block = *(block_it);
    t_blockStruct *  blockStruct;
    blockStruct = (t_blockStruct *) new (t_blockStruct *);
    blockStruct->blockPtr = *(block_it);
    blockStruct->term = 0;

    if ((strcmp(getTerminatorType(blockStruct->blockPtr).c_str(), "If") == 0))
      blockStruct->term = 1;
    else
    if (strcmp(getTerminatorType(blockStruct->blockPtr).c_str(), "While") == 0)
      blockStruct->term = 2;
    else
    if (strcmp(getTerminatorType(blockStruct->blockPtr).c_str(), "DoWhile") == 0)
      blockStruct->term = 3;
    else
    if (strcmp(getTerminatorType(blockStruct->blockPtr).c_str(), "For") == 0)
      blockStruct->term = 4;
    else
    if (strcmp(getTerminatorType(blockStruct->blockPtr).c_str(), "Goto") == 0)
    {    

      clang::CFGTerminator terminator = cfg_block->getTerminator();
      Stmt * stmt;
      stmt = terminator.getStmt();
      GotoStmt * gotostmt;
      gotostmt = (GotoStmt *) stmt;
      LabelDecl * ld;
      ld = gotostmt->getLabel();
      std::string labelName;
      labelName = ld->getNameAsString();
      std::cout << "\nBlock " << cfg_block->getBlockID() << " has a goto stmt.";
      std::cout << "\n\tGoto Label is : " << labelName.c_str();

      std::string whileLabel = ("while_break");
      int comparisonSize = whileLabel.size();

      if((memcmp(whileLabel.data(),labelName.data(),comparisonSize) == 0))
      {    
        blockStruct->term = 5; 
      }    

    }    

    blockStructList[blockStruct->blockPtr->getBlockID()] = blockStruct;
  }

  globalBlockList = blockStructList;
}

/// \brief Print the block structure.
void MyASTVisitor::printBlockStruct()
{
  for (int i = 0; i < globalBlockList.size(); i++)
  {
    std::cout << "\nBlock Id: " << globalBlockList[i]->blockPtr->getBlockID();
    std::cout << "\tTerm Type: " << globalBlockList[i]->term;
  }
}

/// \brief Print the loops encountered in the CFG
void MyASTVisitor::printLoops()
{
  std::cout << "\n\nPrint Loop Lists";

  for (int i = 0; i < loopStructList.size(); i++)
  {
    std::cout << "\nLoop from " << loopStructList[i]->loopStart->getBlockID();
    std::cout << " to " << loopStructList[i]->loopEnd->getBlockID();
  }

  std::cout<< "\n\n";
}

/// \brief Get pointer to the loop given the loop start node
t_loopStruct * MyASTVisitor::getLoopSource(clang::CFGBlock * source)
{
  t_loopStructList::iterator loop_it;
  t_loopStruct * myLoop;

  for (loop_it  = loopStructList.begin();
       loop_it != loopStructList.end();
       loop_it++)
  {
    myLoop = *(loop_it);

    if (myLoop->loopStart == source)
      return myLoop;
  }

  return NULL;
}

/// \brief Get pointer to the loop given the loop end node
t_loopStruct * MyASTVisitor::getLoopDest(clang::CFGBlock * dest)
{
  t_loopStructList::iterator loop_it;
  t_loopStruct * myLoop;

  for (loop_it  = loopStructList.begin();
       loop_it != loopStructList.end();
       loop_it++)
  {
    myLoop = *(loop_it);

    if (myLoop->loopEnd == dest)
      return myLoop;
  }

  return NULL;
}

/// \brief Get traversal list for CFG with breaks.
void MyASTVisitor::getListBreak(MyCFG my_cfg)
{
  unsigned NumberOfBlocks = my_cfg.cfg->getNumBlockIDs();
  myNumBlocks = NumberOfBlocks;
  myBlocksToVisit = (int *) new int [NumberOfBlocks];

  for(int i =0 ; i<NumberOfBlocks; i++)
  {
    myBlocksToVisit[i] = WHITE;
  }

  t_loopStruct * newloop;
  newloop = (t_loopStruct *) new (t_loopStruct);
  currentLoop = (t_loopStruct *) new (t_loopStruct);
  newloop->loopStart = *(my_cfg.cfg->rbegin());
  newloop->loopEnd =  *(--(my_cfg.cfg->rend()));
  newloop->loop_done = true;
  loopStructList.push_back(newloop);


  currentLoop = getLoopSource(*(my_cfg.cfg->rbegin()));
  newGetList(*(my_cfg.cfg->rbegin()), myBlocksToVisit);

  while(currentLoop->loop_done == false)
  {
    std::cerr << "\nloop from "
      << currentLoop->loopStart->getBlockID() << " to "
      << currentLoop->loopEnd->getBlockID() << " is unfinished.\n";
    processPendingList(myBlocksToVisit);
    std::cerr << "\nDone processing pending list for loop from"
      << currentLoop->loopStart->getBlockID() << " to "
      << currentLoop->loopEnd->getBlockID() << " explicitly.\n";
  }

  //Printing tempblock list
  std::cout << "\n\n" << "Printing tempBlockList\n";
  for(int i = 0; i < tempblockList.size(); i++)
  {
    std::cout << " " << tempblockList[i]->getBlockID();
  }
  std::cout << "\n\n";

  blockList = tempblockList;
}

/// \brief Get blockList - list of blocks to be visited in order
void MyASTVisitor::newGetList(clang::CFGBlock* cfg_block, int* myBlocksToVisit)
{
  // Visited <=> Put on the blockList

  // A block can be visited only when its normal predecessors have been visited
  // normal predecessors : Predecessors which are not sources of back-edges.

  // GRAY : Predecessors of Block Not Visited yet.
  // RED  : Predecessors of Block Done, Block Put onto List,
  //        Successors not Visited yet.
  // BLACK : Both Predecessors and Successors of Block Visited.
  // GREEN : Goto Block, Put onto pending list.

  if(cfg_block == NULL)
  {
  std::cerr << "\nCame to NULL Block, returning.";
  return;
  }

  else  if(myBlocksToVisit[cfg_block->getBlockID()]!= BLACK)
  {
  std::cerr << "\nMyBlocks to visit is: \n";


  std::cerr << "\nCurrently in loop from "
    << currentLoop->loopStart->getBlockID() << " to "
    << currentLoop->loopEnd->getBlockID() << "\n";
  for(int i = 0; i< myNumBlocks; i++)
    std::cerr << std::setw(4) << i<<" ";

  std::cerr << "\n";

  for(int i = 0; i< myNumBlocks; i++)
    std::cerr << std::setw(4) << myBlocksToVisit[i] << " ";

  std::cerr << "\n\n";
  if(exit_block && cfg_block)
    std::cerr << "\n\n" << " from block " << exit_block->getBlockID() << " Came to block " << cfg_block->getBlockID() ;
  else if(!(exit_block && cfg_block))
    std::cerr << "\n\n" << " Came to block " << cfg_block->getBlockID() ;

  int blockType = globalBlockList[cfg_block->getBlockID()]->term;
  edgeStruct* myEdge = isADestOfBackEdge(cfg_block);
  edgeStruct* myEdge2 = isASourceOfBackEdge(cfg_block);


  if(myBlocksToVisit[cfg_block->getBlockID()] == BLACK)
  {
    std::cerr << "\nNot processing, since already finished.";
    return;
  }
  //We have reached an unprocessed Goto Block.
  else if(blockType == 5 && myEdge2 == NULL && myBlocksToVisit[cfg_block->getBlockID()]!= BLACK)
  {
    std::cerr << "\nCame into blockType == 5";
    //Check if we have reached the end of the current loop?
    if(currentLoop->loop_done == true)
    {
      predVisit(cfg_block,myBlocksToVisit);
    //safe to now process this block?
    //TODO: Check if this block can always be processed at this point.
    //For eg: Can it ever happen that this block has a predecessor which
    //        has not been processed till now?
      std::cerr << "\nCurrent->loop_done is : TRUE  ";
      if((myBlocksToVisit[cfg_block->getBlockID()] != RED)&&
              (myBlocksToVisit[cfg_block->getBlockID()]!= BLACK))
      {
        //Pushing the block onto the list and marking it RED.
        std::cerr << "\nPushing block " << cfg_block->getBlockID()
                  << " onto tempblockList";
        tempblockList.push_back(cfg_block);
        myBlocksToVisit[cfg_block->getBlockID()] = RED;
      }
      //Go and visit Successors of this block.
      succVisit(cfg_block,myBlocksToVisit);
    }

    //This is the case when the loop end has not been reached
    //Check if the block is not GREEN, nor RED nor BLACK
    //It is the same as checking if the Block is WHITE
    else if(myBlocksToVisit[cfg_block->getBlockID()]== WHITE)
    {
      std::cerr << "\nPushing block " << cfg_block->getBlockID() << " onto loop pending"
                << " list of loop from  " << currentLoop->loopStart->getBlockID() << " to "
                << currentLoop->loopEnd->getBlockID();
      if(cfg_block != NULL)
        currentLoop->pendingList.push_back(cfg_block);

      //Print the Loop Pending List.
      std::cerr << "\nThe list is now : ";
      for(int i=0; i <currentLoop->pendingList.size(); i++)
        std::cerr << " " << currentLoop->pendingList[i]->getBlockID();

      //Mark the Current Block as Green.
      myBlocksToVisit[cfg_block->getBlockID()] = GREEN;

    }

    else
    {
      std::cerr << "\nWhat to do with this block?";
    }
  }

  //This is the case when the Block is the Loop head
  //i.e destination block of a back edge.
  else if(myEdge!=NULL)
  {
    std::cerr << "\nCame into MyEdge != NULL";

    //get the pointer to the current loop.
    currentLoop = getLoopSource(cfg_block);

    //Standard procedure
    //1. Visit all predecessors
    //2. Mark block as visited.
    //3. Visit all Successors.
    //4. Mark block as Processed.
    if (myBlocksToVisit[cfg_block->getBlockID()] != BLACK)
    {
      predVisit(cfg_block,myBlocksToVisit);

      if ((myBlocksToVisit[cfg_block->getBlockID()] != RED) &&
          (myBlocksToVisit[cfg_block->getBlockID()] != BLACK))
      {
        std::cerr << "\nPushing Block " << cfg_block->getBlockID() << " on the list.";
        tempblockList.push_back(cfg_block);
        myBlocksToVisit[cfg_block->getBlockID()] = RED;
      }
      succVisit(cfg_block,myBlocksToVisit);
    }
  }

  //This is the case when Block is the Loop Exit
  //i.e Source of a back edge.
  else if(myEdge2 != NULL)
  {
    std::cerr << "\nCame into MyEdge2 != NULL";

    //get Pointer to the loop.
    currentLoop = getLoopDest(cfg_block);


    if (myBlocksToVisit[cfg_block->getBlockID()] != BLACK)
    {
      //1. Visit all predecessors
      predVisit(cfg_block,myBlocksToVisit);

      if ((myBlocksToVisit[cfg_block->getBlockID()] != RED) &&
         (myBlocksToVisit[cfg_block->getBlockID()] != BLACK))
      {
        //2. Mark block as visited.
        std::cerr << "\nPushing Block " << cfg_block->getBlockID() << " on the list.";
        tempblockList.push_back(cfg_block);
        myBlocksToVisit[cfg_block->getBlockID()] = RED;
        if(currentLoop->loop_done == false)
          {
          exit_block = cfg_block;
          processPendingList(myBlocksToVisit);
          }
      }

      //4. Visit all Successors.
      succVisit(cfg_block,myBlocksToVisit);
    }
  }

  //This is the case when the block is neither a loop head/end
  //and neither a non-finished Goto Block.
  else
  {
    std::cerr << "\nCame into else";
    //Standard procedure
    //1. Visit all predecessors
    //2. Mark block as visited.
    //3. Visit all Successors.
    //4. Mark block as Processed.
    if (myBlocksToVisit[cfg_block->getBlockID()] != BLACK)
    {
      predVisit(cfg_block,myBlocksToVisit);

      if ((myBlocksToVisit[cfg_block->getBlockID()] != RED) &&
          (myBlocksToVisit[cfg_block->getBlockID()] != BLACK))
      {
        std::cerr << "\nPushing block " << cfg_block->getBlockID()
                  << " onto tempblockList";
        tempblockList.push_back(cfg_block);
        myBlocksToVisit[cfg_block->getBlockID()] = RED;
      }

      succVisit(cfg_block,myBlocksToVisit);
    }
  }
  std::cerr << "\nReturning from the newList call of block " << cfg_block->getBlockID();
  }

}

/// \brief Visits all the successors of a given block.
void MyASTVisitor::succVisit(clang::CFGBlock * cfg_block, int * myBlocksToVisit)
{
  if (myBlocksToVisit[cfg_block->getBlockID()] != BLACK)
  {
    clang::CFGBlock::succ_iterator succ_it;

    for (succ_it  = cfg_block->succ_begin();
         succ_it != cfg_block->succ_end();
         succ_it++)
    {
      clang::CFGBlock* succ_block = *succ_it;

      if (succ_block != NULL)
      {
        if ((myBlocksToVisit[succ_block->getBlockID()] != RED) &&
            (myBlocksToVisit[succ_block->getBlockID()] != BLACK))
        {
          newGetList(succ_block, myBlocksToVisit);
        }
      }
    }

    myBlocksToVisit[cfg_block->getBlockID()] = BLACK;
  }
}

/// \brief Visits all the predecessors of a given block.
void MyASTVisitor::predVisit(clang::CFGBlock * cfg_block, int * myBlocksToVisit)
{
  clang::CFGBlock::pred_iterator pred_it;

  if (myBlocksToVisit[cfg_block->getBlockID()] != RED)
  {
    for (pred_it  = cfg_block->pred_begin();
         pred_it != cfg_block->pred_end();
         pred_it++)
    {
      clang::CFGBlock * pred_block = *(pred_it);

      if (!isPresentInBackEdgeList(pred_block, cfg_block))
      {
        if ((myBlocksToVisit[pred_block->getBlockID()] != RED) &&
            (myBlocksToVisit[pred_block->getBlockID()] != BLACK))
        {
          newGetList(pred_block, myBlocksToVisit);
        }
      }
    }
  }
}

/// \brief Visits all the pending blocks of a given loop.
void MyASTVisitor::processPendingList(int * myBlocksToVisit)
{
  //Mark current loop as visited.
  currentLoop->loop_done = true;

  //3. Process the pending list for this block.
  std::cerr << "\nProcessing pending list for loop from "
    << currentLoop->loopStart->getBlockID() << " to "
    << currentLoop->loopEnd->getBlockID();

  //3.1 First Push all the pending blocks which are already not there on the list.
  /*for(int j=0; j< currentLoop->pendingList.size(); j++)
  {
    if((myBlocksToVisit[currentLoop->pendingList[j]->getBlockID()] != RED)
          &&(myBlocksToVisit[currentLoop->pendingList[j]->getBlockID()] != BLACK))
    myBlocksToVisit[currentLoop->pendingList[j]->getBlockID()] = RED;
    tempblockList.push_back(currentLoop->pendingList[j]);
  }*/

  //3.2 Now call getList function on each of the pending blocks.
  //The blocks are already marked red.
  //This part ensures that the successors of the individual pending blocks
  //are visited.
  //This is done till the pending list for that particular loop is empty.
  while(currentLoop->pendingList.size()>0)
  {
    //clang::CFGBlock * myBlock = currentLoop->pendingList[0];

    clang::CFGBlock * myBlock = currentLoop->pendingList[currentLoop->pendingList.size()-1];
    if(myBlock != NULL && myBlocksToVisit[myBlock->getBlockID()] != BLACK )
      {
      std::cerr << "\nCalling newGetList on block " << myBlock->getBlockID();
      newGetList(myBlock, myBlocksToVisit);
      }
    /*std::vector <clang::CFGBlock *> newPendingList;

    //Remove the first element from the pending list.
    for(int j=1;j < currentLoop->pendingList.size(); j++)
    {
      newPendingList.push_back(currentLoop->pendingList[j]);
    }
    currentLoop->pendingList = newPendingList;*/
    if(currentLoop->pendingList.size()>0)
      currentLoop->pendingList.pop_back();

  }
  std::cerr << "\nDone Processing pending list for loop from "
    << currentLoop->loopStart->getBlockID() << " to "
    << currentLoop->loopEnd->getBlockID();
}

/// \brief Set compiler instance
void MyCFG::setCompilerInstance(clang::CompilerInstance* ci)
{
  m_compilerInstance = ci;
}

/// \brief getCFG invokes CFG::buildCFG()
clang::CFG* MyCFG::getCFG(clang::FunctionDecl *functionDecl,
  clang::Stmt *body,
  clang::ASTContext *C,
  clang::CFG::BuildOptions buildOpts)
{
  clang::CFG *cfg = clang::CFG::buildCFG(functionDecl, body,
        &(this->m_compilerInstance->getASTContext()), buildOpts);

  return cfg;
}

/// \brief Add CFG info in structure MyCFGInfo
void MyCFG::addCFGInfo(unsigned blockid,
  clang::CFGBlock* cfg_block,
  std::string termType,
  bool isSourceOfBackEdge,
  AbstractMemory* absMemPtr,
  ap_abstract1_t loopExitAbsValOld)
{
  MyCFGInfoList::iterator it;
  bool found = false;

  MyCFGInfo* myCFGInfo = NULL;
  MyCFGInfo* t_MyCFGInfo;

  // first check if this block cfg_block exists in MyCFGInfoList

  for (it  = myCFGInfoList.begin();
       it != myCFGInfoList.end();
       it++)
  {
    t_MyCFGInfo = *it;

    if (t_MyCFGInfo->cfg_block == cfg_block)
    {
      found = true;

      // if yes, store pointer to MyCFGInfo entry
      myCFGInfo = t_MyCFGInfo;
      break;
    }
  }

  if (!found)
  {
    // add first entry
    t_MyCFGInfo = (MyCFGInfo*) new MyCFGInfo;

    t_MyCFGInfo->blockid = blockid;
    t_MyCFGInfo->cfg_block = cfg_block;
    t_MyCFGInfo->terminatorType = termType;

    if ((strcmp(termType.c_str(), "If") == 0) ||
        (strcmp(termType.c_str(), "While") == 0) ||
        (strcmp(termType.c_str(), "DoWhile") == 0) ||
        (strcmp(termType.c_str(), "For") == 0) )
    {
      // blocks with conditions and loops as teminator
      t_MyCFGInfo->blockAbsVal = absMemPtr->blockAbsVal;
      t_MyCFGInfo->condAbsVal = absMemPtr->condAbsVal;
      t_MyCFGInfo->negCondAbsVal = absMemPtr->negCondAbsVal;
    }
    else
    //if (strcmp(termType.c_str(), "None") == 0)
    if ((strcmp(termType.c_str(), "None") == 0) ||
        (strcmp(termType.c_str(), "Goto") == 0) )
    {
      // blocks with no terminator
      t_MyCFGInfo->blockAbsVal = absMemPtr->blockAbsVal;
    }
    else
    if (strcmp(termType.c_str(), "Empty") == 0)
    {
      // entry and exit blocks
      t_MyCFGInfo->blockAbsVal = absMemPtr->blockAbsVal;
    }

    t_MyCFGInfo->isSourceOfBackEdge = isSourceOfBackEdge;

    // set loopExitAbsValOld for loops
    if (isSourceOfBackEdge)
    {
      t_MyCFGInfo->loopExitAbsValOld = loopExitAbsValOld;
    }

    myCFGInfoList.push_back(t_MyCFGInfo);
  }
  else
  {
    // entry exists, update MyCFGInfo entry
    myCFGInfo->terminatorType = termType;

    if ((strcmp(termType.c_str(), "If") == 0) ||
        (strcmp(termType.c_str(), "While") == 0) ||
        (strcmp(termType.c_str(), "DoWhile") == 0) ||
        (strcmp(termType.c_str(), "For") == 0) )
    {
      // blocks with conditions and loops as teminator
      myCFGInfo->blockAbsVal = absMemPtr->blockAbsVal;
      myCFGInfo->condAbsVal = absMemPtr->condAbsVal;
      myCFGInfo->negCondAbsVal = absMemPtr->negCondAbsVal;
    }
    else
    //if (strcmp(termType.c_str(), "None") == 0)
    if ((strcmp(termType.c_str(), "None") == 0) ||
        (strcmp(termType.c_str(), "Goto") == 0) )
    {
      // blocks with no terminator
      myCFGInfo->blockAbsVal = absMemPtr->blockAbsVal;
    }
    else
    if (strcmp(termType.c_str(), "Empty") == 0)
    {
      // entry and exit blocks
      myCFGInfo->blockAbsVal = absMemPtr->blockAbsVal;
    }

    // set loopExitAbsValOld for loops
    if (isSourceOfBackEdge)
    {
      t_MyCFGInfo->loopExitAbsValOld = loopExitAbsValOld;
    }

  } // else found ends
}

/// \brief Print CFG info in structure MyCFGInfo
void MyCFG::printCFGInfo()
{
  MyCFGInfoList::iterator it;
  MyCFGInfo* t_MyCFGInfo;

  std::cerr << "\nBlockId CFGBlock \tTermType";
  std::cerr << " blockAbsVal \tcondAbsVal \tnegCondAbsVal\tloopExitAbsValOld\n";
  std::cerr << "------------------------------------------------------------";
  std::cerr << "------------------------------------------------------------\n";

  for (it  = myCFGInfoList.begin();
       it != myCFGInfoList.end();
       it++)
  {
    t_MyCFGInfo = *it;

    std::cerr << t_MyCFGInfo->blockid << "\t";
    std::cerr << t_MyCFGInfo->cfg_block << "\t";
    std::cerr << t_MyCFGInfo->terminatorType << "\t";

    if ((strcmp(t_MyCFGInfo->terminatorType.c_str(), "If") == 0) ||
        (strcmp(t_MyCFGInfo->terminatorType.c_str(), "While") == 0) ||
        (strcmp(t_MyCFGInfo->terminatorType.c_str(), "DoWhile") == 0) ||
        (strcmp(t_MyCFGInfo->terminatorType.c_str(), "For") == 0) )
    {
      ap_abstract1_fprint(stderr, man, &(t_MyCFGInfo->blockAbsVal));
      ap_abstract1_fprint(stderr, man, &(t_MyCFGInfo->condAbsVal));
      ap_abstract1_fprint(stderr, man, &(t_MyCFGInfo->negCondAbsVal));
    }
    else
    //if (strcmp(t_MyCFGInfo->terminatorType.c_str(), "None") == 0)
    if ((strcmp(t_MyCFGInfo->terminatorType.c_str(), "None") == 0) ||
        (strcmp(t_MyCFGInfo->terminatorType.c_str(), "Goto") == 0) )
    {
      ap_abstract1_fprint(stderr, man, &(t_MyCFGInfo->blockAbsVal));

      std::cerr << "----\t\t";
      std::cerr << "----\t\t";
    }
    else
    if (strcmp(t_MyCFGInfo->terminatorType.c_str(), "Empty") == 0)
    {
      ap_abstract1_fprint(stderr, man, &(t_MyCFGInfo->blockAbsVal));

      std::cerr << "----\t\t";
      std::cerr << "----\t\t";
    }

    if (t_MyCFGInfo->isSourceOfBackEdge)
    {
      ap_abstract1_fprint(stderr, man, &(t_MyCFGInfo->loopExitAbsValOld));
    }
    else
    {
      std::cerr << "----";
    }

    std::cerr << "\n";
  }
}

