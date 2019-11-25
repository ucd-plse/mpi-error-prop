(*

  May/Must Call Analysis
  ----------------------

  For a given caller function, this pass determines which other
  functions may or must be called by the caller.  This is a strictly
  intraprocedural analysis: we look only for calls directly appearing
  within the caller, without considering transitive calls.  This pass
  is a pure, standalone analysis: it depends on no other passes and
  does not change the AST in any way.

  Output consists of a file with multiple lines, each with three
  space-delimited fields:

  (1) The name of the caller.  This will be some function to which the
  pass was applied.

  (2) The name of a callee that may or must be called by the caller.
  Indirect calls through function pointers are ignored.

  (3) Either "!" if the caller must call the callee regardless of
  the execution path taken, or "?" if the caller calls the callee on
  some paths but does not call it on others.

  From the command line, use "--doMayMustCall" to activate this pass
  and send its output to standard out.  Add "--may-must-call-output
  <filename>" to send that output to <filename> instead.

*)


val feature : Cil.featureDescr
