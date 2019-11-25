open Cil


module Key = struct
  type t = varinfo

  let equal = (==)
      
  let hash var =
    Hashtbl.hash var.vname
end


include HashClass.Make (Key)
