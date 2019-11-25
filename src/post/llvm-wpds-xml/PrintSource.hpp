#ifndef PRINT_SOURCE_GUARD
#define PRINT_SOURCE_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;

class PrintSource : public ModulePass {

public:
  PrintSource() : ModulePass(ID) {
  }
  
  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  static char ID; // Pass identification, replacement for typeid

private:
  // none
  
};


#endif // PRINT_SOURCE_FUNCTION_GUARD
