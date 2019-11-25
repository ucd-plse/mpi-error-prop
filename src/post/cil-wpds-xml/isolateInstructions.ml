open Cil


let isolate =
  function
    | (_ :: _ :: _) as instructions ->
	let statements = List.map mkStmtOneInstr instructions in
	Block (mkBlock statements)
    | other ->
	Instr other


class visitor =
  object
    inherit FunctionBodyVisitor.visitor
	
    method vstmt statement =
      match statement.skind with
      | Instr instructions ->
	  let block = isolate instructions in
	  statement.skind <- block;
	  SkipChildren
      | _ ->
	  DoChildren
  end


let visit func =
  ignore (visitCilFunction new visitor func)


let isolated {skind = skind} =
  match skind with
  | Instr [singleton] ->
      Some singleton
  | Instr (_ :: _ :: _) ->
      ignore (bug "instr should have been atomized");
      failwith "internal error"
  | _ ->
      None
