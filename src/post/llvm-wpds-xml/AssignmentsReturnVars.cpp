#include "AssignmentsReturnVars.hpp"
#include "Calls.hpp"
#include "CollectVariables.hpp"
#include "compat-types.hpp"
#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#else
#include <llvm/Support/CFG.h>
#endif

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h> // delete


#include <list>

using namespace llvm;


bool AssignmentsReturnVars::runOnModule(Module &M) {


  CollectVariables &vars = getAnalysis<CollectVariables>();
  map<const Function*, set<Value*> > &locals = vars.getLocals();

  // iterating through functions
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {

    // looking for block with return statement
    for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {
      if (ReturnInst * const returnInst = dyn_cast<ReturnInst>(b->getTerminator())) {

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
	
	// keeping predecessors of return block in separate list
	// we can't just iterate because branch removal invalidates the iterator
	list<BasicBlock*> predecessors;
	for(pred_iterator p = pred_begin(b), pe = pred_end(b); p != pe; p++) {
	  BasicBlock *pred = *p;
	  predecessors.push_back(pred);
	}

	// iterating through pred blocks
	for(list<BasicBlock*>::iterator it = predecessors.begin(), ite = predecessors.end(); it != ite; it++) {
	  
	  BasicBlock *pred = *it;
	  
	  // recording instructions for later backward traversal
	  list<Instruction*> instrs;
	  for (BasicBlock::iterator i = pred->begin(), ie = pred->end(); i != ie; i++) {
	    instrs.push_back(&*i);
	  }
	  	  
	  // traversing instructions in backward direction
	  for(list<Instruction*>::reverse_iterator it = instrs.rbegin(), ite = instrs.rend(); it != ite; it++) {

	    if (StoreInst* const storeInst = dyn_cast<StoreInst>(*it)) {

	      Value* sender = storeInst->getOperand(0);
	      Value* receiver = storeInst->getOperand(1);
	      
	      storeInst->dump();

	      if (LoadInst* const loadInst = dyn_cast<LoadInst>(sender)) {
		
		// checking to see whether it is the store we are looking for
		if (receiver == returnValue) {

		  Value* realSender = loadInst->getOperand(0);
		  
		  // is this sender (arg 0) a local var?
		  if (locals[f].find(realSender) != locals[f].end()) {
		    
		    if (realSender->hasName()) {

		      // removing the current terminator
		      if (BranchInst *branchInst = dyn_cast<BranchInst>(pred->getTerminator())) {
			branchInst->eraseFromParent();
		      }
		      
		      // adding return statement as the new terminator
		      const DebugLoc debugLoc = loadInst->getDebugLoc();
		      ReturnInst *retInst = ReturnInst::Create(f->getContext(), sender, pred);
		      retInst->setDebugLoc(debugLoc);
		      
		      break;
		    }
		  }
		}
	      }
	      else {
		// storing something other than a load

		errs() << "======================================\n";
		storeInst->dump();
		
		// removing the current terminator
		if (BranchInst *branchInst = dyn_cast<BranchInst>(pred->getTerminator())) {
		  branchInst->eraseFromParent();
		}

		// adding return statement as the new terminator
		const DebugLoc debugLoc = storeInst->getDebugLoc();
		LoadInst *loadInst = new LoadInst(receiver, "loadret", pred);
		ReturnInst *retInst = ReturnInst::Create(f->getContext(), loadInst, pred);
		retInst->setDebugLoc(debugLoc);
		
	      }
	    } // if load
	    //////// 
	  }
	} // for
      }
    }
  }

  return false;
}


void AssignmentsReturnVars::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<Calls>();
  AU.addRequired<CollectVariables>();
}

char AssignmentsReturnVars::ID = 0;
static const RegisterPass<AssignmentsReturnVars> registration("retvars", "Adds trusted assignments for returned variables");
