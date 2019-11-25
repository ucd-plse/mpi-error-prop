open Cil


let rec checkLval = function
  | Var _, _ -> true
  | Mem expr, _ -> isLvalOK expr


and isLvalOK = function
  | CastE(TInt(_,_), Lval lvalue)
  | Lval lvalue -> checkLval lvalue
  | Const CInt64 _ -> true
  | _ -> false
