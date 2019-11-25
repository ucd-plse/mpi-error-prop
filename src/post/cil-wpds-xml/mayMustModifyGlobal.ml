open Cil
open MayMustBase


module Details =
struct
  let prepareToAnalyze = ignore

  let itemsFromInstruction =
    let rec collect extractor = function
      | [] ->
	ItemSet.empty
      | candidate :: candidates ->
	match extractor candidate with
	| Var ({vglob = true} as varinfo), NoOffset
	  when FindVariables.isRelevant varinfo ->
	  ItemSet.add varinfo.vname (collect extractor candidates)
	| _ ->
	  collect extractor candidates
    in
    function
    | Set (lval, _, _)
    | Call (Some lval, _, _, _)
      ->
      collect (fun lval -> lval) [lval]
    | Call (None, _, _, _) ->
      ItemSet.empty
    | Asm (_, _, outputs, _, _, _) ->
      collect (fun (_, _, lval) -> lval) outputs

  let analysisDescription = "modify global"
  let featureName = "ModifyGlobal"
  let outputOptionName = "modify-global"
end


module Analysis = AnalysisBase (Details)

let feature = Analysis.feature
