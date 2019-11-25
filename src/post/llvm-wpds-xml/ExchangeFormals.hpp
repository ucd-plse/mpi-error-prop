#ifndef EXCHANGE_FORMALS_GUARD
#define EXCHANGE_FORMALS_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;


class ExchangeFormals : public ModulePass
{
public:
  ExchangeFormals() : ModulePass(ID) {}

  bool runOnFunction(Function &function);
  
  virtual bool runOnModule(Module &M);

  void getAnalysisUsage(AnalysisUsage &) const;

  static char ID; // Pass identification, replacement for typeid
};

#endif // EXCHANGE_ASSIGNMENTS_GUARD
