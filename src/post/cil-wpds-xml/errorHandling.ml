open Cil
open Pretty
open List
open Reachingdefs
open Inthash
open Cfg

(* Returns a varinfo given an lval *)
let rec getVarInfo = function
  | Var varinfo, _ -> Some varinfo
  | Mem Lval lvalue, _ -> getVarInfo lvalue
  | Mem _, _ -> None


(* Returns a varinfo given an expression *)
let rec getVarInfoFromExp = function
  | CastE (_, expr) -> getVarInfoFromExp expr
  | Lval lvalue -> getVarInfo lvalue
  | AddrOf lvalue -> getVarInfo lvalue
  | _ -> None


exception Non_struct of string;;
(*Some debugging method privates*)
 let printCilObj d_obj = fun obj chan ->
   let d = dprintf "%a" d_obj obj in
   let s = sprint 80 d in
   ignore( fprintf chan "[%s]\n" s )

let printCilExp = printCilObj d_exp
let printCilStmt = printCilObj d_stmt
let printCilLval = printCilObj d_lval


class visitor pds positive = object(self)
  inherit nopCilVisitor

  val clearArgFuns = ["ea_bdebug"; "ext3_warning"; "ext3_error"; "printk"; "reiserfs_warning"; "cERROR"; "cFYI"; "jfs_err"; "ext4_warning"]
  val okAssignAfterFuns = ["ea_bdebug"; "ext3_warning"; "ext3_error"; "reiserfs_warning"; "cERROR"; "cFYI"]
  val okIfCheckedFuns = ["cERROR"; "jfs_err"; "ext4_warning"; "seq_printf"; "ext4_error"; "printk"]
      
  val vars = ref []
  val retry_vars = ref []
  val is_err_vars = ref []
      
  method notTrusted(e:exp)  =
    match e with
    | Const(CInt64(-67737868L,_,_)) -> false;
    | Const(CInt64(-82080000L,_,_)) -> false;
    | _ -> true;
	
  method isChecked(varname:string) =
    List.exists (fun (v, _) -> 
                 match v with
                 | Var vinfo -> vinfo.vname = varname;
                 | _ -> false;
                ) (!vars @ !is_err_vars);
    
    
  method exists(funName:string)(funs:string list) =
    List.exists (fun f -> f = funName) funs;

  method isRetry(targets:label list) =
    List.exists (function
                 | Label(name,_,_) -> name = "retry";
	        | _ -> false; 
	        ) targets;

  method isRealGoto(targets:label list) =
    List.exists (function
                 | Label(_,_,true) -> true;
	        | _ -> false; 
	        ) targets;


  method private rds(s: stmt)(vinfo: varinfo) =
    try
      let (_,_,iosh) = Inthash.find ReachingDef.stmtStartData s.sid in (*rds info for this statement*)
      match Reachingdefs.iosh_lookup iosh vinfo with  (*rds info for this variable*)
      | Some info ->
	  let defs = IOS.cardinal info in (*number of definitions for this variable*)
	  begin
	    match IOS.choose info with (*one of the definitions, chosen at random*)
	    | Some def ->
		if defs == 1 then
		  let stmtdef = (Inthash.find ReachingDef.defIdStmtHash def) in
		  begin
		    match stmtdef.skind with
		    | Instr [Set (receiver, senderExpr, location)] ->
			begin
			  match getVarInfoFromExp senderExpr with
			  | Some lhs ->
			      self#rds stmtdef lhs
			  | None -> None
			end
			  
		    | Instr [Call (Some receiver, (Lval (Var callee, NoOffset)), actuals, _)] ->
			if (String.compare callee.vname "IS_ERR" == 0) then
			  begin
			    let argument = (List.hd actuals) in
			    match argument with
			    | Lval lval
			    | CastE(_, Lval lval) ->
				Some lval
			    | _ -> None;
			  end
			else
			  None
			    
		    | _ -> None
		  end
		else
		  None
	    | None -> None
	  end
      | None -> None

    with Not_found -> 
      begin
	(*failwith("\t[ERROR] Reaching definitions info unavailable.\n");*)
	ignore(fprintf stderr "\t[ERROR] Reaching definitions info unavailable for: %s\n" vinfo.vname);
	None
      end


  method vstmt(s: stmt) =
    let pushVar s varlist lval =
      varlist := lval :: !varlist;
      let popVar s =
	varlist := List.tl !varlist;	
	s
      in
      ChangeDoChildrenPost(s, popVar)
    in

    begin
      match s.skind with
      | If (Lval ((Var vinfo, NoOffset) as lval), _, _, _) 
      | If (UnOp (LNot, Lval ((Var vinfo, NoOffset) as lval), _), _, _, _) ->
	  begin
	    match self#rds s vinfo with
	    | Some lv ->
		pushVar s is_err_vars lv; (* IS_ERR *)
	    | None ->
		pushVar s vars lval (*if no IS_ERR*)
	  end;

      | If (BinOp (Ne, Lval ((Var vinfo, NoOffset) as lval), zero, _), _, _, _)
      | If (BinOp (Ne, zero, Lval ((Var vinfo, NoOffset) as lval), _), _, _, _)
	  when isZero zero ->
	     pushVar s vars lval
	    
      (*new pattern*)
      | If (BinOp (Eq, Lval ((Var vinfo, NoOffset) as lval), Const(CInt64(value,_,_)), _), trueBlock, _, _) ->
          begin
            match Error.byValue value with
            | Some code -> 
              let setInstr = Set(lval, integer(-82080000), get_stmtLoc s.skind) in (*it was -82079999, former ETRANSF fake error code*)
                  trueBlock.bstmts <- [mkStmtOneInstr setInstr] @ trueBlock.bstmts
              
            | _ -> ()
          end;
          pushVar s retry_vars lval

      | If (BinOp (Ne, Lval ((Var vinfo, NoOffset) as lval), _, _), _, _, _)
      | If (BinOp (Eq, Lval ((Var vinfo, NoOffset) as lval), _, _), _, _, _)
      | If (BinOp (Ne, CastE(_, Lval ((Var vinfo, NoOffset) as lval)), _, _), _, _, _)
      | If (BinOp (Eq, CastE(_, Lval ((Var vinfo, NoOffset) as lval)), _, _), _, _, _) ->
	  pushVar s retry_vars lval

      | Instr [Call (receiver, (Lval (Var callee, NoOffset)), actuals, loc) as instruction] ->
	  begin

	    (*ignore(Cil.dumpStmt plainCilPrinter stderr 0 s);*)
	    
	    (* Pattern: 1 *)
	    (* clearing the arguments of those functions in clearArgFuns *)
	    if (self#exists callee.vname clearArgFuns) then
	      begin
		List.iter (function
		  | Lval((Var(varinfo),_) as lval)
		  | CastE(_, Lval((Var(varinfo),_) as lval)) ->  
		      begin
		       let setInstr = Set(lval, integer(-82080000), get_stmtLoc s.skind) in
		           match s.skind with
		           | Instr currentInstructions ->
                                 (* prepending so that an error is not reported for an assignment such as v = printk(v) *)
			      let instructions = setInstr::currentInstructions in
			        s.skind <- Instr instructions;
		           | _ ->
			      failwith "Unexpected smtmkind..\n"
		      end
		  | _ -> ignore();
		) actuals;
	      end;

	    (* Pattern: 2 *)
	    (* for those functions in okAssignAfterFuns, it is OK to overwrite anything right after *)
	    (* the call, thus we insert a trusted assignment that clears the receiver of the subsequent assignment (if any)*) 
	    if (self#exists callee.vname okAssignAfterFuns) then
	      begin
		List.iter (fun succ -> 
		  match succ.skind with
                    | Instr[Set((Var _, NoOffset) as lval, _, _)]
                    | Instr [Call (Some ((Var _, NoOffset) as lval), _, _, _)] ->
		       let setInstr = Set(lval, integer(-82080000), get_stmtLoc s.skind) in
		       begin
		           match s.skind with
		           | Instr currentInstructions ->
			      let instructions = setInstr::currentInstructions in
		                  s.skind <- Instr instructions
			   | _ ->
			       failwith "Unexpected smtmkind..\n"
		       end;
                    | _ -> ignore(); (*the assignment must be right after, do nothing otherwise*) 
		  ) s.succs;
	      end;

	    
             (* Pattern: 3 *)
	    (* for those functions in okIfCheckedFuns, those checked variables are cleared when the function is called*)
	    if (self#exists callee.vname okIfCheckedFuns) then
	      begin
		List.iter (fun lval ->
                    let setInstr = Set(lval, integer(-82080000), get_stmtLoc s.skind) in
		      match s.skind with
		      | Instr currentInstructions ->
			 let instructions = currentInstructions@[setInstr] in
			     s.skind <- Instr instructions
		      | _ ->
			 failwith "Unexpected smtmkind..\n";
		) !vars;
	      end;

	    (* Pattern: 4a *)
	    (* An assignment for the form a = foo(); *)
	    begin
	      match receiver with
	      | Some ((Var vinfo, NoOffset) as lval) ->
		if (self#isChecked vinfo.vname) then
                      let setInstr = Set(lval, integer(-82080000), get_stmtLoc s.skind) in
                          s.skind <- Instr[setInstr; instruction]
	      | _ -> ignore();
	   end;

	  SkipChildren
	  end

      (* Pattern: 4 *)
      (* It is OK to assign to a variable inside of conditionals that check for it *)
      (* This could be done in the else branch, which is still OK (no error in that branch*)
      | Instr[Set((Var vinfo, NoOffset) as lval, expr, _) as instruction] ->
          if ((self#notTrusted expr) && (self#isChecked vinfo.vname)) then
	    begin
                let setInstr = Set(lval, integer(-82080000), get_stmtLoc s.skind) in
                    s.skind <- Instr[setInstr; instruction]
	    end;
	  SkipChildren

      (* Pattern: 5 *)
      (* if a goto retry is found inside a conditional, clear any variables checked *)
      (* the label does not have to be called retry, but it should be a label from the original source code*)
      | Goto(targets,loc) ->
	  if (self#isRealGoto !targets.labels) then
	    begin
	      try
		let lval = List.hd (!is_err_vars @ !retry_vars) in
		let setStmt = mkStmtOneInstr(Set(lval, integer(-82080000), get_stmtLoc s.skind)) in
		let newSkindStmt = Block(mkBlock [setStmt; mkStmt(Goto(targets,loc))]) in
	             s.skind <- newSkindStmt;
              with Failure hd -> ignore(); (*The goto is not in a conditional, do nothing*)
	   end
	  else
	    begin
	      try
		let lval = List.hd !is_err_vars in
		let setStmt = mkStmtOneInstr(Set(lval, integer(-82080000), get_stmtLoc s.skind)) in
		let newSkindStmt = Block(mkBlock [setStmt; mkStmt(Goto(targets,loc))]) in
	             s.skind <- newSkindStmt;
              with Failure hd -> ignore(); (*The goto is not in a conditional, do nothing*)
	    end;
	  SkipChildren

      | _ ->
	  DoChildren
    end

  method vfunc(f: fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    computeRDs f;
    DoChildren;

end



let transferFunctions pds f positive =
  let visitor = new visitor pds positive in
  ignore(visitCilFileSameGlobals (visitor :> cilVisitor) f);


(*Print statement*)
(*ignore(dumpStmt plainCilPrinter stderr 0 succ;)*)
