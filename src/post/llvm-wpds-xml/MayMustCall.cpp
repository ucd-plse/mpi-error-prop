#include "llvm-version.hpp"
#include "MayMustCall.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#include <llvm/IR/InstVisitor.h>
#else
#include <llvm/InstVisitor.h>
#include <llvm/Support/CFG.h>
#endif

#include <llvm/ADT/SetOperations.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>
#include <memory>
#include <set>
#include <string>

using namespace llvm;
using namespace std;


static cl::opt<string> outputFileName("may-must-call-output",
				      cl::value_desc("filename"),
				      cl::desc("Output filename for may/must call analysis"),
				      cl::init("-"));

char MayMustCall::ID;
static const RegisterPass<MayMustCall> registration("may-must-call", "Find other functions that each function may or must directly call", true, true);


////////////////////////////////////////////////////////////////////////
//
//  collect all callees from a given function, basic block, etc.
//


namespace
{
  class CollectCalleesVisitor : public InstVisitor<CollectCalleesVisitor>
  {
  public:
    CollectCalleesVisitor(MayMustCall::ItemSet &);

    void visitCallInst(CallInst &);
    void visitIntrinsicInst(IntrinsicInst &);

  private:
    MayMustCall::ItemSet &callees;
  };
}


inline
CollectCalleesVisitor::CollectCalleesVisitor(MayMustCall::ItemSet &callees)
  : callees(callees)
{
}


inline void
CollectCalleesVisitor::visitCallInst(CallInst &callInst)
{
  Function * const callee = callInst.getCalledFunction();
  if (callee && !callee->empty())
    callees.insert(callee);
}


inline void
CollectCalleesVisitor::visitIntrinsicInst(IntrinsicInst &)
{
}


////////////////////////////////////////////////////////////////////////
//
//  main pass
//


const string &
MayMustCall::getOutputFileName() const
{
  return outputFileName;
}


void
MayMustCall::collectFromBlock(BasicBlock &block, ItemSet &items) const
{
  CollectCalleesVisitor(items).visit(block);
}
