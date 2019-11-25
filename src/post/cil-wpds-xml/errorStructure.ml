open Cil
open Printf
open List
module P = Pretty



(* (P.fprint stderr 80 (P.dprintf "  (i) %a \n" dn_instr i)) ;  *)
(* (P.fprint stderr 80 (P.dprintf "  (s) %a \n" dn_stmt  s)) ;  *)
(* (P.fprint stderr 80 (P.dprintf "  (lv) %a \n" dn_lval l)) ;  *)
(* (P.fprint stderr 80 (P.dprintf "  (a) %a \n" dn_attr attr)) ;  *)
(* (P.fprint stderr 80 (P.dprintf "  (ap) %a \n" dn_attrparam ap)) ; *) 
(* (P.fprint stderr 80 (P.dprintf "    %a \n" dn_exp e)); *)

let pc (achar:string) = 
  begin
    (fprintf stdout "%s" achar);
  end
    

  
type funcNode = {
    dirname: string;
    filename: string;
    fcname: string;
    fd: Cil.fundec;     
    loc: Cil.location;
    mutable nColor: string;
    mutable nJournal: journalnode;
    mutable callers: funcEdge list;
    mutable callees: funcEdge list;
  }

and journalnode = {
    mutable related: bool;
    mutable directCheckAbort: bool;
    mutable exposeCheckAbort: bool;
    mutable confirmedCheckAbort: bool;
    mutable directSetAbort: bool;
    mutable callSetAbort: bool;
    mutable goodNode: bool;
    mutable goodChildren: bool;
    mutable badNode: bool;
  }
      
and funcEdge = {
    mutable caller: funcNode;
    mutable callee: funcNode;
    mutable instr: instr;
  }      

and boolarg = {
    mutable bval: bool;
  }      
      
and intarg = {
    mutable ival: int;
  }      

      
type callGraph = 
    (string, funcNode) Hashtbl.t 

      
(* ------------------------- Build CG  -------------------------- *)
let mypf (fd:Unix.file_descr) (str:string) =
  begin
    let _ = (Unix.write fd str 0 (String.length str)) in ()
  end
    

(* ------------------------- Build CG  -------------------------- *)

(***************************************************)   
(* cg.AddNode                                      *)
(***************************************************)
let cgAddNode (cg: callGraph) (origdirname: string) 
    (fi: Cil.file) (origfd: fundec) (origloc: location) = 
  begin
    let newjournal = {
      related = false;
      directCheckAbort = false;
      exposeCheckAbort = false;
      confirmedCheckAbort = false;
      directSetAbort = false;
      callSetAbort = false;
      goodNode = false;
      goodChildren = false;
      badNode = false;
    } in
    let newnode = { 
      dirname = origdirname;
      filename = fi.fileName;
      fcname = origfd.svar.vname;
      fd = origfd; 
      loc = origloc;
      nColor = "white";
      nJournal = newjournal;
      callers = []; 
      callees = [];
    } in 
    Hashtbl.add cg origfd.svar.vname newnode;
  end

(***************************************************)   
(* cg.AddEdge                                      *)
(***************************************************)
let cgFindNode (cg:callGraph) (fcname:string) = 
  Hashtbl.find cg fcname
    
    
(***************************************************)   
(* cg.AddEdge                                      *)
(***************************************************)
let cgAddEdge (cg:callGraph) (callerName:string) (calleeName:string) 
    (origInstr: instr) =
  begin
    try
      (* n1 = caller node, n2 = callee node *)
      let n1 = cgFindNode cg callerName in
      let n2 = cgFindNode cg calleeName in
      begin
	let newEdge = {
	  caller = n1;
	  callee = n2;
	  instr = origInstr;
	} in
	n1.callees <- newEdge :: n1.callees; 
	n2.callers <- newEdge :: n2.callers;
      end
    with _ -> ()
  end
    

