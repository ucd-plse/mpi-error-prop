open Cil
open Pretty
open List
open Rules



let enumerateMap initial items handler =
  let rec enumerateFrom position =
    function
      | [] -> []
      | head :: tail ->
	  let newHead = handler position head in
	  let newTail = enumerateFrom (position + 1) tail in
	  newHead :: newTail
  in
  enumerateFrom initial items



(* Creates exchange variables and populate tables *)
class createVariablesVisitor file tableExchangeFormals tableExchangeReturn = object(self)
  inherit nopCilVisitor

  method vfunc(f: fundec) =

    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
   
    (*Create a global exchange variable for each formal*)
    let exchanges =
      enumerateMap 1 f.sformals
	begin
	  fun slot formal ->
	    let name = Printf.sprintf "%s$%d" f.svar.vname slot in
	    let varinfo = makeGlobalVar name formal.vtype in
	    let global = GVar (varinfo, {init = None}, locUnknown) in
	    file.globals <- global :: file.globals;
	    varinfo
	end
    in
    tableExchangeFormals#add f.svar exchanges;


    (*Create a global exchange variable for the return value*)    
    begin
    match f.svar.vtype with
    | TFun (returnType, _, _, _) ->
	if not (isVoidType returnType) then
	  begin
	    let name = Printf.sprintf "%s$return" f.svar.vname in
	    let varinfo = makeGlobalVar name returnType in
	    let global = GVar (varinfo, {init = None}, locUnknown) in
            tableExchangeReturn#add f.svar varinfo;
	    file.globals <- global :: file.globals
	  end;
    | _ -> ()
    end;

    SkipChildren 
end



