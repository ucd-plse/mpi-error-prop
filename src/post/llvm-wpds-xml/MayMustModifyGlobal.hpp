#ifndef INCLUDE_MayMustModifyGlobal_hpp
#define INCLUDE_MayMustModifyGlobal_hpp


////////////////////////////////////////////////////////////////////////
//
//  May/Must Modify Global Analysis
//  -------------------------------
//
//  For a given mutator function, this pass determines which global
//  variables may or must be modified (stored into) by the mutator.
//  This is a strictly intraprocedural analysis: we look only for
//  stores directly appearing within the mutator, without considering
//  transitive calls.  Similarly, we do no pointer analysis: we look
//  only for direct stores to named global variables.
//
//  This pass does not change the bitcode in any way.  It depends on
//  the FilterVariables pass to identify interesting globals that are
//  worth analyzing; all other globals are ignored.
//
//  Output consists of a file with multiple lines, each with three
//  space-delimited fields:
//
//  (1) The name of the mutator.  This will be some function to which
//  the pass was applied.
//
//  (2) The name of a global variable that may or must be directly
//  modified (stored into) by the mutator.
//
//  (3) Either "!" if the mutator must modify the global regardless of
//  the execution path taken, or "?" if the mutator modifies the
//  global on some paths but does not modify it on others.
//
//  From the command line, use "-may-must-modify-global" to activate
//  this pass and send its output to standard out.  Add
//  "-may-must-modify-global-output=<filename>" to send that output to
//  <filename> instead.
//


#include "MayMustPass.hpp"


class MayMustModifyGlobal : public MayMustPass
{
public:
  // standard LLVM function pass infrastructure
  MayMustModifyGlobal();
  static char ID;
  void getAnalysisUsage(AnalysisUsage &) const;

  bool runOnFunction(llvm::Function &);

protected:
  const std::string &getOutputFileName() const;
  void collectFromBlock(llvm::BasicBlock &, ItemSet &) const;

private:
  const Variables *relevant;
};


////////////////////////////////////////////////////////////////////////


inline
MayMustModifyGlobal::MayMustModifyGlobal()
: MayMustPass(ID)
{
}


#endif	// !INCLUDE_MayMustModifyGlobal_hpp
