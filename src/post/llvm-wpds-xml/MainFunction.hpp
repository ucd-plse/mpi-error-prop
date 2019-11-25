#ifndef MAIN_FUNCTION_GUARD
#define MAIN_FUNCTION_GUARD 1

#include <llvm/Pass.h>

#include <tr1/unordered_set>

namespace llvm {
  class Value;
}

using namespace std;


class MainFunction : public ModulePass {

public:
  MainFunction() : ModulePass(ID) {
  }
  
  typedef std::tr1::unordered_set<llvm::Function *> FunctionSet;

  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  static char ID; // Pass identification, replacement for typeid

private:
  // none
  
};


#endif // MAIN_FUNCTION_GUARD
