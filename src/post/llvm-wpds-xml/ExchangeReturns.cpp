#include "CollectVariables.hpp"
#include "ExchangeReturns.hpp"

#include <llvm/Support/raw_ostream.h>


bool
ExchangeReturns::runOnModule(Module &M) {

  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    if (!f->isDeclaration()) {
      runOnFunction(*f);
    }
  }

  return false;
}


bool
ExchangeReturns::runOnFunction(Function &function)
{
  // retrieving local and global variables
  CollectVariables &vars = getAnalysis<CollectVariables>();
  globals = vars.getGlobals();
  
  // retrieve exchange variable
  const Twine name = function.getName() + "$return";
  exchangeVar = function.getParent()->getNamedGlobal(name.str());

  return visit(function);
}


bool
ExchangeReturns::visit(BasicBlock &block)
{
  // return instructions can *only* be block terminators, so save some
  // time by ignoring everything else before the terminator
  return visit(block.getTerminator());
}


bool
ExchangeReturns::visitReturnInst(ReturnInst &returnInst)
{
  // insert assignment to exchange return variable (if any)
  Value *returnValue = returnInst.getReturnValue();
  if (!returnValue) return false;

  const SExtInst * const sextInst = dyn_cast<SExtInst>(returnValue); // if 64bit, long returned
  Value *sender = sextInst ? sextInst->getOperand(0) : returnValue;

  if (globals.find(exchangeVar) != globals.end()) { // sender could be anything, if var, Rules will do the checking

    if (sender->getType() == cast<PointerType>(exchangeVar->getType())->getElementType()) {
      Instruction *i = new StoreInst(sender, exchangeVar, &returnInst);
      // Copy debug metadata (source location) to new instruction
      // This dummy instruction shares the same line as the return
      // Needed for --locations
      if (MDNode *node = returnInst.getMetadata("dbg")) {
        i->setMetadata("dbg", node);
      }
    }
    else {
      errs() << "[ExchangeReturns] Type mismatch\n";
      sender->getType()->dump();
      errs() << "\n";
    }
  }

  return false;
}


void
ExchangeReturns::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<CollectVariables>();
}


char ExchangeReturns::ID;
static const RegisterPass<ExchangeReturns> registration("exchange-callee-returns", "Copy return values into global exchange variables");

#pragma GCC diagnostic ignored "-Wunused-parameter"
