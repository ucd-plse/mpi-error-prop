#ifndef PRINT_PROLOGUE_GUARD
#define PRINT_PROLOGUE_GUARD 1

#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#endif

#include <llvm/Pass.h>
#include <map>
#include <set>

namespace llvm {
  class Value;
  class GlobalVariable;
  class raw_fd_ostream;
}

using namespace std;
using namespace llvm;

typedef set<Value*> Variables;

class PrintPrologue : public ModulePass {

public:
  PrintPrologue() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);

  bool runOnFunction(Function &F);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  void printVarElement(const StringRef unique, const string &original);

  void renameAndPrint(Value &value, const Function &function, const StringRef original);

  void printTmpVarElement(Value &value, const Function &function, int counter);
  
  static char ID; // Pass identification, replacement for typeid


private:
  StringRef findVariableName(const GlobalVariable &);

  DebugInfoFinder debugInfo;

  map<const Function*, Variables> locals;

  map<Value*, Value*> argToAlloca;

#if __cplusplus > 199711L
  unique_ptr<raw_fd_ostream> outfile;
#else
  auto_ptr<raw_fd_ostream> outfile;
#endif

};

#endif // PRINT_PROLOGUE_GUARD
