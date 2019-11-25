#ifndef DIAGNOSTIC_GUARD
#define DIAGNOSTIC_GUARD

namespace llvm
{
  class Instruction;
  class raw_ostream;
}


llvm::raw_ostream &diagnostic(const llvm::Instruction &);


#endif	// !DIAGNOSTIC_GUARD
