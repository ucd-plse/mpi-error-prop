open Cil
open Dotify
open Pretty
  

let d_cfg () stmts =
  let out stmt =
    d_node () stmt ++
      seq nil (d_edge () stmt) stmt.succs
  in
  
  text "digraph CFG {" ++ line ++
    (indent 2
       (seq nil out stmts)) ++
    chr '}' ++ line;
