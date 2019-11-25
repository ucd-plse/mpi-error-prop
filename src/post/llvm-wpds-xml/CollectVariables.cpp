#include "Calls.hpp"
#include "CollectVariables.hpp"
#include "llvm-version.hpp"
#include "MainFunction.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#endif

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>


#include <sstream>
#include <set>

using namespace llvm;


static bool
isInteger(const AllocaInst* allocaInst) {
  if (allocaInst && allocaInst->getAllocatedType()->isIntegerTy(32))
    return true;
  
  return false;
}

static bool
isInteger(const Value* value) {
  if (const AllocaInst* allocaInst = dyn_cast<AllocaInst>(value))
    return isInteger(allocaInst);

  return false;  
}

static bool
isInteger(const GlobalVariable* gvar) {

  Type* type = gvar->getType();
  if (CompositeType * const ctype = dyn_cast<CompositeType>(type)) {
    
    unsigned int index = 0;
    const Type *itype = ctype->getTypeAtIndex(index);
    
    if (itype->isIntegerTy(32)) {
      return true;
    }
  }

  return false;
}


bool CollectVariables::runOnModule(Module &M) {

  // Adding exchange variables

  // iterating through functions
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {

    static const StringRef prefix("llvm.eh.");
    if (!f->getName().startswith(prefix)) {

      // iterating through arguments
      for(Function::arg_iterator a = f->arg_begin(), ae = f->arg_end(); a != ae; a++) {
	
	// adding global exchange variables
	if (a->hasName()) {
	  ostringstream formatter;
	  formatter << f->getName().str() << '$' << a->getArgNo() + 1;
	  M.getOrInsertGlobal(formatter.str(), a->getType());
	}
      }
      
      // adding return exchange variables for functions with integer return type
      Type *type = f->getReturnType();
      
      if (type->isIntegerTy(32)) {
	string name = f->getName().str() + "$return";
	StringRef nameRef(name);
	M.getOrInsertGlobal(nameRef, type); // forcing to be 32-bit int (previously f->getReturnType())
      }
    }

  }


  // Collecting variables

  // collecting globals
  for(Module::global_iterator i = M.global_begin(), e = M.global_end(); i != e; i++) {    
    if (!GlobalValue::isPrivateLinkage(i->getLinkage()) && isInteger(i)) {
      globals.insert(i);
    }
  }


  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {

    // populating argument-related maps
    for(Function::arg_iterator a = f->arg_begin(), ae = f->arg_end(); a != ae; a++) {

      for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {
	for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	  
	  if (const StoreInst * const storeInst = dyn_cast<StoreInst>(i)) {	
	    if (storeInst->getOperand(0) == a && isInteger(storeInst->getOperand(1))) {
	      allocaToArg[storeInst->getOperand(1)] = a;
	      argToAlloca[a] = storeInst->getOperand(1);
	      break;
	    }
	  }
	}
      }
    }
    

    // collecting locals (= AllocaInsts)
    // no need to distinguish between original vars and temps
    for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {
      for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	if (AllocaInst * const allocaInst = dyn_cast<AllocaInst>(i)) {	  
	  if (isInteger(allocaInst)) {
	    locals[f].insert(allocaInst);
	  }
	}
      }    
    }

  }  
  return false;    
}  



void CollectVariables::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<Calls>();
  AU.addRequired<MainFunction>();
}

char CollectVariables::ID = 0;
static const RegisterPass<CollectVariables> registration("collect", "Collects global and local variables");
