#ifndef COLLECT_VARIABLES_GUARD
#define COLLECT_VARIABLES_GUARD 1

#include <llvm/Pass.h>
#include <map>
#include <set>

namespace llvm {
  class Value;
  class GlobalVariable;
}

using namespace std;
using namespace llvm;

typedef set<Value*> Variables;

class CollectVariables : public ModulePass {

public:
  CollectVariables() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  const map<Value*, Value*> &getAllocaToArg() const;
  const map<Value*, Value*> &getArgToAlloca() const;
  map<const Function*, Variables> &getLocals();
  Variables &getGlobals();
  
  static char ID; // Pass identification, replacement for typeid
  
private:
  map<Value*, Value*> allocaToArg; // can retrieve names from args
  map<Value*, Value*> argToAlloca; // can use AllocaInst when inserting assignments
  map<const Function*, Variables> locals; // to retrieve locals per function
  Variables globals; // to retrieve globals
  
};


////////////////////////////////////////////////////////////////////////


inline const map<Value*, Value*> &
CollectVariables::getAllocaToArg() const
{
  return allocaToArg;
}


inline const map<Value*, Value*> &
CollectVariables::getArgToAlloca() const
{
  return argToAlloca;
}


inline map<const Function*, Variables> &
CollectVariables::getLocals()
{
  return locals;
}


inline Variables &
CollectVariables::getGlobals()
{
  return globals;
}


#endif // COLLECT_VARIABLES_GUARD
