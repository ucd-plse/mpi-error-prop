#undef NDEBUG

#include "UnknownValue.hpp"

#include <cassert>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm;


const StringRef Name(".error-prop.unknown-value");


bool
UnknownValue::runOnModule(llvm::Module &module)
{
  if (module.getNamedGlobal(Name))
    return false;
  else
    {
      LLVMContext &context = module.getContext();
      IntegerType &type = *Type::getInt1Ty(context);
      value = cast<GlobalVariable>(module.getOrInsertGlobal(Name, &type));
      value->setInitializer(ConstantInt::get(&type, 0));
      value->setLinkage(GlobalValue::PrivateLinkage);
      return true;
    }
}


template <typename Cursor>
Instruction &
UnknownValue::freshAt(const Twine &name, Type &type, Cursor *cursor) const
{
  Type &castType = *PointerType::getUnqual(&type);
  Instruction &cast = *new BitCastInst(value, &castType, name + ".cast", cursor);
  Instruction &load = *new LoadInst(&cast, name, true, cursor);
  return load;
}


Instruction &
UnknownValue::fresh(const Twine &name, Type &type, Instruction *insertBefore) const
{
  return freshAt(name, type, insertBefore);
}


Instruction &
UnknownValue::fresh(const Twine &name, Type &type, BasicBlock *insertAtEnd) const
{
  return freshAt(name, type, insertAtEnd);
}


void
UnknownValue::getAnalysisUsage(AnalysisUsage &usage) const
{
  usage.setPreservesAll();
}


char UnknownValue::ID;
static const RegisterPass<UnknownValue> registration("add-unknown-value", "Provide a global integer variable with unknown value");
