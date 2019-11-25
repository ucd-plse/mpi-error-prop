open Cil


module Key = struct
  type t = fundec

  let equal = (==)
      
  let hash func =
    Hashtbl.hash func.svar.vname
end


include HashClass.Make (Key)
