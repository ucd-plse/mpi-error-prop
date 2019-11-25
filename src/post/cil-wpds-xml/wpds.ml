(*
   Extract a WPDS from C code using CIL
   @author Nicholas Kidd / modified by Cindy Rubio
*)

open Cil
open Pretty
open Ptranal
open Hashtbl
open List
open Rmtmps
open Deadcodeelim
open Cfg
open ConnectFunctionPointers
open Filters
open VarSet

module E = Errormsg

(* Globals *)
let tableExchangeFormals = new VariableNameHash.c 2300
let tableExchangeReturn = new VariableNameHash.c 2300
let tableDerefVars = new VariableNameHash.c 1000
let nowhere = {file = ""; line = 0; byte = 0}
let transfer = ref false
let positive = ref false
let transformation = ref false
let nounreachable = ref false
let noerrorhandling = ref false


(* map across list elements as in List.map, but call mapping function with an element number as well as the element itself *)
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


(*Source visitor begins*)    
class sourceVisitor =
  object(self)
    inherit nopCilVisitor

    val mutable thefundec = dummyFunDec;
      
    method vstmt stmt =
      begin
	match stmt.skind with
	| Instr [Call (None, (Lval (Var varinfo, NoOffset) as callee), actuals, loc)] ->
	    begin
	      match splitFunctionTypeVI varinfo with
	      | TVoid _, _, _, _ ->
		  ()
	      | resultType, _, _, _ ->
		  let receiver = var (makeTempVar thefundec resultType) in
		  stmt.skind <- Instr [Call (Some receiver, callee, actuals, loc)]
	    end
	| _ ->
	    ()
      end;
      DoChildren

    method vfunc(f: fundec) =
      IsolateInstructions.visit f;
      prepareCFG f;
      computeCFGInfo f false;
      
      thefundec <- f;
      DoChildren;

  end
(* End of source visitor *)



let createExchangeVars file =
  let visitor = function
    | GFun (fundec, location) ->
	begin
	  (* create a global exchange variable for each formal *)
	  let exchanges =
	    enumerateMap 1 fundec.sformals
	      begin
		fun slot formal ->
		  let name = Printf.sprintf "%s$%d" fundec.svar.vname slot in
		  let varinfo = makeGlobalVar name formal.vtype in
		  let global = GVar (varinfo, {init = None}, location) in
		  file.globals <- global :: file.globals;
		  varinfo
	      end
	  in
	  tableExchangeFormals#add fundec.svar exchanges;

	  (* create a global exchange variable for the result, if non-void *)
	  match fundec.svar.vtype with
	  | TFun (returnType, _, _, _) ->
	      if not (isVoidType returnType) then
		let name = fundec.svar.vname ^ "$return" in
		let varinfo = makeGlobalVar name returnType in
		let global = GVar (varinfo, {init = None}, location) in
		file.globals <- global :: file.globals;
		tableExchangeReturn#add fundec.svar varinfo
	  | _ ->
	      failwith "internal error: function with non-function type"
	end

    | _ ->
	()
  in
  iterGlobals file visitor


exception Non_struct of string;;
(*Some debugging method privates*)
 let printCilObj d_obj = fun obj chan ->
   let d = dprintf "%a" d_obj obj in
   let s = sprint 80 d in
   ignore( fprintf chan "[%s]\n" s )

let printCilExp = printCilObj d_exp
let printCilStmt = printCilObj d_stmt
let printCilLval = printCilObj d_lval




class wpdsVisitor pdsout pdserr hexastore = object(self)
  inherit nopCilVisitor
      
    (* Current function being analyzed *)
  val mutable currentFunction = dummyFunDec;
    (* Where to begin prestar from *)
  val mutable mainRetId = 0;
    (* The cilPrinter used by wpdsVisitor to output the WPDS *)
  val mutable wpdsPrinter = plainCilPrinter;

  method getPrestarId() : int = mainRetId

  method private getCompInfoForType(t:typ) =
    match t with
    | TVoid(_)
    | TInt(_,_)
    | TFloat(_,_) ->
	begin
          raise (Non_struct "?.getCompInfoForType");
	end;
    | TPtr(t1,_)
    | TArray(t1,_,_) ->
	begin
          self#getCompInfoForType t1;
	end;
    | TComp(cinfo,attrs) ->
	begin
          cinfo;
	end;
    | TNamed(tinfo,attrs) ->
	begin
          self#getCompInfoForType tinfo.ttype;
	end;
    | _ ->
	begin
	  ignore (bug "[getCompInfoForType] not yet completed...\n");
	  failwith "internal error 1";
	end;

  method private getCompInfoForLval(lv:lval) =
    match lv with
    | (Var(vinfo),_) ->
	begin
          self#getCompInfoForType vinfo.vtype;
	end;
    | (Mem(e),off) ->
	begin
          self#getCompInfoForExp e;
	end;

  method private getCompInfoForExp(e:exp) =
    match e with
    | UnOp(_,_,t) | BinOp(_,_,_,t) | SizeOf(t) | AlignOf(t) | CastE(t,_) ->
	begin
          self#getCompInfoForType t;
	end;
    | Lval(lv) ->
	begin
          self#getCompInfoForLval lv;
	end;
    | _ ->
	begin
	  ignore (bug "[getCompInfoForExp] not yet completed...\n");
	  printCilExp e pdserr;
	  failwith "internal error 2";
	end;

  method private getCompInfoForLhost(host:lhost) =
    match host with
    | Var(vinfo) ->
	begin
          self#getCompInfoForType vinfo.vtype;
	end;
    | Mem(e1) ->
	begin
          self#getCompInfoForExp e1;
	end;


	(* Check this*)
  method private doLval( lv: lval ) =
    begin
      match lv with
      | (host,Field(finfo,foff)) ->
	  begin
            begin try
              let cinfo = self#getCompInfoForLhost host in
              begin
		(*ignore( fprintf pdsout "-,-,%s,%s:" cinfo.cname finfo.fname );*) (*Commented for now!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*)
		ignore( fprintf pdsout "'%s'" cinfo.cname );
              end;
            with Non_struct ns -> ignore( fprintf pdserr "[Non_struct] %s\n" ns )
            end;
	  end;
      | (Mem(e1),off) ->
	  begin
            self#doExp e1;
	  end;
      | (Var(vinfo),off) ->
	  begin
            ignore( fprintf pdsout "'%s'" vinfo.vname);
	  end
    end;


