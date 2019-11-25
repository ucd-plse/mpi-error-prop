class type ['key, 'value] t =
  object ('self)
    method copy : 'self
    method clear : unit
    method clear : unit

    method add : 'key -> 'value -> unit
    method remove : 'key -> unit
    method replace : 'key -> 'value -> unit

    method find : 'key -> 'value
    method findAll : 'key -> 'value list
    method mem : 'key -> bool
    method empty : bool

    method iter : ('key -> 'value -> unit) -> unit
    method fold : ('key -> 'value -> 'result -> 'result) -> 'result -> 'result
  end


class ['key, 'value] c : int -> ['key, 'value] t


module type S = sig
  type key
  class ['value] c : int -> [key, 'value] t
end


module Make (Key : Hashtbl.HashedType) : S
    with type key = Key.t
