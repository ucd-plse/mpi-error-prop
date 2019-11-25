open Cil
open Pretty
open List


class visitor file = object(self)
  inherit nopCilVisitor


 method private updateGotos (old_stmt:stmt)(new_stmt:stmt) =
    List.iter (fun p -> match p.skind with
                        | Goto(target, _) -> 
			    target := new_stmt;

			| _ -> ()

              ) old_stmt.preds


  method vexpr(e:exp) =
    match e with
    | CastE(_, UnOp(LNot, UnOp(LNot, expr, _), _))
    | UnOp(LNot, UnOp(LNot, expr, _), _)->
	ChangeTo expr
	  
    | _ -> 
	DoChildren


  method vstmt(s: stmt) =
    match s.skind with
    | Instr [Call (Some receiver, (Lval (Var callee, NoOffset)), args, loc)] ->
	begin

	  (* Only function __builtin_expect for now *)
	  (* temp = __builtin_expect(!!(x), 0) -> temp = x *)
	  if String.compare callee.vname "__builtin_expect" == 0 then

	    match List.hd args with
	    (* For some reason, error variable is double negated, e.g. test136 *)
	    | CastE(_, UnOp(LNot, UnOp(LNot, expr, _), _))
	    | UnOp(LNot, UnOp(LNot, expr, _), _)->
		let setInstr = Set(receiver, expr, get_stmtLoc s.skind) in
		let newStmt = mkStmtOneInstr setInstr in
		begin
		  (* Updating labels and gotos when call is the target of a goto *)
		  newStmt.labels <- s.labels;
		  s.labels <- [];
		  self#updateGotos s newStmt; 

		  ChangeTo newStmt
		end
	    | _ ->
		DoChildren
	  else
	    DoChildren
	end

    | _ -> 
	DoChildren



  method vfunc(f: fundec) =

    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end


let replaceCallsVisitor file =
  let visitor = new visitor file in
    ignore(visitCilFileSameGlobals (visitor :> cilVisitor) file);



