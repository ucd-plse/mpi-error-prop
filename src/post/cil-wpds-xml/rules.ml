open Cil
open FindVariables
open Printf
open VarSet

type receiver = varinfo

type sender =
  | OK
  | Code of Error.code
  | Location of varinfo
  | Compound of compinfo

type trust = Trusted | Untrusted


let rec getReceiver = function
  | Var varinfo, NoOffset ->
      begin
	match unrollTypeDeep varinfo.vtype with
	| TInt _
	| TVoid _
	| TPtr (TInt _, _)
	| TPtr (TVoid _, _)
	| TPtr (TComp _, _)
	| TComp(_, _) ->
	    Some varinfo
	| _ ->
	    None
      end

  | Var varinfo, _ ->
      None

  | Mem Lval lvalue, NoOffset -> getReceiver lvalue
  | Mem (CastE(_, Lval lvalue)), _ -> getReceiver lvalue (*new*)
  | Mem _, _ -> None


let rec getSenderLval = function
  | Var varinfo, _ -> Some (Location varinfo)
  | Mem Lval lvalue, Field(_,_) -> Some OK (*if structName->field on RHS, not structName just OK*)
  | Mem Lval lvalue, _ -> getSenderLval lvalue
  | Mem _, _ -> None


let rec getSender = function
  | CastE (_, expr) -> getSender expr
  | Lval lvalue -> getSenderLval lvalue
  (*| Const (CInt64 (-811824149L, _, _)) -> Some Error*)
  | Const (CInt64 (value, _, _)) ->
      begin
	match Error.byValue value with
	| Some code -> Some (Code code)
	| None -> Some OK
      end
  | AddrOf lvalue -> getSenderLval lvalue

  | Const _
  | SizeOf _
  | SizeOfStr _
  | AlignOf _
  | UnOp _ 
  | SizeOfE _
  | AlignOfE _
  | BinOp _
  | StartOf _
    ->
      Some OK


let rec getRetValueLval = function
  | Var varinfo, _ ->
      begin
	match unrollTypeDeep varinfo.vtype with
	| TInt _
	| TPtr (_, _) ->
	    Some (Location varinfo)
	| _ ->
	    Some OK
      end
  | Mem Lval lvalue, _ -> getRetValueLval lvalue
  | Mem (CastE(_, Lval lvalue)), _ -> getRetValueLval lvalue (*new*)
  | Mem _, _ -> Some OK


let rec getRetValue = function
  | CastE (_, expr) -> getRetValue expr
  | Lval lvalue -> getRetValueLval lvalue
  | Const (CInt64 (value, _, _)) ->
      begin
	match Error.byValue value with
	| Some code -> Some (Code code)
	| None -> Some OK
      end
  | AddrOf lvalue -> getRetValueLval lvalue

  | UnOp(Neg, expr, _) -> 
      getRetValue expr

  | Const _
  | SizeOf _
  | SizeOfStr _
  | AlignOf _
  | SizeOfE _
  | AlignOfE _
  | UnOp _
  | BinOp _
  | StartOf _
    ->
      Some OK


let rec getOperandLval = function
  | Var varinfo, NoOffset ->
      begin
	match unrollTypeDeep varinfo.vtype with
	| TPtr (TInt _, _)
	| TPtr (TVoid _, _)
	| TPtr (TComp _, _) ->
	    Some varinfo
	| _ ->
	    None
      end

  | Var varinfo, _ ->
      None

  | Mem Lval lvalue, NoOffset -> getOperandLval lvalue
  | Mem (CastE(_, Lval lvalue)), _ -> getOperandLval lvalue
  | Mem _, _ -> None



let rec getOperand = function
  | CastE (_, expr)
  | UnOp(Neg, expr, _) -> 
      getOperand expr

  | Lval lvalue
  | AddrOf lvalue -> 
      getOperandLval lvalue

  | Const _
  | SizeOf _
  | SizeOfStr _
  | AlignOf _
  | SizeOfE _
  | AlignOfE _
  | UnOp _
  | BinOp _
  | StartOf _
    ->
      None



let receiverText receiver = receiver.vname


let senderText =
  let reserved () =
    Errormsg.s (Errormsg.bug "location with reserved name")
  in
  function
    | OK ->
	"OK"
    | Code code ->
	Error.name code
    | Location {vname = "OK"}
    | Compound {cname = "OK"} ->
	reserved()
    | Location {vname = name}
    | Compound {cname = name} ->
	match Error.byName name with
	| Some _ -> reserved ()
	| None -> name


let trustText = function
  | Trusted -> "true"
  | Untrusted -> "false"


let isSenderRelevant sender = 
  match sender with
  | Location varinfo -> isRelevant varinfo
  | _ -> true


let setTransfer sink newReceiver =
  ignore (fprintf sink "\t\t\t\t<set to='%s' from='OK' trusted='true'/>\n" (senderText newReceiver))


let set sink receiver sender trusted transfer =
  if Var_set.mem receiver !relevant then
    if isSenderRelevant sender then
      begin
	ignore (fprintf sink "\t\t\t\t<set to='%s' from='%s' trusted='%s'/>\n" (receiverText receiver) (senderText sender) (trustText trusted));
	if transfer then
	  match sender with
	  | Location _
	  | Compound _ -> setTransfer sink sender
	  | _ -> () 
      end
    else
      ignore (fprintf sink "\t\t\t\t<set to='%s' from='OK' trusted='%s'/>\n" (receiverText receiver) (trustText trusted))


let unimplemented sink reason =
  ignore (fprintf sink "\t\t\t\t<unimplemented reason='%s'/>\n" reason)


let source sink {line = line; file = file} =
  if line > 0 then
    ignore (fprintf sink "\t\t\t<source line='%d' file='%s'/>\n" line file)


let return sink expr =
  if isSenderRelevant expr then
    ignore (fprintf sink "\t\t\t<return value='%s'/>\n" (senderText expr))
  else
    ignore (fprintf sink "\t\t\t<return value='OK'/>\n")


let operand sink op =
  match op with
  | Some varinfo ->
      if isRelevant varinfo then
	ignore (fprintf sink "\t\t\t<operand name='%s'/>\n" varinfo.vname)
  | _ -> ()


let input sink varinfo =
  if isRelevant varinfo then
    ignore (fprintf sink "\t\t\t<input name='%s'/>\n" varinfo.vname)


let output sink expr =
  if isSenderRelevant expr then
    ignore (fprintf sink "\t\t\t<output value='%s'/>\n" (senderText expr))
  else
    ignore (fprintf sink "\t\t\t<output value='OK'/>\n")

let outputvar sink varinfo =
  if isRelevant varinfo then
    match unrollTypeDeep varinfo.vtype with
    | TPtr(_,_) ->
	ignore (fprintf sink "\t\t\t<output value='%s'/>\n" varinfo.vname)
    | _ -> ()
