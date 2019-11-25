open Cil
open MayMustBase


module Details =
struct
  let definedFunctions = new VariableNameHash.c 32

  let prepareToAnalyze file =
    definedFunctions#clear;
    iterGlobals file
      (function
	| GFun ({svar = svar}, _) ->
	  definedFunctions#add svar ()
	| _ ->
	  ()
      )

  let itemsFromInstruction = function
    | Call (_, Lval (Var callee, NoOffset), _, _)
	when definedFunctions#mem callee ->
      ItemSet.singleton callee.vname
    | Call _
    | Set _
    | Asm _ ->
      ItemSet.empty

  let analysisDescription = "call"
  let featureName = "Call"
  let outputOptionName = "call"
end


module Analysis = AnalysisBase (Details)

let feature = Analysis.feature
