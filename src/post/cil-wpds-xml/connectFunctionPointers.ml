open Cil
open Printf
open List
open FileIterateAll
module P = Pretty

(*******************************************************************)
(* See status at the bottom of this page                           *)
(*******************************************************************)


(*******************************************************************)
(* connectFunctionPointers:                                        *)
(*                                                                 *)
(* PHASE 1: Find all structures that have function pointers        *)
(*                                                                 *)
(* PHASE 2: Map fp implementations with the corresponding fps      *)
(*                                                                 *)
(* PHASE 3: Insert non-fp calls at every fp call                   *)
(*                                                                 *)
(*******************************************************************)
    
(* THE FILE *)
let mode = [Unix.O_RDWR; Unix.O_CREAT; Unix.O_TRUNC] 

let outf = Unix.openfile "CILOUTPUT.fp" mode 0o644 

(***************************************************)
(* Converter: exp/instr/lval/stmt/type to String   *)
(***************************************************)
let expToStr (e:exp) = (P.sprint 80 (P.dprintf "%a" dn_exp e))
let instrToStr (i:instr) = (P.sprint 80 (P.dprintf "%a" dn_instr i))
let lvalToStr (l:lval) = (P.sprint 80 (P.dprintf "%a" dn_lval l))
let stmtToStr (s:stmt) = (P.sprint 80 (P.dprintf "%a" dn_stmt  s))
let blockToStr (b:block) = (P.sprint 80 (P.dprintf "%a" d_block  b))
let typeToStr (t:typ) = (P.sprint 80 (P.dprintf "%a" dn_type t))
let atoi (i:int) = (P.sprint 80 (P.dprintf "%d" i))
let atoi3 (i:int) = (P.sprint 80 (P.dprintf "%3d" i))
let strToStr20 (s:string) = (P.sprint 80 (P.dprintf "%-20s" s))
let strToStr30 (s:string) = (P.sprint 80 (P.dprintf "%-30s" s))
let boolToStr (b:bool) =
  if (b) then (P.sprint 80 (P.dprintf "true"))
  else (P.sprint 80 (P.dprintf "false"))      
      

type intarg = {
    mutable ival: int;
  }      

and boolarg = {
    mutable bval: bool;
  }      

and stringarg = {
    mutable sval: string;
  }

and lvalarg = {
    mutable lvalval: lval option;
  }

and stmtlistarg = {
    mutable stmtlistval: stmt list;
  }

and instrlistarg = {
    mutable instrlistval: instr list;
  }
      
and stmtarg = {
    mutable stmtval: stmt option;
  }

and blockarg = {
    mutable blockval: block option;
  }
      
(**********************************************************)
(* each fpAssignImpl has the fpName (e.g. .write) and then*)
(* the function implementation name (e.g. ext2_write) and *)
(* the location for the assignment                        *)
(**********************************************************)
type fpAssignImpl = {
    mutable fpImplName: string;
    mutable fpImplVi: varinfo;
    mutable fpAssignImplLoc: location;
  }

(**********************************************************)
(* each fpNode has the name of the function, fpName , and *)
(* fpIndex, which is the i-th field of the structure, and *)
(* has fpAssignImpls which are the assignments of the     *)
(* function pointers                                      *)
(**********************************************************)
type fpNode = {
    mutable fpName: string;
    mutable fpFieldInfo: fieldinfo;
    mutable fpIndex: int;
    mutable fpLoc: location;
    mutable fpAssignImplList: fpAssignImpl list; 
  }

