open Cil


val isolate : instr list -> stmtkind

val visit : fundec -> unit

val isolated : stmt -> instr option
