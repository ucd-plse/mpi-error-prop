open Cil
open Pretty
open List


class visitor pds positive = object(self)
  inherit nopCilVisitor

  method vstmt(s: stmt) =
    let clearVar lval nonErrorBlock =
      let setInstr = Set(lval, integer(-67737868), get_stmtLoc s.skind) in
      nonErrorBlock.bstmts <- (mkStmtOneInstr setInstr) :: nonErrorBlock.bstmts
    in
    
    begin
      match s.skind with	
      | If (Lval ((Var vinfo, NoOffset) as lval), _, nonErrorBlock, _)
      | If (UnOp (LNot, Lval ((Var vinfo, NoOffset) as lval), _), nonErrorBlock, _, _) ->
	  clearVar lval nonErrorBlock;

      | If (BinOp (Eq, zero, Lval ((Var _, NoOffset) as lval), _), nonErrorBlock, _, _)
      | If (BinOp (Eq, Lval ((Var _, NoOffset) as lval), zero, _), nonErrorBlock, _, _)
        when isZero zero ->
	  clearVar lval nonErrorBlock

      | If (BinOp (Gt, Lval ((Var _, NoOffset) as lval), zero, _), trueBlock, falseBlock, _)
      | If (BinOp (Ge, Lval ((Var _, NoOffset) as lval), zero, _), trueBlock, falseBlock, _)
      | If (BinOp (Lt, zero, Lval ((Var _, NoOffset) as lval), _), trueBlock, falseBlock, _)
      | If (BinOp (Le, zero, Lval ((Var _, NoOffset) as lval), _), trueBlock, falseBlock, _)
	  when isZero zero ->
	  if positive then
	    clearVar lval falseBlock
	  else
	    clearVar lval trueBlock;

      | If (BinOp (Lt, Lval ((Var _, NoOffset) as lval), zero, _), trueBlock, falseBlock, _)
      | If (BinOp (Le, Lval ((Var _, NoOffset) as lval), zero, _), trueBlock, falseBlock, _)
      | If (BinOp (Gt, zero, Lval ((Var _, NoOffset) as lval), _), trueBlock, falseBlock, _)
      | If (BinOp (Ge, zero, Lval ((Var _, NoOffset) as lval), _), trueBlock, falseBlock, _)
      | If (BinOp (Ne, Lval ((Var _, NoOffset) as lval), zero, _), trueBlock, falseBlock, _)
      | If (BinOp (Ne, zero, Lval ((Var _, NoOffset) as lval), _), trueBlock, falseBlock, _)
	when isZero zero ->
	  if positive then
	    clearVar lval trueBlock
	  else
	    clearVar lval falseBlock;

      | _ -> ()
    end;
    DoChildren;


  method vfunc(f: fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
   
    DoChildren;

end


let transferFunctions pds f positive =
  let visitor = new visitor pds positive in
      ignore(visitCilFileSameGlobals (visitor :> cilVisitor) f)

