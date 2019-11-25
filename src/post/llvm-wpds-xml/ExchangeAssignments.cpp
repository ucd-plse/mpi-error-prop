#include "CollectVariables.hpp"
#include "ExchangeAssignments.hpp"
#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#else
#include <llvm/Support/CFG.h>
#endif

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <set>

#include <sstream>

typedef set<Value*> Variables;


bool ExchangeAssignments::runOnModule(Module &M) {

  CollectVariables &vars = getAnalysis<CollectVariables>();
  const Variables &globals = vars.getGlobals();

  // iterating through functions
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {

    if (! f->isDeclaration()) {

      /**************************************/
      /* Keeping callInsts in its own block */
      /**************************************/

      // splitting block that contains callInst (not first instruction)   
      for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {      
	
	Instruction &first = b->front();
	
	// iterating through instructions 
	for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	  if (CallInst * const ins = dyn_cast<CallInst>(i)) {
	    if (&first != ins) {
	      b->splitBasicBlock(ins);
	      break;
	    }
	  }
	}
      }
      
      // splitting block that contains callInst (first instruction)
      for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {      
	
	unsigned int numberInst = b->getInstList().size();
	
	if (numberInst > 1) {
	  
	  // iterating through instructions 
	  for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	    if (isa<CallInst>(i)) {
	      Instruction *next = ++i;
	      b->splitBasicBlock(next);
	      break;
	    }
	  }
	}
      }

      
      /*******************************************************************/
      /* Adding exchange-variable assignments BEFORE and AFTER callInsts */
      /*******************************************************************/

      for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {      
	
	Instruction &ins = b->front();
	
	if (CallInst * const callInst = dyn_cast<CallInst>(&ins)) {

	  Function *callee = callInst->getCalledFunction();
	  if (callee && !callee->isDeclaration()) {
	    string fname = callee->getName().str();
	  	  
	    // retrieving exchange variables
	    // adding assignments BEFORE callInst
	    unsigned int numOperands = callInst->getNumArgOperands();
	    
	    for(unsigned int numOperand = 0; numOperand < numOperands; numOperand++) {
	    
	      ostringstream formatter;
	      formatter << fname << '$' << numOperand + 1;
	      GlobalVariable* exchangeVar = M.getNamedGlobal(formatter.str());
	      
	      if (exchangeVar && globals.find(exchangeVar) != globals.end()) {
		Value *arg = callInst->getArgOperand(numOperand);
		new StoreInst(arg, exchangeVar, callInst);
	      }
	    }
	  

	    // retrieving return exchange variable, target to store function return value
	    // adding assignment AFTER call instruction
	    string name = fname + "$return";
	    StringRef nameRef(name);
	    
	    GlobalVariable* exchangeRetVar = M.getNamedGlobal(nameRef);
	    if (exchangeRetVar && globals.find(exchangeRetVar) != globals.end()) {
	      
	      succ_iterator succBlock = succ_begin(b);
	      Instruction *nextInst = &succBlock->front();
	      Instruction *terminator = b->getTerminator();

	      // pattern 1:
	      // %r = call
	      // store %r, receiver

	      if (const StoreInst * const storeInst = dyn_cast<StoreInst>(nextInst)) {

		if (storeInst->getOperand(0) == callInst) {

		  Value* receiver = storeInst->getOperand(1);
		  LoadInst *retval = new LoadInst(exchangeRetVar, exchangeRetVar->getName().str(), terminator);

		  StoreInst *storeInst = new StoreInst(retval, receiver, terminator);
		  storeInst->setMetadata(LLVMContext::MD_dbg, callInst->getMetadata("dbg"));

		  b->splitBasicBlock(retval);		    
		}
	      }
	      else {
		
		// pattern 2 (caller returns long, 64 bit)
		// %r = call
		// %s = trunc %r
		// store %s, receiver
		BasicBlock::iterator it(nextInst);
		++it;
		if (it != nextInst->getParent()->end()) {
		  Instruction *nextNextInst = it;
		  if (const StoreInst * const storeInst = dyn_cast<StoreInst>(nextNextInst)) {
		    if (const TruncInst * const trunInst = dyn_cast<TruncInst>(storeInst->getOperand(0))) {
		      if (trunInst->getOperand(0) == callInst) {

			Value* receiver = storeInst->getOperand(1);
			LoadInst *retval = new LoadInst(exchangeRetVar, exchangeRetVar->getName().str(), terminator);

			////// new code
			Type *type = receiver->getType();
			
			if (CompositeType * const ctype = dyn_cast<CompositeType>(type)) {
			  
			  unsigned int index = 0;
			  const Type *itype = ctype->getTypeAtIndex(index);
		
			  if (itype->isIntegerTy(32)) {

			///// end new code

			    StoreInst *storeInst = new StoreInst(retval, receiver, terminator);
			    storeInst->setMetadata(LLVMContext::MD_dbg, callInst->getMetadata("dbg"));
			    
			    b->splitBasicBlock(retval);
			  }
			}
		      }
		    }
		  }
		} 
	      }
	    }	    

	    // construct and add asignment using block of call inst, so that it gets appended.
	  }

	}
      }
    }
  }
  return false;
}  



void ExchangeAssignments::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<CollectVariables>();
}

char ExchangeAssignments::ID = 0;
static const RegisterPass<ExchangeAssignments> registration("exchange", "Adds assignments for exchange variables");
