open Cil
open Printf
open List
open Errorprint
open Errorbuildgraph
open ConnectFunctionPointers
open Errorheader
open Scanf

module P = Pretty

let entryPoints = ref []


let initialize filename = 
  let buffer = Scanning.from_file filename in
  let add name =
    entryPoints := name :: !entryPoints
  in
  while not (Scanning.end_of_input buffer) do
    bscanf buffer "%s\n" add
  done


(***************************************************)
(* Finding Recursion                               *)
(***************************************************)
let cgPrintFindCycles (outf: Unix.file_descr) (cg: callGraph) (entry: string)  =
  begin
    (mypf outf ("entry point: " ^ entry ^ "\n" ));
    let rec printCallee  (visited: string list) (edge: funcEdge): unit =
      begin
	if (not edge.visitado) then
	  begin

	    if (List.exists (fun n -> if (n = edge.callee.fcname) then true else false) visited) then
	      begin
		(mypf outf ("Recursion: calling " ^ edge.callee.fcname ^ " again from: " ^ edge.caller.fcname ^ "\n"));
		mypf outf ("     " ^ edge.callee.fcname ^ "\n");
		mypf outf ("     " ^ edge.caller.fcname ^ "\n");
		List.iter (fun i -> mypf outf ("     " ^ i ^ "\n");) visited;
		mypf outf ("\n");
	      end;

	    edge.visitado <- true;
             (mypf outf ("callee: " ^ edge.callee.fcname ^ " caller: " ^ edge.caller.fcname ^ "\n"));
	    (List.iter (printCallee (edge.caller.fcname :: visited)) edge.callee.callees );

	  end
	else
	  (mypf outf ("Already visited, back off " ^ edge.caller.fcname ^ " " ^ edge.callee.fcname ^ "\n"));

      end in
    (List.iter (printCallee []) (Hashtbl.find cg entry).callees);	
  end




    
(* ---------------------- Main ------------------- *)
    
(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainFindRecursion (fi : Cil.file) =
  begin
    let cg = Hashtbl.create 511 in
    let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] in 
    let recursionfile = Unix.openfile "CILOUTPUT.recursion" mode 0o644 in

    (connectFunctionPointers fi);
    
    (* 0. Build call graph and construct edges *)
    (cgBuildCallGraph fi cg);
    (cgConstructEdge fi cg false);

    List.iter (cgPrintFindCycles recursionfile cg) !entryPoints;
    (*(cgPrintFindCycles recursionfile cg "xip_file_fault");*)


    (*(cgPrintFindCycles recursionfile cg "main");*)
    (*(cgPrintFindCycles recursionfile cg "send_sigurg");*)
    
    (Unix.close recursionfile)
  end
    
    

(* ------------------------- SETTING ---------------------------------- *)
    
let doFindRecursion = ref false


(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "findrecursion";
    fd_enabled = doFindRecursion;
    fd_description = "Finding cycles in call graph";
    fd_extraopt = [
     ("--entry-points", Arg.String (fun path -> initialize path),
     "<filename> File containing the name of the entry point functions");
  ];
    fd_doit = (function (f: file) -> mainFindRecursion f) ;
    fd_post_check = false;
  }
    
    
