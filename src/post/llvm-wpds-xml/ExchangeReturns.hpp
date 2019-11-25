#ifndef EXCHANGE_RETURNS_GUARD
#define EXCHANGE_RETURNS_GUARD 1

#include "InstBoolVisitor.hpp"

#include <llvm/Pass.h>
#include <map>
#include <set>

namespace llvm
{
  class GlobalVariable;
}

typedef set<Value*> Variables;


class ExchangeReturns : public llvm::ModulePass, public InstBoolVisitor<ExchangeReturns>
{
public:
  ExchangeReturns() : ModulePass(ID) {}

  bool runOnFunction(llvm::Function &);

  virtual bool runOnModule(Module &M);

  void getAnalysisUsage(llvm::AnalysisUsage &) const;

  static char ID; // Pass identification, replacement for typeid

  using InstBoolVisitor<ExchangeReturns>::visit;

  bool visit(llvm::BasicBlock &);

  bool visitReturnInst(llvm::ReturnInst &);

private:
  llvm::GlobalVariable *exchangeVar;

  Variables globals;

};

#endif // EXCHANGE_RETURNS_GUARD