(*5, 6 errors.txt*)
  method private doExp(e:exp) =
    match e with
    | Const (CInt64 (value, _, _)) ->
	let loc =
	  match Error.byValue value with
	  | Some code -> Error.name code
	  | None -> "OK"
	in
	ignore (fprintf pdsout "'%s'" loc)
    | Const(_) | SizeOf(_) | SizeOfStr(_) | AlignOf(_) -> 
	Rules.unimplemented pdsout "'Expression: SizeOf, SizeOfStr and AlignOf'"
    | SizeOfE(e1) | AlignOfE(e1) | UnOp(_,e1,_) (*| CastE(_,e1)*) ->
        (*self#doExp e1;*)
        Rules.unimplemented pdsout "Expression: SizeOfE, AlignOfE and UnOp"
    | CastE(_,e1) ->
        self#doExp e1;
    | BinOp(_,e1,e2,t) ->
        (*self#doExp e1;
          self#doExp e2;*)
        Rules.unimplemented pdsout "Expression: BinOp"
    | Lval(lv) ->
	begin
          self#doLval lv;
	end

    | AddrOf(lv) ->
	begin
          self#doLval lv;
          (*match lv with
             | (Var(vinfo), off) ->
	     ignore( fprintf pdsout "'%s'" vinfo.vname );
             | _ ->  ignore( fprintf pdsout "[ERROR] New error cindy...\n");
           *)
	end
    | StartOf(lv) ->
        Rules.unimplemented pdsout "Expression: StartOf"
	

  
  method private makeWeight (s:stmt)(isTrusted:bool) =
    match s.skind with

    | Instr [Set (receiverLval, senderExpr, location)] ->
	let receiver = Rules.getReceiver receiverLval in
	let sender = Rules.getSender senderExpr in
	begin
	  match receiver, sender with

	  | Some receiver, Some sender ->
	      let trusted =
                if isInteger senderExpr = Some (-67737869L) or isInteger senderExpr = Some (-67737870L) then
                   Rules.Untrusted
		else if isInteger senderExpr = Some (-67737868L) or 
		  isInteger senderExpr = Some (-82080000L) or isTrusted then
		  Rules.Trusted
		else
		  Rules.Untrusted
	      in
	      Rules.set pdsout receiver sender trusted !transfer
	  | Some receiver, None ->
	      begin
		Rules.unimplemented pdsout "assignment with unusual sender";
		Rules.set pdsout receiver Rules.OK Rules.Trusted !transfer
	      end
          | None, Some _ ->
	      Rules.unimplemented pdsout "assignment with unusual receiver"
	  | None, None ->
	      Rules.unimplemented pdsout "assignment with unusual sender and receiver"
	end

    | Instr[Call _] ->
	ignore()

    | Instr[Asm(_,_,receivers,_,clobbers,_)] ->
	Rules.unimplemented pdsout "Instruction: Asm"

    | Instr [] | Block _ | Goto(_,_) | Return( None,_) | Loop _ ->
	ignore() (*identity before*)
    | Instr _ ->
	begin
          ignore (bug "[makeWeight] Instr not atomized?\n");
          failwith "[makeWeight] internal error 3";
	end;
    | If( e,_,_,loc) ->
	ignore();

    | Return(Some e, loc) -> ()
(*	begin
	  let expr = Rules.getRetValue e in
	  match expr with
	  | Some expr  -> 
	      Rules.return pdsout expr
	  | _ -> ()
	end
*)
    | _ ->
	begin
          ignore (bug "[makeWeight] ???\n");
	end



  method private writeBack sender receiver =
    Rules.set pdsout receiver (Rules.Location sender) Rules.Trusted !transfer


  method private createIntList(initial:int)(length:int) =
    if (length <= 0) then
      []
    else
      initial::(self#createIntList (initial + 1) (length - 1));



  method private getFirstN (args:exp list)(n:int) =
    if (n <= 0) then
      []
    else
      (hd args)::(self#getFirstN (tl args) (n - 1));



  method private equal(expr:exp)(var:varinfo) =
    match expr with
    | Lval(Var(vinfo), _)
    | CastE(_, Lval(Var(vinfo),_)) -> 
        if vinfo.vname = var.vname then
          true
        else
          false; 
    | _ -> false;
  


  method private isSpecialBlock(b:block) =
    if ((Cil.hasAttribute "firstBlock" b.battrs) || (Cil.hasAttribute "lastBlock" b.battrs) ||
       (Cil.hasAttribute "callBlock" b.battrs) || (Cil.hasAttribute "afterCall" b.battrs)) then
      true
    else
      false



  method private getSuccId(s1:stmt) =
    match s1.skind with
    | Block block ->
	if not (Cil.hasAttribute "afterCall" block.battrs) then
	  begin
	    (*ignore(fprintf stderr "use id: %d instead of %d\n" (List.hd s1.succs).sid s1.sid);*)
	    if (List.length s1.succs > 0) then
	      let succ = (List.hd s1.succs) in
	      match succ.skind with
	      | Block block ->
		  self#getSuccId succ
	      | _ -> 
		  succ.sid	       
	    else
	      s1.sid
	  end
	else
	  s1.sid
    | _ ->
	s1.sid

  
  method private validType(vinfo:varinfo) =
    match unrollTypeDeep vinfo.vtype with
    | TInt _
    | TPtr (TInt _, _)
    | TPtr (TVoid _, _)
    | TPtr (TComp _, _) ->
	true
    | _ ->
	false

  method private isRelevant name =
    Var_set.exists(fun v -> (v.vname = name) && (self#validType v)) !FindVariables.relevant

	  
  method private printDeref(vinfo:varinfo)(isField:bool) =
    if (String.contains vinfo.vname '*') then
      begin
	let originalName = 
	  let index = String.index vinfo.vname '*' in
	  let funName = String.sub vinfo.vname 0 index in
	  let exchName = String.sub vinfo.vname (index + 1) (String.length vinfo.vname - index - 1) in
              String.concat "" [funName; exchName] in
	if (self#isRelevant originalName) then
	  ignore(fprintf pdsout "\t\t\t<dereference name='%s'/>\n" originalName)
      end
    else if isField then
      if (self#isRelevant vinfo.vname) then
	  ignore(fprintf pdsout "\t\t\t<dereference name='%s'/>\n" vinfo.vname)


  method private printDerefsInLval(lv:lval)(isField:bool) =
    match lv with
    | (Var vinfo, _) ->
	self#printDeref vinfo isField;
	    
    | (Mem e, Field(_,_)) ->
	self#printDerefsInExp e true;

    | (Mem e, _) ->
	self#printDerefsInExp e isField


  method private printDerefsInExp(e:exp)(isField:bool) =
    match e with
    | SizeOfE(e2) | AlignOfE(e2) | UnOp(_,e2,_) | CastE(_, e2) ->
	self#printDerefsInExp e2 isField;

    | BinOp(_, e2, e3, _) ->
	begin
	  self#printDerefsInExp e2 isField;
	  self#printDerefsInExp e3 isField
	end;

    | Lval(lv) | AddrOf(lv) | StartOf(lv) ->
	self#printDerefsInLval lv isField;

    | _ -> ()


  method private printDerefsInStmt(s:stmt)(isTrusted:bool) =  
      match s.skind with
      | Instr[Set(lv, e, _)] ->
	  let trusted =
            if isInteger e = Some (-67737869L) then
              false
	    else if isInteger e = Some (-67737868L) or isTrusted or
	      isInteger e = Some (-67737870L) or  (*untrusted, but not dereference *)
	      isInteger e = Some (-82080000L)
	    then
	      true
	    else
	      false
	  in
	  if (not trusted) then
	    begin
	      self#printDerefsInLval lv false;
	      self#printDerefsInExp e false
	    end;
	  
      | Instr [Call(receiver, calleeExpr, args, _)] ->
	  begin
	    List.iter (fun e -> self#printDerefsInExp e false) (calleeExpr::args);
	    match receiver with
	    | Some lv ->
		self#printDerefsInLval lv false;
	    | None -> ()
	  end;

      | Return(Some e, _) ->
	  self#printDerefsInExp e false;

      | If(e, b1, b2, _) ->
	  self#printDerefsInExp e false;

      | Switch(e, b, stmts, _) ->	  
	  self#printDerefsInExp e false;
	  
      | Loop(b, _, _, _) | Block(b) ->
	  ignore();

      | _ -> ()


  method private printHandledInLval(lv:lval) =
    match lv with
    | (Var vinfo, _) ->
	if (self#isRelevant vinfo.vname) then
	  ignore(fprintf pdsout "\t\t\t<handled name='%s'/>\n" vinfo.vname)
	    
    | (Mem e, NoOffset) ->
	self#printHandledInExp e;

    | _ -> ()


  method private printHandledInExp(e:exp) =
    match e with
    | Lval(lv) | AddrOf(lv) | StartOf(lv) ->
	self#printHandledInLval lv;

    | _ -> ()


  method private printHandledInStmt(s:stmt) =  
      match s.skind with
      | Instr[Set(lv, Const(CInt64(-82080000L,_,_)), _)] ->
	  self#printHandledInLval lv;
	  
      | _ -> ()



  method private printOperandsInExp(e:exp)(b:bool) =
    match e with
    | BinOp(operator, expr1, expr2, _)
    | CastE(_, BinOp(operator, expr1, expr2, _)) ->
	begin
	  match operator with
	  | MinusPI | MinusPP | PlusPI ->
	      begin
		self#printOperandsInExp expr1 true;
		self#printOperandsInExp expr2 true
	      end		
	  | _ -> 
	      begin
		self#printOperandsInExp expr1 b;
		self#printOperandsInExp expr2 b
	      end
	end

    | SizeOfE(expr1) | AlignOfE(expr1) | UnOp(_, expr1, _) | CastE(_, expr1) ->
	self#printOperandsInExp expr1 b;

    | Lval(_) | AddrOf(_) | StartOf(_) ->
	begin
	  if b then
	    let op = Rules.getOperand e in
	        Rules.operand pdsout op
	end

    | _ -> ()



  method private printOperandsInStmt(s:stmt) =  
      match s.skind with
      | Instr [Set (_, e, _)]
      | Return(Some e, _)
      | If(e, _, _, _)
      | Switch(e, _, _, _) ->	  
	  self#printOperandsInExp e false;

      | Instr [Call(_, calleeExpr, args, _)] ->
	  List.iter (fun e -> self#printOperandsInExp e false) (calleeExpr::args);
	  
      | Loop(b, _, _, _) | Block(b) ->
	  ignore();

      | _ -> ()



  method private makeIntraEdge(s:stmt)(s1:stmt) loc =
    let id1 = (self#getSuccId s) in
    let id2 = (self#getSuccId s1) in
    if (id1 != id2) then
      begin
	ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'" currentFunction.svar.vname id1 ); (*s.sid*)
	ignore( fprintf pdsout " toStack1='%s.%d'>\n" currentFunction.svar.vname id2); (*s1.sid*)
	ignore( fprintf pdsout"\t\t\t<Weight basis='identity'>\n" );
	ignore( self#makeWeight s false );
	ignore( fprintf pdsout"\t\t\t</Weight>\n" );
	Rules.source pdsout loc;

	ignore( self#printDerefsInStmt s false );
	ignore( self#printHandledInStmt s );
	ignore( self#printOperandsInStmt s );

	ignore( fprintf pdsout"\t\t</Rule>\n" );
      end



  method private makeEdgeAfterCall(s:stmt)(s1:stmt) =
    let id1 = (self#getSuccId s) in
    let id2 = (self#getSuccId s1) in
    if (id1 != id2) then
      begin
	ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'" currentFunction.svar.vname id1 ); (*s.sid*)
	ignore( fprintf pdsout " toStack1='%s.%d'>\n" currentFunction.svar.vname id2); (*s1.sid*)
	ignore( fprintf pdsout "\t\t\t<Weight basis='identity'>\n");
	
	begin
	  match s.skind with
	  | Block block ->
	      List.iter (fun st -> self#makeWeight st true) block.bstmts
	  | _ -> ();
	end;
	
	ignore( fprintf pdsout "\t\t\t</Weight>\n" );

	begin
	  match s.skind with
	  | Block block ->
	      if (List.length block.bstmts > 0) then
		let loc =
		  match (List.hd block.bstmts).skind with
		  | Instr[Set(_,_,loc)] ->  loc;
		  | _ -> locUnknown;
		in
		Rules.source pdsout loc;
	  | _ -> ();
	end;
		                     
	ignore( fprintf pdsout"\t\t</Rule>\n" );
      end



  method private makeIntraEdgeFirstBlock(s:stmt)(s1:stmt) loc =

    let basis =
	match currentFunction.svar.vname with
	| "main" ->
	    "uninitialized"
	| _ ->
	    "identityGlobals"
    in

    ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'" currentFunction.svar.vname s.sid );
    ignore( fprintf pdsout " toStack1='%s.%d'>\n" currentFunction.svar.vname (self#getSuccId s1)); (*s1.sid*)
    ignore( fprintf pdsout"\t\t\t<Weight basis='%s'>\n" basis);

    begin
      match s.skind with
      | Block block ->
	  List.iter (fun s -> self#makeWeight s true) block.bstmts
      | _ -> ();
    end;

    ignore( fprintf pdsout"\t\t\t</Weight>\n" );
    Rules.source pdsout loc;
    (* Add markers here *)

    List.iter (fun f -> Rules.input pdsout f) (tableExchangeFormals#find currentFunction.svar);
    ignore( fprintf pdsout"\t\t</Rule>\n" );



  method private makePushRule(s:stmt)(s1:stmt) callee (args:exp list) loc =
    let funName = callee.vname in
    try
      if tableExchangeFormals#mem callee then
	begin
          ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'" currentFunction.svar.vname (self#getSuccId s) ); (*s.sid*)

	  if (hexastore || funName <> "panic") then
	    begin
	      (* A regular push rule *)
              ignore( fprintf pdsout " toStack1='%s.0'" funName);
              ignore( fprintf pdsout " toStack2='%s.%d'>\n" currentFunction.svar.vname (self#getSuccId s1) )
	    end
	  else
	    (* An intra edge instead *)
	    ignore( fprintf pdsout " toStack1='_panic_.0'>\n");

          ignore( fprintf pdsout"\t\t\t<Weight basis='identity'>\n" );

	  begin
	    match s.skind with
	    | Block block ->
		List.iter (fun s -> self#makeWeight s true) block.bstmts
	    | _ -> ();
	  end;
          (*ignore( Arithmetic.assumeNonError pdsout s );*) (*?????*)

	  let exchangeVars = tableExchangeFormals#find callee in
	  if exchangeVars <> [] then
	    begin
              let numArgs = List.length exchangeVars in
              if numArgs <> List.length args then
                ignore (fprintf pdserr "Function %s is a variable parameter list function\n" funName);

            end
	end

      else
        (* The function definition is not available... *)
        begin 
          ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'" currentFunction.svar.vname (self#getSuccId s)); (*s.sid*)


	  (* If panic, introduce an existent target key *)
	  if (hexastore || funName <> "panic") then
	    ignore( fprintf pdsout " toStack1='%s.%d'>\n" currentFunction.svar.vname (self#getSuccId s1))
	  else
	    ignore( fprintf pdsout " toStack1='_panic_.0'>\n");


          ignore( fprintf pdsout"\t\t\t<Weight basis='identity'>\n" );
          ignore( fprintf pdserr "Warning: the definition for function %s is not available\n" funName);
	  ignore( Arithmetic.assumeNonError pdsout s);         

        end;

      ignore( fprintf pdsout"\t\t\t</Weight>\n" );
      Rules.source pdsout loc;
      List.iter (fun e -> self#printDerefsInExp e false) args;

      if (funName <> "printk") then
	List.iter (fun e -> self#printOperandsInExp e false) args;

      if (String.compare funName "IS_ERR" == 0) then
	begin
	    let argument = (List.hd args) in
		match argument with
		| Lval(Var(vinfo),_)
		| CastE(_, Lval(Var(vinfo),_)) ->
	  	    ignore (fprintf pdsout "\t\t\t<iserr name='%s'/>\n" vinfo.vname)
		| _ -> ignore();
	end;

      ignore( fprintf pdsout"\t\t</Rule>\n" );
      
    with Not_found -> ignore( fprintf pdsout "[ERROR in makePushRule] Function %s was not found in table\n" funName);
    | Invalid_argument ns -> ignore(fprintf pdsout "[ERROR in makePushRule] Invalid argument %s\n" ns);



  (* This function will obviously not be called when a function's definition is not available *)
  method private makePopRuleRet(s:stmt) =
    try
      ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'>\n" currentFunction.svar.vname (self#getSuccId s) ); (*s.sid*)
      ignore( fprintf pdsout"\t\t\t<Weight basis='identity'>\n" );
      
      begin
	match s.skind with
	| Block block ->
	    List.iter (fun st -> self#makeWeight st true) block.bstmts;
	| _ ->  ()
      end;

      ignore( fprintf pdsout"\t\t\t</Weight>\n" );

      begin
	match s.skind with
	| Block block ->
	    List.iter (fun st ->
	      match st.skind with
	      | Return(Some expr, loc) ->  		    
		  Rules.source pdsout loc;
		  begin
		    let value = Rules.getRetValue expr in
		    match value with
		    | Some value  -> 
			begin
			  Rules.return pdsout value;
			  Rules.output pdsout value;

			  List.iter (fun f -> Rules.outputvar pdsout f) currentFunction.sformals;

			  self#printDerefsInExp expr false;
			  self#printOperandsInExp expr false
			end
		    | _ -> ()
		  end
	      | Return(None, loc) ->
		  Rules.source pdsout loc
	      | _ -> ();
	    ) block.bstmts;
	| _ ->  Rules.source pdsout locUnknown; (*never selected, this is a block with a return*)
      end;

      ignore( fprintf pdsout"\t\t</Rule>\n" );
      
    with Not_found -> ignore( fprintf pdsout "[ERROR in makePopRuleRet] Function %s was not found in table\n" currentFunction.svar.vname);
    | Invalid_argument ns -> ignore(fprintf pdsout "[ERROR in makePopRuleRet] Invalid argument %s\n" ns);



  method private makePopRuleNoRet(s:stmt) loc =
    try 
      ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'>\n" currentFunction.svar.vname (self#getSuccId s) ); (*s.sid*)

      ignore( fprintf pdsout"\t\t\t<Weight basis='identity'>\n" );

      if currentFunction.sformals <> [] then
	begin
	  let exchangeVars = tableExchangeFormals#find currentFunction.svar in
          List.iter2 self#writeBack currentFunction.sformals exchangeVars
	end;

      ignore( fprintf pdsout"\t\t\t</Weight>\n" );
      Rules.source pdsout loc;
      ignore( fprintf pdsout"\t\t</Rule>\n" );

    with Not_found -> ignore(fprintf pdsout "[ERROR in makePopRuleRet] Function %s was not found in table\n" currentFunction.svar.vname);
    | Invalid_argument ns -> ignore(fprintf pdsout "[ERROR in makePopRuleNORet] Invalid argument %s\n" ns);



  method private preMakePushRule (s:stmt) (s1:stmt) =
   begin
      
     match s.skind with
     | Block block ->
	 List.iter (fun st -> 
	   
	   match st.skind with
	   | Instr [Call (receiver, calleeExpr, args, loc)] ->
	       let callees =
		 match calleeExpr with
		 | Lval (Var callee, NoOffset) ->
		     [callee]
		 | _ ->
		     try
		       List.map
			 (fun fundec -> fundec.svar)
			 (Ptranal.resolve_funptr calleeExpr)
		     with Not_found ->
		       ignore( fprintf pdsout "\t\t<Rule from='p' fromStack='%s.%d' to='p'" currentFunction.svar.vname (self#getSuccId s)); (*s.sid*)
		       ignore( fprintf pdsout " toStack1='%s.%d'>\n" currentFunction.svar.vname (self#getSuccId s1)); (*s1.sid*)
		       ignore( fprintf pdsout"\t\t\t<Weight basis='identity'>\n" );
		       ignore( fprintf pdsout"\t\t\t</Weight>\n" );
		       Rules.source pdsout loc;
		       ignore( fprintf pdsout"\t\t</Rule>\n" );

		       ignore ( warn "cannot resolve function pointer %a" d_exp calleeExpr );
		       
		       []
	       in
	       
	       List.iter
		 begin
		   fun callee ->
		     self#makePushRule s s1 callee args loc;
		 end
		 callees
	   | _ -> ()
		 
	) block.bstmts

      | _ -> ();

    end;



  method private handleStmt (s:stmt) = fun (s1:stmt) -> 
    begin 
      match s.skind with
      | Instr[Set(_, _, loc)] 
      | Instr[Asm(_, _, _, _, _, loc)] 
      | If(_, _, _, loc)
      | Goto (_, loc)
      | Loop(_, loc, _, _) ->
          self#makeIntraEdge s s1 loc

      | Instr[] -> 
          self#makeIntraEdge s s1 nowhere

      | Block block ->
	  if (Cil.hasAttribute "firstBlock" block.battrs) then
            self#makeIntraEdgeFirstBlock s s1 nowhere	      
	  else if (Cil.hasAttribute "callBlock" block.battrs) then
	    self#preMakePushRule s s1
	  else if (Cil.hasAttribute "afterCall" block.battrs) then
	    self#makeEdgeAfterCall s s1

      | Instr [Call (receiver, calleeExpr, args, loc)] ->
	  ignore()

      | Instr _ ->
	  begin
            ignore (bug "[handleStmt] Instr should have been atomized");
            ignore(Cil.dumpStmt wpdsPrinter pdserr 0 s);
            failwith "internal error 8";
	  end

      | _ ->
	  begin
            ignore( fprintf pdserr "\n\tOther stmt\n");
            self#makeIntraEdge s s1 nowhere;  
            ignore(Cil.dumpStmt wpdsPrinter pdserr 0 s);
            failwith "internal error 9";
	  end
    end;



 (* Return handled separately b/c it has no successors *)
  method vstmt(s: stmt) = begin
    match s.skind with

   | Block block ->
     if (Cil.hasAttribute "lastBlock" block.battrs) then
       begin
        if currentFunction.svar.vname == "main" then
          ( mainRetId <- s.sid );
	self#makePopRuleRet s;
        SkipChildren
       end
     else if (Cil.hasAttribute "firstBlock" block.battrs) or (Cil.hasAttribute "afterCall" block.battrs) or (Cil.hasAttribute "callBlock" block.battrs) then
       begin
        if (List.length block.bstmts > 0) then
	   let last = List.nth block.bstmts ((List.length block.bstmts)-1) in
	   begin
	     if (List.length last.succs > 0) then (*new*)
               List.iter (self#handleStmt s) last.succs
	     else
	       List.iter (self#handleStmt s) s.succs; (*new*)
	   end
        else
           List.iter (self#handleStmt s) s.succs;
 
        SkipChildren
       end
     else
       begin
        List.iter (self#handleStmt s) s.succs; 
        DoChildren
       end

    | _ ->
	if (List.length s.succs) > 0 then
          List.iter (self#handleStmt s) s.succs
	else
	    (* It is OK to create pop rules for statements other than 'Return' *)
	    (* There may be other statements with no successors due to CIL transformations *)
	    self#makePopRuleNoRet s nowhere;
        DoChildren
  end;


  method vfunc(f: fundec) =
    (*
       This atomizes the statements so that we only ever look
       at them 1 at a time. This makes creating the WPDS easier as
       Call instructions have been pulled out of blocks. However, it
       might make more sense at some time to simply split Blocks at 
       Call Instr...
     *)

    IsolateInstructions.visit f; (*Cindy*)

    (*
       Create the CFG.
       - Cfg.computeFileCFG is not used b/c it does not do the simplification
       prepareCfg does.
     *)

    prepareCFG f;
    computeCFGInfo f false;
    
    (*For debugging purposes*)
    (*    
       ignore(fprintf pdserr "FOR DEBUGGING\n";);
       ignore(fprintf pdserr "--Name of function: %s\n" f.svar.vname);
       List.iter (fun s -> ignore( fprintf stderr "%d -- " s.sid); printCilStmt s stderr) f.sallstmts;
     *)

    (*new*)
    (*List.iter (fun v -> ignore(fprintf pdserr "%s #### %s: " f.svar.vname v.vname)) f.slocals;*)
    (*new until here*)

    currentFunction <- f;
    DoChildren

end


(* This flag determines whether the generated WPDS Query will do prestar.
   It controls 2 output strings in feature.fd_doit *)
let doPrestar = ref false
let pdsName = ref ""
let dtd = ref None
let schema = ref None
let functionfile = ref None

(* If this flag is present, we will not use tentative errors *)
let hexastore = ref false


(* So output channels can be specified on the cmd line.
   --Note: only pdsout can be changed currently. *)
let pdsout = ref stdout
let pdserr = ref stderr


let rec insertBreaks calls =
  if List.length calls <> 0 then
    (hd calls)::(mkStmt (Break(locUnknown)))::(insertBreaks (tl calls))
  else
    []

(* Returns true is a main function is not found *)
let isMainNeeded file =
  let funcNames = List.map (fun g -> match g with 
  | GFun(fdec, loc) ->  fdec.svar.vname;
  | _ -> ""; ) file.globals in
  try 
    begin 
      ignore(List.find (fun name -> name = "main") funcNames);
      false;
    end;
  with
    Not_found -> true

	
(* Returns a statement consisting of a call to the given callee function *)
let createCallStmt file caller counter callee =
  try
    let global = List.find (( fun n g -> match g with 
    | GFun(fdec, loc) -> 
        if fdec.svar.vname = n then true else false;
    | _ -> false;
                             ) callee ) file.globals in
    match global with
    | GFun(fundec, loc) ->
        begin
          let ls_types = List.map (fun vinfo -> vinfo.vtype) fundec.sformals in
          let ls_actuals = List.map (fun t -> Lval(var (makeTempVar caller t))) ls_types in
          
          let call_instr = Call(None, Lval(var fundec.svar), ls_actuals, locUnknown) in
          let call_stmt = mkStmtOneInstr call_instr in
          begin
            call_stmt.labels <- [Case(integer !counter, locUnknown)];
	    incr counter;
            call_stmt;
          end;
        end;
    | _ ->  failwith ("[ERROR] " ^ callee ^ " is not a function")
	  
  with
  | Not_found -> failwith ("[ERROR] Cannot analyze " ^ callee ^ ": no implementation found")


(* Creates an artificial function funcName whose body consists of a list of calls to each 
   of the functions listed in functionsToCall *)
let createArtificalMain file funcName functionsToCall =
  let newFundec = Cil.emptyFunction funcName in

  (* Creating statements *)
  let counter = ref 0 in

  let default_case = mkEmptyStmt() in
  default_case.labels <- [ Default locUnknown ];
  let switch_stmt_list = ( List.map (createCallStmt file newFundec counter) functionsToCall ) @ [ default_case ] in
  let switch_block = mkBlock (insertBreaks switch_stmt_list) in
  let switch_exp = Lval(var(makeTempVar newFundec intType)) in
  let switch_stmt = mkStmt (Switch(switch_exp, switch_block, switch_stmt_list, locUnknown)) in

  let while_stmt = mkStmt (Loop(mkBlock [switch_stmt], locUnknown, None, None)) in
  let return_stmt = mkStmt (Return(None, locUnknown)) in

  (* Creating a block containing the statements *)
  let body = mkBlock [while_stmt; return_stmt] in             
  begin
    newFundec.sbody <- body;
    GFun(newFundec, locUnknown);
  end


let rec read_lines channel =
  try
    let line = input_line channel in
    line::(read_lines channel)
  with End_of_file -> close_in channel; [] 


let getFunctions filename =
  match !filename with
  | Some path ->  read_lines (open_in path);
  | None -> []



let xmlEscape text =
  let buffer = Buffer.create (String.length text) in
  let escapeChar = function
    | '<' | '>' | '&' | '\"' | '\'' as dangerous ->
	Printf.bprintf buffer "&#%d;" (int_of_char dangerous)
    | safe ->
	Buffer.add_char buffer safe
  in
  String.iter escapeChar text;
  Buffer.contents buffer



(* TODO: get rid of this xml somehow. this is no longer generic code *)
let feature : featureDescr = 
  { fd_name = "wpds";
    fd_enabled = ref false;
    fd_description = "save WPDS description and query (see --queryfile)";
    fd_extraopt = [
    ("--queryfile",
     Arg.String
       (fun s ->
         try let c = open_out s in pdsName :=s ; pdsout := c
         with Sys_error sc -> ignore( "Cannot open '%s'\n", s ) ),
     "<filename> Name of file to write WPDS Query XML file");
    ("--poststar",Arg.Unit (fun _ -> doPrestar := false),
     " Generate a poststar query (prestar is default)");
    ("--dtd", Arg.String (fun path -> dtd := Some path),
     "<filename> Attest compliance with the given DTD in the WPDS XML file");
    ("--schema", Arg.String (fun path -> schema := Some path),
     "<filename> Attest compliance with the given XML Schema in the WPDS XML file");
    ("--functionfile", Arg.String (fun path -> functionfile := Some path),
     "<filename> File that contains the names of the functions to be called from the automatically generated main function");
    ("--hexastore", Arg.Unit (fun _ -> hexastore := true),
     " Only use non-tentative errors");
    ("--error-codes", Arg.String (fun path -> Error.initialize path !hexastore),
     "<filename> File containing error code names and values");
    ("--transfer", Arg.Unit (fun _ -> transfer := true),
     " Perform error transfer instead of error copying");
    ("--copy", Arg.Unit (fun _ -> transfer := false),
     " Perform error copying instead of error transfer [default]");
    ("--positive", Arg.Unit (fun _ -> positive := true),
     " Assumes positive error codes");
    ("--negative", Arg.Unit (fun _ -> positive := false),
     " Assumes negative error codes [default]");
    ("--transf", Arg.Unit (fun _ -> transformation := true),
     " Performs error transformation");
    ("--notransf", Arg.Unit (fun _ -> transformation := false),
     " Does not perform error transformation [default]");
    ("--nounreachable", Arg.Unit (fun _ -> nounreachable := true),
     " Removes unreachable code");
    ("--noerrorhandling", Arg.Unit (fun _ -> noerrorhandling := true),
     " Does not recognize error-handling patterns");

  ];
    fd_doit = 
    (function (f: file) ->
      begin
        begin
	  match !dtd with
	  | None -> ()
	  | Some path ->
	      let escaped = xmlEscape path in
	      ignore( fprintf !pdsout "<!DOCTYPE Query SYSTEM '%s'>\n" escaped )
	end;


        (*Pointer analysis*)	    
        (*Ptranal.analyze_mono := false;*) (*Enables context sensitivity in Golf analysis*)
        (*Ptranal.analyze_file f;*)
        (*Ptranal.compute_results true;*)

	(********************Creating articial function main************************************)


        if isMainNeeded f then
          begin
            let functionsToCall = getFunctions functionfile in
            let newGlobal = createArtificalMain f "main" functionsToCall in
            f.globals <- f.globals@[newGlobal];
            ignore(fprintf !pdserr "+ Added an artificial main function\n");
          end;


	(********************Done creating articial function main************************************)

        (************************)
        connectFunctionPointers f;
        (************************)

        let vis0 = new sourceVisitor in
        let vis2 = new wpdsVisitor !pdsout !pdserr !hexastore in
        let query = if !doPrestar then "prestar" else "poststar" in
        begin
          visitCilFileSameGlobals (vis0 :> cilVisitor) f;
	  ReplaceCalls.replaceCallsVisitor f;

	  (* Early so that no exchange variables are created for it *)
	  ErrorTransformation.removeIsErrorFunction f !transformation; 
	  ExchangeVariables.createVariables f tableExchangeFormals tableExchangeReturn;
	  ErrorTransformation.transferFunctions !transformation !transfer !pdsout f !positive tableDerefVars;

	  if (not !noerrorhandling) then
	    ErrorHandling.transferFunctions !pdsout f !positive;

	  ExchangeVariables.insertAssignments f tableExchangeFormals tableExchangeReturn tableDerefVars;
          Conditionals.transferFunctions !pdsout f !positive;

	  if !E.verboseFlag then
	    ignore( E.log "Removing tmps\n" );

	  (* Removing tmps, including functions never called and no entry points *)
	  let entryPoints = getFunctions functionfile in
	  let isDefaultRoot global =
	    match global with
	    | GFun ({svar = v}, _) ->
		if  (String.compare v.vname "main" == 0) || (List.mem v.vname entryPoints) then
		  true
		else
		  false
		    
	    | _ ->
		Rmtmps.isExportedRoot global
	  in
	  Rmtmps.removeUnusedTemps ~isRoot:isDefaultRoot f; 

	  if !nounreachable then
	    RemoveUnreachableCode.removeUnreachableCodeVisitor f;	  

	  ignore (fprintf !pdsout "<Query type='%s'" query);
	  begin
	    match !schema with
	    | None -> ()
	    | Some path ->
		let escaped = xmlEscape path in
		ignore (fprintf !pdsout " xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='file://%s'" escaped)
	  end;
	  ignore (fprintf !pdsout ">\n");
	  ignore (fprintf !pdsout "\t<FWPDS>\n");
	  (*createExchangeVars f;*) (*dead code*)

	  FindVariables.findVariablesVisitor f !pdsout !transformation;

          visitCilFileSameGlobals (vis2 :> cilVisitor) f;
          ignore( fprintf !pdsout "\t</FWPDS>\n");
	  
          ignore( fprintf !pdsout "\t<WFA query='INORDER'>\n" );
          ignore( fprintf !pdsout "\t\t<State Name='p' initial='true'><Weight basis='identity'><zero/></Weight></State>\n" );
          ignore( fprintf !pdsout "\t\t<State Name='accept' final='true'><Weight basis='identity'><zero/></Weight></State>\n" );


          let mainId = if !doPrestar then vis2#getPrestarId() else 0 in
	  if mainId = 0 then
	    ignore( fprintf !pdsout "\t\t<Trans from='p' stack='main.0' to='accept'><Weight basis='identity'><one/></Weight></Trans>\n")
	  else
	    ignore( fprintf !pdsout "\t\t<Trans from='p' stack='main.%d' to='accept'><Weight basis='identity'><one/></Weight></Trans>\n" mainId );

          (*ignore( fprintf !pdsout "\t\t<Trans from='p' stack='ext2_delete_inode.first' to='accept'><Weight><one/></Weight></Trans>\n" );*)

          ignore( fprintf !pdsout "\t</WFA>\n" );
          ignore( fprintf !pdsout "</Query>\n" );
	  
        end;
        (if !pdsName <> "" then
          try close_out !pdsout
          with Sys_error sc -> ignore( fprintf !pdserr "Cannot close '%s'\n" !pdsName));
        
      end;
    );
    fd_post_check = true;
  } 


(*****Checking whether instructions are isolated*****)
(*print_string "before prepareCFG.. \n";
   CheckIsolated.checkIsolated theFile; 
   print_string "after prepareCFG...\n";*)
(*delete*) (*Rmtmps.removeUnusedTemps theFile;*)
(*delete*) (*Deadcodeelim.elim_dead_code f;*)
(*delete*) (*Cfg.cfgFun f;*)
(*delete*) (*ignore(Cil.dumpStmt wpdsPrinter pdserr 0 s);*)
