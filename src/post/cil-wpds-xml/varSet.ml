module Var_set = Set.Make(struct
                           type t = Cil.varinfo
                           let compare = compare
                          end)
