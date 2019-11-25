open Cil
open Printf
open List
open Errorheader
module P = Pretty

(* ------------------------- Build CG  -------------------------- *)
let mypf (fd:Unix.file_descr) (str:string) =
  begin
    let _ = (Unix.write fd str 0 (String.length str)) in ()
  end

(***************************************************)
(* Converter: exp/instr/lval/stmt/type to String   *)
(***************************************************)
let expToStr (e:exp) = (P.sprint 80 (P.dprintf "%a" dn_exp e))
let instrToStr (i:instr) = (P.sprint 80 (P.dprintf "%a" dn_instr i))
let lvalToStr (l:lval) = (P.sprint 80 (P.dprintf "%a" dn_lval l))
let stmtToStr (s:stmt) = (P.sprint 80 (P.dprintf "%a" dn_stmt  s))
let blockToStr (b:block) = (P.sprint 80 (P.dprintf "%a" d_block  b))
let typeToStr (t:typ) = (P.sprint 80 (P.dprintf "%a" dn_type t))
let itoa (i:int) = (P.sprint 80 (P.dprintf "%d" i))
let itoa3 (i:int) = (P.sprint 80 (P.dprintf "%3d" i))
let strToStr20 (s:string) = (P.sprint 80 (P.dprintf "%-20s" s))
let strToStr30 (s:string) = (P.sprint 80 (P.dprintf "%-30s" s))
let boolToStr (b:bool) =
  if (b) then (P.sprint 80 (P.dprintf "true"))
  else (P.sprint 80 (P.dprintf "false"))      



(* ----------------------- Print CG Functions --------------------- *)


