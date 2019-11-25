#ifndef FILTERVARIABLES_GUARD
#define FILTERVARIABLES_GUARD 1

#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>
#include <map>
#include <memory>
#include <set>

namespace llvm {
  class CallInst;
  class GlobalVariable;
  class Instruction;
  class raw_fd_ostream;
  class Value;
}


using namespace std; 
using namespace llvm;
extern cl::opt<string> OutputFileName;

typedef set<Value*> Variables;


class FilterVariables : public ModulePass {
  
public:
  FilterVariables() : ModulePass(ID) {
  }
  
  bool runOnFunction(Function &F);

  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  map<const Function*, Variables> &getLocals();

  Variables &getGlobals();
  
  static char ID; // Pass identification, replacement for typeid

private:  
  map<unsigned int, string> errorNames;
  
  map<Value*, Value*> allocaToArg;

  map<const Function*, Variables> locals;

  Variables globals;

  map<const Function*, Variables> relevantLocals;

  Variables relevantGlobals;

  bool repeat;

  void printRelevantVars(Module &M);

  void printIrrelevantVars(Module &M);
};

#endif // FILTERVARIABLES_GUARD

/////////////////////////////////////////////////////////////////////

inline map<const Function*, Variables> &
FilterVariables::getLocals()
{
  return relevantLocals;
}


inline Variables &
FilterVariables::getGlobals()
{
  return relevantGlobals;
}


