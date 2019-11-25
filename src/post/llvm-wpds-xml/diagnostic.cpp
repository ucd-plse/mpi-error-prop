#include "diagnostic.hpp"
#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#endif

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;


raw_ostream &
diagnostic(const Instruction &instruction)
{
  const DebugLoc &location = instruction.getDebugLoc();
  if (location.isUnknown())
    errs() << "-unknown-";
  else
    {
      const LLVMContext &context = instruction.getParent()->getParent()->getContext();
      const DIScope scope(location.getScope(context));
      const StringRef directory(scope.getDirectory());

      errs() << directory;
      if (directory.back() != '/')
	errs() << '/';

      errs() << scope.getFilename() << ':'
	     << location.getLine();

      const unsigned column = location.getCol();
      if (column)
	errs() << ':' << location.getCol();
    }

  errs() << ':';
  return errs();
}
