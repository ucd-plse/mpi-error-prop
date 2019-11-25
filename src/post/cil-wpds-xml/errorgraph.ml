open Cil
open Printf
open List
open Errorheader
open Errorprint
open Errorbuildgraph
open Erroreio
open Errorsubmit

(* new *)
open ConnectFunctionPointers

module P = Pretty


    
(* ----------------------- Main --------------------- *)
    
(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainErrorGraph (fi : Cil.file) =
  begin
    let cg = Hashtbl.create 511 in
    (* let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_APPEND]  in *)
    let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] in 
    let outf = Unix.openfile "CILOUTPUT.eg2" mode 0o644 in
    let dotfile = Unix.openfile "CILOUTPUT.dot1.c" mode 0o644 in
    let entrypointfile = Unix.openfile "CILOUTPUT.entrypoint" mode 0o644 in

    
    (**************************)
    (connectFunctionPointers fi);
    (**************************)
    
    (mypf outf fi.fileName);
    (mypf outf " : \n");
    
    
    (* 0. Build call graph and construct edges *)
    (cgBuildCallGraph fi cg);
    (cgConstructEdge fi cg true); (*Originally multi-edge*)

    let doEIO = true
    and doSubmit = false in

    (* ********************************* EIO ********************************* *)
    
    if (doEIO) then
      begin
	(* 1. Mark direct EIO and propagate EIO retval *)
	(fprintf stderr "1a\n"); (cgMarkDirectEIOretval fi cg);
	(fprintf stderr "1b\n"); (cgMarkPropagateEIOretval outf cg); 
	
	
	(* 2. Mark direct EIO and propagate EIO arg *)
	if true then
	  begin
	    (fprintf stderr "1c\n"); (cgAnalyzeEndPointEIOretval outf cg);

	    (fprintf stderr "2a\n"); (cgMarkDirectEIOarg outf cg); 
	    (fprintf stderr "2b\n"); (cgMarkPropagateEIOarg outf cg); 
	    (fprintf stderr "2c\n"); (cgAnalyzeEndPointEIOarg outf cg);
	  end
	else ();

	(*(cgPrintAll outf cg);*)
	
	(* Print 3a: all but specific path *)
	(* disable this if want to print all paths *)
	(* (cgPrintSpecificPathUpAndDown dotfile5 cg); *)
	(* argument 3 is printing all nodes (true) or just colored nodes (false) *)
	(cgPrintAllToDot1 dotfile cg false); 
	
	(* Print 3b: not all, but just propagate only *)
	(* (cgPrintAllToDot1 dotfile cg false); *)
	(cgPrintAllToFile outf cg);
	
      end
    else ();
    
    (* ********************************* Buffer ********************************* *)
    if (doSubmit) then
      begin
	(cgMarkSubmitNode outf cg);
	(cgFindBufferEndIO outf cg);
	(cgPrintSubmitAnalysisToFile outf cg);    
      end
    else ();




    
    (Unix.close outf);
    (Unix.close dotfile);
    
  end
    
    

(* ------------------------- SETTING ---------------------------------- *)
    



(* ********************************************************** *)
let doErrorGraph = ref false
    
       
(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "errorgraph";
    fd_enabled = doErrorGraph;
    fd_description = "generation or error graph";
    fd_extraopt = [];
    fd_doit = (function (f: file) ->
      (*
	 if not !Cilutil.makeCFG then begin
         Errormsg.s (Errormsg.error 
	 "--doerrorgraph: you must also specify --domakeCFG\n")
	 end ;
	 *)
      mainErrorGraph f) ;
    fd_post_check = false;
  }
    
    