(* Inserts exchange-variable-related assignments*)
class insertAssignmentVisitor file tableExchangeFormals tableExchangeReturn tableDerefVars = object(self)
  inherit nopCilVisitor

  (* Current function being analyzed *)
  val mutable currentFunction = dummyFunDec;


 method private equal(expr:exp)(var:varinfo) =
    match expr with
    | Lval(Var(vinfo), _)
    | CastE(_, Lval(Var(vinfo),_)) -> 
        if vinfo.vname = var.vname then
          true
        else
          false; 
    | _ -> false;


  method private isAddrOf (expr:exp) =
    match expr with
    | AddrOf _ -> true
    | _ -> false


  method private isNotTrusted (vinfo:varinfo)(formals:varinfo list) =
    if (List.mem vinfo formals) then
      match vinfo.vtype with
      (*| TPtr(TInt(_,_), _) -> false*) (*Only pointers to int/long/etc.*)
      | TPtr(_,_) -> false
      | _ -> true
    else
      true


  method private isDerefVar(var:varinfo) =
    String.contains var.vname '*'


  method private isFormal(var:varinfo)(formals:varinfo list) =
    List.mem var formals


  method private isPointer(var:varinfo) =
    match var.vtype with
    | TPtr(_,_) -> true
    | _ -> false


  method private updateGotos (old_stmt:stmt)(new_stmt:stmt) =
    List.iter (fun p -> match p.skind with
                        | Goto(target,_) -> target := new_stmt;
			| _ -> ignore()
              ) old_stmt.preds


  method private getFirstN (args:exp list)(n:int) =
    if (n <= 0) then
      []
    else
      (hd args)::(self#getFirstN (tl args) (n - 1));


  method vstmt(s: stmt) =
    try
    begin
      match s.skind with	
      | Instr [Call (receiver, (Lval (Var callee, NoOffset)), args, loc)] ->

	  if tableExchangeFormals#mem callee then
	    begin

	      let exchangeVars = tableExchangeFormals#find callee in
	      let numArgs = List.length exchangeVars in
              let outer_block = (mkBlock []) in
	      let inner_block1 = (mkBlock [s]) in 
              let inner_block2 = (mkBlock []) in
	      let inner_block3 = (mkBlock []) in
	      let outer_stmt = mkStmt(Block(outer_block)) in
              begin
		
	      (*Adding assignments of the form funcion$num = arg before the call*)
		List.iter2 (fun exch arg ->

		  match exch.vtype with
		  (* Formal is a pointer *)
		  | TPtr(_,_) ->
		      begin
			match arg with
			| Lval(Var vinfo, _)
			| CastE(_, Lval(Var vinfo, _)) ->
			    begin
			      match vinfo.vtype with
			      (* Actual is a pointer variable *)
			      | TPtr(_,_) ->
				  begin
				    let setInstr1 = Set(var exch, arg, get_stmtLoc s.skind) in (*trusted*)
				    let exchDeref = tableDerefVars#find exch in

				     (* We are not hadling double pointers (test 171)*)
				    if not (String.contains vinfo.vname '*') then
				      begin
					let argDeref = tableDerefVars#find vinfo in
					let setInstr2 = Set(exchDeref, Lval(argDeref), get_stmtLoc s.skind) in (*trusted*)
				            inner_block1.bstmts <- (mkStmtOneInstr setInstr1) :: (mkStmtOneInstr setInstr2) :: inner_block1.bstmts
				      end
				    else
				      inner_block1.bstmts <- (mkStmtOneInstr setInstr1) :: inner_block1.bstmts
				      
				  end
			      (* Actual is not a pointer variable *)
			      | _ ->
				  begin
				    let setInstr = Set(var exch, arg, get_stmtLoc s.skind) in (*trusted*)
				        inner_block1.bstmts <- (mkStmtOneInstr setInstr) :: inner_block1.bstmts
				  end
			    end
			| AddrOf(Var vinfo, _) ->
			    begin
			      try
				let setInstr1 = Set(var exch, integer(-67737868), get_stmtLoc s.skind) in (*trusted*)
				let exchDeref = tableDerefVars#find exch in
				let setInstr2 = Set(exchDeref, Lval(var vinfo), get_stmtLoc s.skind) in (*trusted*)
				begin
			          inner_block1.bstmts <- (mkStmtOneInstr setInstr1) :: (mkStmtOneInstr setInstr2) :: inner_block1.bstmts
				end;
			      with Not_found -> failwith "[ERROR] Exchange deref variable is missing.\n"
			    end
			(* Actual is not a variable or address of *)
			| _ ->
			    begin
			       try
				let setInstr1 = Set(var exch, arg, get_stmtLoc s.skind) in (*trusted*)
				let exchDeref = tableDerefVars#find exch in
				let setInstr2 = Set(exchDeref,  integer(-67737868), get_stmtLoc s.skind) in (*trusted*)
				begin
			          inner_block1.bstmts <- (mkStmtOneInstr setInstr1) :: (mkStmtOneInstr setInstr2) :: inner_block1.bstmts
				end;
			      with Not_found -> failwith "[ERROR] Exchange deref variable is missing.\n"
			    end
			    
		      end
		  (* Formal is not a pointer variable *)
		  | _ ->
		      begin
			let setInstr = Set(var exch, arg, locUnknown) in
			    inner_block1.bstmts <- (mkStmtOneInstr setInstr) :: inner_block1.bstmts
		      end

	        ) exchangeVars (self#getFirstN args numArgs);
            

                (* Write back exchange parameters variables to actuals (if pointer variables) *)
                List.iter2 (fun exch arg -> 
		  match arg with
		  | Lval(Var vinfo, _) ->
		      begin
			match vinfo.vtype with
			| TPtr(_,_) ->
			    (* We are not hadling double pointers (test 171)*)
			    if not (String.contains vinfo.vname '*') then
			      let exchDeref = tableDerefVars#find exch in
			      let argDeref  = tableDerefVars#find vinfo in
			      let setInstr = Set(argDeref, Lval(exchDeref), loc) in
			          inner_block2.bstmts <- inner_block2.bstmts @ [mkStmtOneInstr setInstr]
									   
			  (* arg is a non-pointer variable *)
			| _ ->
			    ignore() (* no value written back *)
		      end	
			
		  | CastE(typ, Lval(Var vinfo, _)) ->
		      begin
			match vinfo.vtype with
			| TPtr(_,_) ->
			    begin
			      match typ with
			      |  TPtr(_,_) ->
				  (* We are not hadling double pointers (test 171)*)
				  if not (String.contains vinfo.vname '*') then
				    let exchDeref = tableDerefVars#find exch in
				    let argDeref  = tableDerefVars#find vinfo in
				    let setInstr = Set(argDeref, Lval(exchDeref), loc) in
				        inner_block2.bstmts <- inner_block2.bstmts @ [mkStmtOneInstr setInstr]

				 (* formal is not of type pointer *)
			      |  _ ->
				  ignore();
			    end
			      
			  (* arg is a non-pointer variable *)
			| _ ->
			    ignore() (* no value written back *)
		      end

		  | AddrOf(Var vinfo, _) ->
		      let exchDeref = tableDerefVars#find exch in
		      let setInstr = Set(var vinfo, Lval(exchDeref), loc) in
		          inner_block2.bstmts <- inner_block2.bstmts @ [mkStmtOneInstr setInstr]
                  | _ ->
		      ignore() (* no value written back *)
		) exchangeVars (self#getFirstN args numArgs);


		(*If there is a receiver, add assignment receiver = function$return after call*)      
		begin            
		  match receiver with
		  | Some lv ->
		      let exchangeRet = tableExchangeReturn#find callee in
		      let setInstr = Set(lv, Lval(var exchangeRet), loc) in
                      inner_block3.bstmts <- inner_block3.bstmts @ [mkStmtOneInstr setInstr]
		  | _ -> ();
		end;

		inner_block1.battrs <- Cil.addAttribute (Attr("callBlock", []))  inner_block1.battrs;
		inner_block2.battrs <- Cil.addAttribute (Attr("afterCall", []))  inner_block2.battrs;
		(*no need to set attribute for block3, retval block*)
		
		outer_block.bstmts <- [mkStmt(Block inner_block1); mkStmt(Block inner_block2); mkStmt(Block inner_block3)];


		(*Updating labels and gotos when call is the target of a goto*)
		outer_stmt.labels <- s.labels;
		s.labels <- [];
		self#updateGotos s outer_stmt;

		ChangeTo outer_stmt
	      end
	    end
	  else
	    (*Implementation is not available, just put call inside callblock*)
	    begin
	      let call_block = (mkBlock [s]) in 
	      call_block.battrs <- Cil.addAttribute (Attr("callBlock", []))  call_block.battrs;

	      ChangeTo (mkStmt(Block(call_block)))
	    end
	 
      
      | Instr [Call (receiver, _, args, loc)] ->
	  begin
	    let call_block = (mkBlock [s]) in 
	      call_block.battrs <- Cil.addAttribute (Attr("callBlock", []))  call_block.battrs;
	      ChangeTo (mkStmt(Block(call_block)))
	  end

   
      | Return (Some retExp, loc) ->
	begin

	  match currentFunction.svar.vtype with
	  | TFun (returnType, _, _, _) ->
	      if not (isVoidType returnType) then
		let block = (mkBlock [s]) in
		let varinfo = tableExchangeReturn#find currentFunction.svar in
		let retAssgInstr = Set(var varinfo, retExp, get_stmtLoc s.skind) in
		let block_stmt = mkStmt(Block(block)) in
		begin

		  (*Adding untrusted assignments at the end of the function*)
                  List.iter (fun varinfo ->

		    if self#equal retExp varinfo then
		      ignore() (* do nothing *)
		    else if self#isFormal varinfo currentFunction.sformals then
		      begin
			let setInstr = Set(var varinfo, integer(-67737869), get_stmtLoc s.skind) in (*untrusted*)
			    block.bstmts <- (mkStmtOneInstr setInstr) :: block.bstmts;
			if self#isPointer varinfo then
			  let derefVar = tableDerefVars#find varinfo in
			  let setInstr2 = Set(derefVar, integer(-67737868), get_stmtLoc s.skind) in (*trusted*)
			      block.bstmts <- (mkStmtOneInstr setInstr2) :: block.bstmts
		      end
		    else if not (self#isDerefVar varinfo) then (* already taken care of *)
		      begin
		      	let setInstr = Set(var varinfo, integer(-67737869), get_stmtLoc s.skind) in (*untrusted*)
			    block.bstmts <- (mkStmtOneInstr setInstr) :: block.bstmts;
			if self#isPointer varinfo then
			  let derefVar = tableDerefVars#find varinfo in
			  let setInstr2 = Set(derefVar, integer(-67737869), get_stmtLoc s.skind) in (*untrusted*)
			      block.bstmts <- (mkStmtOneInstr setInstr2) :: block.bstmts
		      end			
		      
                    ) (currentFunction.slocals @ currentFunction.sformals); 


		  (*Writing values back from formals to exchange global variables*)
		  let exchanges = tableExchangeFormals#find currentFunction.svar in  
		    List.iter2 (fun exch formal ->

		      match formal.vtype with
		      | TPtr(_,_) ->
			  begin
			    let exchDeref = tableDerefVars#find exch in
			    let formalDeref = tableDerefVars#find formal in
			    let setInstr = Set(exchDeref, Lval(formalDeref), loc) in
	 		        block.bstmts <- [mkStmtOneInstr setInstr] @ block.bstmts;
			  end
		      | _ ->
			  ignore() (* do nothing *)

		    ) exchanges currentFunction.sformals; 

		  
		  (*Adding assignment before return foo$return = return_value*)
		  block.bstmts <- (mkStmtOneInstr retAssgInstr) :: block.bstmts;
                  block.battrs <- Cil.addAttribute (Attr("lastBlock", []))  block.battrs;

                  (*Updating labels and gotos when return is the target of a goto*)
		  block_stmt.labels <- s.labels;
		  s.labels <- [];
		  self#updateGotos s block_stmt;

                  ChangeTo block_stmt;
		end
             else
                DoChildren

	  | _ -> 
	      DoChildren
       end


    | Return (None, loc) ->
	let block = (mkBlock [s]) in
	let block_stmt = mkStmt(Block(block)) in
	begin

          (*Adding untrusted assignments at the end of the function*)
          List.iter (fun varinfo ->

	    if self#isFormal varinfo currentFunction.sformals then
	      begin
		let setInstr = Set(var varinfo, integer(-67737869), get_stmtLoc s.skind) in (*untrusted*)
		block.bstmts <- (mkStmtOneInstr setInstr) :: block.bstmts;
		if self#isPointer varinfo then
		  let derefVar = tableDerefVars#find varinfo in
		  let setInstr2 = Set(derefVar, integer(-67737868), get_stmtLoc s.skind) in (*trusted*)
		  block.bstmts <- (mkStmtOneInstr setInstr2) :: block.bstmts
	      end
	    else if not (self#isDerefVar varinfo) then (* already taken care of *)
	      begin
		let setInstr = Set(var varinfo, integer(-67737869), get_stmtLoc s.skind) in (*untrusted*)
		    block.bstmts <- (mkStmtOneInstr setInstr) :: block.bstmts;
		if self#isPointer varinfo then
		  let derefVar = tableDerefVars#find varinfo in
		  let setInstr2 = Set(derefVar, integer(-67737869), get_stmtLoc s.skind) in (*untrusted*)
		    block.bstmts <- (mkStmtOneInstr setInstr2) :: block.bstmts
	      end
	      
          ) (currentFunction.slocals @ currentFunction.sformals);

          block.battrs <- Cil.addAttribute (Attr("lastBlock", []))  block.battrs;


         (*Writing values back from formals to exchange global variables*)
	 let exchanges = tableExchangeFormals#find currentFunction.svar in  
	 List.iter2 (fun exch formal ->


	   match formal.vtype with
	   | TPtr(_,_) ->
	       begin
		 let exchDeref = tableDerefVars#find exch in
		 let formalDeref = tableDerefVars#find formal in
		 let setInstr = Set(exchDeref, Lval(formalDeref), loc) in
	 	 block.bstmts <- [mkStmtOneInstr setInstr] @ block.bstmts;
	       end
	   | _ ->
	       ignore() (* do nothing *)

	   ) exchanges currentFunction.sformals;

	 
         (*Updating labels and gotos when return is the target of a goto*)
	 block_stmt.labels <- s.labels;
	 s.labels <- [];
	 self#updateGotos s block_stmt;

         ChangeTo block_stmt;
	end;


    | _ -> 
	DoChildren
    end;

    with Not_found -> 
      begin
	(*Implementation is not available...*)
	DoChildren
      end



  method vfunc(f: fundec) =

    try
    IsolateInstructions.visit f;
    prepareCFG f;
    computeCFGInfo f false;
    currentFunction <- f;
   
    (*Adding assignments formal1 = foo$1 at the beginning of the function*)
    let exchanges = tableExchangeFormals#find f.svar in
    
    if List.length exchanges == List.length f.sformals then
      begin
      let block = mkBlock [] in

      List.iter2 (fun exch varformal ->

	let setInstr1 = Set(var varformal, Lval(var exch), locUnknown) in
	    block.bstmts <- block.bstmts @ [mkStmtOneInstr setInstr1];

	match varformal.vtype with
	| TPtr(_,_) ->
	    let formalDeref = tableDerefVars#find varformal in
	    let exchDeref = tableDerefVars#find exch in
	    let setInstr2 = Set(formalDeref, Lval(exchDeref), locUnknown) in
	        block.bstmts <- block.bstmts @ [mkStmtOneInstr setInstr2];
	| _ ->
	    ignore()
	      
        ) exchanges f.sformals;

        block.battrs <- Cil.addAttribute (Attr("firstBlock", []))  block.battrs;
	f.sbody.bstmts <- (mkStmt (Block block)) :: f.sbody.bstmts
      end;

    DoChildren;
    
    with Not_found -> 
      begin
	(*Implementation not available...*)
	DoChildren;
      end
end



let createVariables file tableExchangeFormals tableExchangeReturn =
  let visitor = new createVariablesVisitor file tableExchangeFormals tableExchangeReturn in
    ignore(visitCilFileSameGlobals (visitor :> cilVisitor) file)


let insertAssignments file tableExchangeFormals tableExchangeReturn tableDerefVars =
  let visitor = new insertAssignmentVisitor file tableExchangeFormals tableExchangeReturn tableDerefVars in
    ignore(visitCilFileSameGlobals (visitor :> cilVisitor) file)


