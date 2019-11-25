#include "Calls.hpp"
#include "Rules.hpp"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Value.h>

#include <sstream>

using namespace llvm;


bool Calls::runOnModule(Module &M) {

  bool changed = false;

  // iterating through functions
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {

    // call instructions with missing receivers
    typedef vector<CallInst *> Fixups;
    Fixups fixups;

    for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {
      for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	if (CallInst *callInst = dyn_cast<CallInst>(i)) {
	  const Type *returnType = callInst->getType();
          
          bool used = false;
          for (Value::user_iterator I = callInst->user_begin(), E = callInst->user_end(); I != E; ++I) {
            if (isa<StoreInst>(*I)) {
              used = true;
            } 
          }
        
	  if (!Rules::isLLVMFunction(callInst) && returnType->isIntegerTy(32) && !used) {
	    // found missing receiver to fix up later
	    fixups.push_back(callInst);
	  }
	}
      }
    } // end iterating through instructions
    
    // creating local variables
    vector<Value*> tmps;
    for(unsigned i = 1; i <= fixups.size(); i++) {
      
      ostringstream formatter;
      formatter << "__cil_tmp" << i;

      IntegerType *ty = IntegerType::getInt32Ty(f->getContext());
      Instruction *first = f->begin()->begin(); // first block, first instruction
      Value* tmpVar = new AllocaInst(ty, formatter.str(), first);
      tmps.push_back(tmpVar);
    }


    // second pass through instructions
    // iterating through instructions

    for (Fixups::const_iterator fixup = fixups.begin(), end = fixups.end(); fixup != end; ++fixup)
      {
	// retrieving next instruction
	BasicBlock::iterator next(*fixup);
	Instruction *nextInst = ++next;
	      
	// insert receiver	      
	new StoreInst(*fixup, tmps.back(), nextInst);
	tmps.pop_back();
      }

    changed |= !fixups.empty();
  } // end iterating through functions

  return changed;
}  


void Calls::getAnalysisUsage(AnalysisUsage &AU) const {
  // Cannot setPreservesAll because we modify IR
  (void) AU;
}


char Calls::ID = 0;
static const RegisterPass<Calls> registration("calls", "Add missing receiver for non-void funcion calls");
