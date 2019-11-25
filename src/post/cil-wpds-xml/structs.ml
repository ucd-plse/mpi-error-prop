open Cil
open Pretty
open List


exception Non_struct of string;;
(*Some debugging method privates*)
 let printCilObj d_obj = fun obj chan ->
   let d = dprintf "%a" d_obj obj in
   let s = sprint 80 d in
   ignore( fprintf chan "[%s]\n" s )

let printCilType = printCilObj d_type

let tableFieldVars = new FieldNameHash.c 1000

let getName structName fieldName = 
  Printf.sprintf "%s.%s" structName fieldName


let visitVariables file =
  let visitor = function
    | GFun (fundec, _) ->
	begin
	  List.iter (fun varinfo ->
	    begin
	      match unrollTypeDeep varinfo.vtype with
	      | TComp(compinfo,_)
	      |	TPtr(TComp(compinfo,_),_) ->
		  begin
		    List.iter (fun f -> 
		      let newname = getName varinfo.vname f.fname in
		      let newvarinfo = makeLocalVar fundec newname f.ftype in (* added to fundec.slocals *)
		          tableFieldVars#add newname (Cil.var newvarinfo))
		      (compinfo.cfields)
		  end
	      | _ -> ()
	    end	    
	      ) (fundec.sformals @ fundec.slocals);	  
	end

    | GVar (varinfo, _, _)
    | GVarDecl (varinfo, _) ->
	begin
	  match varinfo.vtype with
	  | TComp(compinfo,_)
	  | TPtr(TComp(compinfo,_),_) ->
	      begin
		List.iter (fun f -> 
		  let newname = getName varinfo.vname f.fname in
		  let newvarinfo = makeGlobalVar newname f.ftype in
		      tableFieldVars#add newname (Cil.var newvarinfo))
		  (compinfo.cfields)
	      end
	  | _ -> ()
	end

    | _ -> ()
in
  iterGlobals file visitor;



class replace file = object(self)
  inherit nopCilVisitor


  method vlval (lv: lval) =
    match lv with
      | (Var(vinfo), Field(finfo,_))
      | (Mem(Lval(Var(vinfo),NoOffset)), Field(finfo,_)) ->
	  begin
	  try
	    let newvar = tableFieldVars#find (getName vinfo.vname finfo.fname) in
	    ChangeTo(newvar)
	  with Not_found ->
	    DoChildren
	  end
	    
      |	_ ->
	  DoChildren


  method vfunc(f: fundec) =

    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end


class insertAssignment file = object(self)
  inherit nopCilVisitor


  method vstmt(s: stmt) =
    match s.skind with
    | Instr[Set(lv, _, _) as original] -> 
	begin
	  
	  match lv with
	  | (Var(vinfo), NoOffset)
	  | (Mem(Lval(Var(vinfo),NoOffset)), NoOffset) ->
	      begin
		match vinfo.vtype with
		| TPtr(TComp(compinfo,_),_) ->
		    begin
		      try
			begin
			  let assgns = List.map (fun f -> 
			    Set(tableFieldVars#find (getName vinfo.vname f.fname), integer(-67737869), get_stmtLoc s.skind))
			    (compinfo.cfields) in
			  s.skind <- Instr (original::assgns);
			  DoChildren
			end
		      with Not_found ->
			DoChildren
		    end
		| _ ->
		    DoChildren
	      end
		
	  |  _ ->
	      DoChildren
	end
    | _ ->
	DoChildren



  method vfunc(f: fundec) =

    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    DoChildren;

end


let structsVisitor file =
  begin
    visitVariables file;
    let visitor1 = new replace file in
    let visitor2 = new insertAssignment file in
    begin
      ignore(visitCilFileSameGlobals (visitor1 :> cilVisitor) file);
      ignore(visitCilFileSameGlobals (visitor2 :> cilVisitor) file)
    end
  end



