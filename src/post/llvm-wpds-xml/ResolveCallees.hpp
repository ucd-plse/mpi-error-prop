#ifndef RESOLVE_CALLEES_GUARD
#define RESOLVE_CALLEES_GUARD

#include <llvm/Pass.h>

#include <tr1/unordered_set>
#include <vector>

namespace llvm
{
  class AnalysisUsage;
  class CallInst;
  class Function;
  class LoadInst;
  class Module;
  class Value;
}


class ResolveCallees : public llvm::ModulePass
{
public:
  // standard LLVM module pass infrastructure
  ResolveCallees();
  bool runOnModule(llvm::Module &);
  void getAnalysisUsage(llvm::AnalysisUsage &) const;
  static char ID;

  // a set of functions and/or methods
  typedef std::tr1::unordered_set<llvm::Function *> FunctionSet;

  // call instruction resolver
  //
  // If call instruction can be resolved to a set of possible callees,
  // returns true and updates method set accordingly.  Any elements
  // already in the set before the call are discarded.  Note: it is
  // possible to resolve successfully (returning true) to the empty
  // set of callees.  This can happen at direct calls to LLVM
  // debugging intrinsic functions, such as llvm.dbg.declare.
  //
  // If call instruction cannot be resolved to a set of possible
  // callees, returns false.
  bool resolve(const llvm::CallInst &, FunctionSet &) const;
  bool resolveIndirect(const llvm::CallInst &, FunctionSet &) const;

private:
  // call resolution strategies
  bool resolveVirtual(const llvm::CallInst &, FunctionSet &) const;
  bool resolveVirtualClang(const llvm::Value &, const llvm::LoadInst &, unsigned &) const;
  bool resolveVirtualDragonEgg(const llvm::Value &, const llvm::LoadInst &, unsigned &) const;
  bool resolveVirtualDragonEggSlot0(const llvm::Value &, const llvm::LoadInst &, unsigned &) const;

  // set of methods found at a given slot number in any vtable
  std::vector<FunctionSet> vtablesBySlot;
};


////////////////////////////////////////////////////////////////////////


inline
ResolveCallees::ResolveCallees()
: ModulePass(ID)
{
}


#endif	// !RESOLVE_CALLEES_GUARD
