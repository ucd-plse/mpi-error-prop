open Cil
open Printf
open List
open Errorheader
open Errorprint
open Errorbuildgraph
module P = Pretty


(* ------------------------- Expression visitor ----------------------- *)


(***************************************************)   
(* function node visitor                           *)
(***************************************************)
let findVarinfoInList (vi:varinfo) (i:intarg) (x:intarg) (lvi:varinfo) =
  begin
    i.ival <- i.ival + 1;
    if (vi = lvi) then 
      x.ival <- i.ival
    else () 
  end 
    



(***************************************************)   
(* Expression Visitor: Find EIO in expr            *)
(* given an expression, it checks whether the expr *)
(* contains EIO                                    *)
(*   e.g. in Return (EIO | whatever1 | whatever2)  *)
(***************************************************)
(* better change all EIO occurences in jfs to (-EIO) *)
(* if (value = -5 || value = 5) then *)
(* XFS uses positive value *)
(* if (value = -5) then *)
(* EROFS 30 *)
(* EIO: H-A-R--Y--A-D-I *)
(* EIO: 8-1-18-24-1-4-9 *)
class expvFindEIO (isFoundEIO: boolarg) = object 
  inherit nopCilVisitor
  method vexpr (e:exp) = 
    begin
      (match e with
      | Const(CInt64(x,_,_)) ->      
	  begin
	    let value = (Int64.to_int x) in
	    if ((value >= 82070001 && value <= 82070034) 
	      ||(value >= -82070034 && value <= -82070001)) then
	      isFoundEIO.bval <- true
	    else () 
	  end
      | _ -> ());
      (* DoChildren*)
      SkipChildren
    end
end

(***************************************************)   
(* expr visitor: Find Var in Expr                  *)
(***************************************************)
class expvFindVarInExpr (vi: varinfo) (isFoundVar: boolarg) = object
  inherit nopCilVisitor
  method vexpr (e:exp) = 
    begin
      (match e with
	(* note here .. depends on how precise I want to do this *)
      | BinOp (LAnd,_,_,_)
      | BinOp (LOr,_,_,_)
      | BinOp (BAnd,_,_,_)
      | BinOp (BXor,_,_,_)
      | BinOp (BOr,_,_,_)
      | BinOp (Lt,_,_,_)
      | BinOp (Gt,_,_,_)
      | BinOp (Le,_,_,_)
      | BinOp (Ge,_,_,_)
      | BinOp (Eq,_,_,_)
      | BinOp (Ne,_,_,_)
      | UnOp (_,_,_) -> DoChildren
      | CastE (_,_) -> DoChildren
      | Lval(Var(cvi),NoOffset) -> 
	  begin
	    if (cvi == vi) then
	      isFoundVar.bval <- true
	    else ();
	    SkipChildren
	  end
      | _ -> SkipChildren
      );
    end
end

(***************************************************)   
(* func visitor: Find Var in If Statement          *)
(***************************************************)
class fnvFindVarInIfStmt (vi: varinfo) (isFoundVar: boolarg) = object
  inherit nopCilVisitor
  method vstmt (s:stmt) : stmt visitAction =
    begin
      (match s.skind with
      | If(exp,_,_,_) ->
	  begin
	    let _ = visitCilExpr (new expvFindVarInExpr vi isFoundVar) exp in ()
	  end
      | _ -> ());
      DoChildren
    end
end

(***************************************************)   
(* func visitor: Find Var in Return Statement      *)
(***************************************************)
class fnvFindVarInReturnStmt (vi: varinfo) (isFoundVar: boolarg) = object
  inherit nopCilVisitor
  method vstmt (s:stmt) : stmt visitAction =
    begin
      (match s.skind with
      | Return(Some(exp),_) -> 
	  begin
	    let isFoundVarInExpr = { bval = false } in
	    let _ = visitCilExpr (new expvFindVarInExpr vi isFoundVarInExpr) exp in
	    if isFoundVarInExpr.bval then
	      begin
		isFoundVar.bval <- true;
	      end
	    else ()
	  end
      | _ -> ());
      DoChildren
    end
end


(* |----------------------------------------------------------------------------| *)
(* |                       EIO Retval -- Endpoint Visitor                       | *)
(* |----------------------------------------------------------------------------| *)

