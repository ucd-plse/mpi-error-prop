open Cil

class virtual visitor =
  object
    inherit nopCilVisitor
	
    method vvrbl _ = SkipChildren
    method vvdec _ = SkipChildren
    method vexpr _ = SkipChildren
    method vlval _ = SkipChildren
    method voffs _ = SkipChildren
    method vinitoffs _ = SkipChildren
    method vinst _ = SkipChildren
    method vstmt _ = SkipChildren
    method vblock _ = SkipChildren
    method vfunc _ = SkipChildren
    method vglob _ = SkipChildren
    (*method vinit _ = SkipChildren*)
    method vinit _ _ _ = SkipChildren
    method vtype _ = SkipChildren
    method vattr _ = SkipChildren
  end
