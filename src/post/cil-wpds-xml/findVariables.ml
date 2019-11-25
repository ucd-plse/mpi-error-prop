open Cil
open Printf
open List
open Errorprint
open Errorbuildgraph
open ConnectFunctionPointers
open Errorheader
open Scanf
open Error
open Set
open Str
open VarSet


module P = Pretty

let old = ref 0


let renameVariables file =
  let visitor = function
    | GFun (fundec, _) ->
	begin
	  List.iter (fun v -> let name = Printf.sprintf "%s#%s" fundec.svar.vname v.vname in
	                          v.vname <- name 
	            ) (fundec.sformals @ fundec.slocals);
	end
    | _ -> ()
in
  iterGlobals file visitor


(**********************)
(* Relevant variables *)
(**********************)

let relevant = ref Var_set.empty

let isRelevant varinfo = Var_set.mem varinfo !relevant


(*******************)
(* Visitor class   *)
(*******************)


let undoAlphaRenaming =
  let pattern = regexp "___[0-9]+$" in
  let replace = global_replace pattern "" in
  fun {vname = vname} ->
    replace vname

class visitor pds transformation = object(self)
  inherit nopCilVisitor


  method private printSomeVars element filter =
    ignore (fprintf pds "\t\t\t\t<%s>\n" element);
    Var_set.iter
      (fun varinfo ->
	if filter varinfo then
	  begin
	    ignore (fprintf pds "\t\t\t\t\t<var id='%s'" varinfo.vname);
	    let originalName = undoAlphaRenaming varinfo in
	    if originalName <> varinfo.vname then
	      ignore (fprintf pds " name='%s'" originalName);
	    output_string pds "/>\n"
	  end)
      !relevant;
    ignore (fprintf pds "\t\t\t\t</%s>\n" element);


  method printSet() = 
    begin
      ignore(fprintf pds "\t\t<Prologue>\n");
      ignore(fprintf pds "\t\t\t<Variables>\n");
      self#printSomeVars "Globals" (fun varinfo -> varinfo.vglob);
      self#printSomeVars "Locals" (fun varinfo -> not varinfo.vglob);
      self#printSomeVars "Pointers" (fun varinfo -> isPointerType (unrollTypeDeep varinfo.vtype));
      ignore(fprintf pds "\t\t\t</Variables>\n");
      ignore(fprintf pds "\t\t</Prologue>\n")
    end


  method add(vinfo:varinfo) = 
    if not transformation then
      begin
	match unrollTypeDeep vinfo.vtype with
	| TInt(_,_) ->
	    relevant := Var_set.add vinfo !relevant
	| _ ->
	    ignore()
      end
    else 
      relevant := Var_set.add vinfo !relevant


  method vstmt(s: stmt) =
    begin
      (*ignore(Cil.dumpStmt plainCilPrinter stderr 0 s);*)
      match s.skind with

        | Instr [Call (receiver, (Lval (Var callee, NoOffset)), actuals, loc)] ->
	  begin
	    SkipChildren
	  end

        | Instr[Set((Var vinfo, NoOffset), Const(CInt64(value,_,_)), _)] ->
          begin
            match Error.byValue value with
            | Some code -> 
		self#add vinfo
            | None -> ()
          end;
  	  SkipChildren;
	
	| Instr[Set((Var vinfo1, NoOffset), (Lval (Var vinfo2, NoOffset)), _)] ->
	    if isRelevant vinfo2 then
	      self#add vinfo1;
	    SkipChildren;

	| Instr[Set((Mem(e1),off), Const(CInt64(value,_,_)), _)] ->
            begin
	      match Error.byValue value with
              | Some code -> 
		  begin
		    match e1 with
		    | Lval(Var(vinfo),off) -> 
			self#add vinfo
		    | _ -> ignore();
		  end
              | None -> ()
            end;
	    SkipChildren;

	| Instr[Set((Var vinfo1, NoOffset), AddrOf(Var vinfo2, _), _)] ->
	    if isRelevant vinfo2 then
		self#add vinfo1;
	    SkipChildren;

	| Instr[Set((Var vinfo1, NoOffset), CastE(TInt(_,_), (Lval (Var vinfo2, NoOffset))), _)] ->
	    if isRelevant vinfo2 then
		self#add vinfo1;
	    SkipChildren; 


	(* Only when transformation ON *)
	| Instr[Set((Var vinfo, NoOffset), CastE(_, Const(CInt64(value,_,_))), _)] ->
	    if transformation then
	      begin
		match Error.byValue value with
		| Some code -> 
		    relevant := Var_set.add vinfo !relevant
		| None -> ()
	      end;	    
	    SkipChildren;    

	| Instr[Set((Var vinfo1, NoOffset), CastE(_, (Lval (Var vinfo2, NoOffset))), _)] ->
	    if transformation then
	      begin
		if isRelevant vinfo2 then
		  relevant := Var_set.add vinfo1 !relevant;
	      end;
	    SkipChildren;    

        | _ ->	  
	    DoChildren
    end


  method vfunc(f: fundec) =
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
   
    DoChildren;

end


let findVariablesVisitor fi pds transformation =
  begin

    (* Renaming variables *)
    renameVariables fi;

    (* Finding relevant variables *)
    let visitor = new visitor pds transformation in
        begin 
          ignore(visitCilFileSameGlobals (visitor :> cilVisitor) fi);
          while (!old != (Var_set.cardinal !relevant)) do
              old := (Var_set.cardinal !relevant);
              ignore(visitCilFileSameGlobals (visitor :> cilVisitor) fi);
          done;
	  visitor#printSet()
        end
  end




(*    
let doFindVariables = ref false

let feature : featureDescr =
  { fd_name = "findVariables";
    fd_enabled = doFindVariables;
    fd_description = "Finding relevant variables";
    fd_extraopt = [
     ("--error-codes", Arg.String (fun path -> Error.initialize path),
     "<filename> File containing the error code definitions");
  ];
    fd_doit = (function (f: file) -> mainFindVariables f) ;
    fd_post_check = false;
  }
*)
