open Cil
open Pretty
open Utils

let d_sid _ = function
  | -1 -> text "none"
  | sid -> (chr 'n') ++ (num sid)
	
let d_node _ stmt =
  dprintf "%a [label=\"CFG #%i%cn%a%cn%s\"];@!"
    d_sid stmt.sid
    stmt.sid '\\'
    d_loc (get_stmtLoc stmt.skind) '\\'
    (stmt_what stmt.skind)

let d_edge _ source destination =
  dprintf "%a -> %a;@!"
    d_sid source.sid
    d_sid destination.sid
