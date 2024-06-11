//
// MyASTVisitor.h
//

/// \file
/// \brief Defines AST node visitor class MyASTVisitor inherited from
/// clang::RecursiveASTVisitor
///
#include <vector>

#ifndef MYASTVISITOR_H
# define MYASTVISITOR_H

#include "clang/Basic/FileManager.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Analysis/Analyses/CFGReachabilityAnalysis.h"

#include "Apron.h"

/// \brief Color constants used to mark CFGBlocks and edges in CFG traversal,
/// see edgeColor in structure edgeStruct
const int GREEN = 4;
const int BLACK = 3;
const int RED   = 2;
const int GRAY  = 1;
const int WHITE = 0;

/// \brief Structure to hold CFG info
typedef struct MyCFGInfo
{
  /// \brief basic block id
  unsigned blockid;

  /// \brief Pointer to basic block
  clang::CFGBlock* cfg_block;

  /// \brief Does this block have a terminator
  std::string terminatorType;

  /// \brief Flag to indicate if current block is source of an back edge
  bool isSourceOfBackEdge;

  // AbstractMemory

  /// \brief Abstract value of a CFGBlock
  ap_abstract1_t blockAbsVal;

  /// \brief Abstract value of positive of condition
  ap_abstract1_t condAbsVal;

  /// \brief Abstract value of negative of condition
  ap_abstract1_t negCondAbsVal;

  /// \brief Old Abstract value at exit block of loop
  ap_abstract1_t loopExitAbsValOld;

} MyCFGInfo;

/// \brief Vector of MyCFGInfo records
typedef std::vector <MyCFGInfo* > MyCFGInfoList;

/// \brief Wrapper class for CFG
class MyCFG
{
  public:

    /// \brief Pointer to complete in-memory CFG of a function
    clang::CFG* cfg;

    /// \brief Set compiler instance
    /// @param ci - compiler instance
    /// @return void (nothing)
    void setCompilerInstance(clang::CompilerInstance* ci);

    /// \brief getCFG invokes CFG::buildCFG()
    /// @param functionDecl - pointer to FunctionDecl object
    ///        (coming from VisitFunctionDecl() parameter)
    /// @param body - body of function
    /// @param C - pointer to ASTContext obtained from CompilerInstance
    /// @param buildOpts - CFG build options
    /// @return clang::CFG* - pointer to CFG generated
    clang::CFG* getCFG(clang::FunctionDecl *functionDecl,
      clang::Stmt *body,
      clang::ASTContext *C,
      clang::CFG::BuildOptions buildOpts);

    /// \brief Add CFG info in structure MyCFGInfo
    /// @param blockid - block id of CFGBlock
    /// @param cfg_block - pointer to current CFGBlock
    /// @param termType - terminator type of current basic block
    /// @param isSourceOfBackEdge - is current block source of back edge (loop)
    /// @param absMemPtr - pointer to AbstractMemory structure
    /// @param loopExitAbsValOld - old Abstract value at exit of loop
    /// @return void (nothing)
    void addCFGInfo(unsigned blockid,
      clang::CFGBlock* cfg_block,
      std::string termType,
      bool isSourceOfBackEdge,
      AbstractMemory* absMemPtr,
      ap_abstract1_t loopExitAbsValOld);

    /// \brief Print CFG info in structure MyCFGInfo
    /// @param none
    /// @return void (nothing)
    void printCFGInfo();

private:

    /// \brief CompilerInstance handle
    clang::CompilerInstance* m_compilerInstance;
};

/// \brief Structure to hold edge information
typedef struct edgeStruct
{
  /// \brief Source block of edge
  clang::CFGBlock * sourceBlock;

  /// \brief Destination block of edge
  clang::CFGBlock * destBlock;

  /// \brief Flag to keep track whether loop head has been added to
  /// the end of the loop in the case of while and for loops
  bool hasBeenAddedWhile;

  /// \brief Flag to keep track whether if this is the first pass
  /// through the loop
  bool hasBeenVisited;

  /// \brief Flag to keep track whether this edge is a back edge
  bool isABackEdge;

  /// \brief Counter to keep count of the number of times this
  /// edge has been traversed
  int timesVisited;

  /// \brief Edge marking 0:Normal, 1: Condition True
  /// 2: Condition False, 3: Back Edge
  int edgeColor;

} edgeStruct;

/// \brief Vector of edges
typedef std::vector <edgeStruct *> edgeStructList;

