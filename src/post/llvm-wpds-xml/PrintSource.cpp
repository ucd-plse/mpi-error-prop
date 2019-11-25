#include "llvm-version.hpp"
#include "PrintSource.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#endif

#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

cl::opt<string> SourceFileName("source-file", cl::value_desc("filename"), cl::desc("File with source information"), cl::init("source-info.txt"));


static void printSubprogramDescriptor(raw_ostream &outfile, const DISubprogram subprogramDescriptor) {
  assert(subprogramDescriptor.Verify());
  outfile << subprogramDescriptor.getDirectory().str() << "/";
  outfile << subprogramDescriptor.getFilename().str() << "\n";
}


bool PrintSource::runOnModule(Module &M) {
#if LLVM_VERSION >= 30600
  error_code errorCode;
  raw_fd_ostream outfile(SourceFileName, errorCode, sys::fs::F_None);
#else
  string errorInfo;
  raw_fd_ostream outfile(SourceFileName.c_str(), errorInfo, sys::fs::F_None);
#endif

  DebugInfoFinder finder;
  finder.processModule(M);
#if LLVM_VERSION >= 30500
  for (const DISubprogram subprogramDescriptor : finder.subprograms())
    printSubprogramDescriptor(outfile, subprogramDescriptor);
#else
  for (DebugInfoFinder::iterator subprogram = finder.subprogram_begin(), end = finder.subprogram_end();
       subprogram != end; ++subprogram) {
    const DISubprogram subprogramDescriptor(*subprogram);
    printSubprogramDescriptor(outfile, subprogramDescriptor);
  }
#endif

  return false;
}


void PrintSource::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


char PrintSource::ID = 0;
static const RegisterPass<PrintSource> registration("print-source", "Prints file names where functions are located");
