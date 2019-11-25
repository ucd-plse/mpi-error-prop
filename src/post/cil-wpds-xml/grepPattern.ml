open Cil
open Pretty
open List


(* Global counter *)
let counter = ref 0

class visitor file = object(self)
  inherit nopCilVisitor

  method isFunction(funName:string)(vinfo:varinfo) =
    String.compare funName vinfo.vname == 0;


  method isSameVar(var:varinfo)(args:exp list) =
    match List.hd args with
    | CastE(_, Lval (Var arg, _)) ->
	(String.compare var.vname arg.vname == 0)
    | _ ->
	false


  method vstmt(s: stmt) =
    match s.skind with
    (* var = foo(); *)
    | Instr [Call (Some receiver, (Lval (Var callee, NoOffset)), args, loc)] ->
	begin
	  match receiver with
	  | Var varinfo, _ -> 
	      begin
		match unrollTypeDeep varinfo.vtype with
		| TPtr(TInt _, _)
		| TPtr(TVoid _, _)
		| TPtr(TComp _, _) ->
		    if (List.length s.succs == 1) then
		      let first_succ = List.hd s.succs in
		      begin
			match first_succ.skind with
			(* rc = PTR_ERR(var); *)
			| Instr [Call (Some receiver2, (Lval (Var callee2, NoOffset)), args2, loc2)] ->
			    if self#isFunction "PTR_ERR" callee2 then
			      begin
				if (self#isSameVar varinfo args2) then
				  begin
				    (* there is an intermediate statement that assigns from temporary variable *)
				    if (List.length first_succ.succs > 0) && (List.length (List.hd first_succ.succs).succs > 0) then
				      let second_succ = List.hd (List.hd first_succ.succs).succs in
				      begin
					match second_succ.skind with
					(* temp = IS_ERR(var); *)
					| Instr [Call (Some receiver3, (Lval (Var callee3, NoOffset)), args3, loc3)] ->
					    if (self#isFunction "IS_ERR" callee3) && (self#isSameVar varinfo args3) then
					      begin
						ignore(fprintf stderr "%s:%d\n" loc.file loc.line);
						counter := !counter + 1
					      end
					    
					| _ -> ()
				      end
				  end
			      end
			| _ -> ()
		      end
		| _ -> ()
	      end
	  | _ -> ()
	end;
	DoChildren

    | _ -> 
	DoChildren


  method vfunc(f: fundec) =

    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end



(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainGrepPattern (fi : Cil.file) =
  begin
    let visitor = new visitor fi in
        ignore(visitCilFileSameGlobals (visitor :> cilVisitor) fi);
    ignore(fprintf stderr "\nTotal number of instances found: %d\n" !counter)
      
  end



(* ------------------------- SETTING ------------------------ *)
    
let doGrepPattern = ref false


(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "greppattern";
    fd_enabled = doGrepPattern;
    fd_description = "Counting number of double-error pattern";
    fd_extraopt = [];
    fd_doit = (function (f: file) -> mainGrepPattern f) ;
    fd_post_check = false;
  }
