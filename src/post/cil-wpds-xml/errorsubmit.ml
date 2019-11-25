open Cil
open Printf
open List
module P = Pretty
open Errorheader
open Errorprint
open Errorbuildgraph



(***************************************************)   
(* cg.PrintAll                                     *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgPrintSubmitAnalysisToFile (outf: Unix.file_descr) (cg: callGraph) =
  begin
    
    let printCaller (fcname:string) (node:funcNode) : unit  =
      begin
	
	let printEdge (edge:funcEdge) = 
	  begin 
	  end 
	in
	
	(* if (node.nSubmit.submit && node.nSubmit.wait) then  *)
	(* if (node.nSubmit.wait && node.nSubmit.check) then*)
	if (node.nSubmit.wait) then
	  begin
	    (mypf outf node.fcname);
	    (mypf outf " (");
	    if (node.nSubmit.submit) then  (mypf outf "s,");
	    if (node.nSubmit.wait)   then  (mypf outf "w,");
	    if (node.nSubmit.check)  then  (mypf outf "c,");
	    (mypf outf ") \n");
	  end
	else ();
	    
	(List.iter printEdge node.callees);
      end in
    (Hashtbl.iter printCaller cg);
  end



(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvFindBufferEndIO (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor
   (* visit an instruction; we're only interested in calls *)
  method vinst (i:instr) : instr list visitAction =
    begin
      (* (P.fprint stderr 80 (P.dprintf "  (i) %a \n" dn_instr i)) ; *)
      (match i with 
      | Set(  (Mem(Lval(Var(lvi),_)),Field(fi,_))  ,  AddrOf(Var(rvi),_)   , _  ) ->
	  begin
	    (* grep 'bh' *)
	    (match unrollTypeDeep lvi.vtype with
	    | TPtr(TComp(ci,_),_) -> 
		begin
		  if ((ci.cname = "buffer_head") && (fi.fname = "b_end_io")) then
		    begin
		      (fprintf stderr "BH) %s %s->%s = %s \n"
			 ci.cname lvi.vname fi.fname rvi.vname);
		    end
		  else ();
		  if ((ci.cname = "bio") && (fi.fname = "bi_end_io")) then
		    begin
		      (fprintf stderr "BIO) %s %s->%s = %s \n"
			 ci.cname lvi.vname fi.fname rvi.vname);
		    end
		  else ();
		end
	    | _ -> ()
	    );
	  end
      | _ -> () (*i*)
      );
      DoChildren
    end
end

(***************************************************)   
(* cg.PrintAll                                     *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgFindBufferEndIO (outf: Unix.file_descr) (cg: callGraph) =
  begin
    let analyzeCaller (fcname:string) (node:funcNode) : unit  =
      begin
	let _ = visitCilFunction (new fnvFindBufferEndIO outf node) node.fd 
	in ();	
      end in
    (Hashtbl.iter analyzeCaller cg);
  end


(* <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>> *)
(* <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>> *)
(* <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>>     <<<<< >>>>> *)



(***************************************************)   
(* expr visitor: Find func name in Expr            *)
(***************************************************)
class expvFindFuncInExpr (isFoundFunc: boolarg) = object
  inherit nopCilVisitor
  method vexpr (e:exp) = 
    begin
      (match e with
      | _ -> ());
      DoChildren
    end
end



(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvMarkSubmitNode (outf:Unix.file_descr) (node:funcNode) = object
  inherit nopCilVisitor
      
      (* visit an instruction; we're only interested in calls *)
  method vinst (i:instr) : instr list visitAction =
    begin
      (* (P.fprint stderr 80 (P.dprintf "  (i) %a \n" dn_instr i)) ; *)
      (match i with 
      | Call(_,Lval(Var(vi),_),_,_) ->
	  begin
	    (* submit *)
	    if (vi.vname = "submit_bh" 
	      || vi.vname = "submit_bio" 
	      || vi.vname = "ll_rw_block") then
	      begin
		node.nSubmit.submit <- true;
	      end
	    else ();
	    
	    (* wait *)
	    if (vi.vname = "wait_on_buffer") then
	      begin
		node.nSubmit.wait <- true;		
	      end
	    else ();
	  end
      | _ -> () (*i*)
      );
      
      (match i with 
      | Call(Some(tmpval),Lval(Var(vi),_),_,_) ->
	  begin
	    (* check *)
	    if (vi.vname = "buffer_uptodate") then
	      begin
		node.nSubmit.check <- true;		
	      end
	    else ();
	  end
      | _ -> () (*i*)
      );
      DoChildren
    end
  method vstmt (s:stmt) : stmt visitAction =
    begin
      (* (P.fprint stderr 80 (P.dprintf "  (s) %a \n" dn_stmt  s));  *)
      DoChildren
    end

end


(***************************************************)   
(* cg.PrintAll                                     *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgMarkSubmitNode (outf: Unix.file_descr) (cg: callGraph) =
  begin
    let analyzeCaller (fcname:string) (node:funcNode) : unit  =
      begin
	(* (fprintf stderr "\n%s:\n" fcname);*)
	(* (fprintf stderr "---------------------\n");*)
	let _ = visitCilFunction (new fnvMarkSubmitNode outf node) node.fd 
	in ();	
      end in
    (Hashtbl.iter analyzeCaller cg);
  end