(**********************************************************)
(* each spNode has the struct name, and a list of func    *)
(* pointers represented by fpNode                         *)
(* fpListReversed should not be used when we acessing the *)
(* list. It's only used for creating the list. Since we   *)
(* add fp in the beginning of the list e.g. l = new :: l  *)
(* we need to have a temp list that is the reversed list  *)
(**********************************************************)
type spNode = {
    mutable spName: string;
    mutable spCompInfo: compinfo;
    mutable spLoc: location;
    mutable fpList: fpNode list;
    mutable fpListReversed: fpNode list;
  }
      
type tdpNode = {
    mutable tName: string;
    mutable tInfo: typeinfo;
    mutable tLoc: location;
  }

(**********************************************************)
(* spTable has a bunch of spNodes, which are structures   *)
(* that have func pointeres                               *)
(**********************************************************)
type spTable =
    (string, spNode) Hashtbl.t

type tdpTable = 
    (string, tdpNode) Hashtbl.t
      

(* ------------------------- Build CG  -------------------------- *)
let mypf (fd:Unix.file_descr) (str:string) =
  begin
    let _ = (Unix.write fd str 0 (String.length str)) in ()
  end


    
(* ############################################################################# *)
(* ############################################################################# *)
(* ############         S P - T A B L E    F U N C T I O N S       ############# *)
(* ############################################################################# *)
(* ############################################################################# *)    


(***************************************************)
(*                                                 *)
(***************************************************)  
let rec sptAddSpNode (spt:spTable) 
    (name:string) (compinfo:compinfo) (loc:location) =
  begin
    let newSpNode = {
      spName = name;
      spCompInfo = compinfo;
      spLoc = loc;
      fpList = [];
      fpListReversed = [];
    } in
    Hashtbl.add spt name newSpNode;
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptAddFpNode (spt:spTable) (spName:string) (name:string) 
    (fieldinfo:fieldinfo) (index:int) (loc:location) =
  begin
    let spn = (sptFindSpNode spt spName) in
    let newFpNode = {
      fpName = name;
      fpFieldInfo = fieldinfo;
      fpIndex = index;
      fpLoc = loc;
      fpAssignImplList = [];
    } in
    spn.fpListReversed <- newFpNode :: spn.fpListReversed;
    (* don't forget to reverse it !!! *)
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptAddFpAssignImpl (fpNode:fpNode) (implVi:varinfo) (implName:string) (loc:location) = 
  begin
    let fpAssignImplList = fpNode.fpAssignImplList in
    
    (* I need to check if implName is already in the list   *)
    (* because some implementation e.g. generic_writepage() *)
    (* are used for different function pointers             *)
    let isInList = { bval = false } in 
    (List.iter 
       (sptFindNameInFpAssignImplList isInList implName)
       fpAssignImplList);
    
    (* if it's already in the list, move on *)
    if (isInList.bval) then () 
    else 
      begin
	let newFpAssignImpl = {
	  fpImplName = implName;
	  fpImplVi = implVi;
	  fpAssignImplLoc = loc;
	} in
	
	(* add new assignImp to the list *)
	fpNode.fpAssignImplList <- 
	  newFpAssignImpl :: fpNode.fpAssignImplList;
      end;
  end
    

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptReverseFpList (spt:spTable) (spName:string) = 
  begin
    let spn = (sptFindSpNode spt spName) in
    spn.fpList <- (List.rev spn.fpListReversed) 
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and tdptFind (tdpt:tdpTable) (tName:string) =
  (Hashtbl.find tdpt tName)

    
(***************************************************)
(*                                                 *)
(***************************************************)  
and tdptAdd (tdpt:tdpTable) (name:string) (ti:typeinfo) (loc:location) =
  begin
    let newTdpNode = {
      tName = name;
      tInfo = ti;
      tLoc = loc;
    } in
    Hashtbl.add tdpt name newTdpNode;
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptFindSpNode (spt:spTable) (spName:string) =
  (Hashtbl.find spt spName)


(***************************************************)
(*                                                 *)
(***************************************************)  
and sptFindFpNode (spn:spNode) (fpName:string) =
  begin

    let findFpNodeOffset (r:intarg) (i:intarg) (aName:string) (fpNode:fpNode) =
      begin
	if (r.ival == -1) then
	  begin
	    if ((String.compare aName fpNode.fpName) == 0) then
	      r.ival <- i.ival
	    else ();
   	    i.ival <- i.ival + 1;
	  end
	else ();
      end in 
    
    let x = { ival = -1 }  in
    let k = { ival = 0 }  in
    
    (List.iter (findFpNodeOffset x k fpName) spn.fpList);
    (List.nth spn.fpList x.ival);
    
  end


(***************************************************)
(*                                                 *)
(***************************************************)  
and sptIsFpNameExist (spt:spTable) (spName:string) (fpName:string) (found:boolarg) =
  begin
    try
      let spn = sptFindSpNode spt spName in
      let fpn = sptFindFpNode spn fpName in

      found.bval <- true;
      
    with Not_found ->
      begin
	found.bval <- false;
      end;

  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptIsFpImplExist (spt:spTable) (spName:string) (fpName:string) (found:boolarg) =
  begin
    try
      let spn = sptFindSpNode spt spName in
      let fpn = sptFindFpNode spn fpName in
      
      if ((List.length fpn.fpAssignImplList) > 0) then
	found.bval <- true
      else 
	found.bval <- false      
	    
    with Not_found ->
      begin
	found.bval <- false;
      end;

  end


(***************************************************)
(*                                                 *)
(***************************************************)  
and sptFindNameInFpAssignImplList (isFound:boolarg) (name:string) 
    (fpAssignImpl:fpAssignImpl) = 
  begin
    (* only do if we haven't found the name in list -- performance improvement *)
    if (isFound.bval == false) then
      begin
	if ((String.compare name fpAssignImpl.fpImplName) == 0) then
	  isFound.bval <- true
	else ();
      end
    else ();
  end


(***************************************************)
(*                                                 *)
(***************************************************)  
and sptIsAllFpNodeNull (isAllNull:boolarg) (fpNode:fpNode) =
  begin
    
    (* only do if we haven't seen any assignment implementation *)
    if (isAllNull.bval) then
      begin
	
	(* it's still null *)	
	if ((List.length fpNode.fpAssignImplList) == 0) then ()
	else
	  begin
	    (* we've got one that is not null *)
	    isAllNull.bval <- false;
	  end
      end
    else ();
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptPrintFpAssignImpl (k:intarg) (fpAssignImpl:fpAssignImpl) = 
  begin
    (mypf outf "\t\tAssignImpl [");
    (mypf outf (atoi3 k.ival));
    (mypf outf "]   ");
    (mypf outf (strToStr30 fpAssignImpl.fpImplName));
    (mypf outf " ");
    (mypf outf (strToStr20 fpAssignImpl.fpAssignImplLoc.file));
    (mypf outf "\n");

    k.ival <- k.ival + 1;
  end

    
(***************************************************)
(*                                                 *)
(***************************************************)  
and sptPrintFpNode (fpNode:fpNode) =
  begin
    
    (* distinguish field that has implementation and those that don't *)
    if ((List.length fpNode.fpAssignImplList) == 0) then
      (mypf outf "\tNullNd [")
    else
      (mypf outf "\tFpNode [");
    
    (mypf outf (atoi3 fpNode.fpIndex));
    (mypf outf "] ");
    (mypf outf fpNode.fpName);
    (mypf outf " : \n");
    
    let k = { ival = 1 } in
    (List.iter (sptPrintFpAssignImpl k) fpNode.fpAssignImplList); 
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptPrintSpNode (sCount:intarg) (spName:string) (spNode:spNode) =
  begin
    
    (* check if there is no single implementation for any fps or *)
    (* this structure, i.e. this structure has one or more fps   *)
    (* but within the modules we're looking at, there is no      *)
    (* assignment implementations at all                         *)
    let isAllFpNodeNull = { bval = true } in
    (List.iter (sptIsAllFpNodeNull isAllFpNodeNull) spNode.fpList);
    
    
    (* print spNode information *)
    (mypf outf "------------------------------------------------- ");
    (mypf outf ("[" ^ (atoi sCount.ival) ^ "] \n"));
    
    if (isAllFpNodeNull.bval) then
      (mypf outf ("S-Null : " ^ spNode.spName ^ "\n"))
    else
      (mypf outf ("Struct : " ^ spNode.spName ^ "\n"));


    (mypf outf ("File   : " ^ spNode.spLoc.file ^ 
		" (line " ^ (atoi spNode.spLoc.line) ^ ") \n"));
    (mypf outf ("Fields : \n"));
     
    (List.iter sptPrintFpNode spNode.fpList);
    (mypf outf "\n");
    
    sCount.ival <- sCount.ival + 1;
    
  end

(***************************************************)
(*                                                 *)
(***************************************************)  
and sptPrintAll (spt:spTable) =
  begin

    (mypf outf "*****************************************\n");
    (mypf outf " SP TABLE : \n");
    (mypf outf "*****************************************\n\n");

    let sCount = { ival = 1 } in

    (Hashtbl.iter (sptPrintSpNode sCount) spt);

    (mypf outf "*****************************************\n");
    (mypf outf " END SP TABLE  \n");
    (mypf outf "*****************************************\n\n\n");

  end
    

(* ############################################################################# *)
(* ############################################################################# *)
(* ###########          S H A R E D       I T E R A T O R S       ############## *)
(* ############################################################################# *)
(* ############################################################################# *)


(***************************************************)
(*                                                 *)
(***************************************************)
let rec checkFpStruct (spt:spTable) (foundFps:boolarg) (spName:stringarg) 
    (aName:string) =
  begin
    try 
      let spNode = (sptFindSpNode spt aName) in
      spName.sval <- spNode.spName;
      foundFps.bval <- true;		  
    with Not_found -> ();
  end 

(***************************************************)
(*                                                 *)
(***************************************************)
and checkFpType (spt:spTable) (foundFps:boolarg) (spName:stringarg) (aType:typ) =
  begin
    (match aType with

      (* recursive type *)
    | TPtr(typ,_)     -> (checkFpType spt foundFps spName typ);
    | TArray(typ,_,_) -> (checkFpType spt foundFps spName typ);
    | TNamed(ti,_)    -> (checkFpType spt foundFps spName ti.ttype);
	
	(* got compinfo *)
    | TComp(ci,_) -> (checkFpStruct spt foundFps spName ci.cname);
    | _ -> ());
  end 

(***************************************************)   
(* fcomp:compinfo, fname:string                    *)
(* ftype:typ, floc:location                        *)
(***************************************************)
and traverseIsFieldInfoAnFp (spt:spTable) (tdpt:tdpTable)
    (isFieldAnFp:boolarg) (fi:fieldinfo) = 
  begin
    (match fi.ftype with
    | TPtr(typ,_) ->
	begin 
	  (match typ with 
	  | TNamed(ti,_) ->
	      begin
		try
		  let n = (tdptFind tdpt ti.tname) in
		  (* we found it *)

		  (* (mypf outf ("Found " ^ ti.tname ^ "\n")); *)
		  isFieldAnFp.bval <- true;

		(* if we can't find the typedef, then ignore *)
		with Not_found -> 
		  begin
		  end

	      end
	  | TFun(_,_,_,_) -> 
	      begin
		(*********************************************************)
		(* This field is a function pointer!                     *)
		(* we need to tell this to the caller so that the caller *)
		(* could store each field information for this structure *)
		(*********************************************************)
		(* (mypf outf ("" ^ fi.floc.file ^ "\t" ^  *)
		(* fi.fcomp.cname ^ "\t" ^ fi.fname ^ "\n")); *)
		isFieldAnFp.bval <- true;
	      end
	  | _ -> ());
	end
    | _ -> ());
  end
    


(***************************************************)
(* ...->...->i_op->fname                           *)
(* This visitor will get the last field, which     *)
(* will be the fp field name                       *)
(***************************************************)
class expvGetFpName (spt:spTable) (foundFpName:boolarg) (fpName:stringarg) = object
  inherit nopCilVisitor

  method voffs (offset:offset) =
    begin
      (match offset with
        Field(fi,_) ->
          begin
	    foundFpName.bval <- true;
            fpName.sval <- fi.fname;
          end
      | _ -> ());
      DoChildren
    end
end
    


(********************************************************************)
(* There are many ways a function pointer is used  to make a call.  *)
(* A function pointer call is basically a form of (exp),            *)
(* Thus, what we have to do is to use expVisitor to visit each      *)
(* element (e.g. offsets, lvals, exp, etc) and try to find the      *)
(*                                                                  *)
(* Here is a list of how a function pointer is called:              *)
(*   1. s_op->f();                 (soln: LvalV)                    *)
(*   2. s_op.f();                                                   *)
(*   3. something->s_op->f();      (soln: OffsetV)                  *)
(*   4. something->s_op.f();                                        *)
(*   5. ((s_op_t * ) s)->f();      (soln: CastE at ExpV)            *)
(*   6. ((struct s_op * ) s)->f();                                  *)
(*   7. s_op[i]->f                 (soln: TArray at LvalV)          *)
(*   8. something->s_op[i]->f      (soln: TArray at OffsetV)        *)
(*                                                                  *)
(*                                                                  *)
(********************************************************************)
class expvGetSpName (spt:spTable) (spName:stringarg) (foundSpName:boolarg) = object
  inherit nopCilVisitor

     
  method voffs (offset:offset) =
    begin
      (match offset with
      | Field(fi,_) -> (checkFpType spt foundSpName spName fi.ftype);
      | _ -> ());
      DoChildren
    end

  method vlval (lval:lval) =
    begin
      (match lval with
      | (Var(vi),offset) -> (checkFpType spt foundSpName spName vi.vtype);
      | _ -> ());
      DoChildren
    end
      
  method vexpr (exp:exp) =
    begin
      (match exp with
      | CastE(castType,_) -> (checkFpType spt foundSpName spName castType);
      | _ -> ());
      DoChildren
    end
      
end



(***************************************************)
(* ...->...->i_op->fname                           *)
(* This visitor will get the last field, which     *)
(* will be the fp field name                       *)
(***************************************************)
class lvalvGetFpName (spt:spTable) (tdpt:tdpTable)
    (fpName:stringarg) (foundFpName:boolarg) = object
  inherit nopCilVisitor

  method voffs (offset:offset) =
    begin
      (match offset with
        Field(fi,_) ->
          begin
	    let isFieldAnFp = { bval = false } in
	    (traverseIsFieldInfoAnFp spt tdpt isFieldAnFp fi);
	    if (isFieldAnFp.bval) then
	      begin
		  foundFpName.bval <- true;
		  fpName.sval <- fi.fname;
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
class lvalvGetSpName (spt:spTable) (spName:stringarg) (foundSpName:boolarg) = object
  inherit nopCilVisitor
      
  method voffs (offset:offset) =
    begin
      (match offset with
      | Field(fi,_) -> (checkFpType spt foundSpName spName fi.ftype);
      | _ -> ());
      DoChildren
    end

  method vlval (lval:lval) =
    begin
      (match lval with
      | (Var(vi),offset) -> (checkFpType spt foundSpName spName vi.vtype);
      | _ -> ());
      DoChildren
    end
      
  method vexpr (exp:exp) =
    begin
      (match exp with
      | CastE(castType,_) -> (checkFpType spt foundSpName spName castType);
      | _ -> ());
      DoChildren
    end
      
end






(* ############################################################################# *)
(* ############################################################################# *)
(* ###########          P H A S E  1      I T E R A T O R S       ############## *)
(* ############################################################################# *)
(* ############################################################################# *)



(***************************************************)   
(*                                                 *)
(***************************************************)
let rec traverseFieldInfoToCreateFpList (spt:spTable)
    (spName:string) (k:intarg) (fi:fieldinfo) = 
  begin
    (*****************************************)
    (* Information that needs to be stored:  *)
    (*   fi.fname                            *)
    (*   fi itself                           *)
    (*   fi.floc.file                        *)
    (*   fi.floc.line                        *)
    (*****************************************)
    (sptAddFpNode spt spName fi.fname fi k.ival fi.floc);
    k.ival <- k.ival + 1;
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and traverseFieldInfoListToCreateFpList (spt:spTable) 
    (spName:string) (cfields:fieldinfo list) =
  begin
    let k = { ival = 0 } in
    List.iter (traverseFieldInfoToCreateFpList spt spName k) cfields;    
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and traverseFieldInfoListToFindAnFp (spt:spTable) (tdpt:tdpTable)
    (isStructHasFp:boolarg) (cfields:fieldinfo list) =
  begin
    let isFieldAnFp = { bval = false } in
    List.iter (traverseIsFieldInfoAnFp spt tdpt isFieldAnFp) cfields;
    isStructHasFp.bval <- isFieldAnFp.bval;
  end

(***************************************************)   
(*                                                 *)
(***************************************************)
and traverseGCompTag (spt:spTable) (tdpt:tdpTable)
    (compinfo:compinfo) (loc:location) =
  begin
    let isStructHasFp = { bval = false } in
    let spName = compinfo.cname in 

    (* first, find out if the structure has a function pointer *)
    (traverseFieldInfoListToFindAnFp spt tdpt isStructHasFp compinfo.cfields);
    
    if (isStructHasFp.bval) then
	begin
	  (* the structure has fp, so we build an spNode here *)
	  (sptAddSpNode spt spName compinfo loc);
	  
	  (* now we traverse the list again and insert the *)
	  (* field info into the new spNode *)
	  let spNode = (sptFindSpNode spt compinfo.cname) in
	  (traverseFieldInfoListToCreateFpList spt spName compinfo.cfields);

	  (* after fp list created, don't forget to reverse it *) 
	  (sptReverseFpList spt spName);
	end
    else ();
	
  end
   
(***************************************************)   
(*                                                 *)
(***************************************************)
and traverseGType (tdpt:tdpTable) (ti:typeinfo) (loc:location) =
  begin
    (match ti.ttype with
      TFun(_,_,_,_) ->
	begin
	  (* this typedef is a function pointer, let's add to the hash table *)
	  (tdptAdd tdpt ti.tname ti loc);
	  (* (mypf outf ("Add " ^ ti.tname ^ "\n")); *)
	end
    | _ -> ());
  end




(* ############################################################################# *)
(* ############################################################################# *)
(* ###########          P H A S E  2      I T E R A T O R S       ############## *)
(* ############################################################################# *)
(* ############################################################################# *)

let rec notUsed (x:int) = 
  begin
  end


(***************************************************)
(* handle:                                         *)
(*   1. ..->i_op->read = ext2_read                 *)
(*   2. ..->i_op->read = (read_t) dumb_read        *)
(*   3. ..->i_op->read = f_op->read                *)
(***************************************************)
and traverseExpToGetFpImpl (spt:spTable) (fpNode:fpNode) (loc:location) 
    (exp:exp) =
  begin
    (* only reach here if spNode and fpNode are valid *)
    (match exp with

      (* case 1 and 2 *)
    | (CastE(_,AddrOf(Var(vi),_))) -> (sptAddFpAssignImpl fpNode vi vi.vname loc);
    | (AddrOf(Var(vi),_))          -> (sptAddFpAssignImpl fpNode vi vi.vname loc);
    | _ -> ());
  end
    
(***************************************************)
(* handle:                                         *)
(*   3. ..->i_op->read = f_op->read                *)
(***************************************************)
and traverseExpToGetSharedFpImpl (spt:spTable) (fpNode:fpNode) 
    (loc:location) (exp:exp) =
  begin
    (match exp with
    | (CastE(_,AddrOf(Var(vi),_))) -> ();
    | (AddrOf(Var(vi),_))          -> ();
    | _ -> 
	begin
	  let spName = { sval = "" } in
	  let fpName = { sval = "" } in
	  let foundSpName = { bval = false } in
	  let foundFpName = { bval = false } in
	  let _ = visitCilExpr 
	      (new expvGetSpName spt spName foundSpName) exp in ();
	  
	  (* found, so we get the field name, e.g. write/unlink/etc. *)
	  if (foundSpName.bval) then
	    begin
	      let _ = visitCilExpr (new expvGetFpName spt foundFpName fpName) exp in ();
	      
	      if (foundFpName.bval) then
		(* debug *)
		(* (mypf outf (spName.sval ^ " / " ^ fpName.sval ^ " "));  *)
		
		try
		  let spn2 = sptFindSpNode spt spName.sval in
		  let fpn2 = sptFindFpNode spn2 fpName.sval in
		  
		  if ((List.length fpNode.fpAssignImplList) == 0) then
		    begin
		      (* x_op->write = y_op->write                *)
		      (* in this case x_op->write impl list is 0  *)
		      (* so we just connect x_op->write impl list *)
		      (* y_op->write impl list                    *)
		      fpNode.fpAssignImplList <- fpn2.fpAssignImplList;
		      
		    end
		  else ();
		with Not_found -> ();
	      else ();
	    end
	  else ();
	end
    );
  end
    

(***************************************************)
(* each init, is an assignment                     *)
(* init can be in the form of                      *)
(*   .write = ext2_write      (AddrOf(ext2_write)) *)
(*   .write = (write_t) nop   (CastE(.. ))         *)
(*                                                 *)
(*                                                 *)
(***************************************************)
and traverseOffsetInit (spt:spTable) (spNode:spNode) (loc:location)
    (k:intarg) (offsetinit:offset*init) =
  begin
    (match offsetinit with
      (offset,init) ->
        begin
          (match init with
	  | SingleInit(initExp) ->
              begin
		let fpNode = List.nth spNode.fpList k.ival in
		(traverseExpToGetFpImpl spt fpNode loc initExp);
              end
          | _ -> ());
        end
    );
    k.ival <- k.ival + 1;
  end
    

(***************************************************)
(* offsetinitlist contains the fp assignments      *)
(* traverseFpAssignment will parse each assignment  *)
(***************************************************)
and traverseOffsetInitList (spt:spTable) (spNode:spNode) (loc:location)
    (offsetinitlist:(offset*init)list) =
  begin
    let k = { ival = 0 } in
    List.iter (traverseOffsetInit spt spNode loc k) offsetinitlist;
  end


(***************************************************)
(* I only see the offsetinit list, if the type of  *)
(* CompoundInit is equal to TComp(compinfo) and    *)
(* compinfo.cname is in the table                  *)
(* I handle 2 cases:                               *)
(*   where initType is "struct inode_op" (TComp)   *)
(*     and initType is "inode_op_t"      (TNamed)  *)
(***************************************************)
class globalvIterateInit (spt:spTable) (loc:location) = object
  inherit nopCilVisitor
      

  method vinit (vi:varinfo) (off:offset) (init:init) =
    begin
      (match init with
      | CompoundInit(TComp(ci,_),offsetinitlist) ->
	  begin
	    try
	      let spNode = (sptFindSpNode spt ci.cname) in
	      (traverseOffsetInitList spt spNode loc offsetinitlist);
	    with Not_found -> ();
	  end
      | CompoundInit(TNamed(ti,_),offsetinitlist) ->
	  begin
	    (match ti.ttype with
	    | TComp(ci,_) -> 
		begin
		  try
		    let spNode = (sptFindSpNode spt ci.cname) in
		    (traverseOffsetInitList spt spNode loc offsetinitlist);		  
		  with Not_found -> ();
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
let traverseGVar (spt:spTable) (g:global) (loc:location) =
  begin
    let _ = visitCilGlobal (new globalvIterateInit spt loc) g in ();
  end



(***************************************************)
(* This traverse the instruction and try to get the *)
(* spName, fpName, and implName                    *)
(* for example:                                    *)
(*  ...->i_op->write = ext2_write                  *)
(* spName = inode_operations                       *)
(* fpName = write                                  *)
(* implName = ext2_write                           *)
(***************************************************)
let traverseInstrToFindFpAssignments (phase:int) (spt:spTable) (tdpt:tdpTable) (instr:instr) =
  begin
    (match instr with
    | Set(lval,exp,loc) -> 
	begin
	  let spName = { sval = "" } in
	  let fpName = { sval = "" } in
	  let foundSpName = { bval = false } in
	  let _ = visitCilLval 
	      (new lvalvGetSpName spt spName foundSpName) lval in ();
	  
	  (* found, so we get the field name, e.g. write/unlink/etc. *)
	  if (foundSpName.bval) then
	    begin
	      let foundFpName = { bval = false } in
	      let _ = visitCilLval 
		  (new lvalvGetFpName spt tdpt fpName foundFpName) lval in ();
	      
	      if (foundFpName.bval) then
		begin
		  try 
		    (* now we need to get spNode *)
		    let spNode = (sptFindSpNode spt spName.sval) in
		    let fpNode = (sptFindFpNode spNode fpName.sval) in
		    
		    (* now see the exp, and traverse the exp *)

		    if (phase == 1) then
		      (traverseExpToGetFpImpl spt fpNode loc exp)
		    else if (phase == 2) then
		      (traverseExpToGetSharedFpImpl spt fpNode loc exp)		      
		    else ();
		    
		  with Not_found -> ();
		end
	      else ();
	    end
	  else ();
	end
    | _ -> ());
  end

(***************************************************)
(* function node visitor                           *)
(***************************************************)
class fnvIterFuncFindFpAssignments (ph:int) (spt:spTable) (tdpt:tdpTable)
  (fd:fundec) (loc:Cil.location) = object
  inherit nopCilVisitor
      
   (* visit an instruction; we're only interested in calls *)
  method vinst (i:instr) : instr list visitAction =
    begin
      (traverseInstrToFindFpAssignments ph spt tdpt i);
      DoChildren
    end
end

(***************************************************)
(*                                                 *)
(***************************************************)
let traverseFuncFindFpAssignments (ph:int) (spt:spTable) (tdpt:tdpTable)
    (fi: Cil.file) (fd: fundec) (loc: Cil.location) =
  begin
    let _ = visitCilFunction 
	(new fnvIterFuncFindFpAssignments ph spt tdpt fd loc) 
	fd in ();
  end





(* ############################################################################# *)
(* ############################################################################# *)
(* ###########          P H A S E  3      I T E R A T O R S       ############## *)
(* ############################################################################# *)
(* ############################################################################# *)


(***************************************************)
(* function node visitor                           *)
(***************************************************)
class fnvTraverseToAssignStmtNumbers (k:intarg) = object
   inherit nopCilVisitor

   (* visit an instruction; we're only interested in calls *)
  method vstmt (s:stmt) : stmt  visitAction =
    begin
      s.sid <- k.ival;
      k.ival <- k.ival + 1;
      DoChildren
    end
end


(***************************************************)
(*                                                 *)
(***************************************************)
let rec traverseFuncCallLhost (spt:spTable) (lhost:lhost) 
    (isFuncPointer:boolarg) =
  begin
    (match lhost with
    | Var(_) -> ()
    | Mem(_) ->
        begin
          isFuncPointer.bval <- true
        end
    );
  end

(***************************************************)
(*                                                 *)
(***************************************************)
and traverseFuncCallExp (spt:spTable) (isFuncPointer:boolarg) (exp:exp) =
  begin
    (match exp with
    | Lval(lhost,offset) ->
        begin
          (traverseFuncCallLhost spt lhost isFuncPointer);
        end
    | _ -> ());
  end





(***************************************************)
(*                                                 *)
(***************************************************)
and traverseInstrToMatchFpInstr (fpInstr:instr) (foundFpInstr:boolarg) (instr:instr) =
  begin
    if (fpInstr == instr) then
      begin
	foundFpInstr.bval <- true;
      end
    else ();
  end

(***************************************************)
(*                                                 *)
(***************************************************)
and traverseStmtToGetSFP (fpInstr:instr) (foundSFP:boolarg)
    (sFP:stmtarg) (stmt:stmt) = 
  begin
    if (foundSFP.bval == false) then
      begin
	
	(match stmt.skind with 
	| Instr(instrlist) -> 
	    begin
	      let foundFpInstr = { bval = false } in
	      (List.iter (traverseInstrToMatchFpInstr fpInstr foundFpInstr) instrlist);
	      if (foundFpInstr.bval == true) then
		begin
		  foundSFP.bval <- true;
		  sFP.stmtval <- Some(stmt);
		end
	      else ();
	    end
	| _ -> ());
	
      end
    else ();
  end
    

(***************************************************)
(* function node visitor                           *)
(***************************************************)
class fnvTraverseToGetBFPAndStmt (fpInstr:instr) (foundBFP:boolarg) (bFP:blockarg) (foundSFP:boolarg) (sFP:stmtarg) = object 
  inherit nopCilVisitor
      
      (* visit an instruction; we're only interested in calls *)
  method vblock (block:block) : block visitAction =
    begin
      if (foundBFP.bval == false) then
	begin
	  
	  (List.iter 
	     (traverseStmtToGetSFP fpInstr foundSFP sFP)
	     block.bstmts);
	  
	  if (foundSFP.bval == true) then
	    begin
	      foundBFP.bval <- true;
	      bFP.blockval <- Some(block);
	    end
	  else
	    ();
	end
      else ();
      DoChildren
    end
end

let rec testMytestMy (x:int) = 
  begin
  end
    

(***************************************************)
(* sFP contains instr list ... i1, i2, i3, i4, i5  *)
(* if i3 == fpInstr, then                          *)
(* sA will only contains i1 and i2                 *)
(***************************************************)
and createStmtA (fpInstr:instr) (sA:stmtarg) (sFP:stmtarg) =
  begin

    let instrListSA = { instrlistval = [] } in
    let keepFilling = { bval = true } in
    
    let fillInstrListForSA (instr:instr) =
      begin
	if (instr == fpInstr) then
	  keepFilling.bval <- false
	else ();
	if (keepFilling.bval) then
	  begin
            instrListSA.instrlistval <- instr :: instrListSA.instrlistval;
	  end
	else ();
      end in
    
    (match sFP.stmtval with
    | Some(stmtSFP) ->
        begin
          (match stmtSFP.skind with
          | Instr(instrlist) ->
              begin
                (List.iter (fillInstrListForSA) instrlist);
		
                let stmtSA = {
                  labels = []; 
                  skind = Instr(List.rev instrListSA.instrlistval);
                  sid = 1021;
                  succs = [];
                  preds = [];
                } in
                sA.stmtval <- Some(stmtSA);
		
              end
          | _ -> ());
        end
    | None -> ());

  end

(***************************************************)
(* sFP contains instr list ... i1, i2, i3, i4, i5  *)
(* if i3 == fpInstr, then                          *)
(* sC will only contains i4 and i5                 *)
(***************************************************)
and createStmtC (fpInstr:instr) (sC:stmtarg) (sFP:stmtarg) =
  begin
    let instrListSC = { instrlistval = [] } in
    let startFilling = { bval = false } in
    
    let fillInstrListForSC (instr:instr) =
      begin
	if (startFilling.bval) then
	  begin
            instrListSC.instrlistval <- instr :: instrListSC.instrlistval;
	  end
	else ();
	if (instr == fpInstr) then
	  startFilling.bval <- true
	else ();
      end in
    
    (match sFP.stmtval with
    | Some(stmtSFP) ->
        begin
          (match stmtSFP.skind with
          | Instr(instrlist) ->
              begin
                (List.iter (fillInstrListForSC) instrlist);
		
                let stmtSC = {
                  labels = []; 
                  skind = Instr(List.rev instrListSC.instrlistval);
                  sid = 1021;
                  succs = [];
                  preds = [];
                } in
                sC.stmtval <- Some(stmtSC);
		
              end
          | _ -> ());
        end
    | None -> ());
    
  end

(***************************************************)
(*                                                 *)
(***************************************************)
and debugStmt (stmtstr:string) (sarg:stmtarg) =
  begin
    let dbg = true in 
    if (dbg) then
      begin
	(mypf outf "\n====================================== ");
	(mypf outf (stmtstr ^ "\n"));
	(match sarg.stmtval with
	| Some(stmt) -> (mypf outf (stmtToStr stmt));
	| None -> ());
	(mypf outf "\n");
      end
    else ();
  end
    
(***************************************************)
(*                                                 *)
(***************************************************)
and debugBlock (blockstr:string) (blkarg:blockarg) =
  begin
    let dbg = true in 
    if (dbg) then
      begin
	(mypf outf "\n====================================== ");
	(mypf outf (blockstr ^ "\n"));
	(match blkarg.blockval with
	| Some(blk) -> (mypf outf (blockToStr blk));
	| None -> ());
	(mypf outf "\n");
      end
    else ();
  end
    

(***************************************************)
(*                                                 *)
(***************************************************)
and createNewBlock (newBlock:blockarg) (sA:stmtarg) (sB:stmtarg) (sC:stmtarg) =
  begin
    (match sA.stmtval with
    | Some (stmtSA) ->
	(match sB.stmtval with
	| Some (stmtSB) ->
	    (match sC.stmtval with
	    | Some (stmtSC) ->
		begin
		  let aBlock = {
		    battrs = [];
		    bstmts = [ stmtSA ; stmtSB ; stmtSC ];
		  } in
		  newBlock.blockval <- Some(aBlock);
		end
	    | None -> ());
	| None -> ());
    | None -> ());
  end

(***************************************************)
(*                                                 *)
(***************************************************)
and attachNewBlockToSFP (sFP:stmtarg) (bNew:blockarg) =
  begin
    (match sFP.stmtval with
    | Some(stmtSFP) ->
	(match bNew.blockval with
	| Some (blockNew) ->
	    begin
	      stmtSFP.skind <- Block(blockNew);
	    end
	| None -> ());
    | None -> ());
  end


(***************************************************)
(* basically what we need to do is to change:      *)
(* sFp --> sNew --> sA, sB, sC                     *)
(***************************************************)
and traverseFunDecToInsertSwitchStmt (spt:spTable) (fd:fundec) 
    (fpInstr:instr) (newSwitchStmt:stmt) =
  begin
    (* number all stmts first *)
    let k = { ival = 1 } in
    let _ = visitCilFunction (new fnvTraverseToAssignStmtNumbers k) fd in ();
    
    let bFP = { blockval = None } in
    let sFP = { stmtval = None } in
    let foundBFP = { bval = false } in
    let foundSFP = { bval = false } in
    let _ = visitCilFunction 
	(new fnvTraverseToGetBFPAndStmt fpInstr foundBFP bFP foundSFP sFP) 
	fd in ();
    
    (* at this point we know the block and the *)
    (* statment      that contains the fp      *)
    (* Now, it's time to create sA, sC         *)
    (* put sA, sB, sc in a newBlock            *)
    (* and change sFP to become                *)
    (* Block(newBlock)                         *)

    let sA  = { stmtval = None } in
    let sB  = { stmtval = Some(newSwitchStmt) } in
    let sC  = { stmtval = None } in
    (createStmtA fpInstr sA sFP);
    (createStmtC fpInstr sC sFP); 

    let bNew = { blockval = None } in
    (createNewBlock bNew sA sB sC);
    
    (* (debugBlock "bFP" bFP); *)
    (* (debugStmt  "sFP" sFP); *)
    (* (debugStmt  "sA"  sA); *)
    (* (debugStmt  "sB"  sB); *)
    (* (debugStmt  "sC"  sC); *)
    (* (debugBlock "bNew" bNew); *)
    

    (attachNewBlockToSFP sFP bNew);

    (* (debugStmt  "sFP After" sFP);  *)
    (* (debugBlock "bFP After" bFP); *)

  end

(* abc *)

(***************************************************)
(*                                                 *)
(***************************************************)
and insertFpImpl (k:intarg) (retvalArg:lvalarg) (arglist:exp list) 
    (loc:location) (stmtListArg:stmtlistarg) (fpAssignImpl:fpAssignImpl) = 
  begin
    
    let mkInstrCall (retvalArg) (leftExp) (arglist) (loc:location) =
      begin
	(match retvalArg.lvalval with
	| None -> Call(None,leftExp,arglist,loc)
	| Some(lval) -> Call(Some(lval),leftExp,arglist,loc)
	);
      end
    in
	
    (* create an instr *)
    let offset = NoOffset in
    let lhost = Var(fpAssignImpl.fpImplVi) in
    let lval = (lhost,offset) in
    let leftExp = Lval(lval) in
    let caseInstr = (mkInstrCall retvalArg leftExp arglist loc) in
    let caseInstrList = caseInstr :: [] in

    (* building the case statement cs *)
    (* stmt - s.sid = -1 *)
    (*      - s.labels - label - Case(exp) - exp - Const *)
    let caseExp = Const(CInt64((Int64.of_int k.ival),IInt,None)) in
    let csId = (1000 + k.ival) in
    let csLabel = Case(caseExp,loc) in
    let csLabels = csLabel :: [] in
    let csSuccs = [] in
    let csPreds = [] in
    let csKind = Instr(caseInstrList) in

    let caseStmt = {
      labels = csLabels;
      skind = csKind;
      sid = csId;
      succs = csSuccs;
      preds = csPreds;
    } in
    
    (* add the statement to the list *)
    stmtListArg.stmtlistval <- caseStmt :: stmtListArg.stmtlistval;
    
    k.ival <- k.ival + 1;
    
  end




(***************************************************)
(*                                                 *)
(***************************************************)
and insertFpImpls (spt:spTable) (fd:fundec) (fpInstr:instr) (retvalArg:lvalarg) 
    (arglist:exp list) (loc:location)
    (spName:string) (fpName:string) =
  begin
    let spn = sptFindSpNode spt spName in
    let fpn = sptFindFpNode spn fpName in

    let rec insertBreaks calls =
        if List.length calls <> 0 then
           (hd calls)::(mkStmt (Break(locUnknown)))::(insertBreaks (tl calls))
        else
        [] 
    in 

    let switchStmtListArg = { stmtlistval = [] } in
    
    let k = { ival = 1 } in

    (List.iter 
       (insertFpImpl k retvalArg arglist loc switchStmtListArg) 
       fpn.fpAssignImplList);    
    
    (* stmt list is created, now create the block, and switch stmt *)
    let switchExp = Const(CInt64((Int64.of_int 1234),IInt,None)) in
    let switchStmtList = (List.rev switchStmtListArg.stmtlistval) in
    let switchBlock = {
      bstmts = insertBreaks switchStmtList; (*modified*)
      battrs = [];
    } in

    let ssKind = Switch(switchExp,switchBlock,switchStmtList,loc) in
    let ssId = 1000 in
    let ssLabels = [] in
    let ssSuccs = [] in
    let ssPreds = [] in
    
    let switchStmt = {    
      labels = ssLabels;
      skind = ssKind;
      sid = ssId;
      succs = ssSuccs;
      preds = ssPreds;      
    } in
    
    
    let dbg = false in
    if (dbg) then
      begin
	(mypf outf "----------------------------------------------------\n");
	(iterateStmt outf 2 switchStmt);
	(mypf outf "----------------------------------------------------\n");    
      end
    else ();
    

    (* at this point the switch statment is ready *)
    (* basically what we need to do is to change: *)
    (traverseFunDecToInsertSwitchStmt spt fd fpInstr switchStmt);
    
  end


(***************************************************)
(*                                                 *)
(***************************************************)
and printFuncPointerInfo 
    (spt:spTable) (count:intarg) (instr:instr) (loc:location)
    (foundSpName:boolarg) (spName:stringarg) (fpName:stringarg)
    (isFpNameExist:boolarg) (isFpImplExist:boolarg) = 
  begin
    
    (mypf outf ("--------------------------------------------------\n"));
    (mypf outf ("Function Pointer #" ^ (atoi count.ival) ^ ": \n"));
    (mypf outf ("\tInst : " ^ (instrToStr instr) ^ "\n"));
    (* (mypf outf ("\tFile : " ^ loc.file ^ "\n"));*)
    (* (mypf outf ("\tLine : " ^ (atoi loc.line) ^ "\n"));*)
    (mypf outf ("\tType : " ));

    if (foundSpName.bval) then
      begin
	(* okay print now *)
	(mypf outf (spName.sval ^ " / " ^ fpName.sval ^ " "));


	(* we assume that once spName is found, then we will have fpName too *)
	(* the idea is, it is impossible to have a function pointer call     *)
	(* in this form:  ( * inode_op)() .. it must be (inode_op->fName)    *)
	if (isFpNameExist.bval == false) then
	  (mypf outf "WEIRD ") (* should never happen *)
	else
	  if (isFpImplExist.bval == false) then
	    (* (mypf outf ("NO IMPL " ^ (atoi count.ival)))*)
	    (mypf outf ("NO IMPL "))
	  else ();
      end
	
    else
      begin
	(mypf outf "NOT SUPPORTED");
	(mypf outf "| ");
	(mypf outf (spName.sval ^ " / " ^ fpName.sval));		  
	(mypf outf "| ");
	(* (mypf outf (instrToStr instr));*)
      end;      

    (mypf outf (" [ " ^ loc.file ^ " - ")); 
    (mypf outf ("" ^ (atoi loc.line) ^ " ] \n"));
    
    (mypf outf "\n");
  end


(***************************************************)
(*                                                 *)
(***************************************************)
and analyzeFuncPointerInstr (spt:spTable) (fd:fundec) (count:intarg) (instr:instr) =
  begin
    
    let retvalArg = { lvalval = None } in
    (match instr with
    | Call(None,exp,argexplist,loc) -> retvalArg.lvalval <- None;
    | Call(Some(retLval),exp,argexplist,loc) -> retvalArg.lvalval <- Some(retLval)
    | _ -> ());
    
    (match instr with
    | Call(_,exp,argexplist,loc) ->
        begin
	  
	  (* first see if we have an fpStruct (e.g. inode_operations) *)
	  let spName = { sval = "" } 
	  and fpName = { sval = "" } 
	  and foundSpName = { bval = false } 
	  and foundFpName = { bval = false } 
	  and isFpNameExist = { bval = false } 
	  and isFpImplExist = { bval = false } in
	  
	  
	  let _ = visitCilExpr 
	      (new expvGetSpName spt spName foundSpName) exp in ();
	  
	  (* found, so we get the field name, e.g. write/unlink/etc. *)
	  if (foundSpName.bval) then
	    begin
	      let _ = visitCilExpr (new expvGetFpName spt foundFpName fpName) exp in ();
	      
	      (* let's double check if we have the impls *)
	      
	      (sptIsFpNameExist spt spName.sval fpName.sval isFpNameExist);
	      (sptIsFpImplExist spt spName.sval fpName.sval isFpImplExist);
	      
	      
	      if (isFpNameExist.bval && isFpImplExist.bval) then
		begin
		  (* (insertFpImpls spt instr argexplist spName.sval fpName.sval); *)
		  (insertFpImpls spt fd instr retvalArg argexplist 
		     loc spName.sval fpName.sval);
		end
	      else ();
	      
	    end
	  else ();
	  
	  (printFuncPointerInfo spt count instr loc 
	     foundSpName spName fpName
	     isFpNameExist isFpImplExist);

	end
    | _ -> ());
  end

(***************************************************)
(*                                                 *)
(***************************************************)
and traverseInstr (spt:spTable) (fd:fundec) (count:intarg) (instr:instr) =
  begin
    (match instr with
    | Call(_,exp,argexplist,loc) ->
        begin
          let isFuncPointer = { bval = false } in
          (traverseFuncCallExp spt isFuncPointer exp);

	  (* if it's a function pointer *)
          if (isFuncPointer.bval) then
            begin

	      (analyzeFuncPointerInstr spt fd count instr);
              count.ival <- count.ival + 1;
            end
        end
    | _ -> ());
  end


(***************************************************)
(* function node visitor                           *)
(***************************************************)
class fnvIterate (spt:spTable) (fd:fundec) (loc:Cil.location)
    (count:intarg) = object
   inherit nopCilVisitor

   (* visit an instruction; we're only interested in calls *)
  method vinst (i:instr) : instr list visitAction =
    begin
      (traverseInstr spt fd count i);
      DoChildren
    end
end


(***************************************************)
(*                                                 *)
(***************************************************)
let traverseFunction (spt:spTable) (fi: Cil.file) (fd: fundec)
    (loc: Cil.location) (count:intarg) =
  begin
    let _ = visitCilFunction (new fnvIterate spt fd loc count) fd in ();
  end
    


(* ############################################################################# *)
(* ############################################################################# *)
(* ###########               P  H  A  S  E  S                     ############## *)
(* ############################################################################# *)
(* ############################################################################# *)



(***************************************************)   
(* PHASE 1: Find all structures that have function *)
(*          pointers                               *)
(***************************************************)
let phaseCfpOne (fi:Cil.file) (spt:spTable) (tdpt:tdpTable) =
  begin

    (*****************************************)
    (* phase 1a: first you go throuh all     *)
    (* typedefs, to build tpt                *)
    (*****************************************)
    (*(fprintf stderr "Phase 1 (a) ...\n");*)
    iterGlobals fi (
    fun g -> match g with 
    | GType(ti,loc) -> (traverseGType tdpt ti loc);
    |_ -> ());


    (*****************************************)
    (* phase 1b: first you go throuh all     *)
    (* struct definitions                    *)
    (*****************************************)    
    (*(fprintf stderr "Phase 1 (b) ...\n");*)

    iterGlobals fi (
    fun g -> match g with 
    | GCompTag(compinfo,loc) -> (traverseGCompTag spt tdpt compinfo loc);
    |_ -> ());
  end


(***************************************************)   
(* PHASE 2: Get func-pointer assignments and then  *)
(*          map fp implementations with the        *)
(*          corresponding fps                      *)
(* Example: struct inode_op ext2_inode_op {        *)
(*            .write = ext2_write;                 *)
(*            .read  = ext2_read;                  *)
(*          }                                      *)
(* We'll map ext2_write to inode_op->write         *)
(***************************************************)
let phaseCfpTwo (fi:Cil.file) (spt:spTable) (tdpt:tdpTable) =
  begin

    (*****************************************)
    (* get structure instances               *)
    (* e.g. struct inode_operations ext2_ops *)
    (*****************************************)
    (*(fprintf stderr "Phase 2 (a) ...\n");*)
    iterGlobals fi (
    fun g -> match g with
    | GVar(vi,initinfo,loc) -> (traverseGVar spt g loc);
    |_ -> ());

    
    (* at this point all fp assignments that are initialized via  *)
    (* systematic structure has been allocated ... the next thing *)
    (* is to go to each instruction and get the function impl     *)

    (*****************************************)
    (* get fp assignment in instructions     *)
    (* e.g.  ..->i_op->write = ext2_write;   *)
    (*****************************************)
    (*(fprintf stderr "Phase 2 (b) ...\n");*)
    let subphase = 1 in
    iterGlobals fi (
    fun g -> match g with
      GFun(fd,loc) -> (traverseFuncFindFpAssignments subphase spt tdpt fi fd loc);
    | _ -> ());

    (*****************************************)
    (* get shared fp assignment              *)
    (* ..->i_op->write = ..->f_op->write     *)
    (*****************************************)
    (*(fprintf stderr "Phase 2 (c) ...\n");*)
    let subphase = 2 in
    iterGlobals fi (
    fun g -> match g with
      GFun(fd,loc) -> (traverseFuncFindFpAssignments subphase spt tdpt fi fd loc);
    | _ -> ());
    
    
  end


(***************************************************)
(* PHASE 3: Insert non-fp calls at every fp call   *)    
(***************************************************)
let phaseCfpThree (fi:Cil.file) (spt:spTable) (tdpt:tdpTable) =
  begin

    (*(fprintf stderr "Phase 3 ...\n");*)

    let count = { ival = 1 } in
    
    iterGlobals fi (
    fun g -> match g with
      (* I only traverse over functions, others could be *)
      (* GVar, GEnumTag ... *)
      GFun(fd,loc) -> (traverseFunction spt fi fd loc count);
    | _ -> ());
    
  end


(***************************************************)   
(*                                                 *)
(***************************************************)
let connectFunctionPointers (fi:Cil.file) =
  begin
    
    let spt = Hashtbl.create 511 in
    let tdpt = Hashtbl.create 511 in
    
    (* PHASE 1: Find all structures that have function pointers *)
    (phaseCfpOne fi spt tdpt);

    (* PHASE 2: Map fp implementations with the corresponding fps *)
    (phaseCfpTwo fi spt tdpt);

    (* Print *)
    (sptPrintAll spt);
    
    (* PHASE 3: Insert non-fp calls at every fp call *)    
    (phaseCfpThree fi spt tdpt);


  end
    


(* ############################################################################# *)
(* ############################################################################# *)
(* ###########              M  A  I  N                            ############## *)
(* ############################################################################# *)
(* ############################################################################# *)

    
    
(***************************************************)   
(* main.main                                       *)
(***************************************************)
let mainConnectFunctionPointers (fi:Cil.file) =
  begin
    (connectFunctionPointers fi);

  end
    


(* ********************************************************** *)
let doConnectFunctionPointers = ref false

       
(* ********************************************************** *)
let feature : featureDescr =
  { fd_name = "connectFunctionPointers";
    fd_enabled = doConnectFunctionPointers;
    fd_description = "File Iterate All";
    fd_extraopt = [];
    fd_doit = (function (f:file) ->
      mainConnectFunctionPointers f) ;
    fd_post_check = false;
  }
    
    



(*******************************************************************)
(* See status at the bottom of this page                           *)
(*                                                                 *)
(*                                                                 *)
(* aug27: cover almost all FPs, now inserting switch statement     *)
(*                                                                 *)
(*******************************************************************)
