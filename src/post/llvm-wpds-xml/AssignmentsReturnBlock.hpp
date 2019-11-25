#ifndef ASSIGNMENTS_RETURN_BLOCK_GUARD
#define ASSIGNMENTS_RETURN_BLOCK_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;


class AssignmentsReturnBlock : public ModulePass {
  
public:
  AssignmentsReturnBlock() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  static char ID; // Pass identification, replacement for typeid
  
};

#endif // ASSIGNMENTS_RETURN_BLOCK_GUARD
