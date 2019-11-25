#include "Calls.hpp"
#include "llvm-version.hpp"
#include "MainFunction.hpp"
#include "UnknownValue.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#endif

#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;


#define DEBUG_MAIN(info) DEBUG_WITH_TYPE("main-function", dbgs() << info)

static inline raw_ostream &operator<<(raw_ostream &out, const DIDescriptor &info) {
  info.print(out);
  return out;
}


static void possibleEntryFunction(const DISubprogram subprogramDescriptor, MainFunction::FunctionSet &entryFunctions) {
  assert(subprogramDescriptor.Verify());

  if (!subprogramDescriptor.isDefinition()) {
    DEBUG_MAIN("  ignoring: not a definition: " << subprogramDescriptor.getName() << '\n');
    return;
  }

  const DISubprogram declarationDescriptor(subprogramDescriptor.getFunctionDeclaration());
  if (declarationDescriptor.isPrivate()) {
    DEBUG_MAIN("  ignoring: is private: " << subprogramDescriptor.getName() << '\n');
    return;
  }

  if (Function * const function = subprogramDescriptor.getFunction()) {
    DEBUG_MAIN("  keeping: " << subprogramDescriptor.getName() << '\n');
    entryFunctions.insert(function);
  } else {
    DEBUG_MAIN("  ignoring: cannot get function: " << subprogramDescriptor.getName() << '\n');
    return;
  }
}


bool MainFunction::runOnModule(Module &M) {

  static const StringRef mainName("main");
  if (M.getFunction(mainName)) {
    DEBUG_MAIN("module already has a " << mainName << " function\n");
    return false;
  }

  // collect API entry points
  DEBUG_MAIN("collecting API entry points for synthetic " << mainName << " function\n");
  FunctionSet entryFunctions;
  DebugInfoFinder finder;
  finder.processModule(M);
#if LLVM_VERSION >= 30500
  for (const auto subprogramDescriptor : finder.subprograms())
    possibleEntryFunction(subprogramDescriptor, entryFunctions);
#else
  for (DebugInfoFinder::iterator subprogram = finder.subprogram_begin(), end = finder.subprogram_end(); subprogram != end; ++subprogram)
    possibleEntryFunction(DISubprogram(*subprogram), entryFunctions);
#endif

  DEBUG_MAIN("collected " << entryFunctions.size() << " API entry points\n");

  // creating function main
  LLVMContext &context = M.getContext();
  IntegerType *ty = IntegerType::getInt32Ty(context);
  FunctionType *fty = FunctionType::get(ty, false);
  Function *function = Function::Create(fty, GlobalValue::ExternalLinkage, mainName, &M);

  // creating last block
  BasicBlock *lastBlock = BasicBlock::Create(context, "last", function);
  ConstantInt *zero = ConstantInt::get(ty, 0);
  ReturnInst::Create(context, zero, lastBlock);

  // creating switch block
  BasicBlock *switchBlock = BasicBlock::Create(context, "switch", function, lastBlock);
  
  const UnknownValue &unknownValue = getAnalysis<UnknownValue>();
  IntegerType &unknownType = *Type::getInt32Ty(context);
  Instruction &unknown = unknownValue.fresh("select", unknownType, switchBlock);
  SwitchInst &switchInst = *SwitchInst::Create(&unknown, lastBlock, entryFunctions.size(), switchBlock);
  

  // creating function calls and switch cases
  for (FunctionSet::const_iterator entryFunction = entryFunctions.begin(), end = entryFunctions.end(); 
      entryFunction != end; ++entryFunction) {

    // creating block
    BasicBlock *callBlock = BasicBlock::Create(context, "call", function, lastBlock);

    // creating a function call

    SmallVector<Value *, 8> actuals;
    const FunctionType *calleeType = (*entryFunction)->getFunctionType();
    const unsigned numArgs = calleeType->getNumParams();
    actuals.reserve(numArgs);

    for (unsigned argSlot = 0; argSlot != numArgs; ++argSlot) {
      Type &neededParamType = *calleeType->getParamType(argSlot);
      Instruction &neededParam = unknownValue.fresh("actual", neededParamType, callBlock);
      actuals.push_back(&neededParam);
    }
    
    CallInst::Create(*entryFunction, actuals, "", callBlock);
    BranchInst::Create(lastBlock, callBlock);

    // adding switch case
    ConstantInt &choiceNum = *ConstantInt::get(&unknownType, switchInst.getNumCases());
    switchInst.addCase(&choiceNum, callBlock);
    
  }

  return true;
}  


void MainFunction::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<UnknownValue>();
}


char MainFunction::ID = 0;
static const RegisterPass<MainFunction> registration("main", "Adds a main function, if missing");