/// \brief Structure to store information of predecessors and
/// successors of predecessors
typedef struct MySuccOfPredInfo
{
  /// \brief Current basic block
  clang::CFGBlock* cfgBlock;

  /// \brief Predecessor of current basic block
  clang::CFGBlock* predBlock;

  /// \brief Number of successors of predecessors of current basic block
  int numSuccOfPred;

  /// \brief Pointer to the current link.
  edgeStruct * pointerToEdge;

  /// \brief List of successors of predecessors of current basic block
  std::vector<clang::CFGBlock *> succOfPredList;

  /// \brief Pointer to next entry of structure MySuccOfPredInfo
  struct MySuccOfPredInfo* next;

} MySuccOfPredInfo;

/// \brief Temporary blockStruct for traversal list
typedef struct t_blockStruct
{
    clang::CFGBlock * blockPtr;
    int term;
} t_blockStruct;

/// \brief List of temporary blockStruct for traversal list
typedef std::vector <t_blockStruct *> t_blockStructList;

/// \brief Temporary global block list
extern t_blockStructList globalBlockList;

/// \brief Loop Structures to keep track of Loops
typedef struct t_loopStruct
{
  clang::CFGBlock * loopStart;
  clang::CFGBlock * loopEnd;
  std::vector <clang::CFGBlock *> pendingList;
  bool loop_done;
} t_loopStruct;

/// \brief List of temporary loopStruct
typedef std::vector<t_loopStruct *> t_loopStructList;

/// \brief Temporary list of loopStruct
extern t_loopStructList loopStructList;

/// \brief Loop counter
extern int loopCounter;

/// \brief AST visitor class inherited from clang::RecursiveASTVisitor
class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor>
{
  public:

    /// \brief Set compiler instance
    /// @param ci - compiler instance
    /// @return void (nothing)
    void setCompilerInstance(clang::CompilerInstance* ci);

    /// \brief Get terminator type (If/While/Do/For/None) for basic block
    /// @param cfg_block - pointer to current CFGBlock
    /// @return std::string - terminator type If / While / DoWhile / For
    std::string getTerminatorType(clang::CFGBlock* cfg_block);

    // Override AST visitor functions below
    // - Returning false from one of the overridden visitor functions
    //   will abort the entire traversal (no further going down in AST)

    /// \brief Override VisitDecl (for declarations outside FunctionDecl)
    /// @param D - pointer to declaration
    /// @return bool - flag to stop ast traversal (false) or continue (true)
    bool VisitDecl(clang::Decl *D);

    /// \brief Override VisitFunctionDecl() to traverse every FunctionDecl
    ///
    /// - generates CFG for every function
    ///
    /// - iterates over every basic block in CFG for the function
    ///
    /// - iterates over every statement in the basic block of the CFG
    ///
    /// @param FD - pointer to FunctionDecl object
    /// @return bool - flag to stop ast traversal (false) or continue (true)
    bool VisitFunctionDecl(clang::FunctionDecl *FD);

    // Functions required for CFG traversal and updating abstract value

    /// \brief Do pre-processing before analysis begins on CFG block list
    /// @param my_cfg - MyCFG structure
    /// @return void (nothing)
    void doPreprocessingBeforeAnalysis(MyCFG my_cfg);

    /// \brief Remove Unreachable Blocks from traversal List
    /// @param my_cfg - MyCFG structure
    /// @return void (nothing)
    void removeUnreachableBlocks(MyCFG my_cfg);

    /// \brief Return edge structure given source and destination blocks
    /// @param sourceBlock - pointer to source CFGBlock
    /// @param destBlock - pointer to destination CFGBlock
    /// @return edgeStruct* - pointer to edge info 
    edgeStruct* getEdge(clang::CFGBlock* sourceBlock,
      clang::CFGBlock* destBlock);

    /// \brief Mark an edge visited based on given in indices in blockList
    /// @param thisBlock - pointer to current CFGBlock
    /// @param nextBlock - pointer to next CFGBlock in blocklist
    /// @return void (nothing)
    void markVisited(clang::CFGBlock* thisBlock,
      clang::CFGBlock* nextBlock);

    /// \brief Get blockList - list of blocks to be visited in order
    /// @param cfg_block - pointer to current CFGBlock
    /// @param blocksToVisit - pointer to list of blockIDs marked visited
    /// @return void (nothing)
    void getList(clang::CFGBlock* cfg_block,
      int* blocksToVisit);

    /// \brief Does a depth first search top-down to detect the back edges.
    /// @param cfg_block - pointer to current CFGBlock
    /// @param blocksToVisit - pointer to list of blockIDs marked visited
    /// @return void (nothing)
    void findBackEdges(clang::CFGBlock* cfg_block,
      int* blocksToVisit);