(***************************************************)   
(* func visitor: Find Var in Assignment Statement  *)
(*          2a. A variable that is returned        *)
(*          2b. A variable that is checked         *)
(*          2c. A pointer in argument              *)
(*          2d. A structure (evolve)               *)
(*          2e. stored elsewhere again             *)
(*              (do recursive on 3)                *)
(***************************************************)
class fnvFindTransferRuleValid (outf) (node:funcNode) (rvi:varinfo) (isValid:boolarg) = object
  inherit nopCilVisitor
  method vinst (i:instr) : instr list visitAction =
    begin
      (match i with
      | Set(lval,exp,_) ->
	  begin
	    let isFoundVar = { bval = false } in
	    let _ = visitCilExpr (new expvFindVarInExpr rvi isFoundVar) exp in
	    (* whatever in the rval has been assigned to lval *)
	    if (isFoundVar.bval) then
	      begin
		(match lval with
		| (Var(lvi),_) 
		| (Mem(Lval(Var(lvi),_)),_) ->
		    begin
		      (* now check rules here *)
		      (* 2a, 2b *)
		      let isVarInIf = { bval = false } in
		      let _ = visitCilFunction (new fnvFindVarInIfStmt lvi isVarInIf) node.fd in
		      
		      (* let isVarInRet = { bval = false } in *)
		      (* let _ = visitCilFunction (new fnvFindVarInReturnStmt lvi isVarInRet) node.fd in *)
		      
		      (* if rule is matched *)
		      if (isVarInIf.bval) then
			begin
			  isValid.bval <- true;
			end
			  
		      	  (* else, check the new lval *)
		      else
			begin
			  let tmpIsValid = { bval = false } in
			  let _ = visitCilFunction (new fnvFindTransferRuleValid outf node 
						      lvi tmpIsValid) node.fd in
			  if (tmpIsValid.bval) then
			    begin
			      isValid.bval <- true; 
			    end
			end
		    end
		| _ -> ());
	      end
	    else ()
	  end
      | _ -> ());
      DoChildren
    end
end


(* |----------------------------------------------------------------------------| *)
(* |                       EIO Retval -- Propagate Visitor                      | *)
(* |----------------------------------------------------------------------------| *)
(***************************************************)   
(* func visitor: Find all variables where EIO      *)
(* could propagate through                         *)
(* e.g.                                            *)
(*      a = f1();                                  *)
(*      b = a | 2;                                 *)
(*      c = b | 3;                                 *)
(*      return c;                                  *)
(*                                                 *)
(* This function checks all chain of assignment    *)
(* and for each lval, it checks if the lval is     *)
(* returned or not                                 *)
(***************************************************)
class fnvCheckPropVarReturned (outf: Unix.file_descr) (node:funcNode) 
    (rvi:varinfo) (vilist: varinfo list) (isReturned:boolarg) = object (self)
  inherit nopCilVisitor
      (* first it visits all instructions, and check if it has rvi in the *)
      (* right hand expression *)
  method vinst (i:instr) : instr list visitAction =
    begin

      (* check if a = b = c = a *)
      let checkLviInList (isLviInList:boolarg) (tmp1:varinfo) (tmp2:varinfo) =
	begin
	  if (tmp1 == tmp2) then isLviInList.bval <- true else ()
	end
      in
      
      (match i with
      | Set(lval,exp,_) ->
	  begin
	    
	    let isFoundVar = { bval = false } in
	    let _ = visitCilExpr (new expvFindVarInExpr rvi isFoundVar) exp in
	    
	    (* rvi is found in right hand expression *)
	    (* whatever in the rval has been assigned to lval *)
	    if (isFoundVar.bval) then
	      begin
		(match lval with
		| (Var(lvi),_) ->
		    begin
		      
		      (* check if lvi is alredy in vilist *)
		      (* be careful with infinite loop e.g. : written = written + 1 *)
		      let isLviInList = { bval = false } in
		      List.iter (checkLviInList isLviInList lvi) vilist;

		      (* &&  not x = x  *)
		      if (not isLviInList.bval && (not (lvi == rvi))) then
			begin
			  
			  (* now check if lvi is ever occur in any Return statement *)
			  let isVarInRet = { bval = false } in
			  let _ = visitCilFunction (new fnvFindVarInReturnStmt lvi isVarInRet) 
			      node.fd in 
			  
			  (* if lvi so, then it is returned *)
			  if (isVarInRet.bval) then
			    begin
			      isReturned.bval <- true;
			    end
		      	      (* else, we must recursively check the new lval *)
			  else
			    begin
			      let tmpIsReturned = { bval = false } in

			      (* Out_of_memory !!!!  *)
			      (* the solution is when to change expvFindInVar *)
			      (* to only include logical operator, but not    *)
			      (* arithmetic operator                          *)

			      let _ = visitCilFunction  
                                  (new fnvCheckPropVarReturned outf node  
				     lvi (lvi :: vilist) tmpIsReturned) node.fd in
			      
			      if (tmpIsReturned.bval) then
				begin
				  isReturned.bval <- true;
				end
			    end
			end
		    end
		| _ -> ());
	      end
	    else ()
	  end
      | _ -> ());
      DoChildren 
    end
