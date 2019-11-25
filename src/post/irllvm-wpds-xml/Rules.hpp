#ifndef RULESPASS_H
#define RULESPASS_H

#include "Rule.hpp"
#include "NameGraph.hpp"
#include "Names.hpp"
#include "BranchSafety.hpp"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstVisitor.h"
#include <fstream>

using namespace std;

class RulesPass : public llvm::ModulePass, public llvm::InstVisitor<RulesPass> {
public:

  static char ID;
  RulesPass() : llvm::ModulePass(ID) {}
  RulesPass(string wpds_out_path, string schema_path) :
    llvm::ModulePass(ID), wpds_out_path(wpds_out_path),
    schema_path(schema_path) {}

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

  bool runOnModule(llvm::Module &M) override;
  void visitStoreInst(llvm::StoreInst &I);
  void visitReturnInst(llvm::ReturnInst &I);
  void visitICmpInst(llvm::ICmpInst &I);
  void visitCallInst(llvm::CallInst &I);
  void visitLoadInst(llvm::LoadInst &I);
  void visitSwitchInst(llvm::SwitchInst &I);
  void visitBinaryOperator(llvm::BinaryOperator &I);
  void visitSelectInst(llvm::SelectInst &I);
  void visitBranchInst(llvm::BranchInst &I);

  // Default case
  void visitInstruction(llvm::Instruction &I);

  vector<FRule> get_rules() const { return rules; }

private:
  static RulesPrinter& RP;

  unsigned tmp_cnt = 1;
  map<llvm::Function*, vector<VarName>> tmp_names;

  void addRule(FRule &R);

  NamesPass *names;
  BranchSafetyPass *safety;

  string prev;

  set<string> local_names;
  set<string> global_names;

  vector<FRule> rules;

  string wpds_out_path;
  string schema_path;
};

#endif
