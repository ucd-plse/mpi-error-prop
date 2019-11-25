#include "AssignmentsReturnBlock.hpp"
#include "CollectVariables.hpp"
#include "Conditionals.hpp"
#include "ExchangeAssignments.hpp"
#include "ExchangeFormals.hpp"
#include "ExchangeReturns.hpp"
#include "FilterVariables.hpp"
#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#include <llvm/Support/CFG.h>
#endif

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>
#include <sstream>

using namespace llvm;


cl::opt<string> ErrorCodesFileName("error-codes", cl::value_desc("filename"), cl::desc("Error codes file"), cl::init("error-codes.txt"));


bool FilterVariables::runOnModule(Module &M) {

  // populating errorNames map
  ifstream inFile(ErrorCodesFileName.c_str());
  string name;
  int value;

  if (!inFile) {
    errs() << "Unable to open " << ErrorCodesFileName << '\n';
    exit(1);
  }

  while(inFile >> name >> value) {
    errorNames[value] = "TENTATIVE_" + name;
  }

  CollectVariables &vars = getAnalysis<CollectVariables>();
  allocaToArg = vars.getAllocaToArg();
  locals = vars.getLocals();
  globals = vars.getGlobals();
  
  repeat = true;

  while(repeat) {

    repeat = false;
    for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
      if (!f->isDeclaration()) {
	runOnFunction(*f);
      }
    }
  }

  return false;
}

bool FilterVariables::runOnFunction(Function &F) {
  int counter_tmp = 0;

  for(Function::iterator b = F.begin(), be = F.end(); b != be; b++) {    
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
      
      Instruction *currentInst = i;
      
      if (const StoreInst * const storeInst = dyn_cast<StoreInst>(currentInst)) {
    
	Value* sender = storeInst->getOperand(0);
	Value* receiver = storeInst->getOperand(1);
	
	Function* function = currentInst->getParent()->getParent();
	
	if (globals.find(receiver) != globals.end() ||
	    locals[function].find(receiver) != locals[function].end()) {
	  
	  string receiverName;
      
	  if (allocaToArg.find(receiver) != allocaToArg.end()) {
	    receiverName = allocaToArg.find(receiver)->second->getName().str();
	  }
	  else {
	    receiverName = receiver->getName().str();
	    
	    if (receiverName.empty()) {
	      ostringstream formatter;
	      formatter << "cabs2cil_" << counter_tmp++;
	      receiver->setName(formatter.str());
	    }	    
	  } 
	  
	  if (!receiverName.empty()) {

	    if (const ConstantInt * const constantInt = dyn_cast<ConstantInt>(sender)) {

	      int numberValue = constantInt->getLimitedValue();
	      
	      if (errorNames.find(numberValue) != errorNames.end()) {
		
		if (globals.find(receiver) != globals.end()) {
		  pair<Variables::iterator,bool> ret = relevantGlobals.insert(receiver);
		  repeat = repeat || ret.second;
		}
		else if (locals[function].find(receiver) != locals[function].end()) {
		  pair<Variables::iterator, bool> ret = relevantLocals[function].insert(receiver);
		  repeat = repeat || ret.second;
		}
	      }
	      
	    }
	    else {
	      
	      // sender is a register
	      if (const LoadInst *loadInst = dyn_cast<LoadInst>(sender)) {
		Value* realSender = loadInst->getOperand(0);

		if (globals.find(realSender) != globals.end() ||
		    locals[function].find(realSender) != locals[function].end()) {

		  string senderName;
		  if (allocaToArg.find(realSender) != allocaToArg.end()) {
		    senderName = allocaToArg.find(realSender)->second->getName().str();
		  }
		  else {
		    senderName = realSender->getName().str();
		    
		    if (senderName.empty()) {
		      ostringstream formatter;
		      formatter << "cabs2cil_" << counter_tmp++;
		      realSender->setName(formatter.str());
		    }
		  } 	
		  

		  if ((!loadInst->isVolatile()) && !senderName.empty()) {

		    if (relevantGlobals.find(realSender) != relevantGlobals.end() ||
			relevantLocals[function].find(realSender) != relevantLocals[function].end()) {
		      
		      if (globals.find(receiver) != globals.end()) {			
			pair<Variables::iterator,bool> ret = relevantGlobals.insert(receiver);
			repeat = repeat || ret.second;
		      }
		      else if (locals[function].find(receiver) != locals[function].end()) {
			pair<Variables::iterator, bool> ret = relevantLocals[function].insert(receiver);
			repeat = repeat || ret.second;
		      }
		    }
		  }
		} // if load
	      }	
	    }
	  }
	}
      }      
    }
  }
  return false;
}


void FilterVariables::printRelevantVars(Module &M) {

  errs() << "Globals:\n";
  for(Variables::iterator it = relevantGlobals.begin(), ite = relevantGlobals.end(); it != ite; it++) {
    errs() << "\t" << (*it)->getName() << "\n";
  }
  
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    errs() << "Function: " << f->getName() << "\n";
    for(Variables::iterator l = relevantLocals[f].begin(), le = relevantLocals[f].end(); l != le; l++) {
      errs() << "\t" << (*l)->getName() << "\n";      
    }
  }

  return;
}


void FilterVariables::printIrrelevantVars(Module &M) {

  errs() << "Globals:\n";  
  for(Variables::iterator it = globals.begin(), ite = globals.end(); it != ite; it++) {
    if (relevantGlobals.find(*it) == relevantGlobals.end()) {
      errs() << "\t" << (*it)->getName() << "\n";
    }
  }
  
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    errs() << "Function: " << f->getName() << "\n";
    for(Variables::iterator l = locals[f].begin(), le = locals[f].end(); l != le; l++) {
      if (relevantLocals[f].find(*l) == relevantLocals[f].end()) {
	errs() << "\t" << (*l)->getName() << "\n";  
      }    
    }
  }

  return;
}


// Check this when we decide when this pass is to be run
void FilterVariables::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<AssignmentsReturnBlock>();
  AU.addRequired<Conditionals>();
  AU.addRequired<ExchangeAssignments>();
  AU.addRequired<ExchangeFormals>();
  AU.addRequired<ExchangeReturns>();
  AU.addRequired<CollectVariables>();
  return;
}


char FilterVariables::ID = 0;
static const RegisterPass<FilterVariables> registration("filter", "Filters out irrelevant variables");
