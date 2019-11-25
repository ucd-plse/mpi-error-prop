#ifndef RULES_GUARD
#define RULES_GUARD 1

#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>
#include <map>
#include <memory>
#include <set>

namespace llvm {
  class CallInst;
  class GlobalVariable;
  class Instruction;
  class raw_fd_ostream;
  class Value;
  class DILocation;
}


using namespace std; 
using namespace llvm;
extern cl::opt<string> OutputFileName;
extern cl::opt<string> ErrorCodesFileName;

typedef set<Value*> Variables;


class Rules : public ModulePass {
  
public:
  Rules() : ModulePass(ID) {
  }
  
  virtual bool runOnModule(Module &M);

  bool runOnFunction(Function &F);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  static char ID; // Pass identification, replacement for typeid

  static bool isLLVMFunction(CallInst *callInst);
  
private:
  Instruction* getNextInstruction(Instruction* i);

  CallInst* getCallInst(BasicBlock* block);

  void printWeight(Instruction *currentInst, bool &source);
  
  void printBlockRules(BasicBlock *block, Function *function);
  
  void printIntraRuleOpen(unsigned int fromStack, unsigned int toStack, Function *function);
  
  void printRuleClose();
  
  void printPushRule(BasicBlock *block, Function *caller, Function *callee, CallInst *callInst);
  
  void printPopRule(BasicBlock *block, Function *function);
  
  DILocation getSource(Instruction *currentInst, bool tryharder=false);

  void printSource(Instruction *currentInst);

  void printPred(Instruction *currentInst);
  
  void printInstructionWeight(Instruction *currentInst, bool &source);
  
  map<Instruction*, unsigned int> InstructionID;
  
  map<unsigned int, string> errorNames;
  
#if __cplusplus > 199711L
  unique_ptr<raw_fd_ostream> outfile;
  unique_ptr<raw_fd_ostream> locfile;
#else
  auto_ptr<raw_fd_ostream> outfile;
#endif

  map<const Function*, Variables> locals;

  Variables globals;
};

#endif // RULES_GUARD


