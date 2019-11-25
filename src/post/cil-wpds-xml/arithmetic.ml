open Cil


class visitor pds =
  object (self)
    inherit nopCilVisitor

    val receivers = Hashtbl.create 2
    method result = receivers

    method private collectOperand = function
      | Lval lvalue ->
	  begin
	    match Rules.getReceiver lvalue with
	    | Some receiver ->
		Hashtbl.add receivers receiver ();
		Rules.set pds receiver Rules.OK Rules.Trusted false
	    | None -> ()
	  end
      | _ -> ()

    method vblock _ = SkipChildren

    method vexpr expression =
      begin
	match expression with
	| Const _
	| Lval _
	| SizeOf _
	| SizeOfE _
	| SizeOfStr _
	| AlignOf _
	| AlignOfE _
	| UnOp (LNot, _, _)
	| BinOp(Lt, _, _, _)
	| BinOp(Gt, _, _, _)
	| BinOp(Le, _, _, _)
	| BinOp(Ge, _, _, _)
	| BinOp(Eq, _, _, _)
	| BinOp(Ne, _, _, _)
	| BinOp (LAnd, _, _, _)
	| BinOp (LOr, _, _, _)
	| CastE _
	| AddrOf _
	| StartOf _
	  ->
	    ()
	| UnOp (Neg, operand, _)
	| UnOp (BNot, operand, _)
	  ->
	    self#collectOperand operand
	| BinOp (PlusA, operand1, operand2, _)
	| BinOp (PlusPI, operand1, operand2, _)
	| BinOp (IndexPI, operand1, operand2, _)
	| BinOp (MinusA, operand1, operand2, _)
	| BinOp (MinusPI, operand1, operand2, _)
	| BinOp (MinusPP, operand1, operand2, _)
	| BinOp (Mult, operand1, operand2, _)
	| BinOp (Div, operand1, operand2, _)
	| BinOp (Mod, operand1, operand2, _)
	| BinOp (Shiftlt, operand1, operand2, _)
	| BinOp (Shiftrt, operand1, operand2, _)
	| BinOp (BAnd, operand1, operand2, _)
	| BinOp (BXor, operand1, operand2, _)
	| BinOp (BOr, operand1, operand2, _)
	  ->
	    self#collectOperand operand1;
	    self#collectOperand operand2
      end;
      DoChildren
  end


let assumeNonError pds statement =
  let visitor = new visitor pds in
  ignore (visitCilStmt (visitor :> cilVisitor) statement);
  visitor#result
