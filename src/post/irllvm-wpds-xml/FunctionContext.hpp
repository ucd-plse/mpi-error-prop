#ifndef FUNCTIONCONTEXT_HPP
#define FUNCTIONCONTEXT_HPP

#include "VarName.hpp"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include <map>
#include <set>
#include <unordered_set>

// Map instructions to (in,out) sets
typedef std::map<VarName, std::set<std::string>> Facts;
typedef std::pair<Facts, Facts> FactsPair;

class FunctionContextPass : public llvm::ModulePass {
public:
  static char ID;
  FunctionContextPass() : ModulePass(ID) {}

  bool runOnModule(llvm::Module &M);
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  std::map<llvm::Instruction*, FactsPair> facts;
  void dump_facts(llvm::Instruction *inst);

private:
  void runOnFunction(llvm::Function &F);
  bool runOnBlock(llvm::BasicBlock &BB);
  bool join(llvm::Instruction *predecessor, llvm::Instruction *inst);
  bool transfer(llvm::Instruction *inst);


};

#endif
