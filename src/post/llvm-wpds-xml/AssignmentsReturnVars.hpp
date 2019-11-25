#ifndef ASSIGNMENTS_RETURN_VARS_GUARD
#define ASSIGNMENTS_RETURN_VARS_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;


class AssignmentsReturnVars : public ModulePass {
  
public:
  AssignmentsReturnVars() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);

  //bool runOnFunction(Function &F);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  static char ID; // Pass identification, replacement for typeid
  
};

#endif // ASSIGNMENTS_RETURN_VARS_GUARD
