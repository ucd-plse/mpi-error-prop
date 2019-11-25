#ifndef INST_BOOL_VISITOR_GUARD
#define INST_BOOL_VISITOR_GUARD

#include "llvm-version.hpp"

#if LLVM_VERSION >= 30500
#include <llvm/IR/InstVisitor.h>
#else
#include <llvm/InstVisitor.h>
#endif


template <typename SubClass>
class InstBoolVisitor : public llvm::InstVisitor<SubClass, bool>
{
public:
  template <class Iterator> bool visit(Iterator, Iterator);

  bool visit(llvm::Module &);
  bool visit(llvm::Function &);
  bool visit(llvm::BasicBlock &);
  bool visit(llvm::Instruction &);

  bool visit(llvm::Module *);
  bool visit(llvm::Function *);
  bool visit(llvm::BasicBlock *);
  bool visit(llvm::Instruction *);

  bool visitInstruction(llvm::Instruction &);
};


////////////////////////////////////////////////////////////////////////


template <typename SubClass>
template <class Iterator>
inline bool
InstBoolVisitor<SubClass>::visit(Iterator begin, Iterator end)
{
  bool result = false;
  while (begin != end)
    result |= static_cast<SubClass*>(this)->visit(*begin++);
  return result;
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::Module &module)
{
  static_cast<SubClass*>(this)->visitModule(module);
  return visit(module.begin(), module.end());
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::Function &function)
{
  static_cast<SubClass*>(this)->visitFunction(function);
  return visit(function.begin(), function.end());
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::BasicBlock &block)
{
  static_cast<SubClass*>(this)->visitBasicBlock(block);
  return visit(block.begin(), block.end());
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::Instruction &instruction)
{
  return llvm::InstVisitor<SubClass, bool>::visit(instruction);
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::Module *module)
{
  return visit(*module);
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::Function *function)
{
  return visit(*function);
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::BasicBlock *block)
{
  return visit(*block);
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visit(llvm::Instruction *instruction)
{
  return visit(*instruction);
}


template <typename SubClass>
inline bool
InstBoolVisitor<SubClass>::visitInstruction(llvm::Instruction &)
{
  return false;
}


#endif // !INST_BOOL_VISITOR_GUARD
