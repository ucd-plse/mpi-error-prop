#include "diagnostic.hpp"
#include "llvm-version.hpp"
#include "ResolveCallees.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/InstVisitor.h>
#else
#include <llvm/InstVisitor.h>
#endif

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;


namespace
{
  class ResolveCalleesTester : public llvm::BasicBlockPass, public llvm::InstVisitor<ResolveCalleesTester>
  {
  public:
    // standard LLVM basic block pass infrastructure
    ResolveCalleesTester();
    bool runOnBasicBlock(llvm::BasicBlock &);
    void getAnalysisUsage(llvm::AnalysisUsage &) const;
    static char ID;

    void visitCallInst(const CallInst &) const;
  };
}


ResolveCalleesTester::ResolveCalleesTester()
: BasicBlockPass(ID)
{
}


bool
ResolveCalleesTester::runOnBasicBlock(BasicBlock &block)
{
  visit(block);
  return false;
}


void
ResolveCalleesTester::visitCallInst(const CallInst &callInst) const
{
  const ResolveCallees &resolver = getAnalysis<ResolveCallees>();
  ResolveCallees::FunctionSet callees;

  const bool resolved = resolver.resolve(callInst, callees);
  if (resolved && callees.empty())
    return;

  raw_ostream &out = diagnostic(callInst);
  out << " possible callees:";

  if (resolved)
    for (ResolveCallees::FunctionSet::const_iterator callee = callees.begin(), end = callees.end(); callee != end; ++callee)
      out << ' ' << (*callee)->getName();
  else
    out << " -unknown-";

  out << '\n';
}


void
ResolveCalleesTester::getAnalysisUsage(AnalysisUsage &usage) const
{
  usage.setPreservesAll();
  usage.addRequired<ResolveCallees>();
}


char ResolveCalleesTester::ID;
static const RegisterPass<ResolveCalleesTester> registration("resolve-callees-tester", "Test the resolve-callees pass by printing the possible callees at each call site");

#pragma GCC diagnostic ignored "-Wunused-parameter"
