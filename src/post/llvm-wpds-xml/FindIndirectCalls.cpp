#undef NDEBUG

#include "FindIndirectCalls.hpp"

#include <llvm/Support/raw_ostream.h>

using namespace llvm;


bool
FindIndirectCalls::runOnFunction(llvm::Function &function)
{
  indirectCalls.clear();
  visit(function);
  return false;
}


void
FindIndirectCalls::visitCallInst(CallInst &callInst)
{
  if (!callInst.getCalledFunction())
    indirectCalls.push_back(&callInst);
}

void
FindIndirectCalls::getAnalysisUsage(AnalysisUsage &usage) const
{
  usage.setPreservesAll();
}


char FindIndirectCalls::ID;
static const RegisterPass<FindIndirectCalls> registration("find-indirect-calls", "Find indirect call sites");


#pragma GCC diagnostic ignored "-Wunused-parameter"
