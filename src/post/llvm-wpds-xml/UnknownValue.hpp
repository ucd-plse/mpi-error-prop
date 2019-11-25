#ifndef UNKNOWN_VALUE_GUARD
#define UNKNOWN_VALUE_GUARD

#include <llvm/Pass.h>

namespace llvm
{
  class AnalysisUsage;
  class BasicBlock;
  class GlobalVariable;
  class Instruction;
  class Module;
  class Twine;
  class Type;
}


class UnknownValue : public llvm::ModulePass
{
public:
  // standard LLVM module pass infrastructure
  UnknownValue();
  bool runOnModule(llvm::Module &);
  void getAnalysisUsage(llvm::AnalysisUsage &) const;
  static char ID;

  llvm::Instruction &fresh(const llvm::Twine &name, llvm::Type &, llvm::Instruction *insertBefore) const;
  llvm::Instruction &fresh(const llvm::Twine &name, llvm::Type &, llvm::BasicBlock *insertAtEnd) const;

private:
  llvm::GlobalVariable *value;

  template <typename Cursor> llvm::Instruction &freshAt(const llvm::Twine &, llvm::Type &, Cursor *) const;
};


////////////////////////////////////////////////////////////////////////


inline
UnknownValue::UnknownValue()
: ModulePass(ID)
{
}


#endif	// !UNKNOWN_VALUE_GUARD
