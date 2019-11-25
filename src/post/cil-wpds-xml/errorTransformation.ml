open Cil
open Hashtbl
open List
open Pretty
open Reachingdefs
open Inthash
open Set

let printCilObj d_obj = fun obj chan ->
    let d = dprintf "%a" d_obj obj in
    let s = sprint 80 d in
    ignore( fprintf chan "[%s]\n" s )

let printCilLval = printCilObj d_lval
let printCilExp = printCilObj d_exp
let printCilType = printCilObj d_type
let printCilStmt = printCilObj d_stmt

(* Removes a given function from the list of globals *)
let rec remove funName globals =
  try
    match hd globals with
    | GFun(fdec, _) -> 
	if (String.compare fdec.svar.vname funName == 0) then
	  remove funName (List.tl globals)
	else
	  (List.hd globals) :: remove funName (List.tl globals)			  
    | _ -> 
	(List.hd globals) :: remove funName (List.tl globals)
			      
  with Failure hd -> []


(* Returns name for dereference variable. If single is false, then it is assumed to be a double dereference *)
let derefName name single = 
  let derefs = if single then "*" else "**" in
  if String.contains name '$' then
    begin
      let index = String.index name '$' in
      let funName = String.sub name 0 (index + 1) in
      let exchName = String.sub name (index + 1) (String.length name - index - 1) in
	String.concat "" [funName; derefs; exchName]
    end
  else
      Printf.sprintf "%s%s" derefs name


(* Creates a deref variable for each pointer variable *)
let createDerefVariables file tableDerefVars =
  let visitor = function
    | GFun (fundec, _) ->
	begin
	  List.iter (fun varinfo -> 
	    match unrollTypeDeep varinfo.vtype with
	    | TPtr(typ,_) ->
		let name = derefName varinfo.vname true in
		let newvarinfo = makeLocalVar fundec name typ in (* added to fundec.slocals *)
		    tableDerefVars#add varinfo (Cil.var newvarinfo)
	    | _ -> ()
	  ) (fundec.sformals @ fundec.slocals);
	end
    | GVar (varinfo, _, _)
    | GVarDecl (varinfo, _) ->	
    begin
	  match unrollTypeDeep varinfo.vtype with
	  | TPtr(typ,_) ->
	      begin
		let name = derefName varinfo.vname true in
		let newvarinfo = makeGlobalVar name typ in
		let global = GVar(newvarinfo, {init = None}, locUnknown) in
		begin
	          file.globals <- global :: file.globals;
		  tableDerefVars#add varinfo (Cil.var newvarinfo)
		end
	      end

	  | _ -> ()
	end
    | _ -> ()
in
  iterGlobals file visitor


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


