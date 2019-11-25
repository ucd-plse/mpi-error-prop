open Cil

  
class virtual visitor =
  object
    inherit SkipVisitor.visitor

    method vglob = function
      | GFun _ -> DoChildren
      | _ -> SkipChildren
	    
    method vfunc _ = DoChildren

    method vblock _ = DoChildren
  end
