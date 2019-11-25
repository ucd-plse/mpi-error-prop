#ifndef LOWER_INDIRECT_CALLS_GUARD
#define LOWER_INDIRECT_CALLS_GUARD

#include <llvm/Pass.h>

namespace llvm
{
  class AnalysisUsage;
  class Function;
}


class LowerIndirectCalls : public llvm::FunctionPass
{
public:
  // standard LLVM function pass infrastructure
  LowerIndirectCalls();
  bool runOnFunction(llvm::Function &);
  void getAnalysisUsage(llvm::AnalysisUsage &) const;
  static char ID;
};


////////////////////////////////////////////////////////////////////////


inline
LowerIndirectCalls::LowerIndirectCalls()
: FunctionPass(ID)
{
}


#endif	// !LOWER_INDIRECT_CALLS_GUARD