    /// \brief Creates edges between two connected nodes in the cfg
    /// in the forward direction.
    /// @param source - pointer to source CFGBlock
    /// @param dest - pointer to destination CFGBlock
    /// @return void (nothing)
    void createEdges(clang::CFGBlock* source,
      clang::CFGBlock* dest);

    /// \brief Creates edges between two connected nodes in the
    /// cfg in the reverse direction.
    /// @param source - pointer to source CFGBlock
    /// @param dest - pointer to destination CFGBlock
    /// @return void (nothing)
    void createReverseEdges(clang::CFGBlock* source,
      clang::CFGBlock* dest);

    /// \brief If an edge is found to be a back edge in the DFS,
    /// set the corresponding flag.
    /// @param source - pointer to source CFGBlock
    /// @param dest - pointer to destination CFGBlock
    /// @return void (nothing)
    void createBackEdge(clang::CFGBlock* source,
      clang::CFGBlock* dest);

    /// \brief Displays all edges in forward direction in no particular order
    /// @param none
    /// @return void (nothing)
    void displayEdges();

    /// \brief Displays all edges in reverse direction in no particular order
    /// @param none
    /// @return void (nothing)
    void displayReverseEdges();

    /// \brief Displays all the back edges
    /// @param none
    /// @return void (nothing)
    void displayBackEdges();

    /// \brief Find if this edge is present in the list of back edges
    /// @param sourceBlock - pointer to source CFGBlock
    /// @param destBlock - pointer to destination CFGBlock
    /// @return bool - true if source->destination edge is a back edge
    bool isPresentInBackEdgeList(clang::CFGBlock * sourceBlock,
      clang::CFGBlock * destBlock);

    /// \brief Get number of predecessors
    /// @param block - pointer to current CFGBlock
    /// @return int - number of predecessors
    int getNumPredecessors(clang::CFGBlock* block);

    /// \brief Get number of successors
    /// @param block - pointer to current CFGBlock
    /// @return int - number of successors
    int getNumSuccessors(clang::CFGBlock* block);

    /// \brief Get list of predecessors
    /// @param block - pointer to current CFGBlock
    /// @return std::vector of clang::CFGBlock * - predecessor list
    std::vector<clang::CFGBlock *> getPredecessors(clang::CFGBlock* block);

    /// \brief Get list of successors
    /// @param block - pointer to current CFGBlock
    /// @return std::vector of clang::CFGBlock * - successor list
    std::vector<clang::CFGBlock *> getSuccessors(clang::CFGBlock* block);

    /// \brief Add an entry for successors of a predecessor of a basic block
    /// @param cfg_block - pointer to current CFGBlock
    /// @param pred_block - pointer to predecessor of current CFGBlock
    /// @return void (nothing)
    void addSuccOfPredInfo(clang::CFGBlock* cfg_block,
      clang::CFGBlock* pred_block);

    /// \brief Show successors of predecessors
    /// @param none
    /// @return void (nothing)
    void showSuccOfPredInfo();

    /// \brief Find if this block is unique successor -
    /// block with one predecessor and predecessor has only one successor
    /// @param cfg_block - pointer to current CFGBlock
    /// @return bool - true if current block is unique successor
    bool isUniqueSuccessor(clang::CFGBlock* cfg_block);

    /// \brief Get a row of structure MyCFGInfo
    /// @param block - pointer to current CFGBlock
    /// @return MyCFGInfo* - pointer to entry of current block in MyCFGInfo
    MyCFGInfo* getRowOfMyCFGInfo(clang::CFGBlock* block);

    /// \brief Find if this block is first successor of its predecessor -
    /// return true for first successor, false otherwise
    /// @param cfg_block - pointer to current CFGBlock
    /// @return bool - true if current block is first successor
    bool isFirstSuccOfPred(clang::CFGBlock* cfg_block);

    /// \brief In case of for loops and while loops, we add the loop head
    /// at the end of the loop tail in the traversal list
    /// @param cfg_block - pointer to current CFGBlock
    /// @return void (nothing)
    void processListWhileFor(clang::CFGBlock * cfg_block);

    /// \brief In case of Dowhile loops, we add the loop head to
    /// the block before loop tail at the end of the loop tail
    /// in the traversal list
    /// @param cfg_block - pointer to current CFGBlock
    /// @return void (nothing)
    void processListDoWhile(clang::CFGBlock * cfg_block);

