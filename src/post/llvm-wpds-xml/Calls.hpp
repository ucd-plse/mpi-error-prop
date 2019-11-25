#ifndef CALLS_GUARD
#define CALLS_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;


class Calls : public ModulePass {
  
public:
  Calls() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  static char ID; // Pass identification, replacement for typeid

};

#endif // CALLS_GUARD
