#undef NDEBUG

#include "CollectVariables.hpp"
#include "Conditionals.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>

using namespace llvm;

enum ErrorCodesSign { Negative, Positive };

cl::opt<ErrorCodesSign>
Sign(
     "sign",
     cl::desc("Error codes sign:"),
     cl::values(
		clEnumValN(Negative, "negative", "negative (default)"),
		clEnumValN(Positive, "positive", "positive"),
		clEnumValEnd
		)
     );

bool
Conditionals::maybeClearError(const Value &maybeIntVar, const Value &maybeZero,
		const BranchInst &branchInst, bool forOutcome)
{
  const LoadInst * const loadInst = dyn_cast<LoadInst>(&maybeIntVar);
  if (!loadInst) return false;

  const Type &type = *loadInst->getType();
  if (!type.isIntegerTy(32))
    return false;

  const ConstantInt * const constantInt = dyn_cast<ConstantInt>(&maybeZero);
  if (!constantInt) return false;
  if (!constantInt->isZero()) return false;

  Value &intVar = *loadInst->getOperand(0);
  BasicBlock &successorBlock = *branchInst.getSuccessor(forOutcome ? 0 : 1);
  Instruction &insertBefore = successorBlock.front();
  LLVMContext &context = intVar.getContext();
  IntegerType &ty = *IntegerType::getInt32Ty(context);
  ConstantInt &trustedOK = *ConstantInt::get(&ty, -67737868);


  const Function *function = loadInst->getParent()->getParent();

  if (globals.find(&intVar) != globals.end() || 
      locals[function].find(&intVar) != locals[function].end()) {
    new StoreInst(&trustedOK, &intVar, &insertBefore);
  }
  
  return true;
}


bool
Conditionals::maybeClearError(const Value &op0, const Value &op1,
		const BranchInst &branchInst,
		bool intZeroOutcome, bool zeroIntOutcome)
{
  return
    maybeClearError(op0, op1, branchInst, intZeroOutcome) ||
    maybeClearError(op1, op0, branchInst, zeroIntOutcome);
}


bool
Conditionals::runOnModule(Module &M) {

  // retrieving local and global variables
  CollectVariables &vars = getAnalysis<CollectVariables>();
  locals = vars.getLocals();
  globals = vars.getGlobals();

  return visit(M);
}


bool
Conditionals::visitBranchInst(const BranchInst &br)
{
  if (!br.isConditional()) return false;
  const Value &condition = *br.getCondition();
  const ICmpInst * const cmp = dyn_cast<ICmpInst>(&condition);
  if (!cmp) return false;

  const Value &op0 = *cmp->getOperand(0);
  const Value &op1 = *cmp->getOperand(1);

  switch (cmp->getSignedPredicate())
    {
    case ICmpInst::ICMP_SLT:
    case ICmpInst::ICMP_SLE:
      return maybeClearError(op0, op1, br, Sign == Positive, Sign == Negative);

    case ICmpInst::ICMP_SGT:
    case ICmpInst::ICMP_SGE:
      return maybeClearError(op0, op1, br, Sign == Negative, Sign == Positive);

    // case ICmpInst::ICMP_NE used for both if(x) and if (!x) - LLVM switches branches
    // Example:
    // if (x) -> if (x != 0) -> want to clear the false branch (Sign == Negative)
    case ICmpInst::ICMP_NE:
      return maybeClearError(op0, op1, br, Sign == Positive, Sign == Negative);

    case ICmpInst::ICMP_EQ:
      return maybeClearError(op0, op1, br, true, true);

    default:
      // not a pattern we recognize
      break;
    }

  return false;
}


void Conditionals::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredID(LowerSwitchID);
  AU.addRequired<CollectVariables>();
}


char Conditionals::ID = 0;
static const RegisterPass<Conditionals> registration("conditionals", "Adds OK assignments in corresponding conditional branches");

#pragma GCC diagnostic ignored "-Wunused-parameter"
