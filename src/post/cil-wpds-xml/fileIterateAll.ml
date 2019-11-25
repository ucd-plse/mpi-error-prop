open Cil
open Printf
open List
module P = Pretty


(*******************************************************************)
(* fileIterateAll:                                                 *)
(* This file iterates a function, and tries to understand the      *)
(* CIL representations of the code. This code DOES not merge       *)
(* all files, because it does not create the graphs first          *)
(*******************************************************************)


(***************************************************)
(* Converter: exp/instr/lval/stmt/type to String   *)
(***************************************************)
let expToStr (e:exp) = (P.sprint 80 (P.dprintf "%a" dn_exp e))
let instrToStr (i:instr) = (P.sprint 80 (P.dprintf "%a" dn_instr i))
let lvalToStr (l:lval) = (P.sprint 80 (P.dprintf "%a" dn_lval l))
let stmtToStr (s:stmt) = (P.sprint 80 (P.dprintf "%a" dn_stmt  s))
let typeToStr (t:typ) = (P.sprint 80 (P.dprintf "%a" dn_type t))
let atoi (i:int) = (P.sprint 80 (P.dprintf "%d" i))


type intarg = {
    mutable ival: int;
  }      

and boolarg = {
    mutable bval: bool;
  }      
      
(* ------------------------- Build CG  -------------------------- *)
let mypf (fd:Unix.file_descr) (str:string) =
  begin
    let _ = (Unix.write fd str 0 (String.length str)) in ()
  end

    

