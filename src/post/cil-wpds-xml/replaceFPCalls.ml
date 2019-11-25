open Cil
open Pretty
open List


open Pretty
let printCilObj d_obj = fun obj chan ->
    let d = dprintf "%a" d_obj obj in
    let s = sprint 80 d in
    ignore( fprintf chan "[%s]\n" s )

let printCilExp = printCilObj d_exp
let printCilType = printCilObj d_type

class visitor file = object(self)
  inherit nopCilVisitor


 method private updateGotos (old_stmt:stmt)(new_stmt:stmt) =
    List.iter (fun p -> match p.skind with
                        | Goto(target, _) -> 
			    target := new_stmt;

			| _ -> ()

              ) old_stmt.preds

  method private findGlobal (name:string) =
    List.find (( fun n g -> match g with
	| GFun(fdec, loc) -> 
	        if fdec.svar.vname = n then true else false;
        | _ -> false;
    ) name ) file.globals


  method vstmt(s: stmt) =
    match s.skind with
    | Instr [Call (receiver, callee, args, loc)] ->
	begin
	   match callee with
	   | Lval (Mem (Lval (Var vinfo, _)), _) ->
	     begin
	     match unrollTypeDeep vinfo.vtype with
	     | TPtr(TFun(TPtr(_,_),_,_,_), _) ->
		begin
	   	  ignore(fprintf stderr "++A function call through a function pointer: %s \n" vinfo.vname);
		  let global = self#findGlobal("ERR_PTR") in
	          let error = integer(-85000000) in
		  match global with
                  | GFun(fundec, _) ->
			begin
			  let call_instr = Call(receiver, Lval(var fundec.svar), [error], loc) in
        	          let call_stmt = mkStmtOneInstr call_instr in
				begin
				   call_stmt.labels <- s.labels;
		 		   s.labels <- [];
		  		   self#updateGotos s call_stmt; 
				   ChangeTo call_stmt
				end
			end
		  | _ ->
    	    	     DoChildren
		end;
	     | _ ->
		begin
		  ignore(fprintf stderr "--No pointer return type: %s \n" vinfo.vname);
		  DoChildren
                end
             end;

           | _ -> 
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