end




(* |----------------------------------------------------------------------------| *)
(* |                                                                            | *)
(* |                       EIO Retval                                           | *)
(* |                                                                            | *)
(* |----------------------------------------------------------------------------| *)


(* |----------------------------------------------------------------------------| *)
(* |                       EIO Retval -- Direct                                 | *)
(* |----------------------------------------------------------------------------| *)

(***************************************************)   
(* Class cgvMarkDirectEIOretval (cg)               *)
(***************************************************)
class cgvMarkDirectEIOretval (cg:callGraph) = object
  inherit nopCilVisitor
  val the_fun_name = ref None
      
      (* case: return .. | -EIO | .. *)
  method vstmt (s:stmt) : stmt visitAction =
    begin
      (match s.skind with
	(* Find all Return statement *)
      | Return(Some(exp),_) -> 
	  begin
	    let isFoundEIO = { bval = false } in 
	    let _ = visitCilExpr (new expvFindEIO isFoundEIO) exp in  
	    if (isFoundEIO.bval) then 
	      begin
		match !the_fun_name with
		  None -> failwith "cgv: call outside of any function"
		| Some(fcname) -> 
		    begin
		      try
			let n1 = cgFindNode cg fcname in
			n1.nRetvalEIO.direct <- true;
			n1.nRetvalEIO.involve <- true;
		      with _ -> ()		    
		    end
	      end
	    else ()
	  end
      | _ -> ());
      DoChildren
    end

      (* case: ret = -EIO;   return ret; *)
  method vinst (i:instr) =
    begin
      (match !the_fun_name with
	None -> failwith "cgv: call outside of any function"
      | Some(fcname) -> 
	  begin
	    let n1 = cgFindNode cg fcname in
	    begin
	      (match i with
	      | Set ((Var(vi),_), exp, _) ->
		  begin
		    let isFoundEIO = { bval = false } in 
		    let _ = visitCilExpr (new expvFindEIO isFoundEIO) exp in  
		    if (isFoundEIO.bval) then 
		      let isVarInRet = { bval = false } in
		      let _ = visitCilFunction (new fnvFindVarInReturnStmt vi isVarInRet) 
			  n1.fd in
		      if (isVarInRet.bval) then
			begin
			  try
			    n1.nRetvalEIO.direct <- true;
			    n1.nRetvalEIO.involve <- true;
			  with _ -> ()		    
			end
		      else ();
		  end
	      | _ -> ());
	    end
	  end);
      DoChildren
    end
  method vfunc f = the_fun_name := Some(f.svar.vname); DoChildren