(*######################################################################################*)
(*############                     TEMPLATE  START                       ###############*)
(*######################################################################################*)

(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvAnalyze (outf:Unix.file_descr) (d:int) (fd:fundec) = object
  inherit nopCilVisitor
      
  method vstmt (s:stmt) : stmt visitAction =
    begin
      (P.fprint stderr 80 (P.dprintf "  (s) %a \n" dn_stmt  s));  
      DoChildren
    end
      
  method vvdec (v: varinfo) =
    begin
      DoChildren
    end
      
      (* visit an instruction; we're only interested in calls *)
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
    
(* ##########################################################  *)


(***************************************************)   
(*                                                 *)
(***************************************************)
let find_substring s1 s2 =
  (* find s2 in s1, or raise Not_found *)
  if String.length s2 > String.length s1 then raise Not_found;
  let b = String.create (String.length s2) in
  let rec search k =
    if k > String.length s1 - String.length s2 then raise Not_found;
    String.blit s1 k b 0 (String.length s2);
    if b = s2 then
      k
    else search (k+1)
  in
  search 0
;;



(* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ *)

(***************************************************)   
(*                                                 *)
(***************************************************)
let rec iterateLhost (outf:Unix.file_descr) (d:int) (lhost:lhost) =
  begin
    (mypf outf ((atoi d)^"[lhost] "));
    (match lhost with
    | Var(vi) -> 
	begin
	  (mypf outf ((atoi d)^"[Var] "));
	  (iterateVarInfo outf (d+1) vi);
	end
    | Mem(exp) -> 
	begin
	  (mypf outf ((atoi d)^"[Mem] "));
	  (iterateExp outf (d+1) exp);
	end
    );
  end
    

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateLval (outf:Unix.file_descr) (d:int) (lval:lval) =
  begin
    (mypf outf ((atoi d)^"[lval] "));
    (match lval with
      (lhost,offset) ->
	begin
	  (iterateLhost outf (d+1) lhost);
	  (iterateOffset outf (d+1) offset);	  
	end
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateIkind (outf:Unix.file_descr) (d:int) (ikind:ikind) =
  begin
    (mypf outf ((atoi d)^"[ikind] "));	  
    (match ikind with
    | IChar -> ();
    | ISChar -> ();
    | IUChar -> ();
    | IInt -> (mypf outf ((atoi d)^"[IInt] "));
    | IUInt -> ();
    | IShort -> ();
    | IUShort -> ();
    | ILong  -> ();
    | IULong -> ();
    | ILongLong -> ();
    | IULongLong  -> ();
    )
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateConstant (outf:Unix.file_descr) (d:int) (constant:constant) =
  begin
    (match constant with
    | CInt64(int64,ikind,None) -> 
	begin
	  (mypf outf ((atoi d)^"[CInt64A] "));	  
	  (mypf outf ((atoi d)^"[int64:" ^ (atoi (Int64.to_int int64)) ^ "] "));	  	  
	  (iterateIkind outf (d+1) ikind);
	end
    | CInt64(int64,ikind,Some(str)) -> 
	begin
	  (mypf outf ((atoi d)^"[CInt64B] "));	  	  
	end
    | CStr(str) -> ();
    | CWStr(int64list) -> ();
    | CChr(char) -> ();
    | CReal(float,fkind,None) -> ();
    | CReal(float,fkind,str) -> ();
    | CEnum(exp,string,enuminfo) -> ();
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateExp (outf:Unix.file_descr) (d:int) (exp:exp) =
  begin
    (mypf outf ((atoi d)^"[exp] "));
    (match exp with
      
    | Const(constant) -> 
	begin
	  (mypf outf ((atoi d)^"[Const] "));
	  (iterateConstant outf (d+1) constant);
	end
    | Lval(lval) -> 
	begin
	  (mypf outf ((atoi d)^"[Lval] "));
	  (iterateLval outf (d+1) lval);
	end
    | SizeOf(_) -> (mypf outf ((atoi d)^"[SizeOf] "));
    | SizeOfE(_) -> (mypf outf ((atoi d)^"[SizeOfE] "));
    | AlignOf(_) -> (mypf outf ((atoi d)^"[AlignOf] "));
    | AlignOfE(_) -> (mypf outf ((atoi d)^"[AlignOfE] "));
    | UnOp(_,_,_) -> (mypf outf ((atoi d)^"[UnOp] "));
    | BinOp(_,_,_,_) -> (mypf outf ((atoi d)^"[BinOp] "));
    | CastE(typ,exp2) -> 
	begin
	  (mypf outf ((atoi d)^"[CastE] "));
	  (iterateTyp outf (d+1) typ);
	  (iterateExp outf (d+1) exp2);
	end
    | AddrOf(lval) -> 
	begin
	  (mypf outf ((atoi d)^"[AddrOf] "));
	  (iterateLval outf (d+1) lval);
	end
    | StartOf(_) -> (mypf outf ((atoi d)^"[StartOf] "));
    | _ -> ());
  end
    

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateInstr (outf:Unix.file_descr) (d:int) (instr:instr) = 
  begin
    
    (mypf outf ((atoi d)^"[instr] "));
    (* (mypf outf ((atoi d)^(instrToStr instr)));*)
    
    (match instr with 
    | Set(lval,exp,loc) ->  
	begin
	  (mypf outf ((atoi d)^"[Set] "));
	  (iterateLval outf (d+1) lval);
	  (iterateExp outf (d+1) exp);
	end
    | Call(ret,exp,explist,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Call(?,fexp,arglist,loc] "));
	  (match ret with
	  | None -> (mypf outf ((atoi d)^"[NoRet] "));
	  | Some(lval) -> 
	      begin
		(mypf outf ((atoi d)^"[lval] "));
		(iterateLval outf (d+1) lval);
	      end
	     );
	  (mypf outf ((atoi d)^"[fexp] "));
	  (iterateExp outf (d+1) exp);
	  (mypf outf ((atoi d)^"[arglist] "));
	  (List.iter (iterateExp outf (d+1)) explist);
	end
    | Asm(_,_,_,_,_,_) ->  (mypf outf ((atoi d)^"[Asm] "));
    );
    (mypf outf "\n\n");
  end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateFieldInfoList (outf: Unix.file_descr) (d:int) (cfields: fieldinfo list) =
  begin
    List.iter (iterateFieldInfo outf d) cfields;
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateCompInfo (outf: Unix.file_descr) (d:int) (compinfo:compinfo) = 
  begin
    (mypf outf ((atoi d)^"[compinfo] "));
    (mypf outf ((atoi d)^"a[cn:" ^ compinfo.cname ^ "] "));
    (mypf outf ((atoi d)^"b[cfields:] "));

    (iterateFieldInfoList outf (d+1) compinfo.cfields);
    (* (iterateAttributes compinfo.attributes);*)
  end


(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateTyp (outf: Unix.file_descr) (d:int) (vtype: typ) = 
  begin
    (mypf outf ((atoi d)^"[typ] "));    
    (match vtype with 
      TVoid(_) -> (mypf outf ((atoi d)^"[TVoid] "));
    | TInt(_,_) -> (mypf outf ((atoi d)^"[TInt] "));
    | TFloat(_,_) -> (mypf outf ((atoi d)^"[TFloat] "));
    | TPtr(typ,attrs) -> 
	begin
	  (mypf outf ((atoi d)^"[TPtr] "));
	  (iterateTyp outf (d+1) typ);
	  (* (iterateAttrs outf (d+1) attrs);*)	  
	end
    | TArray(typ,_,_) -> 
	begin
	  (mypf outf ((atoi d)^"[TArray] "));
	  (iterateTyp outf (d+1) typ);
	end
    | TFun(_,_,_,_) -> 
	begin
	  (mypf outf ((atoi d)^"[TFun] "));
	end
    | TNamed(ti,_) -> 
	begin
	  (mypf outf ((atoi d)^"[TNamed] "));
	  (iterateTypeInfo outf (d+1) ti);
	end
    | TComp(compinfo,attrs) -> 
	begin
	  (mypf outf ((atoi d)^"[TComp] "));
	  (iterateCompInfo outf (d+1) compinfo);
	end
    | TEnum(_,_) -> (mypf outf ((atoi d)^"[TEnum] "));
    | TBuiltin_va_list(_) -> (mypf outf ((atoi d)^"[TBuiltin] "));
    );
  end


(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateVarInfo (outf: Unix.file_descr) (d:int) (vi:varinfo) = 
  begin
    (mypf outf ((atoi d)^"[varinfo] "));
    (mypf outf ((atoi d)^"a[vn:" ^ vi.vname ^ "] "));
    (mypf outf ((atoi d)^"b[vt:] "));
    
    (* disable for a while *)
    (iterateTyp outf (d+1) vi.vtype);
  end

(***************************************************)   
(* fcomp:compinfo, fname:string                    *)
(* ftype:typ, floc:location                        *)
(***************************************************)
and iterateFieldInfo (outf: Unix.file_descr) (d:int) (fi: fieldinfo) = 
  begin
    (mypf outf ((atoi d)^"[fieldinfo] "));    
    (mypf outf ((atoi d)^"a[fn:" ^ fi.fname ^ "] "));
    (mypf outf ((atoi d)^"b[floc:" ^ fi.floc.file ^ "/" ^ (atoi fi.floc.line) ^ "] "));
    (mypf outf ((atoi d)^"c[ft:] "));
    (* (iterateCompInfo outf (d+1) fi.fcomp); --- this is a pointer loop, NEVER CALL *)
    (iterateTyp outf (d+1) fi.ftype); 
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateOffset (outf: Unix.file_descr) (d:int) (offset: offset) = 
  begin
    (mypf outf ((atoi d)^"[offset] "));

    (match offset with
      NoOffset -> 
	begin
	  (mypf outf ((atoi d)^"[NoOffset] "));
	end
    | Field(fieldinfo,offset) -> 
	begin
	  (mypf outf ((atoi d)^"[Field] "));
	  (iterateFieldInfo outf (d+1) fieldinfo);
	  (iterateOffset outf (d+1) offset);
	end
    | Index(exp,offset) -> 
	begin
	  (mypf outf ((atoi d)^"[Index] "));
	  (iterateExp outf (d+1) exp);
	  (iterateOffset outf (d+1) offset);
	end
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateOffsetInit (outf: Unix.file_descr) (d:int) (offsetinit:offset*init) =
  begin
    (mypf outf ((atoi d) ^ "[offset+init] "));          
    (match offsetinit with
      (offset,init) ->
	begin
	  (iterateOffset outf (d+1) offset);
	  (iterateInit outf (d+1) init);
	end
    );
  end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateOffsetInitList (outf: Unix.file_descr) (d:int)
  (offsetinitlist:(offset*init)list) = 
  begin
    List.iter (iterateOffsetInit outf d) offsetinitlist;
  end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateInit (outf: Unix.file_descr) (d:int) (init:init) = 
  begin
    (mypf outf ((atoi d) ^ "[init] "));    
    (match init with
      SingleInit(exp) -> 
	begin
	  (mypf outf ((atoi d)^"[SingleInit] "));
	  (iterateExp outf (d+1) exp);
	end
    | CompoundInit(typ,offsetinitlist) -> 
	begin
	  (mypf outf ((atoi d)^"[CompoundInit] "));
	  (iterateTyp outf (d+1) typ); 
	  (iterateOffsetInitList outf (d+1) offsetinitlist);
	end
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateInitInfo (outf: Unix.file_descr) (d:int) (initinfo:initinfo) = 
  begin
    (mypf outf ((atoi d) ^ "[initinfo] "));    
    (match initinfo.init with
      None -> 
	begin
	  (mypf outf ((atoi d)^"[None] "));
	end
    | Some(init) -> 
	begin
	  (mypf outf ((atoi d)^"[Some] "));
	  (iterateInit outf (d+1) init);
	end
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateTypeInfo (outf: Unix.file_descr) (d:int) (ti:typeinfo) =
  begin
    (mypf outf ((atoi d) ^ "[typeinfo] "));
    (mypf outf ((atoi d) ^ "[tn:" ^ ti.tname ^ "] "));
    (iterateTyp outf (d+1) ti.ttype); 
  end
    
(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateLabel (outf: Unix.file_descr) (d:int) (l:label) =
  begin
    (mypf outf ((atoi d) ^ "[label] "));    
    (match l with
    | Label (string,loc,bool) -> 
	begin
	  (mypf outf ((atoi d)^"[Label] "));
	end
    | Case (exp,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Case(exp)] "));
	  (iterateExp outf (d+1) exp);
	end	
    | Default (loc) ->
	begin
	  (mypf outf ((atoi d)^"[Default] "));
	end 
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateStmtKind (outf: Unix.file_descr) (d:int) (sk:stmtkind) =
  begin
    (mypf outf ((atoi d) ^ "[stmtkind] "));    
    (match sk with
    | Instr(instrlist) -> 
	begin
	  (mypf outf ((atoi d)^"[Instr(instrlist)] "));
	  (List.iter (iterateInstr outf (d+1)) instrlist);
	end
    | Return(_,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Return] "));
	end
    | Goto(stmt,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Goto] "));
	end
    | Break(loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Break] "));
	end
    | Continue(loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Continue] "));
	end
    | If(exp,blk1,blk2,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[If] "));
	end
    | Switch(exp,block,stmtlist,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[Switch(exp,block,stmtlist,loc)] "));
	  (iterateExp outf (d+1) exp);
	  (iterateBlock outf (d+1) block);
	  (List.iter (iterateStmt outf (d+1)) stmtlist);
	end
    | Loop(block,loc,_,_) -> 
	begin
	  (mypf outf ((atoi d)^"[Loop] "));
	end
    | Block(block) -> 
	begin
	  (mypf outf ((atoi d)^"[Block] "));
	  (iterateBlock outf (d+1) block);
	end
    | TryFinally(blk1,blk2,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[TryFinally] "));
	end
    | TryExcept(blk1,instrlistandexp,blk2,loc) -> 
	begin
	  (mypf outf ((atoi d)^"[TryExcept] "));
	end
    );
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateStmtSuccs (outf: Unix.file_descr) (d:int) (s:stmt) =
  begin
    (mypf outf ((atoi d) ^ "[succ-id:" ^ (atoi s.sid) ^"] "));
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateStmtPreds (outf: Unix.file_descr) (d:int) (s:stmt) =
  begin
    (mypf outf ((atoi d) ^ "[pred-id:" ^ (atoi s.sid) ^"] "));
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateStmt (outf: Unix.file_descr) (d:int) (s:stmt) =
  begin
    (mypf outf ((atoi d) ^ "[stmt] "));
    (mypf outf ((atoi d) ^ "a[sid:" ^ (atoi s.sid) ^ "] "));

    (mypf outf ((atoi d) ^ "b[s.labels] "));
    (List.iter (iterateLabel outf (d+1)) s.labels);
    (mypf outf ((atoi d) ^ "c[s.succs] "));
    (List.iter (iterateStmtSuccs outf (d+1)) s.succs);
    (mypf outf ((atoi d) ^ "d[s.preds] "));
    (List.iter (iterateStmtPreds outf (d+1)) s.preds);

    (mypf outf ((atoi d) ^ "e[s.skind] "));    
    (iterateStmtKind outf (d+1) s.skind);

  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateAttr (outf: Unix.file_descr) (d:int) (attr:attribute) =
  begin
    (mypf outf ((atoi d) ^ "[attr] "));
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and iterateBlock (outf: Unix.file_descr) (d:int) (block:block) =
  begin
    (mypf outf ((atoi d) ^ "[block] "));
    (mypf outf ((atoi d) ^ "[bstmts:] "));
    (List.iter (iterateStmt outf (d+1)) block.bstmts);
    (mypf outf ((atoi d) ^ "[battrs:] "));
    (List.iter (iterateAttr outf (d+1)) block.battrs); 
    
  end
    

(* ############################################################################## *)
(* ############################################################################## *)
(* ############################################################################## *)


(***************************************************)   
(* function node visitor                           *)
(***************************************************)
class fnvIterateFuncDec (outf:Unix.file_descr) (d:int) (fd:fundec) = object
   inherit nopCilVisitor
   
   method vstmt (s:stmt) : stmt visitAction =
   begin
     DoChildren
   end
      
  method vvdec (v: varinfo) =
    begin
      DoChildren
    end
    
   (* visit an instruction; we're only interested in calls *)
  method vinst (i:instr) : instr list visitAction =
    begin
      (iterateInstr outf (d) i);
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
(* If I want to check a function of interest, then *)
(* just call this checker before analyzing the     *)
(* function                                        *)
(***************************************************)
let isFunctionOfInterest (fcname:string) = 
  if (fcname = "buffer_uptodate"
    || fcname = "constant_test_bit"
) then
    true
  else
    false
      
(***************************************************)   
(*                                                 *)
(***************************************************)
let iterateGFunc (outf: Unix.file_descr) (d:int) (fd:fundec) (loc:location) = 
  begin
    (mypf outf ("[GFunc] " ^ loc.file ^ " " ^ (atoi loc.line) ^"\n"));    
    let doGFunc = true in
    if (doGFunc) then
      begin

	(iterateBlock outf d fd.sbody);

	(* let _ = visitCilFunction (new fnvIterateFuncDec outf d fd) fd in (); *)
      end
    else ();
    (mypf outf "\n");
  end


(***************************************************)   
(*                                                 *)
(***************************************************)
let iterateGVarDecl (outf: Unix.file_descr) (d:int) 
    (vi:varinfo) (loc:location) =
  begin
    (mypf outf ("[GVarDecl] " ^ loc.file ^ " " ^ (atoi loc.line) ^"\n"));
    (mypf outf "\n");
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let iterateGCompTag (outf: Unix.file_descr) (d:int) 
    (compinfo:compinfo) (loc:location) =
  begin
    (mypf outf ("[GCompTag] " ^ loc.file ^ " " ^ (atoi loc.line) ^"\n"));
    let doGCompTag = false in
    if (doGCompTag) then
      begin
	(mypf outf ((atoi d)^"[GCompTag] "));
	(iterateCompInfo outf (d+1) compinfo);
      end
    else ();
    (mypf outf "\n\n");
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let iterateGVar (outf: Unix.file_descr) (d:int) 
    (vi:varinfo) (initinfo:initinfo) (loc:location) =
  begin
    (mypf outf ("[GVar] " ^ loc.file ^ " " ^ (atoi loc.line) ^"\n"));
    let doGVar = false in
    if (doGVar) then
      begin
	(mypf outf ((atoi d)^"[GVar] "));
	(iterateVarInfo outf (d+1) vi);
	(iterateInitInfo outf (d+1) initinfo);
      end
    else ();
    (mypf outf "\n\n");
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let iterateGType (outf: Unix.file_descr) (d:int) 
    (ti:typeinfo) (loc:location) =
  begin
    (mypf outf ("[GType] " ^ loc.file ^ " " ^ (atoi loc.line) ^"\n"));
    let doGType = false in
    if (doGType) then
      begin
	(mypf outf ((atoi d)^"[GType] "));
	(iterateTypeInfo outf (d+1) ti); 
      end
    else ();
    (mypf outf "\n\n");
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
let fileIterateAll (fi: Cil.file) (outf: Unix.file_descr) =
  begin
    let d = 1 in 

    iterGlobals fi (
    fun g -> match g with 
      GType(typeinfo,loc) -> (iterateGType outf d typeinfo loc);
    | GCompTag(compinfo,loc) -> (iterateGCompTag outf d compinfo loc);
    | GCompTagDecl(compinfo,loc) -> (mypf outf "a");
    | GVarDecl(vi,loc) -> (iterateGVarDecl outf d vi loc); 
    | GVar(vi,initinfo,loc) -> (iterateGVar outf d vi initinfo loc); 
    | GFun(fd,loc) -> (iterateGFunc outf d fd loc);
    | _ -> ());
  end

    


(*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ WAIT/WAKE $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*)


    
    
(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainFileIterateAll (fi : Cil.file) =
  begin
    let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] in 
    let outf = Unix.openfile "CILOUTPUT.iter" mode 0o644 in
    (mypf outf "--------------------------------------\n\n");
    (fileIterateAll fi outf);
  end
    


(* ********************************************************** *)
let doFileIterateAll = ref false
    
       
(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "fileIterateAll";
    fd_enabled = doFileIterateAll;
    fd_description = "File Iterate All";
    fd_extraopt = [];
    fd_doit = (function (f: file) ->
      mainFileIterateAll f) ;
    fd_post_check = false;
  }
    
    