(* Replaces pointer references with new deref variable *)
class replaceMemVisitor tableDerefVars = object(self)
  inherit nopCilVisitor

  method vlval(lv:lval) =
    try
      begin
	match lv with
	|  (Var(vinfo), _) ->
	    DoChildren
	| (Mem(Lval(Var(vinfo),NoOffset)), NoOffset) ->
	    begin
	      match vinfo.vtype with
	      | TPtr(_,_) ->
		  let derefVar = tableDerefVars#find vinfo in
	            ChangeTo(derefVar);		      
		  
		  (* It is a dereference of the type r.results->a see test 173*)
	      | _ ->
		  SkipChildren
	    end

	| (Mem(Lval(Var(vinfo),NoOffset)), Field(_,_)) -> (* new *)
	    begin
	      match vinfo.vtype with
	      | TPtr(_,_) ->
		    DoChildren
		  
		  (* It is a dereference of the type r.results->a see test 173*)
	      | _ ->
		  SkipChildren
	    end

	| _ ->
	    DoChildren
      end
    with Not_found -> 
      failwith("\t[ERROR] Missing 'deref' for pointer variables.\n");


  method vfunc(f:fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end



(* Clears variables right after dereference *)
class clearDerefsVisitor tableDerefVars = object(self)
  inherit nopCilVisitor


  method private findDerefsInLval(lv:lval) =
    match lv with
    | (Var vinfo, _) -> []
	    
    | (Mem(Lval(Var(vinfo),NoOffset)), NoOffset) ->
	begin
	  match vinfo.vtype with
	  | TPtr(_,_) -> [vinfo]
	      
	   (* It is a dereference of the type r.results->a see test 173*)
	  | _ -> []
	end

    | (Mem(Lval(Var(vinfo),NoOffset)), Field(_,_)) -> (*only add to list*)
	begin
	  match vinfo.vtype with
	  | TPtr(_,_) -> [vinfo]	      
		  
          (* It is a dereference of the type r.results->a see test 173*)
	  | _ -> []
	end
	  
    | _ -> []



  method private findDerefsInExp(e:exp) =
    match e with
    | SizeOfE(e2) | AlignOfE(e2) | UnOp(_,e2,_) | CastE(_, e2) ->
	self#findDerefsInExp e2;

    | BinOp(_, e2, e3, _) ->
	  (self#findDerefsInExp e2) @ (self#findDerefsInExp e3);

    | Lval(lv) | AddrOf(lv) | StartOf(lv) ->
	self#findDerefsInLval lv;

    | _ -> []



  method private printVars(vars:varinfo list) =
    List.iter (fun v -> ignore(fprintf stderr "\t%s\n" v.vname)) vars;


  method vstmt(s: stmt) = 
    match s.skind with
    | Instr[Set(lv, e, _) as original] ->
	begin
	  let vars = (self#findDerefsInLval lv) @ (self#findDerefsInExp e) in
	  let assgns = List.map(fun vinfo -> Set(var vinfo, integer(-67737868), get_stmtLoc s.skind)) vars in
	  s.skind <- Instr (original::assgns);
	  SkipChildren
	end
	  
    | Instr [Call(receiver, (Lval (Var _, NoOffset)), args, _) as original] ->
	begin
	  let vars = List.flatten (List.map (fun e -> self#findDerefsInExp e) args) in
	  let assgns = List.map (fun vinfo -> Set(var vinfo, integer(-67737868), get_stmtLoc s.skind)) vars in
	  match receiver with
	  | Some lv ->
	      begin
		(* TODO: receiver *)
		s.skind <- Instr (original::assgns);
		SkipChildren
	      end;
	  | None -> 
	      begin
		s.skind <- Instr (original::assgns);
		SkipChildren
	      end
	end;
	
    | Return(Some e, _) ->
	DoChildren
	
    | If(e, b1, b2, _) ->
	begin
	  let vars = self#findDerefsInExp e in
	  let assgns = List.map(fun vinfo -> mkStmtOneInstr (Set(var vinfo, integer(-67737868), get_stmtLoc s.skind))) vars in
	  begin
	    b1.bstmts <- assgns @ b1.bstmts;
	    b2.bstmts <- assgns @ b2.bstmts;
	    DoChildren
	  end
	end
	
    | Switch(_, _, _, _) | Loop(_, _, _, _) | Block(_) ->
	  DoChildren
	
    | _ ->
	DoChildren
	  

  method vfunc(f: fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end



(* Inserts special assignments *)
class insertAssgnVisitor tableDerefVars = object(self)
  inherit nopCilVisitor

  method vstmt(s: stmt) = 
    try
      begin
	match s.skind with
	(* v1 = &v2; becomes v1 = OK; (untrusted) *v1 = v2;*)
	| Instr[Set((Var vinfo1, NoOffset) as lval1, AddrOf((Var vinfo2, _) as lval2), _)] ->
	    begin
              let setInstr1 = Set(lval1, integer(-67737870), get_stmtLoc s.skind) in (* untrusted, but does not trigger a deref marker *)
	      let derefVar = tableDerefVars#find vinfo1 in
	      let setInstr2 = Set(derefVar, Lval lval2, get_stmtLoc s.skind) in
                  s.skind <- Instr[setInstr1; setInstr2]
	    end

	(* *v = constant; remains the same with addidion of v = OK; (untrusted) *)
	| Instr[Set((Mem(Lval((Var vinfo, _) as lval)), off), Const(CInt64(value,_,_)), loc) as instruction] ->
	    let setInstr = Set(lval, integer(-67737869), get_stmtLoc s.skind) in
	        s.skind <- Instr[instruction; setInstr]

	(* v = (cast)constant; where v is a pointer variable, remains the same with addition of *v = OK; (untrusted) *)
	| Instr[Set((Var vinfo, NoOffset), _, loc) as instruction] ->
	    begin
	      match vinfo.vtype with
	      | TPtr(_,_) ->
		  let derefVar = tableDerefVars#find vinfo in
		  let setInstr = Set(derefVar, integer(-67737870), get_stmtLoc s.skind) in (* untrusted, but does not trigger a deref marker *)
		      s.skind <- Instr[instruction; setInstr]
	      | _ -> ignore();
	    end

	| _ -> ignore()
      end;
      DoChildren

    with Not_found ->
      ignore(fprintf stderr "[ERROR] Exception in vstmt.\n");
      DoChildren

  method vfunc(f: fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end



(* Implement transfer functions (both transformation ON and OFF) *)
class transferFunctionVisitor transformation transfer pds positive = object(self)
  inherit rdVisitorClass


  method private rds(s: stmt)(vinfo: varinfo)(nonErrorBlock: block)(loc:location) =
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
			      self#rds stmtdef lhs nonErrorBlock loc
			  | None -> ignore()
			end
			  
		    | Instr [Call (Some receiver, (Lval (Var callee, NoOffset)), actuals, _)] ->
			if (String.compare callee.vname "IS_ERR" == 0) then
			  begin
			    let argument = (List.hd actuals) in
			    match argument with
			    | Lval lval
			    | CastE(_, Lval lval) ->
				begin
				  let setInstr = Set(lval, integer(-67737868), loc) in
				  nonErrorBlock.bstmts <- (mkStmtOneInstr setInstr) :: nonErrorBlock.bstmts
				end
			    | _ -> ignore();
			  end
			    
		    | _ -> ignore()
		  end
	    | None -> ignore()
	  end
      | None -> ignore()

    with Not_found -> 
      (*failwith("\t[ERROR] Reaching definitions info unavailable.\n");*)
      ignore(fprintf stderr "\t[ERROR] Reaching definitions info unavailable.\n");


  method vstmt(s: stmt) =
    begin
      (*ignore(Cil.dumpStmt plainCilPrinter stderr 0 s);*)
      match s.skind with

	(* Taking care of IS_ERR pattern *)
      | If (Lval (Var vinfo, NoOffset), _, nonErrorBlock, loc)
      | If (UnOp (LNot, Lval (Var vinfo, NoOffset), _), nonErrorBlock, _, loc) ->
	  begin
	    self#rds s vinfo nonErrorBlock loc;
	    DoChildren
	  end

	
      | Instr [Call (receiver, (Lval (Var callee, NoOffset)), actuals, loc) as instruction] ->
	  begin
	    if (not transformation) then
	      begin
		if (String.compare callee.vname "ERR_PTR" == 0) then
		  
                  (* Clear arguments *)
		  List.iter (function
		    | Lval((Var(varinfo),_) as lval)
		  | CastE(_, Lval((Var(varinfo),_) as lval)) ->  
		      begin
			let setInstr = Set(lval, integer(-67737868), get_stmtLoc s.skind) in
		        match s.skind with
		        | Instr currentInstructions ->
			    let instructions = setInstr::currentInstructions in
			    s.skind <- Instr instructions
		        | _ ->
			    failwith "Unexpected smtmkind..\n";
		      end
		  | _ -> ignore();
		      ) actuals
		    
		else if (String.compare callee.vname "PTR_ERR" == 0) then
		  
		  (* Clear receiver *)
		  begin
		    match receiver with
		    | Some var ->
			let setInstr = Set(var, integer(-67737868), get_stmtLoc s.skind) in
			s.skind <- Instr[instruction; setInstr]
		    | None -> ignore()
		  end;		    		
	      end;
	      SkipChildren
	  end

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



(* Removing the definition for IS_ERR; we do not want to analyze its body *)
let removeIsErrorFunction file transformation = 
  begin
    file.globals <- remove "IS_ERR" file.globals;
    if not transformation then
      begin
	file.globals <- remove "ERR_PTR" file.globals;
	file.globals <- remove "PTR_ERR" file.globals
      end
  end



(* Applying transfer functions *)
let transferFunctions transformation transfer pds file positive tableDerefVars =

  (* Creating deref variables *)
  createDerefVariables file tableDerefVars;

  let visitor1 = new insertAssgnVisitor tableDerefVars in
  let visitor2 = new clearDerefsVisitor tableDerefVars in
  let visitor3 = new replaceMemVisitor tableDerefVars in
  begin
    (* Inserting assignments *)
    ignore(visitCilFileSameGlobals (visitor1 :> cilVisitor) file);
    (* Clearing variables after dereference *)
    ignore(visitCilFileSameGlobals (visitor2 :> cilVisitor) file);
    (* Replacing Mem expressions with Lval expressions *)
    ignore(visitCilFileSameGlobals (visitor3 :> cilVisitor) file)
  end;


  (* Applying transfer functions *)
  let visitor = new transferFunctionVisitor transformation transfer pds positive in
      ignore(visitCilFileSameGlobals (visitor :> cilVisitor) file)


(*ignore(Cil.dumpStmt plainCilPrinter stderr 0 s);*)