    /// \brief When widening needs to be done, the pointer of the blocklist
    /// is shifted back to the loop head. Should be called from the source
    /// of a back edge.
    /// @param cfg_block - pointer to current CFGBlock
    /// @param original - blockid marker in blocklist to shift back for widening
    /// @return int - 
    int modifyListWiden(clang::CFGBlock * cfg_block,
      int original);

    /// \brief Find if this block is the source of a back edge
    /// @param cfg_block - pointer to current CFGBlock
    /// @return edgeStruct* - pointer to edge info otherwise NULL
    edgeStruct* isASourceOfBackEdge(clang::CFGBlock * cfg_block);

    /// \brief Find if this block is the destination of a back edge
    /// @param cfg_block - pointer to current CFGBlock
    /// @return edgeStruct* - pointer to edge info otherwise NULL
    edgeStruct* isADestOfBackEdge(clang::CFGBlock * cfg_block);

    /// \brief See if this is first visit to block leading to back edge
    /// @param cfg_block - pointer to current CFGBlock
    /// @return bool - true if exit block of loop is visited first time
    bool isFirstTime(clang::CFGBlock* cfg_block);

    /// \brief Reset timesVisited counter for back edge if fix point is reached
    /// @param cfg_block - pointer to current CFGBlock
    /// @return void (nothing)
    void resetTimesVisited(clang::CFGBlock* cfg_block);

    /// \brief Return loopExitAbsValOld for given cfg_block
    /// @param cfg_block - pointer to current CFGBlock
    /// @return ap_abstract1_t - old abstract value at exit of loop
    ap_abstract1_t getLoopExitAbsValOld(clang::CFGBlock* cfg_block);

    /// \brief Support member for Tarzan's Algorithm
    /// @param cfg_block - pointer to current CFGBlock
    /// @return bool - 
    bool isPresentinS(clang::CFGBlock * cfg_block);

    /// \brief Support member for Tarzan's Algorithm
    /// @param cfg_block - pointer to current CFGBlock
    /// @return void (nothing)
    void stronglyConnect(clang::CFGBlock* cfg_block);

    /// \brief Use Tarzan's Algorithm to find SCC in the CFG
    /// @param none
    /// @return void (nothing)
    void tarzanAlgo();

    /// \brief Populate the blockStructList containing the block pointer
    /// and the block terminator type
    /// @param my_cfg - MyCFG structure
    /// @return void (nothing)
    void createBlockList(MyCFG my_cfg);

    /// \brief Print the block structure
    /// @param none
    /// @return void (nothing)
    void printBlockStruct();

    /// \brief Print the loops encountered in the CFG
    /// @param none
    /// @return void (nothing)
    void printLoops();

    /// \brief Get traversal list for CFG with breaks
    /// @param my_cfg - MyCFG structure
    /// @return void (nothing)
    void getListBreak(MyCFG my_cfg);

    /// \brief Get pointer to the loop given the loop start node
    /// @param source -
    /// @return t_loopStruct * -
    t_loopStruct * getLoopSource(clang::CFGBlock * source);

    /// \brief Get pointer to the loop given the loop end node
    /// @param dest -
    /// @return t_loopStruct * -
    t_loopStruct * getLoopDest(clang::CFGBlock * dest);

    /// \brief Get blockList - list of blocks to be visited in order
    /// @param cfg_block - 
    /// @param myBlocksToVisit -
    /// @return void (nothing)
    void newGetList(clang::CFGBlock* cfg_block,
      int* myBlocksToVisit);

    /// \brief Visits all the successors of a given block
    /// @param cfg_block - 
    /// @param myBlocksToVisit -
    /// @return void (nothing)
    void succVisit(clang::CFGBlock * cfg_block,
      int * myBlocksToVisit);

    /// \brief Visits all the predecessors of a given block
    /// @param cfg_block - 
    /// @param myBlocksToVisit -
    /// @return void (nothing)
    void predVisit(clang::CFGBlock * cfg_block,
      int * myBlocksToVisit);

    /// \brief Visits all the pending blocks of a given loop
    /// @param myBlocksToVisit - to keep track if block has been visited
    /// @return void (nothing)
    void processPendingList(int * myBlocksToVisit);

  private:

    /// \brief CompilerInstance handle
    clang::CompilerInstance* m_compilerInstance;

    /// \brief Pointer to list of structure MySuccOfPredInfo
    MySuccOfPredInfo* SuccOfPredInfo;
};

#endif //MYASTVISITOR_H

