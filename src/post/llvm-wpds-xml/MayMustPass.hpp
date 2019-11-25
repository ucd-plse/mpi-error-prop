#ifndef INCLUDE_MayMustPass_hpp
#define INCLUDE_MayMustPass_hpp

#include "llvm-version.hpp"

#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <set>
#include <string>

namespace llvm
{
  class GlobalValue;
}


class MayMustPass : public llvm::FunctionPass
{
public:
  void getAnalysisUsage(llvm::AnalysisUsage &) const;
  bool doInitialization(llvm::Module &);
  bool runOnFunction(llvm::Function &);
  bool doFinalization(llvm::Module &);

  typedef std::set<llvm::GlobalValue *> ItemSet;

protected:
  MayMustPass(char &);

  virtual const std::string &getOutputFileName() const = 0;
  virtual void collectFromBlock(llvm::BasicBlock &, ItemSet &) const = 0;

private:
#if __cplusplus > 199711L
  std::unique_ptr<llvm::raw_fd_ostream> outputFile;
#else
  std::auto_ptr<llvm::raw_fd_ostream> outputFile;
#endif
#if LLVM_VERSION >= 30600
  std::error_code errorCode;
#else
  std::string errorInfo;
#endif
};


////////////////////////////////////////////////////////////////////////


inline
MayMustPass::MayMustPass(char &id)
: FunctionPass(id)
{
}


#endif	// !MAY_MUST_CALL_GUARD
