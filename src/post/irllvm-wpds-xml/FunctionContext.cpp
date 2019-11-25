#include "FunctionContext.hpp"
#include "BranchSafety.hpp"
#include "VarName.hpp"
#include "Utility.hpp"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include <iostream>

using namespace llvm;
using namespace std;

// The MemoryDependenceAnalysis pass appears unable to return non-local dependences
// That is, we cannot get instructions for dependences across basic blocks
// In practice, this results in about 20% of handlers not having a function context.

bool FunctionContextPass::runOnModule(Module &M)  {
  for (auto fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
    runOnFunction(*fi);
  }

  return false;
}

void FunctionContextPass::runOnFunction(Function &F)  {
  if (F.isIntrinsic() || F.isDeclaration()) return;

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto bbi = F.begin(), bbe = F.end(); bbi != bbe; ++bbi) {
      bool changed_here = runOnBlock(*bbi);
      changed = changed || changed_here;
    }
  }
  
}

bool FunctionContextPass::runOnBlock(BasicBlock &BB) {
  bool changed = false;
  vector<Instruction*> predecessors;

  // The first instruction in a basic block is special because we might
  // have to join multiple preceding blocks.
  auto ii = BB.begin(), ie = BB.end();  
  Instruction *first_inst = &*ii;
  
  // Join preceding basic blocks
  for (auto pi = pred_begin(&BB), pe = pred_end(&BB); pi != pe; ++pi) {
    BasicBlock *pred = *pi;   
    bool changed_here = join(pred->getTerminator(), first_inst);
    changed = changed || changed_here;
  }
 
  bool changed_here = transfer(first_inst);
  changed = changed || changed_here;
  Instruction *prev = first_inst;
  (void)prev;
  
  // Go through the rest of the instructions
  for (++ii; ii != ie; ++ii) {
    Instruction *current = &*ii;
    
    // Join with previous instruction
    changed_here = join(prev, current);
    changed = changed || changed_here;
    changed_here = transfer(current);
    changed = changed || changed_here;

    prev = current;
  }

  return changed;
}

void FunctionContextPass::dump_facts(Instruction *inst) {
  inst->dump();
  FactsPair inst_facts = facts[inst];
  cerr << "IN: ";
  Facts in_facts = inst_facts.first;
  for (auto fact_var : in_facts) {      
    cerr << fact_var.first.name() << ": ";
    for (string fn : fact_var.second) {
      cerr << fn << " ";
    }    
    cerr << ", ";
  }
  cerr << endl;

  Facts out_facts = inst_facts.second;
  cerr << "OUT: ";
  for (auto fact_var : out_facts) {      
    cerr << fact_var.first.name() << ": ";
    for (string fn : fact_var.second) {
      cerr << fn << " ";
    }
    cerr << ", ";
  }
 
  cerr << endl << endl;
}


bool FunctionContextPass::join(Instruction *predecessor, Instruction *inst) {
  bool changed = false;
  Facts pred_out_facts = facts[predecessor].second;
  Facts old_in_facts   = facts[inst].first;
  Facts new_in_facts   = old_in_facts;

  for (auto var_key : pred_out_facts) {
    VarName var = var_key.first;
    set<string> pred_out_var_fact = var_key.second;
    set<string> new_in_var_fact = new_in_facts[var];
    set_union(pred_out_var_fact.begin(), pred_out_var_fact.end(),
	      new_in_var_fact.begin(), new_in_var_fact.end(),
	      inserter(new_in_var_fact, new_in_var_fact.begin()));
    new_in_facts[var] = new_in_var_fact;
  }

  if (new_in_facts != old_in_facts) {
    changed = true;
    facts[inst].first = new_in_facts;
  }    

  return changed;
}

bool FunctionContextPass::transfer(Instruction *inst) {
  bool changed = false;
  bool call_store = false;
  NamesPass &NP = getAnalysis<NamesPass>();

  Facts old_in_facts  = facts[inst].first;
  Facts old_out_facts = facts[inst].second;
  Facts new_out_facts = old_in_facts;
  
  if (StoreInst *store = dyn_cast<StoreInst>(inst)) {
    Value *sender = store->getOperand(0);
    if (CallInst *call_sender = dyn_cast<CallInst>(sender)) {

      // This only works if store sender is a call instruction directly      
      vn_t receiver = NP.getVarName(store->getOperand(1));      

      // Store kills previous values for this variable
      if (receiver) {
	new_out_facts[*receiver] = set<string>();
	for (string name : NP.getCalleeNames(*call_sender)) {
	  new_out_facts[*receiver].insert(name);
	}

	if (new_out_facts != old_out_facts) {
	  facts[inst].second = new_out_facts;
	  changed = true;
	}

	call_store = true;
      }
    }
  } 

  if (old_in_facts != old_out_facts && !call_store) {
    // Default is to just copy in facts to out facts
    facts[inst].second = facts[inst].first;
    changed = true;
  }

  return changed;
}

void FunctionContextPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BranchSafetyPass>();
  AU.addRequired<NamesPass>();
  AU.setPreservesAll();
}

char FunctionContextPass::ID = 0;
static RegisterPass<FunctionContextPass> X("functioncontext", "Hello Memdep World", false, false);
