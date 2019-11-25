val findVariablesVisitor : Cil.file -> out_channel -> bool -> unit

val relevant : VarSet.Var_set.t ref
val isRelevant : Cil.varinfo -> bool
