#ifndef TRACES_HPP
#define TRACES_HPP

#include "FlowGraph.hpp"
#include "Item.hpp"
#include "TraceVisitors.hpp"
#include "Utility.hpp"
#include "BranchSafety.hpp"
#include "Names.hpp"
#include "ControlFlow.hpp"
#include "Dataflow.hpp"
#include "FunctionContext.hpp"
#include "llvm/Analysis/PostDominators.h"
#include <boost/graph/reverse_graph.hpp>
#include <set>
#include <unordered_set>

class Trace {
public:
  Trace(string stack_id) : stack_id(stack_id) {}  

  // Serves as identifier for error handler
  std::string stack_id;

  // The response items in the trace
  std::vector<Item> items;

  // Trace contexts
  std::vector<Item> contexts;

  Location location;

  std::ostream& raw(std::ostream &OS) const {
    OS << "ID|" << stack_id << " ";
    for (Item c : contexts) {
      c.raw(OS) << " ";
    }
    for (Item i : items) {
      i.raw(OS) << " ";
    }
    return OS;
  }
};

class PreActionTrace : public Trace {
public:
  PreActionTrace(string stack_id) : Trace(stack_id) {}

  std::ostream &raw(std::ostream &OS) const {
    OS << "PRE_INTRA|" << stack_id << " ";
    for (Item c : contexts) {
      c.raw(OS) << " ";
    }
    for (Item i : items) {
      i.raw(OS) << " ";
    }
    return OS;
  };
  
};

class PostActionTrace : public Trace {
public:
  PostActionTrace(string stack_id) : Trace(stack_id) {}

  std::ostream &raw(std::ostream &OS) const {
    OS << "POST_INTRA|" << stack_id << " ";
    for (Item c : contexts) {
      c.raw(OS) << " ";
    }
    for (Item i : items) {
      i.raw(OS) << " ";      
    }
    return OS;
  };

};

class Traces {
public:
  // Uses a DataflowResult (such as from DataflowWali, the "lightweight" analysis)
  Traces(ControlFlowPass *control_flow, std::string db_path,
	 BranchSafetyPass *safety, NamesPass *names, llvm::PostDominatorTree *postdom);
  
  // Print the traces in a human readable format
  std::ostream& format(std::ostream &OS) const;

  std::ostream& generate(std::ostream &OS) const;

  // Give Traces a reference to FunctionContext pass
  // This has the effect of enabling function contexts in trace output,
  void setFunctionContextPass(FunctionContextPass *fcp) {
    this->fcp = fcp;
  }

  // Write unformatted traces to OS / db
  std::ostream& write_traces(std::ostream &OS) const;

  // Print nesting test output
  std::ostream& test_nesting(std::ostream &OS) const;
  // Configuration
  bool prepost_context  = false;
  bool function_context = false;
  bool ec_context       = false;

  // One of these needs to be called before generate / format
  void read_predicates(DataflowResult dflow_res);
  void read_predicates(string preds_path);

private:
  ControlFlowPass *control_flow;
  FlowGraph &FG;
  std::string db_path;

  BranchSafetyPass *safety;
  NamesPass *names;
  llvm::PostDominatorTree *postdom;
  FunctionContextPass *fcp = nullptr;

  // The individual error-handler traces
  // EH stack id -> trace
  map<string, Trace> traces;

  // The pre-actions (intraprocedural context) for each error-handler
  // EH stack id -> set of pre-actions
  map<string, PreActionTrace> pre_actions;

  // The post-actions for each error-handler
  // Post-actions are a superset of error-handler traces that do no stop at postdom
  // EH stack id -> set of ppost-actions
  map<string, PostActionTrace> post_actions;

  // Predicate Loc -> Instances being tested
  // The values in the predicate map or no longer used
  // TODO: Get rid of them
  std::map<Location, std::set<Location>> predicates;

  // Predicate Loc -> Error codes being tested
  std::map<Location, std::set<string>> error_codes;

  // handler bbe -> error code context
  // Specific error codes that allowed into a handler  
  // From branch safety, support for err == EC and err != EC
  std::map<string, std::set<string>> error_contexts;

  // Handler stack -> Function Names return value being tested (if enabled)
  std::map<string, std::unordered_set<string>> fn_contexts;

  // Predicate Loc -> variable names that can hold error codes
  std::map<Location, std::set<string>> pred_vars;

  // Handler stack -> Predicate eloc
  std::map<string, Location> handler2pred;

  // Handler On Stack -> Handler off stack
  std::map<string, string> handlers_stop;

  // Use this for testing if a stack location is a handler.
  std::unordered_set<string> handlers;

  // Branch instruction to handler bbe
  // Populated in resolveBranch
  // Used by resolveICmp to determine handler stack
  std::map<llvm::BranchInst*, string> branch_handlers;

  // List of nested error handlers that we discover during DFS
  // Pair of handler stack locations (parent, child)
  std::vector<std::pair<string, string>> nesting_pairs;

  // To give traces IDs
  unsigned id_cnt = 0; 

  void initialize();
  void resolveHandlers();  
  // Main place the handlers are identified
  void resolveBranch(llvm::BranchInst *branch, FlowVertex V); 
  // For function contexts
  void resolveICmp(llvm::ICmpInst *icmp);

  Trace collectHandlerActions(flow_vertex_t start, string stop);
  std::pair<PreActionTrace, PostActionTrace> collectPrePostActions(flow_vertex_t handler);
};

#endif
