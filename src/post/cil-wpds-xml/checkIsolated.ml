open Cil


class visitor =
  object
    inherit FunctionBodyVisitor.visitor

    method vstmt statement =
      ignore (IsolateInstructions.isolated statement);
      DoChildren
  end


let checkIsolated =
  visitCilFileSameGlobals (new visitor)
