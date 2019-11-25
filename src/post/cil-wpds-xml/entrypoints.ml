open Cil
open Printf
open List
open Errorprint
open ErrorStructure
open Errorbuildgraph
open Errorheader
open ConnectFunctionPointers

module P = Pretty

let found = ref false

(***************************************************)
(* Clear visited flag                              *)
(***************************************************)
let cgClearVisited (outf: Unix.file_descr) (fcname:string) (node:funcNode) : unit  =
  begin
    (*(mypf outf "\tClearing: ");
    (mypf outf node.fcname);
    (mypf outf "\n");*)
    node.isVisited <- false;
  end


(***************************************************)
(* Finds entry points for a given goal function    *)
(***************************************************)
let cgIsReachableFromEntryPoint (outf: Unix.file_descr) (cg: callGraph) (goal: string) =
  begin
    found := false;
    let rec printCaller (edge: funcEdge): unit =
      begin
	if (not edge.caller.isVisited) then
          if (not !found) then
	  begin
	    edge.caller.isVisited <- true;
	    if (List.length edge.caller.callers == 0) then
	      begin
                (* This function is reachable from an entry point *)
                found := true
	      end;
	    (List.iter printCaller edge.caller.callers)
	  end
      end in
    (List.iter printCaller (Hashtbl.find cg goal).callers);
  end


(***************************************************)
(* Finds entry points for a given goal function    *)
(***************************************************)
let cgPrintSelectedEntryPoints (outf: Unix.file_descr) (cg: callGraph) (goal: string) =
  begin
    (mypf outf ("caller: " ^ goal ^ "\n" ));
    let rec printCaller (edge: funcEdge): unit =
      begin
	if (not edge.caller.isVisited) then
	  begin
	    edge.caller.isVisited <- true;
	    if (List.length edge.caller.callers == 0) then
	      begin
		(mypf outf ("{*} "));
		(mypf outf ("caller: " ^ edge.caller.fcname ^ "\n"))
	      end;
	    (List.iter printCaller edge.caller.callers)
	  end
      end in
    (List.iter printCaller (Hashtbl.find cg goal).callers);	
  end


(***************************************************)
(* Finds coverage given an entry point             *)
(***************************************************)
let cgPrintCoverage (outf: Unix.file_descr) (cg: callGraph) (entry: string) =
  begin
    (mypf outf ("caller: " ^ entry ^ "\n" ));
    let rec printCallee (edge: funcEdge): unit =
      begin
	if (not edge.callee.isVisited) then
	  begin
	    edge.callee.isVisited <- true;
             (mypf outf ("callee: " ^ edge.callee.fcname ^ "\n"));
	    (List.iter printCallee edge.callee.callees)
	  end
      end in
    (List.iter printCallee (Hashtbl.find cg entry).callees);	
  end


(***************************************************)
(* Finds the set of entry points                   *)
(***************************************************)
let cgPrintEntryPointsToFile (outf: Unix.file_descr) (cg: callGraph) =
  begin

    let printEntryPoint (fcname:string) (node:funcNode) : unit  =
      begin
	if ((List.length node.callers) == 0) then
          begin
	    (mypf outf (fcname ^ "\n"))
          end
        else
           begin
              (* Make sure this is not part of a disconnected cycle *)
 	      (cgIsReachableFromEntryPoint outf cg fcname);
	      if (not !found) then
                 (mypf outf (fcname ^ "\n"));
           end;
           (* Clear visited flags *)
	   (Hashtbl.iter (cgClearVisited outf) cg)
      end in
    (Hashtbl.iter printEntryPoint cg);

  end



    
(* ----------------------- Main --------------------- *)
    
(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainEntryPoints (fi : Cil.file) =
  begin
    let cg = Hashtbl.create 511 in
    let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] in 
    let entrypointfile = Unix.openfile "CILOUTPUT.entrypoint" mode 0o644 in
    let cgfile = Unix.openfile "CILOUTPUT.cgfile" mode 0o644 in
    (*let coveragefile = Unix.openfile "CILOUTPUT.coverage" mode 0o644 in*)
    let callersfile = Unix.openfile "CILOUTPUT.callers" mode 0o644 in

    (connectFunctionPointers fi);
    
    (* 0. Build call graph and construct edges *)
    (cgBuildCallGraph fi cg);
    (cgConstructEdge fi cg true); (*true because so that it is multi-edged*)

    (cgPrintEntryPointsToFile entrypointfile cg);

    (*Printing a list of functions reached from a given function*)
    (*(cgPrintCoverage coveragefile cg "xfs_iunpin_wait");*)
    (*(cgPrintSelectedEntryPoints callersfile cg "__d_find_alias");*)
    (*(cgPrintSelectedEntryPoints callersfile cg "chrdev_open");*)

    
    (*Printing call graph*)
    (cgPrintAllToDot1 cgfile cg true);

    (Unix.close entrypointfile)
    
  end
    
    

(* ------------------------- SETTING ---------------------------------- *)
    
let doEntryPoints = ref false


(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "entrypoints";
    fd_enabled = doEntryPoints;
    fd_description = "Finding entry points";
    fd_extraopt = [];
    fd_doit = (function (f: file) -> mainEntryPoints f) ;
    fd_post_check = false;
  }
    
    
