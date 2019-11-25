open Cil
open Dataflow
open Pretty


(**********************************************************************)

  
(** set of names of items that may/must appear along a path *)
module ItemSet = Set.Make(
  struct
    type t = string
    let compare = compare
  end
)


(**********************************************************************)


(** everything needed to instatiate a specific may/must analysis *)
module type TransferDetails = sig

  (** prepare to analyze a single file *)
  val prepareToAnalyze : file -> unit

  (** collectable items that appear in a given instruction *)
  val itemsFromInstruction : instr -> ItemSet.t

  (** string fragment for help text *)
  val analysisDescription : string

  (** string fragment for command-line option to activate analysis *)
  val featureName : string

  (** string fragment for command-line option to redirect analysis output *)
  val outputOptionName : string
end


(**********************************************************************)


(** concrete analysis instantation as selectable cilly feature *)
module type AnalysisBase = sig
  val feature : featureDescr
end


(** generic, instantiable may/must analysis *)
module AnalysisBase = functor (Details : TransferDetails) -> struct

  (** output stream for writing analysis results *)
  let outStream = ref stdout

  (** add instruction's collectable item (if any) to a set *)
  let addFromInstruction instruction =
    ItemSet.union (Details.itemsFromInstruction instruction)

  (** dataflow transfer module for use with {! ForwardsDataFlow } *)
  module Transfer = struct

    (** simple utility function; used in {! copy } and {! computeFirstPredecessor } *)
    let identity thing = thing

    let name = Printf.sprintf "may/must %s analysis" Details.analysisDescription

    let debug = ref false

    type t = ItemSet.t

    let copy = identity

    let stmtStartData =
      (* initial size is selected arbitrarily *)
      Inthash.create 32

    let pretty () items =
      dprintf "{%a}" (docList text) (ItemSet.elements items)

    let computeFirstPredecessor _ = identity

    let combinePredecessors _ ~old:prior updated =
      (* facts are equal iff they are sets with equal elements *)
      if ItemSet.equal prior updated then
	None
      else
	(* must analysis, so use intersection to join facts *)
	Some (ItemSet.inter prior updated)

    let doInstr instruction incoming =
      (* delegate single-instruction analysis to instantiation *)
      Done (addFromInstruction instruction incoming)

    let doStmt _ _ = SDefault

    let doGuard _ _ = GDefault

    let filterStmt _ = true
  end


  (** instantiated analysis using transfer module *)
  module DataFlow = ForwardsDataFlow (Transfer)


  (** destructively replace statement start data with statement finish data *)
  class convertStartToFinishVisitor = object
    inherit SkipVisitor.visitor

    (** finish data for statement currently being visited *)
    val mutable statementFinishData = ItemSet.empty

    method vblock _ = DoChildren

    method vstmt statement =
      try
	(* initialize finish data to be same as start data *)
	statementFinishData <- Inthash.find Transfer.stmtStartData statement.sid;
	let postAction statement =
	  (* replace start data with computed finish data after visiting children *)
	  Inthash.replace Transfer.stmtStartData statement.sid statementFinishData;
	  statement
	in
	ChangeDoChildrenPost (statement, postAction)
      with Not_found ->
	(* statement is unreachable; nothing else to do here *)
	SkipChildren

    method vinst instruction =
      (* add collectable item from this instruction (if any) *)
      statementFinishData <- addFromInstruction instruction statementFinishData;
      SkipChildren
  end


  (** perform may/must analysis of all functions in a single file *)
  (* note: CFG must be up-to-date for all functions in file *)
  let analyze file =
    (* dumpFile plainCilPrinter stderr "" file; *)
    Details.prepareToAnalyze file;
    iterGlobals file
      (function
      | GFun (fundec, _) ->
	let facts = Transfer.stmtStartData in
	let body = fundec.sbody in

	(* collectable items that {i must} arise on all paths *)
	let must =
	  (* initialize start data as empty set for first statement *)
	  let start = List.hd body.bstmts in
	  Inthash.clear facts;
	  Inthash.add facts start.sid ItemSet.empty;

	  (* compute dataflow fixed-point *)
	  DataFlow.compute [start];

	  (* convert facts to reflect state after each statemnt instead of before *)
	  ignore (visitCilBlock (new convertStartToFinishVisitor) body);

	  (** compute intersection of two sets, either of which might be missing due to unreachable code *)
	  let intersectIfAvailable a b =
	    match a, b with
	    | None, b -> b
	    | a, None -> a
	    | Some a, Some b -> Some (ItemSet.inter a b)
	  in

	  (** look up analysis fact; could be missing if unreachable code *)
	  let factFor statement =
	    try
	      Some (Inthash.find facts statement.sid)
	    with Not_found ->
	      (* statement is unreachable *)
	      None
	  in

	  (** return statements: endpoints for dataflow analysis *)
	  let exitStatements = snd (Dataflow.find_stmts fundec) in

	  (** intersection of must sets across all reachable return statements *)
	  let intersection =
	    List.fold_left
	      (fun accumulated exitStatement ->
		intersectIfAvailable accumulated (factFor exitStatement))
	      None
	      exitStatements
	  in

	  (* must items for entire function *)
	  match intersection with
	  | Some must -> must
	  | None -> ItemSet.empty
	in

	(** collectable items that {i may} arise on some paths *)
	let may =
	  (* union of must sets across all reachable statements *)
	  Inthash.fold
	    (fun _ -> ItemSet.union)
	    facts
	    ItemSet.empty
	in

	(* print analysis results *)
	ItemSet.iter
	  (fun item ->
	    let certainty =
	      if ItemSet.mem item must then
		'!' (* must *)
	      else
		'?' (* may *)
	    in
	    (* one line per function, per collectable item *)
	    ignore (fprintf !outStream "%s %s %c\n" fundec.svar.vname item certainty))
	  may
      | _ ->
	())

  (** analysis as selectable cilly feature *)
  let feature = {
    fd_name = "MayMust" ^ Details.featureName;
    fd_enabled = ref false;
    fd_description = Transfer.name;
    fd_extraopt = [
      Printf.sprintf "--may-must-%s-output" Details.outputOptionName,
      Arg.String
	(function path ->
	  try
	    outStream := open_out path
	  with Sys_error explanation ->
	    ignore (Printf.eprintf "cannot write to %s: %s\n" path explanation)) ,
      "<filename> Output filename for " ^ Transfer.name
    ];
    fd_post_check = false;
    fd_doit = analyze;
  }
end
