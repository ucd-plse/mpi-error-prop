#include "FilterVariables.hpp"
#include "llvm-version.hpp"
#include "MayMustModifyGlobal.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#include <llvm/IR/InstVisitor.h>
#else
#include <llvm/InstVisitor.h>
#include <llvm/Support/CFG.h>
#endif

#include <llvm/ADT/SetOperations.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/PassAnalysisSupport.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>
#include <memory>
#include <set>
#include <string>

using namespace llvm;
using namespace std;


static cl::opt<string> outputFileName("may-must-modify-global-output",
				      cl::value_desc("filename"),
				      cl::desc("Output filename for may/must modify global analysis"),
				      cl::init("-"));

char MayMustModifyGlobal::ID;
static const RegisterPass<MayMustModifyGlobal> registration("may-must-modify-global", "Find global variables that each function may or must directly modify", true, true);


////////////////////////////////////////////////////////////////////////
//
//  collect all globals modified by a given function, basic block, etc.
//


namespace
{
  class CollectGlobalStoresVisitor : public InstVisitor<CollectGlobalStoresVisitor>
  {
  public:
    CollectGlobalStoresVisitor(const Variables &, MayMustModifyGlobal::ItemSet &);

    void visitStoreInst(StoreInst &);

  private:
    const Variables &relevant;
    MayMustModifyGlobal::ItemSet &collected;
  };
}


inline
CollectGlobalStoresVisitor::CollectGlobalStoresVisitor(const Variables &relevant, MayMustModifyGlobal::ItemSet &collected)
  : relevant(relevant),
    collected(collected)
{
}


inline void
CollectGlobalStoresVisitor::visitStoreInst(StoreInst &storeInst)
{
  Value * const destination = storeInst.getPointerOperand();
  if (GlobalVariable * const global = dyn_cast<GlobalVariable>(destination))
    if (relevant.find(global) != relevant.end())
      collected.insert(global);
}


////////////////////////////////////////////////////////////////////////
//
//  main pass
//


void
MayMustModifyGlobal::getAnalysisUsage(AnalysisUsage &usage) const
{
  MayMustPass::getAnalysisUsage(usage);
  usage.addRequired<FilterVariables>();
}


bool
MayMustModifyGlobal::runOnFunction(llvm::Function &function)
{
  relevant = &getAnalysis<FilterVariables>().getGlobals();
  return MayMustPass::runOnFunction(function);
}


const string &
MayMustModifyGlobal::getOutputFileName() const
{
  return outputFileName;
}


void
MayMustModifyGlobal::collectFromBlock(BasicBlock &block, ItemSet &items) const
{
  CollectGlobalStoresVisitor(*relevant, items).visit(block);
}
