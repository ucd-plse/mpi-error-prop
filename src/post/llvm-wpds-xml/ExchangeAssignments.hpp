#ifndef EXCHANGE_ASSIGNMENTS_GUARD
#define EXCHANGE_ASSIGNMENTS_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;


class ExchangeAssignments : public ModulePass {
  
public:
  ExchangeAssignments() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  static char ID; // Pass identification, replacement for typeid

};

#endif // EXCHANGE_ASSIGNMENTS_GUARD
