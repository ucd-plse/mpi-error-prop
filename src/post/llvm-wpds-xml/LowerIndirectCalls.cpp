#undef NDEBUG

#include "FindIndirectCalls.hpp"
#include "LowerIndirectCalls.hpp"
#include "ResolveCallees.hpp"
#include "UnknownValue.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using namespace llvm;
using namespace std;


bool
LowerIndirectCalls::runOnFunction(llvm::Function &function)
{
  // collect the indirect calls we need to work on
  typedef FindIndirectCalls::IndirectCalls Targets;
  const FindIndirectCalls &findIndirectCalls = getAnalysis<FindIndirectCalls>();
  const Targets &targets = findIndirectCalls.found();
  
  // nothing to do?
  if (targets.empty())
    return false;

  // assorted information we use across all indirect call sites
  bool changed = false;
  LLVMContext &context = function.getParent()->getContext();
  const ResolveCallees &resolver = getAnalysis<ResolveCallees>();
  ResolveCallees::FunctionSet callees;
  const UnknownValue &unknownValue = getAnalysis<UnknownValue>();

  // transform each indirect call into a switch
  for (Targets::const_iterator target = targets.begin(), end = targets.end(); target != end; ++target)
    {
      CallInst &indirectCallInst = **target;
      if (resolver.resolve(indirectCallInst, callees) && !callees.empty())
	{
	  // resolved to non-empty callee set, so time to transform!
	  changed = true;
	  const DebugLoc debugLoc = indirectCallInst.getDebugLoc();
	  BasicBlock::iterator nextIterator(&indirectCallInst);
	  ++nextIterator;
	  Instruction &afterCallInst = *nextIterator;
	  BasicBlock &indirectCallBlock = *indirectCallInst.getParent();
	  BasicBlock &afterCallBlock = *SplitBlock(&indirectCallBlock, &afterCallInst, this);

	  // returned-value collection point immediately after the switch
	  const unsigned reserve = callees.size();
	  PHINode &phiInst = *PHINode::Create(indirectCallInst.getType(), reserve, "phi", &afterCallBlock.front());
	  phiInst.setDebugLoc(debugLoc);

	  // remove the indirect call and the branch that immediately
	  // follows it; these will be replaced by the switch
	  assert(isa<BranchInst>(indirectCallBlock.back()));
	  indirectCallBlock.back().eraseFromParent();
	  assert(&indirectCallBlock.back() == &indirectCallInst);
	  indirectCallBlock.back().removeFromParent();

	  // read a non-deterministic value and switch on it
	  IntegerType &unknownType = *Type::getInt32Ty(context);
	  Instruction &unknown = unknownValue.fresh("random", unknownType, &indirectCallBlock);
	  unknown.setDebugLoc(debugLoc);
	  SwitchInst &switchInst = *SwitchInst::Create(&unknown, 0, callees.size(), &indirectCallBlock);
	  switchInst.setDebugLoc(debugLoc);

	  // each possible direct callee becomes a case of the switch
	  for (ResolveCallees::FunctionSet::const_iterator callee = callees.begin(), end = callees.end(); callee != end; ++callee)
	    {
	      BasicBlock &directCallBlock = *BasicBlock::Create(context, "choice", &function, &afterCallBlock);

	      // collect actuals, casting them to match the direct
	      // callee's expected types if necessary
	      SmallVector<Value *, 8> actuals;
	      const FunctionType &calleeType = *(*callee)->getFunctionType();
	      const unsigned numArgs = calleeType.getNumParams();
	      actuals.reserve(numArgs);
	      for (unsigned argSlot = 0; argSlot != numArgs; ++argSlot)
		{
		  Type &neededParamType = *calleeType.getParamType(argSlot);
		  Value *argument = indirectCallInst.getArgOperand(argSlot);
		  if (argument->getType() != &neededParamType)
		    {
		      BitCastInst &cast = *new BitCastInst(argument, &neededParamType, "arg", &directCallBlock);
		      cast.setDebugLoc(debugLoc);
		      argument = &cast;
		    }
		  actuals.push_back(argument);
		}

	      // create the direct call, followed by a jump to the phi
	      // statement's block
	      CallInst &directCall = *CallInst::Create(*callee, actuals, "direct", &directCallBlock);
	      directCall.setDebugLoc(debugLoc);
	      BranchInst::Create(&afterCallBlock, &directCallBlock)->setDebugLoc(debugLoc);

	      // plug this case into the switch statement
	      ConstantInt &choiceNum = *ConstantInt::get(&unknownType, switchInst.getNumCases());
	      switchInst.addCase(&choiceNum, &directCallBlock);

	      // tell the phi statement to anticipate our arrival
	      phiInst.addIncoming(&directCall, &directCallBlock);
	    }

	  // the switch's default case can replace one of the
	  // specific-value cases
	  switchInst.setSuccessor(0, switchInst.getSuccessor(1));
#if LLVM_VERSION_MAJOR > 3 || (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 1)
	  SwitchInst::CaseIt caseIt(&switchInst, 1);
	  switchInst.removeCase(caseIt);
#else
	  switchInst.removeCase(1);
#endif
	  phiInst.takeName(&indirectCallInst);
	  indirectCallInst.replaceAllUsesWith(&phiInst);
	  delete &indirectCallInst;
	}
    }

  return changed;
}


void
LowerIndirectCalls::getAnalysisUsage(AnalysisUsage &usage) const
{
  usage.setPreservesAll();
  usage.addRequired<ResolveCallees>();
  usage.addRequired<UnknownValue>();
  usage.addRequired<FindIndirectCalls>();
}


char LowerIndirectCalls::ID;
static const RegisterPass<LowerIndirectCalls> registration("lower-indirect-calls", "Rewrite each indirect call site as a non-deterministic choice among direct call sites");
