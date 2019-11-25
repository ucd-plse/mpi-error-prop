#ifndef MAY_MUST_CALL_GUARD
#define MAY_MUST_CALL_GUARD


////////////////////////////////////////////////////////////////////////
//
//  May/Must Call Analysis
//  ----------------------
//
//  For a given caller function, this pass determines which other
//  functions may or must be called by the caller.  This is a strictly
//  intraprocedural analysis: we look only for calls directly
//  appearing within the caller, without considering transitive calls.
//  This pass is a pure, standalone analysis: it depends on no other
//  passes and does not change the bitcode in any way.
//
//  Output consists of a file with multiple lines, each with three
//  space-delimited fields:
//
//  (1) The name of the caller.  This will be some function to which
//  the pass was applied.
//
//  (2) The name of a callee that may or must be called by the caller.
//  Indirect calls through function pointers are ignored.  Consider
//  using the LowerIndirectCalls pass to transform these away if more
//  information is needed about indirect calls.
//
//  (3) Either "!" if the caller must call the callee regardless of
//  the execution path taken, or "?" if the caller calls the callee on
//  some paths but does not call it on others.
//
//  From the command line, use "-may-must-call" to activate this pass
//  and send its output to standard out.  Add
//  "-may-must-call-output=<filename>" to send that output to
//  <filename> instead.
//


#include "MayMustPass.hpp"


class MayMustCall : public MayMustPass
{
public:
  // standard LLVM function pass infrastructure
  MayMustCall();
  static char ID;

protected:
  const std::string &getOutputFileName() const;
  void collectFromBlock(llvm::BasicBlock &, ItemSet &) const;
};


////////////////////////////////////////////////////////////////////////


inline
MayMustCall::MayMustCall()
: MayMustPass(ID)
{
}


#endif	// !MAY_MUST_CALL_GUARD
