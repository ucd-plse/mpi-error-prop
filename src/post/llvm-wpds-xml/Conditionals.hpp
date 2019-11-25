#ifndef CONDITIONALS_GUARD
#define CONDITIONALS_GUARD 1

#include "InstBoolVisitor.hpp"

#include <llvm/Pass.h>
#include <map>
#include <set>

namespace llvm {
  class ConstantInt;
  class LLVMContext;
  class Value;
}

using namespace std;
using namespace llvm;

typedef set<Value*> Variables;


class Conditionals : public ModulePass, public InstBoolVisitor<Conditionals>
{
  
public:
  Conditionals() : ModulePass(ID) {
  }

  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  static char ID; // Pass identification, replacement for typeid

  bool visitBranchInst(const BranchInst &);

private:
  bool maybeClearError(const Value &maybeIntVar, const Value &maybeZero,
				     const BranchInst &branchInst, bool forOutcome);
  
  bool maybeClearError(const Value &op0, const Value &op1,
				     const BranchInst &branchInst,
				     bool intZeroOutcome, bool zeroIntOutcome);

  map<const Function*, Variables> locals;

  Variables globals;

};

#endif // CONDITIONALS_GUARD
