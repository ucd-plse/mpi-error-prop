#include "AssignmentsReturnBlock.hpp"
#include "Calls.hpp"
#include "CollectVariables.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h> // delete

#include <list>

using namespace llvm;


bool AssignmentsReturnBlock::runOnModule(Module &M) {


  CollectVariables &vars = getAnalysis<CollectVariables>();
  map<const Function*, set<Value*> > &locals = vars.getLocals();

  // iterating through functions
  for (Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {


    // splitting block which contains return instruction    
    for (Function::iterator b = f->begin(), be = f->end(); b != be; b++) {

      unsigned int numberInst = b->getInstList().size();

      if (numberInst > 1) {
	Instruction* terminator = b->getTerminator();

	if (isa<ReturnInst>(terminator)) {
	  b->splitBasicBlock(terminator);
	}
      }
    }
    
    // iterating through instructions to insert assignments
    for (Function::iterator b = f->begin(), be = f->end(); b != be; b++)
      if (ReturnInst * const returnInst = dyn_cast<ReturnInst>(b->getTerminator()))

	// insert untrusted assignments for integer local variables at the end of the function
	for (set<Value*>::iterator it = locals[f].begin(), eit = locals[f].end(); it != eit; it++) {

	  Value *localVar = *it;
	  Value *returnValue = returnInst->getReturnValue();

	  if (const LoadInst * const loadInst = dyn_cast_or_null<LoadInst>(returnValue)) {
	    returnValue = loadInst->getOperand(0);
	  }
	  // if function return type is long (64 bit machine)
	  else if (const SExtInst * const sextInst = dyn_cast_or_null<SExtInst>(returnValue)) {
	    if (const LoadInst * const loadInst = dyn_cast_or_null<LoadInst>(sextInst->getOperand(0))) {
	      returnValue = loadInst->getOperand(0);
	    }
	  }

	  if (!returnValue || (localVar != returnValue)) {
	    
	    Type *type = localVar->getType();
	      
	    if (CompositeType * const ctype = dyn_cast<CompositeType>(type)) {
		
	      unsigned int index = 0;
	      const Type *itype = ctype->getTypeAtIndex(index);
		
	      if (itype->isIntegerTy(32)) { // so that it does not include char i8

		IntegerType *ty = IntegerType::getInt32Ty(f->getContext()); // could cast itype?
		ConstantInt *errorCode = ConstantInt::get(ty, -67737869);
		new StoreInst(errorCode, localVar, returnInst);
	      }
	    }
	  } // if !returnValue ...
	}

  }

  return false;
}  


void AssignmentsReturnBlock::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<Calls>();
  AU.addRequired<CollectVariables>();
}

char AssignmentsReturnBlock::ID = 0;
static const RegisterPass<AssignmentsReturnBlock> registration("assignments", "Adds assignments at the end of functions");