end
    

    
(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvMarkDirectEIOarg (outf: Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor
      
  method vinst (i:instr) : instr list visitAction =
    begin
      (match i with
      | Set(lval, exp, _) ->
	  begin
	    let isFoundEIO = { bval = false } in 
	    let _ = visitCilExpr (new expvFindEIO isFoundEIO) exp in
	    if (isFoundEIO.bval) then 
	      begin
		(match lval with 
		|(Mem(Lval(Var(vi),_)),_) ->
		    begin
		      (match unrollTypeDeep vi.vtype with
		      | TPtr (TInt (x,_),_) -> 
			  begin
			    (* only do for int*, otherwise it will accoutn struct* and *)
			    (* you will have memory errors *)
			    let x = { ival = 0 } 
			    and i = { ival = 0 } in
			    (List.iter (findVarinfoInList vi i x) node.fd.sformals);
			    if (x.ival > 0) then
			      begin
				(* found an EIO arg, now saved the argument number and varinfo  *)
				(* (fprintf stderr " MEM of %s at %d ..\n" vi.vname x.ival);  *)
				node.nArgEIO.direct <- true;
				node.nArgEIO.errloc <- x.ival;
				node.nArgEIO.involve <- true;
			      end
			  end
		      | _ -> ());
		    end
		| _ -> ());
	      end
	  end
      | _ -> ());
      DoChildren
    end
end



(***************************************************)   
(* main.operateCallGraphNodes                      *)
(* List all stuffs that we want to initialize on   *)
(* per node basis                                  *)
(*   1. mark direct EIOretval returns                    *)
(***************************************************)
let rec cgMarkDirectEIOretval (fi: Cil.file) (cg: callGraph) =
  begin
    visitCilFileSameGlobals (new cgvMarkDirectEIOretval cg) fi ;
  end


(* |----------------------------------------------------------------------------| *)
(* |                       EIO Retval -- Propagate                              | *)
(* |----------------------------------------------------------------------------| *)


    
(***************************************************)   
(* main.cgMarkPropagateEIOretval                   *)
(* DEFINITION: propagate if the saved return value *)
(* is returned OR the saved return value is stored *)
(* elsewhere and is returned also                  *)
(***************************************************)
    
    (* 3. analyze the caller edges *)
and cgMarkPropagateEIOretvalCallerEdge (outf:Unix.file_descr) (edge:funcEdge) = 
  begin
    (* first analyze the edge, no matter what *)
    (match edge.eInstr with
	  (* err = func() *)
    | Call(Some(Var(lvi),_),Lval(flval),_,_) ->
	begin
	  
	  (* since the value is stored in the caller, *)
	  (* this edge propagates the EIO *)
	  edge.eRetvalEIO.edgePropagate <- true;
	  
	  let isVarInRet = { bval = false }
	  and isReturned = { bval = false } in
	  
	  let _ = visitCilFunction (new fnvFindVarInReturnStmt lvi isVarInRet) 
	      edge.caller.fd in
	  
	  (* Out_of_memory here *)
	  let visitor = (new fnvCheckPropVarReturned) in
	  let _ = visitCilFunction (visitor outf edge.caller  
				      lvi [] isReturned) edge.caller.fd in
	  
	  (* let _ = visitCilFunction (new fnvCheckPropVarReturned outf edge.caller  *)
	  (* lvi [] isReturned) edge.caller.fd in *)
	  

	  (* if the caller already propagate EIO of other calls, *)
	  (* then no need to proceed, but nevertheless need to check, if this *)
	  (* particular  retval is propagated *)
	  if edge.caller.nRetvalEIO.propagate then 
	    begin
	      if (isVarInRet.bval || isReturned.bval) then
		edge.eRetvalEIO.callerPropagate <- true
	      else ()
	    end
	      
	      (* Else, check if the caller propagates this particular EIO or not by *)
	      (* checking all return values. One or more return value should *)
	      (* contain lvi, is so then the node propagate EIO *)	      
	  else
	    begin
	      if (isVarInRet.bval || isReturned.bval) then
		begin
		  (* if lvi is returned by the caller to the upper caller, *)
		  (* then mark caller propagate, and traverse up *)
		  (*               OR                            *)
		  (* lvi might be stored elsewhere, and elsewhere might be returned *)
		  edge.eRetvalEIO.callerPropagate <- true;
		  edge.caller.nRetvalEIO.propagate <- true;
		  edge.caller.nRetvalEIO.involve <- true;
		  (cgMarkPropagateEIOretvalFuncNode outf edge.caller.fcname edge.caller);
		end
	      else 
		begin
		  (* endpoint !!!!!!!!! *)
		  (* this is an endpoint, need to saved lvi *)
                  edge.eRetvalEIO.callerPropagate <- false;
		  edge.eRetvalEIO.endpoint <- true;
		  edge.eRetvalEIO.endpointVarInfo = lvi;
		  edge.caller.nRetvalEIO.involve <- true;
		end
	    end
	end
	  (* case where retval is not saved .. VIOLATION *)
	  (* ALSO case where retval is saved to errro pointer .. VIOLATION *)
    | Call(None,Lval(flval),_,_)
    | Call(Some((Mem(Lval(flval)),NoOffset)),_,_,_) -> 
	begin
	  (* endpoint !!!!!!!!! *)
	  (* but no lvi, so basically this is just a VIOLATION *)
          edge.eRetvalEIO.edgePropagate <- false;
          edge.eRetvalEIO.callerPropagate <- false;
	  edge.eRetvalEIO.endpoint <- true;
	  (* edge.eRetvalEIO.endpointVarInfo = lvi; *)
	  edge.caller.nRetvalEIO.involve <- true;
	end
    | _ -> ());
  end
    
    (* 2. Check the node, if direct/propagate, traverse the caller edges *)
and cgMarkPropagateEIOretvalFuncNode (outf:Unix.file_descr) 
    (fcname:string) (node:funcNode) : unit  =
  begin
    
    if node.nRetvalEIO.direct then
      (List.iter (cgMarkPropagateEIOretvalCallerEdge outf) node.callers)
    else if node.nRetvalEIO.propagate then
      (List.iter (cgMarkPropagateEIOretvalCallerEdge outf) node.callers)
    else ()
  end

and cgMarkPropagateEIOretval (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (* 1. traverse the callgraph *)
    (Hashtbl.iter (cgMarkPropagateEIOretvalFuncNode outf) cg)
  end
    
    
	

(* |----------------------------------------------------------------------------| *)
(* |                       EIO Retval -- Endpoint                               | *)
(* |----------------------------------------------------------------------------| *)



(***************************************************)   
(* Mark endpoint                                   *)
(* At end point, what is the rule?                 *)
(*   1. Saved and check (saved and if)             *)
(*   2. Saved and store elsewhere                  *)
(*        elsewhere must be:                       *)
(*          2a. A variable that is returned        *)
(*          2b. A variable that is checked         *)
(*          2c. A pointer in argument              *)
(*          2d. A structure (evolve)               *)
(*          2e. stored elsewhere again             *)
(*              (do recursive on 3)                *)
(***************************************************)
and cgAnalyzeEndPointEIOretval (outf: Unix.file_descr) (cg: callGraph) =
  begin
    let analyzeCalleeEdge (edge:funcEdge) = 
      begin
	if (edge.eRetvalEIO.endpoint) then
	  begin
	    if (not edge.eRetvalEIO.edgePropagate) then
	      begin
		(* *errp = func1, is considered not propagate so do stuff here *)
		(* check if it evolves to arg pointer *)
		(match edge.eInstr with
		  
		  (* 2) this covers retval saved to pointer *)
		  (*    for example:                        *)
		  (*      *err = func ();                   *)
		  (*    Need to check if err is propagated  *)
		  (*    retval has evolved to pointers      *)
		| Call(Some((Mem(exp),NoOffset)),_,_,_) -> 
		    begin
		      (* (P.fprint stderr 80 (P.dprintf "  (2) %a \n" dn_instr edge.instr)) ;  *)
		      (match exp with
		      | Lval (Var(vi),_) ->
			  begin
			    (match unrollTypeDeep vi.vtype with
			    | TPtr (TInt (x,_),_) -> 
				begin
				  
				  let x = { ival = 0 } 
				  and i = { ival = 0 } in
				  (List.iter (findVarinfoInList vi i x) edge.caller.fd.sformals);
				  if (x.ival > 0) then
				    begin
				      edge.eRetvalEIO.endpointSaved <- true;
				      edge.eArgEIO.edgePropagate <- true;
				      edge.caller.nArgEIO.propagate <- true;
				      edge.caller.nArgEIO.errloc <- x.ival;
				      edge.caller.nArgEIO.involve <- true;
				      (cgMarkPropagateEIOargFuncNode 
					 edge.caller.fcname edge.caller); 
				    end
				end
			    | _ -> ());
			  end
		      | _ -> ());
		    end
		      (* double checked *)
		| Call(None,_,_,_) -> 
		    begin
		      edge.eRetvalEIO.endpointSaved <- true;
		    end
		| _ -> ());
	      end
		(* else, edge.RetvalEIO.edgePropagate == true *)
		(* case 1, the error is saved AND checked *)
	    else 
	      begin
		edge.eRetvalEIO.endpointSaved <- true;
		(match edge.eInstr with
		  
		  (* 1) ok, this covers simple thing like *)
		  (*      err = func ();                  *)
		  (*      err2 = err;                     *)
		  (*      return err2;                     *)
		| Call(Some(Var(vi),_),_,_,_) -> 
		    begin
		      let str = " ## " ^ vi.vname ^ "\n" in
		      (* (mypf outf str); *)
		      let isFoundVar = { bval = false } in
		      let _ = visitCilFunction (new fnvFindVarInIfStmt vi isFoundVar) 
			  edge.caller.fd in
		      if (isFoundVar.bval) then
			begin
			  edge.eRetvalEIO.endpointChecked <- true;
			end
		      else
			begin
			  let isTransferValid = { bval = false } in
			  let _ = visitCilFunction (new fnvFindTransferRuleValid outf edge.caller
						      vi isTransferValid) edge.caller.fd in
			  if (isTransferValid.bval) then
			    begin
			      edge.eRetvalEIO.endpointChecked <- true;
			    end
			end
		    end
		| _ -> ());
	      end
	  end
      end
    in
    
    let analyzeCaller (fcname:string) (node:funcNode) : unit  =
      (List.iter analyzeCalleeEdge node.callees)
    in
    
    (Hashtbl.iter analyzeCaller cg);
  end
  

    


(* |----------------------------------------------------------------------------| *)
(* |                                                                            | *)
(* |                       EIO Argument                                         | *)
(* |                                                                            | *)
(* |----------------------------------------------------------------------------| *)


(* |----------------------------------------------------------------------------| *)
(* |                       EIO  Argument  --  Direct                            | *)
(* |----------------------------------------------------------------------------| *)
    
    
    
(***************************************************)   
(*                                                 *)
(***************************************************)
and cgNodeMarkDirectEIOarg (outf: Unix.file_descr) (fcname:string) (node:funcNode) : unit  =
  begin
    (* let _ = visitCilFunction (new fnvMarkDirectEIOarg outf node) node.fd  *)
    let _ = visitCilFunction (new fnvMarkDirectEIOarg outf node) node.fd 
    in ();
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and cgMarkDirectEIOarg (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (cgNodeMarkDirectEIOarg outf) cg);
  end


(* |----------------------------------------------------------------------------| *)
(* |                       EIO  Argument  --  Propagate                         | *)
(* |----------------------------------------------------------------------------| *)



(***************************************************)
(* void f1 (a, b, c, err) {                     *)
(*     f2 (x, y, err);                          *)
(* }                                            *)
(* exp : is one of the arguments in a call e.g. x / y                 *)   
(* k   : will be 1,2,3 as it iterates x, y, err                       *)
(* loc : in this case is 3, since err in f2's formals is at 3rd       *)
(* x   : after findVarInfoInList, x should be 4, because f1's formals *)
(*     : if x = 0 it means f1's formal does not containt err          *)   
(* i   : will be 1,2,3,4 as it iterates a,b,c,err                     *)
(***************************************************)
and checkIfCallerPropagate (gotoProp:boolarg) (k:intarg) (loc:int)
    (edge:funcEdge) (exp:exp) =
  begin
    if (k.ival = loc) then
      begin
	(match exp with 
	  (* expr is the argument ... could be Var() or Mem () *)
	| Lval(Var(argvi),_) 
	| AddrOf(Var(argvi),_) ->
	    begin
	      let x = { ival = 0 } 
	      and i = { ival = 0 } in
	      (List.iter (findVarinfoInList argvi i x) edge.caller.fd.sformals);
	      (* found *)
	      if (x.ival > 0) then
		begin
		  (* (fprintf stderr "   # %s (%s[%d]) (%s[%d]) ..\n"  *)
		  (* argvi.vname edge.caller.fcname x.ival edge.callee.fcname loc);*)
		  
		  if (edge.caller.nArgEIO.propagate) then
		    gotoProp.bval <- false
		  else 
		    gotoProp.bval <- true;
		  
		  edge.eArgEIO.callerPropagate <- true;
		  edge.caller.nArgEIO.propagate <- true;
		  edge.caller.nArgEIO.errloc <- x.ival;
		  edge.caller.nArgEIO.involve <- true;
		  
		end
	      else 
		begin
		  (* abc *)
		  (* I also must check if it is returned the it's fine ???? *)
		  edge.eArgEIO.callerPropagate <- false;
		  edge.eArgEIO.endpoint <- true;
		  edge.eArgEIO.endpointVarInfo <- argvi;
		  edge.caller.nArgEIO.propagate <- false;
		  edge.caller.nArgEIO.errloc <- 0;
		  edge.caller.nArgEIO.involve <- true;
		end;
	    end
	| _ -> ());
      end
    else ();
    k.ival <- k.ival + 1;
  end
    
    
(*****************************************)
(* 3. analyze the caller edges           *)
(*****************************************)    
and cgMarkPropagateEIOargCallerEdge (edge:funcEdge) = 
  begin
    (* first analyze the edge, no matter what *)
    (match edge.eInstr with
    | Call(_,Lval(flval),arglist,_) ->
	begin
	  (* Now we obtain the argument list of this call           *)
	  (* Do this:                                               *)
	  (*   get the argument                                     *)
	  (*   check if the argument is propagated again            *)
	  (*                                                        *)
	  
	  (* since the formal contains the EIOarg *)
	  (* this edge propagates the EIOarg *)
	  edge.eArgEIO.edgePropagate <- true;	      
	  edge.caller.nArgEIO.involve <- true;
	  
	  let k = { ival = 1 } in
	  let gotoProp = { bval = false } in
	  (List.iter (checkIfCallerPropagate gotoProp k edge.callee.nArgEIO.errloc edge) arglist);
	  
	  if (gotoProp.bval) then
	    begin
	      (* found an EIO arg, now saved the argument number and varinfo *)
	      (cgMarkPropagateEIOargFuncNode edge.caller.fcname edge.caller);
	    end
	end
    | _ -> ());
  end
    

(*****************************************)
(* 2. Check the node, if direct/propagate, traverse the caller edges *)
(*****************************************)    
and cgMarkPropagateEIOargFuncNode (fcname:string) (node:funcNode) : unit  =
  begin
    if (node.nArgEIO.direct || node.nArgEIO.propagate) then
      begin
	(List.iter cgMarkPropagateEIOargCallerEdge node.callers);
      end
    else ()
  end
    
(*****************************************)
(*                                       *)
(*****************************************)    
and cgMarkPropagateEIOarg (outf:Unix.file_descr) (cg:callGraph) =
  begin
    (* 1. traverse the callgraph *)
    (Hashtbl.iter cgMarkPropagateEIOargFuncNode cg)
  end
    
    

(* |----------------------------------------------------------------------------| *)
(* |                       EIO  Argument  -- Endpoint                           | *)
(* |----------------------------------------------------------------------------| *)


(***************************************************)   
(*                                                 *)
(***************************************************)
and cgAnalyzeEndPointEIOarg (outf: Unix.file_descr) (cg: callGraph) =
  begin
    let analyzeCalleeEdge (edge:funcEdge) = 
      begin
	if (edge.eArgEIO.endpoint) then
	  begin
	    (* now check if lvi is ever occur in any Return statement *)
	    let isVarInRet = { bval = false } in
	    let _ = visitCilFunction (new fnvFindVarInReturnStmt 
					edge.eArgEIO.endpointVarInfo isVarInRet) edge.caller.fd in

	    (* osdi bug *)
            let isFoundVar = { bval = false } in
	    let _ = visitCilFunction (new fnvFindVarInIfStmt edge.eArgEIO.endpointVarInfo 
					isFoundVar) edge.caller.fd in ();
	    begin
	      (* if lvi so, then it is returned *)
	      if (isVarInRet.bval) then
		begin
		  (* the edge now propagates !!, don't forget to update both edge and node *)
		  edge.eRetvalEIO.edgePropagate <- true;
		  edge.eRetvalEIO.callerPropagate <- true;
		  edge.caller.nRetvalEIO.propagate <- true;
		  edge.caller.nRetvalEIO.involve <- true;
		  edge.eArgEIO.endpointChangeMechanism <- true;  
		  (cgMarkPropagateEIOretvalFuncNode outf edge.caller.fcname edge.caller);
		end;
	      (* else, we must recursively check the new lval ?? e.g. a = *err; return a *)

	      (* osdi bug *)
              if (isFoundVar.bval) then
		begin
                  edge.eArgEIO.endpointChecked <- true;
		end;
	    end
	      
	  end
      end
    in
    
    (* at this point, check if it evolves to something else *)
    (*   1) check if it is in return values                 *)
    let analyzeNode (fcname:string) (node:funcNode) : unit  =
      begin
	(List.iter analyzeCalleeEdge node.callees);	
      end
	
    in
    
    (Hashtbl.iter analyzeNode cg);
  end
  

    





    
    