(***************************************************)   
(* cg.PrintAll                                     *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgPrintAll (out: out_channel) (cg: callGraph) =
  begin
    (fprintf stderr "STEP: Printing call graph ... \n");
    let printCaller (fcname:string) (node:funcNode) : unit  =
      begin
	(fprintf out "-----------------------------\n");
	let printEdge (edge:funcEdge) = 
	  (fprintf out "%s (p:%B) --> %s (d:%B) (p:%B) \n" 
	     fcname 
	     edge.caller.nRetvalEIO.propagate
	     edge.callee.fcname 
	     edge.callee.nRetvalEIO.direct
	     edge.callee.nRetvalEIO.propagate)
	in
	if ((List.length node.callees) == 0) then
	  (fprintf out "%s (p:%B) --> \n"
	     node.fcname node.nRetvalEIO.propagate)
	else
    	  (List.iter printEdge node.callees);
      end in
    (Hashtbl.iter printCaller cg)
  end


(***************************************************)   
(* cg.PrintAll                                     *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgPrintAllToFile (outf: Unix.file_descr) (cg: callGraph) =
  begin

    (mypf outf "*****************************************\n");
    (mypf outf " CALLGRAPH: \n");
    (mypf outf "*****************************************\n");
    
    let printCaller (fcname:string) (node:funcNode) : unit  =
      begin

	(mypf outf "\n>> ");
	(mypf outf fcname);
	(mypf outf " : \n");
	
	let printEdge (edge:funcEdge) = 
	  begin 
	    (* caller *)
	    (mypf outf "  ");
	    (mypf outf fcname);
	    (mypf outf " (r:");
	    if edge.caller.nRetvalEIO.direct    then (mypf outf "d");
	    if edge.caller.nRetvalEIO.propagate then (mypf outf "p");
	    if edge.caller.nRetvalEIO.involve   then (mypf outf "i");
	    (mypf outf ")(a:");	    
	    if edge.caller.nArgEIO.direct    then (mypf outf "d");
	    if edge.caller.nArgEIO.propagate then (mypf outf "p");
	    if edge.caller.nArgEIO.involve   then (mypf outf "i");
	    (mypf outf ")");	    
	    (mypf outf ("[" ^ edge.caller.dirname ^ "]"));
	    
	    (mypf outf " -- (r:");
	    if edge.eRetvalEIO.edgePropagate   then (mypf outf "ep,");
	    if edge.eRetvalEIO.callerPropagate then (mypf outf "cp,");
	    if edge.eRetvalEIO.endpoint        then (mypf outf "e,");
	    if edge.eRetvalEIO.endpointSaved   then (mypf outf "s,");
	    if edge.eRetvalEIO.endpointChecked then (mypf outf "c,");
	    (mypf outf ")(a:");	    
	    if edge.eArgEIO.edgePropagate   then (mypf outf "ep,");
	    if edge.eArgEIO.callerPropagate then (mypf outf "cp,");
	    if edge.eArgEIO.endpointChangeMechanism then (mypf outf "cm,");
	    if edge.eArgEIO.endpoint        then (mypf outf "e,");
	    if edge.eArgEIO.endpointSaved   then (mypf outf "s,");
	    if edge.eArgEIO.endpointChecked then (mypf outf "c,");

	    (mypf outf ") --> ");
	    
	    (* callee *)
	    (mypf outf edge.callee.fcname);
	    (mypf outf " (r:");
	    if edge.callee.nRetvalEIO.direct    then (mypf outf "d");
	    if edge.callee.nRetvalEIO.propagate then (mypf outf "p");
	    if edge.callee.nRetvalEIO.involve   then (mypf outf "i");
	    (mypf outf ")(a:");	    
	    if edge.callee.nArgEIO.direct    then (mypf outf "d");
	    if edge.callee.nArgEIO.propagate then (mypf outf "p");
	    if edge.callee.nArgEIO.involve   then (mypf outf "i");
	    (mypf outf ")");	    
	    (mypf outf ("[" ^ edge.callee.dirname ^ "]"));

	    (mypf outf "\n");
	  end  
	in

	(* does not call anyone *)
	if ((List.length node.callees) == 0) then
	  begin
	    (mypf outf " ");
	    (mypf outf node.fcname);
	    (mypf outf " ");
	    if node.nRetvalEIO.direct 
	    then (mypf outf "(d:1)")
	    else (mypf outf "(d:0)");
	    if node.nRetvalEIO.propagate 
	    then (mypf outf "(p:1)")
	    else (mypf outf "(p:0)");
	    (mypf outf " --> () \n");
	  end
	else
    	  (List.iter printEdge node.callees);
      end in
    (Hashtbl.iter printCaller cg);
    (mypf outf "*****************************************\n");
  end



(***************************************************)   
(* cgColorNodeAndEdge                              *)
(***************************************************)
let cgColorNodeAndEdge (outf: Unix.file_descr) (cg: callGraph) =
  begin

    (* ********************* color edge ***************************** *)
    let colorFuncEdge (edge:funcEdge) = 
      begin

	(**********************************************)
	(*  Saved Propagate Checked                   *)
	(*  ----------------------------------------- *)
	(*    N      .         .      red/brown       *)
	(*    Y      Y         .      green/lightblue *)
	(*    Y      N         N      red/brown       *)
	(*    Y      N         Y      pink            *)
	(*  Direct                    orange/yellow   *)
	(**********************************************)	    
	
	(* VIOLATION !! *)
	(* v1. caller not save return value *)
	if ((edge.callee.nRetvalEIO.propagate || edge.callee.nRetvalEIO.direct) 
	      && 
	    ((not edge.eRetvalEIO.edgePropagate) && (not edge.eArgEIO.edgePropagate))) then
	  begin
	    edge.caller.nColor <- "red";
	    edge.eColor <- "red";
	  end;
	
	(* v2. callee propagate *err, caller not *)
	if ((edge.callee.nArgEIO.propagate || edge.callee.nArgEIO.direct)
	      && 
	    ((not edge.eRetvalEIO.edgePropagate) && (not edge.eArgEIO.edgePropagate))) then
	  begin
	    edge.caller.nColor <- "red";
	    edge.eColor <- "red";
	  end;

	if (edge.eArgEIO.endpoint && (not edge.eArgEIO.endpointChecked) && 
	    (not edge.eArgEIO.endpointChangeMechanism))
	    (* && (not edge.eRetvalEIO.edgePropagate)) *) (* because of evolve *)
	then 
	  begin
	    if (edge.caller.nColor="") then 
	      edge.caller.nColor <- "brown";
	    if (edge.eColor="") then 
	      edge.eColor <- "brown";
	  end;
	
	if (edge.eRetvalEIO.endpoint && (not edge.eRetvalEIO.endpointChecked)) then	
	  begin
	    if (edge.caller.nColor="") then 
	      edge.caller.nColor <- "pink";
	    if (edge.eColor="") then 
	      edge.eColor <- "pink";
	  end;


	
	(* PROPAGATION !! *)
	if (edge.eRetvalEIO.edgePropagate) then 
	  begin
	    if (edge.caller.nColor="") then 
	      edge.caller.nColor <- "green";
	    if (edge.eColor="") then 
	      edge.eColor <- "green";
	  end;
	
	if (edge.eArgEIO.edgePropagate) then
	  begin
	    if (edge.caller.nColor="") then 
	      edge.caller.nColor <- "lightblue";
	    if (edge.eColor="") then 
	      edge.eColor <- "lightblue";
	  end;
	
      end in
    
    
    (* ********************* color node ***************************** *)
    let colorFuncNode (fcname:string) (node:funcNode) : unit  =
      begin

	if (node.nArgEIO.direct && node.nColor="") then
	  node.nColor <- "orange" else ();
	
	if (node.nRetvalEIO.direct && node.nColor="") then
	  node.nColor <- "yellow" else ();

	
	(* iterate edges *)
	(List.iter colorFuncEdge node.callees);
	
      end in
    (* end of printNode *)


    (Hashtbl.iter colorFuncNode cg);
    
  end


(***************************************************)   
(*                                                 *)
(***************************************************)
let cgClassifyNodeAndEdge (outf: Unix.file_descr) (cg: callGraph) =
  begin

    let classifyFuncEdge (edge:funcEdge) = 
      begin

	if ((String.compare edge.caller.dirname edge.callee.dirname) == 0) then
	  begin
	    edge.eClass1 <- "CODE-SAME-DIR";
	    if ((String.compare edge.caller.floc.file edge.callee.floc.file) == 0) then
	      edge.eClass2 <- "CODE-SAME-FILE"
	    else
	      edge.eClass2 <- "CODE-DIFF-FILE";
	  end
	else
	  begin
	    edge.eClass1 <- "CODE-DIFF-DIR";
	    edge.eClass2 <- "N.A.";
	  end;
	
	
	(* abc *)
	if (edge.eColor = "red" || edge.eColor = "brown") then
	  begin
	    if (edge.callee.nRetvalEIO.direct || edge.callee.nArgEIO.direct) then
	      edge.eClass3 <- "BAD-DIRECT"
	    else
	      edge.eClass3 <- "BAD-IMPLICIT"
	  end
	else
	  begin
	    if (edge.callee.nRetvalEIO.direct || edge.callee.nArgEIO.direct) then
	      edge.eClass3 <- "GOOD-DIRECT"
	    else	    
	      edge.eClass3 <- "GOOD-IMPLICIT"
	  end
	    
      end in
    
    let classifyFuncNode (fcname:string) (node:funcNode) : unit  =
      begin
	(List.iter classifyFuncEdge node.callees);
      end in
    
    (Hashtbl.iter classifyFuncNode cg);
    
  end
    

(***************************************************)   
(* cg.PrintAllToDot1                               *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgPrintAllToDot1 (outf: Unix.file_descr) (cg: callGraph) (printnormal:bool) =
  begin

    (* print black edges ?? *)
    let printNormalCall = printnormal in

    (* edge *)
    let printEdgeWithClassify (edge:funcEdge) =
      begin
	if (edge.caller.nColor = "") then edge.caller.nColor <- "white";
	if (edge.eColor = "") then edge.eColor <- "black";

	(* haryadi -- new for fast *)
	if (edge.eColor = "red") then
	  begin
	    let str = "  " ^ edge.caller.fcname ^ " -> " ^ edge.callee.fcname 
	      ^ " [style=\"setlinewidth(5)\" arrowsize=1 color=\"" ^ edge.eColor ^ "\"]; " 
	      ^ "/* " ^ edge.eLoc.file ^ " " ^ (itoa edge.eLoc.line) ^ " */ "
	      ^ "\"" ^ edge.eClass1 ^ "\" "
	      ^ "\"" ^ edge.eClass2 ^ "\" "
	      ^ "\"" ^ edge.eClass3 ^ "\" "
	      ^ "\n"
	    in
	    (mypf outf str);	
	  end
	else
	  begin
	    let str = "  " ^ edge.caller.fcname ^ " -> " ^ edge.callee.fcname 
	      ^ " [style=\"setlinewidth(1)\" arrowsize=1 color=\"" ^ edge.eColor ^ "\"]; " 
	      (* ^ "/* " ^ edge.caller.floc.file ^ " */ "   *)
	      ^ "/* " ^ edge.eLoc.file ^ " " ^ (itoa edge.eLoc.line) ^ " */ "
	      ^ "\"" ^ edge.eClass1 ^ "\" "
	      ^ "\"" ^ edge.eClass2 ^ "\" "
	      ^ "\"" ^ edge.eClass3 ^ "\" "
	      ^ "\n"
	    in
	    (mypf outf str);		  
	  end
      end 
	
	
	(* edge *)
    and printEdge (edge:funcEdge) =
      begin
	if (edge.caller.nColor = "") then edge.caller.nColor <- "white";
	if (edge.eColor = "") then edge.eColor <- "black";

	(* haryadi -- new for fast *)
	if (edge.eColor = "red") then
	  begin
	    let str = "  " ^ edge.caller.fcname ^ " -> " ^ edge.callee.fcname 
	      ^ " [style=\"setlinewidth(5)\" arrowsize=1 color=\"" ^ edge.eColor ^ "\"]; " 
	      ^ "/* " ^ edge.eLoc.file ^ " " ^ (itoa edge.eLoc.line) ^ " */ "
	      ^ "\n"
	    in
	    (mypf outf str);	
	  end
	else
	  begin
	    let str = "  " ^ edge.caller.fcname ^ " -> " ^ edge.callee.fcname 
	      ^ " [style=\"setlinewidth(1)\" arrowsize=1 color=\"" ^ edge.eColor ^ "\"]; " 
	      (* ^ "/* " ^ edge.caller.floc.file ^ " */ "   *)
	      ^ "/* " ^ edge.eLoc.file ^ " " ^ (itoa edge.eLoc.line) ^ " */ "
	      ^ "\n"
	    in
	    (mypf outf str);		  
	  end
      end 
	
        (* node *)
    and printNode (node:funcNode) =
      begin
	if (node.nColor = "") then node.nColor <- "white";
	let str = "  " ^ node.fcname 
	  ^ " [ fillcolor=\"" ^ node.nColor ^ "\" style=filled fontsize=80"
	  ^ " label=\"" ^ node.fcname ^ "\" ]; " 
          ^ "/* " ^ node.floc.file ^ " */"
	  ^ "\n" 
	in
	(mypf outf str);
      end
    in	

    let printBoth (edge:funcEdge) =
      begin
	(printEdge edge);
	(printNode edge.caller);
      end
    in

    (* ********************* print edge ***************************** *)
    let printFuncEdge (edge:funcEdge) = 
      begin
	if ((edge.caller.visited.up && edge.caller.visited.down) &&
	    (edge.callee.visited.up && edge.callee.visited.down)) then
	  if ((edge.caller.nRetvalEIO.involve && edge.callee.nRetvalEIO.involve) 
	    || (edge.caller.nArgEIO.involve && edge.callee.nArgEIO.involve)
	    || printNormalCall) then
	    (printBoth edge);
      end in
    
    
    (* ********************* print node ***************************** *)
    let printFuncNode (fcname:string) (node:funcNode) : unit  =
      begin

	if (node.visited.up && node.visited.down) then
	  if (node.nRetvalEIO.involve || node.nArgEIO.involve || printNormalCall) then
	    (printNode node);
	
	(* iterate edges *)
	(List.iter printFuncEdge node.callees);
	
      end in
    (* end of printNode *)

    (cgColorNodeAndEdge outf cg);


    (cgClassifyNodeAndEdge outf cg);
    
    
    (Hashtbl.iter printFuncNode cg);
    
  end
    



(* ----------------------- Specific Path from function X --------------------- *)

(***************************************************)   
(* main.cg                                         *)
(***************************************************)
let cgPrintSpecificPathUpAndDown (outf: Unix.file_descr) (cg: callGraph) =
  begin

    let rec printEdge (edge:funcEdge) = 
      begin
	if ((edge.caller.visited.up && edge.caller.visited.down) &&
	    (edge.callee.visited.up && edge.callee.visited.down))
	then
	  let str = "  " ^ edge.caller.fcname ^ 
	    " -> " ^ edge.callee.fcname ^ 
	    " [style=\"setlinewidth(1)\" arrowsize=1]; \n" 
	  in
	  (mypf outf str);
	else ();
      end
	
    and printSpecificNode (fcname:string) (node:funcNode) : unit  =
      begin
	if (fcname = "ll_rw_block" 
	  || fcname = "sys_fsync" 
	  || fcname = "ext2_writepage" 
	  || fcname = "ext2_sync_file" 
	  || fcname = "generic_make_request") then
	  let str = "  " ^ fcname ^ 
	    " [ fillcolor=yellow style=filled fontsize=80" ^
	    " label=\"" ^ node.fcname ^ "\" ]; \n" in
	  (mypf outf str)
	else
	  if (node.visited.up && node.visited.down) then
	    begin
	      let str = "  " ^ fcname ^ 
		"  [ fillcolor=yellow style=filled fontsize=80" ^
		" label=\"" ^ node.fcname ^ "\" ]; \n" in
	      (mypf outf str)
	    end
	  else ();
	(* iterate edges *)
    	(List.iter printEdge node.callees);
	
      end 
	(* end of printCaller *)
	
	
    (* -------------------- down -------------------------- *)
	and traverseAndMarkCalleeEdgeDown (edge:funcEdge) = 
      begin
	if edge.callee.visited.down then () 
	else (traverseAndMarkFuncNodeDown edge.callee.fcname edge.callee);
      end
	
    and traverseAndMarkFuncNodeDown (fcname:string) (node:funcNode) : unit  =
      begin
	node.visited.down <- true;
	(List.iter traverseAndMarkCalleeEdgeDown node.callees); 
      end
	
    and findAndMarkFuncNodeDown (fcname:string) (node:funcNode) : unit  =
      begin
	(* specific start *)
	if (fcname = "sys_fsync") then
	  begin
	    node.visited.down <- true;
	    node.visited.up <- true;
	    (List.iter traverseAndMarkCalleeEdgeDown node.callees);  
	  end
	else ()
      end

	(* -------------------- up -------------------------- *)
    and traverseAndMarkCallerEdgeUp (edge:funcEdge) = 
      begin
	if edge.caller.visited.up then () 
	else (traverseAndMarkFuncNodeUp edge.caller.fcname edge.caller);
      end
	
    and traverseAndMarkFuncNodeUp (fcname:string) (node:funcNode) : unit  =
      begin
	node.visited.up <- true;
	(List.iter traverseAndMarkCallerEdgeUp node.callers); 
      end
	
    and findAndMarkFuncNodeUp (fcname:string) (node:funcNode) : unit  =
      begin
	(* specific stop *)
	if (fcname = "ll_rw_block" || fcname = "generic_make_request" 
      || fcname = "submit_bio" || fcname = "submit_bh") then
	  begin
	    node.visited.down <- true;
	    node.visited.up <- true;
	    (List.iter traverseAndMarkCallerEdgeUp node.callers);  
	  end
	else ()
      end
	
	(* -------------------- algo -------------------------- *)
    and clearVisitedFlag (fcname:string) (node:funcNode) : unit  =
      begin
	node.visited.any <- false;
	node.visited.up <- false;
	node.visited.down <- false;
      end
    in
    
    (* 1. traverse the callgraph *)
    (Hashtbl.iter clearVisitedFlag cg);
    (Hashtbl.iter findAndMarkFuncNodeDown cg);
    (Hashtbl.iter findAndMarkFuncNodeUp cg);
    
    (* (Hashtbl.iter printSpecificNode cg); *)
    
  end

