
open Cil
open Printf
open List
open Errorheader
module P = Pretty




(* ------------------------- Build CG  -------------------------- *)

(***************************************************)   
(* cg.AddNode                                      *)
(***************************************************)
let cgAddNode (cg: callGraph) (fi: Cil.file) (origfd: fundec) (origloc: location) = 
  begin
    
    let getDirName (loc:location) =
      begin
	try
	  let ri = (String.rindex loc.file '/') in
	  let dir = (String.sub loc.file 0 (ri+1)) in	  
	  dir
	with Not_found -> 
	  begin
	    "N.A.";
	  end
      end in
    

    let newvisited = {
      any = true;
      up = true;
      down = true;
    } 
    and newerrornode1 = {
      direct = false;
      propagate = false;
      involve = false;
      errloc = 0;
    } 
    and newerrornode2 = {
      direct = false;
      propagate = false;
      involve = false;
      errloc = 0;
    } 
    and newsubmit = {
      submit = false;
      wait = false;
      check = false;
    } in
    let newnode = { 
      dirname = (getDirName origloc);
      filename = fi.fileName;
      fcname = origfd.svar.vname;
      fd = origfd; 
      floc = origloc;
      callers = []; 
      callees = [];
      nRetvalEIO = newerrornode1;
      nArgEIO = newerrornode2;
      nSubmit = newsubmit;
      visited = newvisited;
      nColor = "";
      isVisited = false;
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
    (origInstr: instr) (callLoc: location) (multiEdge: bool) =

  let existsCallee (n:funcNode) (name:string) =
    List.exists(fun (e:funcEdge) ->
      if e.callee.fcname = name then
	true
      else
	false) n.callees;  in
  
  let existsCaller (n:funcNode) (name:string) =
    List.exists(fun (e:funcEdge) ->
      if e.caller.fcname = name then
	true
      else
	false) n.callers;  in

  begin
    try
      (* n1 = caller node, n2 = callee node *)
      let n1 = cgFindNode cg callerName in
      let n2 = cgFindNode cg calleeName in
      begin
	let newerroredge1 = {
	  edgePropagate = false;
	  callerPropagate = false;
	  endpoint = false;
	  endpointSaved = false;
	  endpointChecked = false;
	  endpointVarInfo = n1.fd.svar; (* this is just a junk *)
	  endpointChangeMechanism = false;  
	}
	and newerroredge2 = {
	  edgePropagate = false;
	  callerPropagate = false;
	  endpoint = false;
	  endpointSaved = false;
	  endpointChecked = false;
	  endpointVarInfo = n1.fd.svar; (* this is just a junk *)
	  endpointChangeMechanism = false;  
	} in
	let newEdge = {
	  caller = n1;
	  callee = n2;
	  eInstr = origInstr;
	  eLoc = callLoc;
	  eRetvalEIO = newerroredge1; (* cannot be the same !!! *)
	  eArgEIO = newerroredge2;
	  eColor = "";
	  eClass1 = "";
	  eClass2 = "";
	  eClass3 = "";
	  visitado = false;
	} in
	

	if (multiEdge) then
	  begin
	    n1.callees <- newEdge :: n1.callees; 
	    n2.callers <- newEdge :: n2.callers;
	  end
	else
	(* A single node per function if the cg is used for finding recursion multiEdge = false*)
	  begin	    
	    if not (existsCallee n1 calleeName) then
	      n1.callees <- newEdge :: n1.callees; 
	    if not (existsCaller n2 callerName) then
	      n2.callers <- newEdge :: n2.callers;
	  end

      end
    with _ -> ()
  end
    

(***************************************************)   
(* main.initCallGraph                              *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgBuildCallGraph (fi: Cil.file) (cg: callGraph) =
  begin
    iterGlobals fi 
      (
       fun g -> match g with
	 GFun(fd,loc) -> cgAddNode cg fi fd loc
       | _ -> ()
      );
  end

   
(* ------------------------- Construct Edge  -------------------------- *)

(***************************************************)   
(* Class cgvConstructEdge (cg)                     *)
(***************************************************)
class cgvConstructEdge (cg:callGraph) (multiEdge: bool) = object
  inherit nopCilVisitor
  val the_fun_name = ref None
    

  
  method vinst (origInstr:instr) =
    let _ = match origInstr with
      Call(_,Lval(Var(calleeVarInfo),NoOffset),_,callLoc) -> begin
        (* known function call *)
        match !the_fun_name with
          None -> failwith "cgv: call outside of any function"
        | Some(callerName) -> 
	    (cgAddEdge cg callerName calleeVarInfo.vname origInstr callLoc multiEdge);
      end
    | _ -> ()
    in DoChildren
      
  method vfunc f = the_fun_name := Some(f.svar.vname) ; DoChildren
end

(***************************************************)   
(* main.createCallGraph                            *)
(* Note: this method will not cover printf !!      *)
(***************************************************)
let cgConstructEdge (fi: Cil.file) (cg: callGraph) (multiEdge: bool) =
  begin
    visitCilFileSameGlobals (new cgvConstructEdge cg multiEdge) fi ;
  end
