#ifndef FIND_INDIRECT_CALLS_GUARD
#define FIND_INDIRECT_CALLS_GUARD

#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/InstVisitor.h>
#else
#include <llvm/InstVisitor.h>
#endif

#include <llvm/Pass.h>

#include <vector>

namespace llvm
{
  class AnalysisUsage;
  class CallInst;
  class Function;
}


class FindIndirectCalls : public llvm::FunctionPass, public llvm::InstVisitor<FindIndirectCalls>
{
public:
  // standard LLVM function pass infrastructure
  FindIndirectCalls();
  bool runOnFunction(llvm::Function &);
  void getAnalysisUsage(llvm::AnalysisUsage &) const;
  static char ID;

  void visitCallInst(llvm::CallInst &);

  typedef std::vector<llvm::CallInst *> IndirectCalls;
  const IndirectCalls &found() const;

private:
  IndirectCalls indirectCalls;
};


////////////////////////////////////////////////////////////////////////


inline
FindIndirectCalls::FindIndirectCalls()
: FunctionPass(ID)
{
}


inline const FindIndirectCalls::IndirectCalls &
FindIndirectCalls::found() const
{
  return indirectCalls;
}


#endif	// !FIND_INDIRECT_CALLS_GUARD
