open Cil
open Pretty


val d_sid : unit -> int -> doc
val d_node : unit -> stmt -> doc
val d_edge : unit -> stmt -> stmt -> doc