(***************************************************)   
(* main.initCallGraph                              *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgBuildFirst (dirname: string) (fi: Cil.file) (cg: callGraph) =
  begin
    iterGlobals fi 
      (
       fun g -> match g with
	 GFun(fd,loc) -> cgAddNode cg dirname fi fd loc
       | _ -> ()
      );
  end

   
(* ------------------------- Construct Edge  -------------------------- *)

(***************************************************)   
(* Class cgvConstructEdge (cg)                     *)
(***************************************************)
class cgvConstructEdge (cg:callGraph) = object
  inherit nopCilVisitor
  val the_fun_name = ref None
      
  method vinst (origInstr:instr) =
    let _ = match origInstr with
      Call(_,Lval(Var(calleeVarInfo),NoOffset),_,_) -> begin
        (* known function call *)
        match !the_fun_name with
          None -> failwith "cgv: call outside of any function"
        | Some(callerName) -> 
	    cgAddEdge cg callerName calleeVarInfo.vname origInstr
      end
    | _ -> ()
    in DoChildren
      
  method vfunc f = the_fun_name := Some(f.svar.vname) ; DoChildren
end

(***************************************************)   
(* main.createCallGraph                            *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgConstructEdge (fi: Cil.file) (cg: callGraph) =
  begin
    visitCilFileSameGlobals (new cgvConstructEdge cg) fi ;
  end


(***************************************************)
(*                                                 *)
(***************************************************)
let cgAnalyzeFileSorted (outf: Unix.file_descr) (fi: Cil.file) (cg: callGraph) =
  begin

  end




(* ----------------------- Analyze --------------------- *)

(***************************************************)   
(* expression visitor                              *)
(***************************************************)
class expvFindEIO (isFoundEIO: boolarg) = object
  inherit nopCilVisitor
  method vexpr (e:exp) = 
    begin
      (match e with
      | Const(CInt64(x,_,_)) ->      
	  begin
	    let value = (Int64.to_int x) in
	    if (value = -5 || value = 5) then
	      isFoundEIO.bval <- true
	    else ()
	  end
      | _ -> ());
      DoChildren
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
      | Lval(Var(cvi),_) ->
	  if (cvi = vi) then
	    isFoundVar.bval <- true
      | _ -> ());
      DoChildren
    end
end







(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $              P  R  I  N  T  E  R                        $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)

(***************************************************)   
(* cg.PrintAll                                     *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgPrintToFile (fi:Cil.file) (outf: Unix.file_descr) (cg: callGraph) =
  begin

    let printEdge (edge:funcEdge) = 
      begin 
      end 
    in

    let printCaller (fcname:string) (node:funcNode) : unit  =
      begin
	
	if (node.nJournal.related && 
	    (node.nJournal.directCheckAbort 
	   || node.nJournal.exposeCheckAbort
	   || node.nJournal.directSetAbort
	   || node.nJournal.callSetAbort
	   || node.nJournal.confirmedCheckAbort
	   || node.nJournal.goodNode
	   || node.nJournal.goodChildren
	    )) then
	  begin
	    (pc "  ");
	    if (node.nJournal.related) then (pc "Rt ") else (pc "   ");
	    if (node.nJournal.directCheckAbort) then (pc "Cd ") else (pc "   ");
	    if (node.nJournal.exposeCheckAbort) then (pc "Ce ") else (pc "   ");
	    if (node.nJournal.confirmedCheckAbort) then (pc "Cc ") else (pc "   ");
	    pc ("  ");
	    if (node.nJournal.directSetAbort)   then (pc "Sd ") else (pc "   ");
	    if (node.nJournal.callSetAbort)     then (pc "Sc ") else (pc "   ");
	    (pc "  ");
	    if (node.nJournal.goodNode)     then (pc "Gn ") else (pc "   ");
	    if (node.nJournal.goodChildren) then (pc "Gc ") else (pc "   ");
	    (pc "  ");
	    if (node.nJournal.badNode)      then (pc "Bn ") else (pc "   ");
	    (pc "    ");	    
	    (pc node.fcname);
	    (pc "\n");
	  end
	else ();
	
	(List.iter printEdge node.callees);
      end in

    
    (* not sorted *)
    (* (Hashtbl.iter printCaller cg); *)


    iterGlobals fi
      (
       fun g -> match g with
         GFun(fd,_) ->
           begin
             try
               let n1 = cgFindNode cg fd.svar.vname in
               (printCaller n1.fcname n1);
             with Not_found -> ()
           end
       | _ -> ()
      );

  end



(***************************************************)
(* cgColorNodeAndEdge                              *)
(***************************************************)
let cgColorNode (outf: Unix.file_descr) (cg: callGraph) =
  begin
    let colorFuncNode (fcname:string) (node:funcNode) : unit  =
      begin
        (* specific node *)
	
        if (node.nJournal.confirmedCheckAbort) then node.nColor <- "pink" else ();

        if (node.nJournal.directSetAbort || node.nJournal.callSetAbort) then node.nColor <- "yellow" else ();
	
        if (node.nJournal.badNode) then node.nColor <- "green" else ();
	
      end in
    (Hashtbl.iter colorFuncNode cg);
  end

    
let interestingJournalNode (node:funcNode) =
  if (node.nJournal.confirmedCheckAbort || node.nJournal.directSetAbort || 
  node.nJournal.callSetAbort || node.nJournal.badNode) then true else false
      
(************************************)
(* main.cg                          *)
(************************************)
let printEdge (outf: Unix.file_descr) (edge:funcEdge) =
  begin
    let str = "  " ^ edge.caller.fcname ^ " -> " ^ edge.callee.fcname
      ^ " [style=\"setlinewidth(1)\" arrowsize=1]; \n" in
    (mypf outf str)
  end

(************************************)
(* main.cg                          *)
(************************************)
let printNode (outf: Unix.file_descr) (node:funcNode) =
  begin
    let str = "  " ^ node.fcname ^
      " [ fillcolor=\"" ^ node.nColor ^ "\" style=filled fontsize=80" ^
      " label=\"" ^ node.fcname ^ "\" ]; \n" in
    (mypf outf str);

  end

    
(****************************)
(*                          *)
(****************************)
let cgPrintJournalStructure (outf: Unix.file_descr) (cg: callGraph) =
  begin
    let doEdge (edge:funcEdge) =
      begin
        (* if (edge.caller.nJournal.related && edge.callee.nJournal.related) then *)
        if (interestingJournalNode edge.callee) then 
          (printEdge outf edge);
      end
    in
    let doNode (fcname:string) (node:funcNode)  =
      begin
        (* if (node.nJournal.related) then *)
        if (interestingJournalNode node) then
	  begin
            (printNode outf node);
            (List.iter doEdge node.callees);
	  end
      end in
    (Hashtbl.iter doNode cg);
  end



    
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $              T  E  M  P  L  A  T  E                     $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvAnalyze (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor
      
  method vstmt (s:stmt) : stmt visitAction =
    begin
      DoChildren
    end
      
  method vvdec (v: varinfo) =
    begin
      DoChildren
    end
    

  method vinst (i:instr) : instr list visitAction =
    begin
      (P.fprint stdout 80 (P.dprintf "  (i) %a \n" dn_instr i)) ;
      DoChildren
    end
      
  method vattr (attr: attribute) = 
    begin
      DoChildren
    end

  method vattrparam (ap: attrparam) = 
    begin 
      DoChildren
    end

      
  method vlval (l: lval) = 
    begin
      DoChildren
    end
      
end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
let cgAnalyzeFuncNode (outf:Unix.file_descr) (fcname:string) (node:funcNode) : unit  =
  begin
    let _ = visitCilFunction (new fnvAnalyze outf node) node.fd 
    in ();
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let cgAnalyzeFile (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (cgAnalyzeFuncNode outf) cg);
  end

    
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $   M A R K    R E L A T D    N O D E S                   $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvMarkRelatedNode (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor

      (* a declaration of journal_t and handle_t *)
  method vvdec (v: varinfo) =
    begin
      (match unrollType v.vtype with
      | TPtr(TNamed(ti,_),_) ->
          begin
            if (ti.tname = "Journal_t") then
              begin
                node.nJournal.related <- true;
              end
            else ();
            if (ti.tname = "handle_t") then
              begin
		node.nJournal.related <- true;
              end
            else ();
          end
      | _ -> ());
      DoChildren
    end
end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
let funcMarkRelatedNode (outf:Unix.file_descr) (fcname:string) (node:funcNode) : unit  =
  begin
    let _ = visitCilFunction (new fnvMarkRelatedNode outf node) node.fd 
    in ();
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let cgMarkRelatedNode (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkRelatedNode outf) cg);
  end



(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $              D I R E C T    C H E C K                   $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(*******************************)
(* Find JFS Abort              *)
(*******************************)
class expvFindJAbort (isFoundJAbort: boolarg) = object
  inherit nopCilVisitor
  method vexpr (e:exp) =
    begin
      (match e with
      | Const(CInt64(x,IULong,_)) ->
          begin
            let value = (Int64.to_int x) in
            if (value = 2) then
              isFoundJAbort.bval <- true
            else ()
          end
      | _ -> ());
      DoChildren
    end
end



(*******************************)
(* Find JFS Flags              *)
(*******************************)
class expvFindJFlags (isFoundJFlags: boolarg) = object
  inherit nopCilVisitor
  method vexpr (e:exp) =
    begin
      (match e with
      | Lval(lval) ->
          begin
            (match lval with
            | (Mem(Lval(Var(lvi),_)),Field(fi,_)) ->
                begin
                  (match unrollType lvi.vtype with
                  | TPtr(TComp(ci,_),_) -> ();

                  | TPtr(TNamed(ti,_),_) ->
                      begin
                        let structname = ti.tname
                        and fieldname = fi.fname in
			if (structname = "Journal_t" && fieldname = "j_flags") then
			  begin
			    isFoundJFlags.bval <- true;
			  end;
                      end
                  | _ -> ());
                end
            | _ -> ());
          end
      | _ -> ());
      DoChildren
    end
end
    


(**********************************************************)
(* Given an varinfo, try to find if it is being returned  *)
(**********************************************************)
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



(******************************************************************)
(* Given an if-block, try to find all assignment on left-values   *)
(* Then, do see if the left-values is retured                     *)
(* If so, this node exposes abort check                           *)
(******************************************************************)
class blockvDirectCheck (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor
      
  method vinst (i:instr) : instr list visitAction =
    begin
      (match i with 
      | Set((Var(lvi),_),_,_) ->
	  begin
	    let isVarInRet = { bval = false } in
            let _ = visitCilFunction (new fnvFindVarInReturnStmt lvi isVarInRet) node.fd in
	    if (isVarInRet.bval) then
	      begin
		node.nJournal.exposeCheckAbort <- true;
	      end
	    else ()
	  end
      | _ -> ());	      
      DoChildren
    end
end
    


(**************************************************************************************)   
(* This function visitor asks:                                                        *)
(* How does a structural error is being checked??                                     *)
(* Here are some instances:                                                           *)
(*   1. return ((int __attribute__((__always_inline__))  )(journal->j_flags & 2UL));  *)
(*      in is_journal_aborted                                                         *)
(*   2. if (journal->j_flags & 2 UL)                                                  *)
(*      in journal_err_no                                                             *)
(**************************************************************************************)
class fnvMarkDirectCheck (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor

  method vstmt (s:stmt) : stmt visitAction =
    begin
      (**************************************************)
      (* CASE 1: return ...                             *)
      (* we just try a short cut here:                  *)
      (* In return expr, try to find the occurence of:  *)
      (*     journal_t->j_flags                         *)
      (*     2UL                                        *)
      (* If found just mark it is marked as:            *)
      (*     direct abort check                         *)
      (*     expose abort check                         *)
      (**************************************************)
      (match s.skind with
      | Return(Some(exp),_) ->
	  begin
            let isFoundJAbort = { bval = false } 
	    and isFoundJFlags = { bval = false } in
            let _ = visitCilExpr (new expvFindJAbort isFoundJAbort) exp 
            and _ = visitCilExpr (new expvFindJFlags isFoundJFlags) exp in
            if (isFoundJAbort.bval && isFoundJFlags.bval) then
              begin
		(* if found, then this is a direct check abort *)
		(* and also expose check abort  *)
		node.nJournal.directCheckAbort <- true;
		node.nJournal.exposeCheckAbort <- true;
              end
            else ()
          end
      | _ -> ());
      
      (**************************************************)
      (* CASE 2: if ( flags .. expr )                   *)
      (* we just try a short cut here:                  *)
      (* In   if   expr, try to find the occurence of:  *)
      (*     journal_t->j_flags                         *)
      (*     2UL                                        *)
      (* If found just mark it is marked as:            *)
      (*     direct abort check                         *)
      (* Now need to now if it exposes abort check,     *)
      (* Hence, need to analyze if block and find this  *)
      (* pattern:                                       *)
      (*     if-body {                                  *)
      (*         lval = ...;                            *)
      (*     }                                          *)
      (*     return lval;                               *)
      (* If found, marked as expose abort check         *)
      (*     expose abort check                         *)
      (**************************************************)
      (match s.skind with
      | If(exp,tblk,_,_) ->
	  begin
            let isFoundJAbort = { bval = false } 
	    and isFoundJFlags = { bval = false } in
            let _ = visitCilExpr (new expvFindJAbort isFoundJAbort) exp 
            and _ = visitCilExpr (new expvFindJFlags isFoundJFlags) exp in
            if (isFoundJAbort.bval && isFoundJFlags.bval) then
	      begin
		(* if found, then this is a direct check abort *)
		node.nJournal.directCheckAbort <- true;

                (* now check if this is also expose check abort *)
		let _ = visitCilBlock (new blockvDirectCheck outf node) tblk in ();

	      end
	    else () 
	  end
      | _ -> ());
      DoChildren
    end
end

(*********************************)
(*                               *)
(*********************************)
let funcMarkDirectCheck (outf: Unix.file_descr) (cg: callGraph) (fcname:string) (node:funcNode) =
  begin
    let _ = visitCilFunction (new fnvMarkDirectCheck outf node) node.fd 
    in ();	
  end 
    
    
(***************************************************)   
(*                                                 *)
(***************************************************)
let cgMarkDirectCheck (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkDirectCheck outf cg) cg);
  end

    

(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $              E X P O S E     C H E C K                  $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


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
    (rvi:varinfo) (vilist: varinfo list) (isReturned:boolarg) = object
  inherit nopCilVisitor
      (* first it visits all instructions, and check if it has rvi in the *)
      (* right hand expression *)
  method vinst (i:instr) : instr list visitAction =
    begin

      (* check if a = b = c = a *)
      let checkLviInList (isLviInList:boolarg) (tmp1:varinfo) (tmp2:varinfo) =
        begin
          if (tmp1 = tmp2) then isLviInList.bval <- true else ()
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
                      if (not isLviInList.bval) then
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
                              let _ = visitCilFunction (new fnvCheckPropVarReturned outf node
                                                          lvi (lvi :: vilist) tmpIsReturned)
                                  node.fd in
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



(*********************************)
(*                               *)
(*********************************)
let rec edgeMarkExposeCheck (outf:Unix.file_descr) (edge:funcEdge) =
  begin
    (* if the caller already expose error, then no need to proceed *)
    (* but nevertheless need to mark this node appropriately       *)
    if (edge.caller.nJournal.exposeCheckAbort) then
      begin

      end
	(* Else, check if the caller also expose *)
    else 
      begin
	(match edge.instr with
	  (* This is a format for:  err = func() *)
	| Call(Some(Var(lvi),_),Lval(flval),_,_) ->
            begin
	      
	      (* now check if the lvi is also expose in this function *)
	      (* First, check if it is returned directly *)
              let isVarInRet = { bval = false } in
              let _ = visitCilFunction (new fnvFindVarInReturnStmt lvi isVarInRet)
		  edge.caller.fd in
	      
	      (* Second, check dataflow analysis of the form: *)
	      (* a = f1();  b = a; c = b; return c; *)
	      let isReturned = { bval = false } in
              let _ = visitCilFunction (new fnvCheckPropVarReturned outf edge.caller
					  lvi [] isReturned) edge.caller.fd in 

	      (* it is exposed! Mark this function as expose *)
              if (isVarInRet.bval || isReturned.bval) then
                begin
                  edge.caller.nJournal.exposeCheckAbort <- true;
                  (funcMarkExposeCheck outf edge.caller.fcname edge.caller);
                end;
            end
	| _ -> ());
      end
  end
    

(*********************************)
(*                               *)
(*********************************)
and funcMarkExposeCheck (outf: Unix.file_descr) (fcname:string) (node:funcNode) =
  begin
    if (node.nJournal.related && node.nJournal.exposeCheckAbort) then 
      begin
	(List.iter (edgeMarkExposeCheck outf) node.callers)
      end
    else ();
  end 
    

(***********************************************************************)   
(* An expose function:                                                 *)
(*   1. It calls function that expose the check                        *)
(*   2. The return value is saved and then returned                    *)
(*   3. Then this function is also an expose                           *)
(*                                                                     *)
(* Algorithm:                                                          *)
(*   1. Start with an expose function                                  *)
(*   2. Recursively call the paresnts                                  *)
(*                                                                     *)
(***********************************************************************)   
and cgMarkExposeCheck (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkExposeCheck outf) cg);
  end



(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $       M A R K     D I R E C T     S E T                 $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(*************************************************************)   
(* Mark direct set:                                          *)
(* The rule is then find all occurences of:                  *)
(*     lval = expr                                           *)
(* where lval is journal->j_flags, and expr contains 2UL     *)
(*************************************************************)
class fnvMarkDirectSet (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor

(* 200215    journal->j_flags = 2UL; *)
(* 202380    journal->j_flags = journal->j_flags | 2UL; *)

  method vinst (i:instr) : instr list visitAction =
    begin
      let isFoundJAbort = { bval = false } in
      
      (match i with
      | Set((Mem(Lval(Var(lvi),_)),Field(fi,_)),exp,_) ->
          begin
            (match unrollType lvi.vtype with
            | TPtr(TComp(ci,_),_) -> ();

            | TPtr(TNamed(ti,_),_) ->
                begin
                  let structname = ti.tname
                  and fieldname = fi.fname in
                  if (structname = "Journal_t" && fieldname = "j_flags") then
                    begin
		      let _ = visitCilExpr (new expvFindJAbort isFoundJAbort) exp in
		      if (isFoundJAbort.bval) then
			begin
			  node.nJournal.directSetAbort <- true;			  
			end;
		    end;
                end
            | _ -> ());
          end
      | _ -> ());
      
      DoChildren
    end
end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
let funcMarkDirectSet (outf:Unix.file_descr) (fcname:string) (node:funcNode) : unit  =
  begin
    let _ = visitCilFunction (new fnvMarkDirectSet outf node) node.fd 
    in ();
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let cgMarkDirectSet (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkDirectSet outf) cg);
  end


(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $       M A R K     C A L L    S E T                      $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)



(*********************************)
(*                               *)
(*********************************)
let rec edgeMarkCallSet (outf:Unix.file_descr) (edge:funcEdge) =
  begin
    (* if the caller already mark as, call set, the no need to proceed *)
    if (edge.caller.nJournal.callSetAbort) then
      begin

      end
        (* Else, mark and propagate (if caller is related!!!) *)
    else
      begin
	if (edge.caller.nJournal.related) then
	  begin
            edge.caller.nJournal.callSetAbort <- true;
            (funcMarkCallSet outf edge.caller.fcname edge.caller);
	  end;
      end;
  end


(*********************************)
(*                               *)
(*********************************)
and funcMarkCallSet (outf: Unix.file_descr) (fcname:string) (node:funcNode) =
  begin
    if (node.nJournal.related && 
	(node.nJournal.directSetAbort || node.nJournal.callSetAbort)) then
      begin
        (List.iter (edgeMarkCallSet outf) node.callers)
      end
    else ();
  end

(***********************************************************************)
(* An call set function:                                               *)
(*   1. It calls function that call-set function                       *)
(*   2. Recursive                                                      *)
(***********************************************************************)
and cgMarkCallSet (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkCallSet outf) cg);
  end




(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $       M A R K     G O O D     N O D E                   $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(***********************************************************************)
(* Definition of a good node:                                          *)
(*   A good node is of course:                                         *)
(*   1. It must be either direct set or call set                       *)
(*   2. It also must be a direct check or call expose check            *)
(*        (confirmedCheck)                                             *)
(*                                                                     *)
(* Algorithm:                                                          *)
(*   1. First mark all functions that:                                 *)
(*        a. direct check, expose check                                *)
(*        b. AND calls (direct check / expose check)                   *)
(*      As confirmed check                                             *)
(***********************************************************************)

(*********************************)
(*                               *)
(*********************************)
let rec funcMarkGoodNode (outf: Unix.file_descr) (fcname:string) (node:funcNode) =
  begin
    
    if (node.nJournal.related && 
	(node.nJournal.directSetAbort || node.nJournal.callSetAbort) &&
	(node.nJournal.confirmedCheckAbort)) then
      begin
	node.nJournal.goodNode <- true;
      end;
  end


(*********************************)
(*                               *)
(*********************************)
and edgeMarkConfirmedCheckAbort (outf:Unix.file_descr) (edge:funcEdge) =
  begin
    (* if the caller already mark as, call set, the no need to proceed *)
    if (edge.caller.nJournal.related) then
      begin
	edge.caller.nJournal.confirmedCheckAbort <- true;
      end;
  end

(*********************************)
(*                               *)
(*********************************)
and funcMarkConfirmedCheckAbort (outf: Unix.file_descr) (fcname:string) (node:funcNode) =
  begin

    if (node.nJournal.related && 
	(node.nJournal.directCheckAbort || node.nJournal.exposeCheckAbort)) then
      begin
	(* first mark confirmed check for direct check and expose check *)
	node.nJournal.confirmedCheckAbort <- true;

	(* now iter the callers as confirmed check *)
        (List.iter (edgeMarkConfirmedCheckAbort outf) node.callers)
      end;
  end

(***********************************************************************)
(* An call set function:                                               *)
(*   1. It calls function that call-set function                       *)
(*   2. Recursive                                                      *)
(***********************************************************************)
and cgMarkGoodNode (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkConfirmedCheckAbort outf) cg);
    (Hashtbl.iter (funcMarkGoodNode outf) cg);
  end





(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $       M A R K     G O O D     C H I L D R E N           $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(***********************************************************************)
(* Definition of a good children:                                      *)
(*   The descendants of good node                                      *)
(*                                                                     *)
(* Algorithm:                                                          *)
(*   1. First starts from good node and propagate down                 *)
(***********************************************************************)
let rec edgeMarkGoodChildren (outf:Unix.file_descr) (edge:funcEdge) =
  begin
    if (edge.callee.nJournal.goodChildren) then
      begin
	(* do nothing if callee already a good children *)
      end
    else
      begin
	if (edge.callee.nJournal.related) then
	  begin
	    edge.callee.nJournal.goodChildren <- true;
            (funcMarkGoodChildren outf edge.callee.fcname edge.callee);
	  end;
      end;
  end

(*********************************)
(*                               *)
(*********************************)
and funcMarkGoodChildren (outf: Unix.file_descr) (fcname:string) (node:funcNode) =
  begin
    if (node.nJournal.related && 
	(node.nJournal.goodNode || node.nJournal.goodChildren)) then
      begin
	(* calleess !! propagate down *)
        (List.iter (edgeMarkGoodChildren outf) node.callees);
      end;
  end

(***********************************************************************)
(* An call set function:                                               *)
(*   1. It calls function that call-set function                       *)
(*   2. Recursive                                                      *)
(***********************************************************************)
and cgMarkGoodChildren (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkGoodChildren outf) cg);
  end
    




(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $       M A R K     B A D     N O D E                     $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


(***********************************************************************)
(* Definition of a bad node:                                           *)
(*   1. It is either call set or direct set (Sc/Sd)  AND               *)
(*   2. Not confirmed check (!c)    AND                                *)
(*   3. Not a good children (!Gc)                                      *)
(***********************************************************************)

(*********************************)
(*                               *)
(*********************************)
let rec funcMarkBadNode (outf: Unix.file_descr) (fcname:string) (node:funcNode) =
  begin
    
    if (node.nJournal.related && 
	(node.nJournal.directSetAbort || node.nJournal.callSetAbort) &&
	(not node.nJournal.confirmedCheckAbort) &&
	(not node.nJournal.goodChildren)) then
      begin
	node.nJournal.badNode <- true;
      end;
  end


(***********************************************************************)
(* An call set function:                                               *)
(*   1. It calls function that call-set function                       *)
(*   2. Recursive                                                      *)
(***********************************************************************)
and cgMarkBadNode (outf: Unix.file_descr) (cg: callGraph) =
  begin
    (Hashtbl.iter (funcMarkBadNode outf) cg);
  end





    
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $              M    A   I   N                             $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $                                                         $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)
(* $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ *)


    
(* ----------------------- Main --------------------- *)
    
(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainErrorStructure (fi : Cil.file) =
  begin
    let cg = Hashtbl.create 511 in
    let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] in 
    let outf = Unix.openfile "CILOUTPUT" mode 0o644 in
    let dotfile = Unix.openfile "CILOUTPUT.dot1.c" mode 0o644 in
    let dirnamefile = open_in "MYDIRNAME" in
    let dirname = input_line dirnamefile in
    (close_in dirnamefile);
    
    (cgBuildFirst dirname fi cg);
    (cgConstructEdge fi cg);
    
    (***************************************************** *)
    (cgMarkRelatedNode outf cg);
    (cgMarkDirectCheck outf cg);
    (cgMarkExposeCheck outf cg); 
    (cgMarkDirectSet outf cg); 
    (cgMarkCallSet outf cg); 
    (cgMarkGoodNode outf cg); 
    (cgMarkGoodChildren outf cg); 
    (cgMarkBadNode outf cg); 

    (* (cgOverwriteBadNode outf cg); *)

    (cgPrintToFile fi outf cg);

    (cgColorNode dotfile cg);
    (cgPrintJournalStructure dotfile cg);
    
    
    (***************************************************** *)

    (Unix.close dotfile);

    
  end
    


(* ------------------------- SETTING ---------------------------------- *)
    



(* ********************************************************** *)
let doErrorStructure = ref false
    
       
(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "errorStructure";
    fd_enabled = doErrorStructure;
    fd_description = "Error Structure";
    fd_extraopt = [];
    fd_doit = (function (f: file) ->
      mainErrorStructure f) ;
    fd_post_check = false;
  }
    
    
