open Cil
open VarSet


(* Find lvalues being used arithmetically.  Quietly assume *)
(* that these must not have contained unchecked errors. *)

val assumeNonError : out_channel -> stmt -> (Rules.receiver, unit) Hashtbl.t
