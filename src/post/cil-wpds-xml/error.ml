open Scanf

type code = int64

let newTable () = Hashtbl.create 131
let names = newTable ()
let values = newTable ()

let initialize filename hexastore =
  let buffer = Scanning.from_file filename in
  let insert name value =
    if hexastore then
      Hashtbl.add names value (name)
    else
      Hashtbl.add names value ("TENTATIVE_" ^ name)
  in
  while not (Scanning.end_of_input buffer) do
    bscanf buffer "%s\t%Ld\n" insert
  done

let byValue value =
  if Hashtbl.mem names value then
    Some value
  else
    None

let byName name =
  try Some (Hashtbl.find values name)
  with Not_found -> None

let name code =
  Hashtbl.find names code
