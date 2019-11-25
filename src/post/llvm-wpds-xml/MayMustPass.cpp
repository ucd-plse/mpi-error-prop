#include "llvm-version.hpp"
#include "MayMustPass.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#else
#include <llvm/Support/CFG.h>
#endif

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SetOperations.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/FileSystem.h>

using namespace llvm;
using namespace std;


static raw_ostream & operator<<(raw_ostream &, const MayMustPass::ItemSet &) __attribute__((unused));

static raw_ostream &
operator<<(raw_ostream &out, const MayMustPass::ItemSet &items)
{
  out << '{';
  for (MayMustPass::ItemSet::const_iterator item = items.begin(), first = item, done = items.end(); item != done; ++item)
    {
      if (item != first)
	out << ", ";
      out << **item;
    }
  out << '}';
  return out;
}


bool
MayMustPass::doInitialization(Module &)
{
#if LLVM_VERSION >= 30600
  outputFile = make_unique<raw_fd_ostream>(getOutputFileName(), errorCode, sys::fs::F_None);
#else
  outputFile.reset(new raw_fd_ostream(getOutputFileName().c_str(), errorInfo, sys::fs::F_None));
#endif
  return false;
}


bool
MayMustPass::runOnFunction(Function &function)
{
  // must items directly from each basic block
  typedef DenseMap<const BasicBlock *, ItemSet> BlockMap;

  // blocks under reconsideration for fixed point; would probably
  // converge faster if maintained as FIFO without duplicates instead
  // of arbitrarily-ordered hash set
  typedef DenseSet<BasicBlock *> BlockWorkList;
  BlockWorkList workList;
  assert(function.begin() != function.end());
  workList.insert(function.begin());

  // fixed-point computation of must items along CFG paths
  BlockMap mustAlongPath;
  do
    {
      // fetch some block to reconsider
      BlockWorkList::iterator workItem = workList.begin();
      BasicBlock * const block = *workItem;
      workList.erase(workItem);

      // updated dataflow fact, computed as...
      ItemSet refined;

      // ...intersection across all predecessors...
      pred_iterator predecessor = pred_begin(block), done = pred_end(block);
      if (predecessor != done)
	{
	  refined = mustAlongPath[*predecessor];
	  for (; predecessor != done; ++predecessor)
	    set_intersect(refined, mustAlongPath[*predecessor]);
	}

      // ... unioned with direct items from this block
      collectFromBlock(*block, refined);

      // check for fact change from previous iteration
      const BlockMap::const_iterator mapping = mustAlongPath.find(block);
      if (mapping == mustAlongPath.end() || mapping->second != refined)
	{
	  // changed, so reconsider all successors
	  mustAlongPath[block] = refined;
	  for (succ_iterator successor = succ_begin(block), done = succ_end(block); successor != done; ++successor)
	    workList.insert(*successor);
	}
    }
  while (!workList.empty());

  // final may set is union across reachable blocks
  ItemSet may;
  for (BlockMap::const_iterator mapping = mustAlongPath.begin(), done = mustAlongPath.end(); mapping != done; ++mapping)
    set_union(may, mapping->second);

  // final must set is intersection across all reachable return blocks
  ItemSet must;
  bool alreadyFoundFirstReturnBlock = false;
  for (BlockMap::const_iterator mapping = mustAlongPath.begin(), done = mustAlongPath.end(); mapping != done; ++mapping)
    {
      const BasicBlock * const block = mapping->first;
      if (isa<ReturnInst>(block->getTerminator()))
	{
	  if (alreadyFoundFirstReturnBlock)
	    set_intersect(must, mapping->second);
	  else
	    {
	      must = mapping->second;
	      alreadyFoundFirstReturnBlock = true;
	    }
	}
    }

  // store results for downstream processing
  const ItemSet::const_iterator notInMust = must.end();
  for (ItemSet::const_iterator item = may.begin(), done = may.end(); item != done; ++item)
    {
      *outputFile << function.getName() << ' ';
      if (*item)
	*outputFile << (*item)->getName();
      else
	*outputFile << '*';
      *outputFile << ' '
		 << (must.find(*item) == notInMust ? '?' : '!')
		 << '\n';
    }

  return false;
}


bool
MayMustPass::doFinalization(Module &)
{
  // close and dispose of file; last chance for errors
  outputFile.reset(NULL);

  // report errors, if any arose
#if LLVM_VERSION >= 30600
  if (errorCode)
    {
      errs() << "cannot write to " << getOutputFileName() << ": " << errorCode.message() << '\n';
      exit(1);
    }
#else
  if (!errorInfo.empty())
    {
      errs() << "cannot write to " << getOutputFileName() << ": " << errorInfo << '\n';
      exit(1);
    }
#endif

  return false;
}


void
MayMustPass::getAnalysisUsage(AnalysisUsage &usage) const
{
  // changes nothing; requires nothing
  usage.setPreservesAll();
}
