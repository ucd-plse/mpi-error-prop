open Cil
open Pretty
open List
open Cfg


(* Some print functions to be used for debugging *)
 let printCilObj d_obj = fun obj chan ->
   let d = dprintf "%a" d_obj obj in
   let s = sprint 80 d in
   ignore( fprintf chan "[%s]\n" s )

let printCilStmt = printCilObj d_stmt



class visitor file = object(self)
  inherit nopCilVisitor

  (* Current function being analyzed *)
  val mutable currentFunction = dummyFunDec;

  (* For each pred of s, remove s from the list of successors *)
  method private updateSuccessors(s: stmt) =
    List.iter (fun pred -> pred.succs <- List.filter (fun succ -> succ != s) pred.succs) s.preds;

 (* For each succ of s, remove s from the list of predecessors *)  
  method private updatePredecessors(s: stmt) =
    List.iter (fun succ -> succ.preds <- List.filter (fun pred -> pred != s) succ.preds) s.succs;

 (* We print a warning message for each removed unreachable statement *)
  method private printWarning(s: stmt)(loc: location) =
    begin
      ignore(fprintf stderr "Warning: Removed statement at %s:%d,%s\n" loc.file loc.line currentFunction.svar.vname);
      printCilStmt s stderr
    end


  method private removeStmt(s: stmt)(loc: location) =
    begin
      self#updateSuccessors(s);
      self#updatePredecessors(s);
      self#printWarning s loc;
      ChangeTo(mkEmptyStmt())
    end


  method vstmt(s: stmt) =
    match s.skind with
    | Instr[Set(_, _, loc)]
    | Instr[Call(_, _, _, loc)]
    | Instr[Asm(_, _, _, _, _, loc)]
    | Goto(_, loc)
    | Break loc
    | Continue loc
    | Switch(_, _, _, loc)
    | Loop(_, loc, _, _)
    | TryFinally(_, _, loc)
    | TryExcept(_, _, _, loc)
    | Return(_, loc) ->
	begin
	  if ((List.length s.preds) == 0) then
	    self#removeStmt s loc
	  else
	    DoChildren
	end

    | Block block ->
	if (Cil.hasAttribute "firstBlock" block.battrs) then
	    SkipChildren
	else 
	  begin	    
	    if ((List.length s.preds) == 0) then
	      self#removeStmt s locUnknown
	    else
	      DoChildren		
	  end

    | Instr _ ->
	begin
	  if ((List.length s.preds) == 0) then
	    self#removeStmt s locUnknown
	  else
	    DoChildren
	end

    (* TODO: Special case, blocks might be shared!*)
    | If(_, _, _, _) ->
	DoChildren


  method vfunc(f: fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    currentFunction <- f;
    DoChildren
end


let rec removeUnreachableCodeVisitor file =
  let visitor = new visitor file in
    ignore(visitCilFileSameGlobals (visitor :> cilVisitor) file)



