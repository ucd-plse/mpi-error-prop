open Cil


(** set of names of items that may/must appear along a path *)
module ItemSet : Set.S with type elt = string


(** everything needed to instatiate a specific may/must analysis *)
module type TransferDetails = sig

  (** prepare to analyze a single file *)
  val prepareToAnalyze : file -> unit

  (** collectable item that appears in a given instruction, if any *)
  val itemsFromInstruction : instr -> ItemSet.t

  (** string fragment for help text *)
  val analysisDescription : string

  (** string fragment for command-line option to activate analysis *)
  val featureName : string

  (** string fragment for command-line option to redirect analysis output *)
  val outputOptionName : string
end


(** concrete analysis instantation as selectable cilly feature *)
module type AnalysisBase = sig
  val feature : featureDescr
end


(** generic, instantiable may/must analysis *)
module AnalysisBase :
  functor (Details : TransferDetails) ->
    AnalysisBase
