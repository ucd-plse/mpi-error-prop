open Cil
open Set

(* components of a set rule *)
type receiver = varinfo
type sender = OK | Code of Error.code | Location of varinfo | Compound of compinfo
type trust = Trusted | Untrusted

(* convert CIL constructs into set rule components, if implemented *)
(* return None if the CIL construct is currently unimplemented *)
val getReceiver : lval -> receiver option
val getSender : exp -> sender option
val getRetValue : exp -> sender option
val getOperand : exp -> receiver option

(* emit a trusted or untrusted set rule from the sender to the receiver *)
val set : out_channel -> receiver -> sender -> trust -> bool -> unit

(* emit an unimplemented marker for the given reason *)
val unimplemented : out_channel -> string -> unit

(* emit source location information *)
val source : out_channel -> location -> unit

(* emit a return rule where sender is the value returned by the function *)
val return : out_channel -> sender -> unit

(* emit an operand rule where receiver is an arithmetic operand *)
val operand : out_channel -> receiver option -> unit

(* emit an input rule where varinfo is a parameter *)
val input : out_channel -> varinfo -> unit

(* emit an output rule where sender is a return value or a parameter variable *)
val output : out_channel -> sender -> unit

(* emit an output rule where varinfo is a pointer parameter *)
val outputvar : out_channel -> varinfo -> unit
