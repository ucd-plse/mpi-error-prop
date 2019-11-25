open Cil


module Key = struct
  type t = string

  let equal = (=)
      
  let hash name =
    Hashtbl.hash name

end


include HashClass.Make (Key)
