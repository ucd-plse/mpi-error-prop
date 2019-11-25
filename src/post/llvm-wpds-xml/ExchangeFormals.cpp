#include "CollectVariables.hpp"
#include "ExchangeFormals.hpp"

#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <set>

#include <sstream>

using namespace llvm;

typedef set<Value*> Variables;


bool ExchangeFormals::runOnModule(Module &M) {
  
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    if (!f->isDeclaration()) {
      runOnFunction(*f);
    }
  }

  return false;
}

bool ExchangeFormals::runOnFunction(Function &function) {
  
  Module &module = *function.getParent();
  CollectVariables &vars = getAnalysis<CollectVariables>();
  const map<Value*, Value*> &argToAlloca = vars.getArgToAlloca();
  const Variables &globals = vars.getGlobals();
  map<const Function*, Variables> &locals = vars.getLocals();
  
  // splitting entry block
  BasicBlock &entryBlock = function.getEntryBlock();
  for(BasicBlock::iterator i = entryBlock.begin(), ie = entryBlock.end(); i != ie; i++)
    if (!isa<AllocaInst>(i)) {
      entryBlock.splitBasicBlock(i);
      break;
    }
  
  // adding assignments at the BEGINNING of the function
  unsigned numArgument = 1;
  for(Function::arg_iterator a = function.arg_begin(), ae = function.arg_end(); a != ae; a++) {
    
    map<Value*, Value*>::const_iterator it = argToAlloca.find(a);
    if (it == argToAlloca.end()) continue;
    
    ostringstream formatter;
    formatter << function.getName().str() << '$' << numArgument++;
    GlobalVariable* exchangeVar = module.getNamedGlobal(formatter.str());
    
    Instruction *terminator = entryBlock.getTerminator();
    Value* receiver = it->second;
    
    if (locals[&function].find(receiver) != locals[&function].end() &&
	globals.find(exchangeVar) != globals.end()) {
      
      LoadInst *loadInst = new LoadInst(exchangeVar, exchangeVar->getName().str(), terminator);
      new StoreInst(loadInst, receiver, terminator);
    }
  }

  return false;
}  



void ExchangeFormals::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<CollectVariables>();
}

char ExchangeFormals::ID = 0;
static const RegisterPass<ExchangeFormals> registration("exchange-formals", "Copy global exchange variables into formal arguments");
