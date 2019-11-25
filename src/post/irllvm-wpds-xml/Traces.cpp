#include "Traces.hpp"
#include "FlowGraph.hpp"
#include "Location.hpp"
#include "Utility.hpp"
#include "TraceDatabase.hpp"
#include "Dataflow.hpp"
#include "FunctionContext.hpp"
#include <llvm/IR/Instructions.h>
#include <iostream>
#include <stack>
#include <vector>

using namespace std;
using namespace ep;
using namespace llvm;

#define DEBUG 0
bool debug = false;


Traces::Traces(ControlFlowPass *control_flow, string db_path,
    BranchSafetyPass *safety, NamesPass *names, PostDominatorTree *postdom) : 
      control_flow(control_flow), FG(control_flow->FG), 
      db_path(db_path), safety(safety), names(names), postdom(postdom) {

}

void Traces::initialize() {
  resolveHandlers();

  // boost::graph_traits<_FlowGraph>::vertices_size_type vtxs_processed = 0;
  // boost::graph_traits<_FlowGraph>::vertices_size_type num_vertices = boost::num_vertices(FG.G);

  flow_vertex_iter vi, vi_end;
  for (tie(vi, vi_end) = vertices(FG.G); vi != vi_end; ++vi) {
    FlowVertex vtx = FG.G[*vi];

    // cerr << "Processing " << vtx.stack << " (" << vtxs_processed++ << " of " << num_vertices << ")" << endl;

    auto stop = handlers_stop.find(vtx.stack);
    if (stop != handlers_stop.end()) {

      // Collect the actions from this handler
      traces.emplace(stop->first, collectHandlerActions(*vi, stop->second));
      
      if (prepost_context) {
	std::pair<PreActionTrace, PostActionTrace> pre_post = collectPrePostActions(*vi);
	pre_actions.emplace(stop->first, std::get<0>(pre_post));
	post_actions.emplace(stop->first, std::get<1>(pre_post));
      }
    }
  }
}

std::ostream& Traces::test_nesting(std::ostream &OS) const {
  for (std::pair<string, string> p : nesting_pairs) {
    OS << p.first << " -> " << p.second << "\n";
  }
  return OS;
}

// TODO: Convert to simple path enumeration over graph of connected handlersx
// TODO: Investigate partial overlap of instances in connected handlers
std::ostream& Traces::generate(std::ostream &OS) const {
  set<set<string>> done;

   // For each error-handler trace
  for (const auto &trace_it : traces) {
    string handler_id = trace_it.first;
    Trace trace = trace_it.second;
    
    cerr << handler_id << endl;

    if (trace.items.empty()) {
      continue;
    }

    set<string> handler_set;
    handler_set.insert(handler_id);

    if (done.find(handler_set) != done.end()) {
      continue; 
    } else {
      done.insert(handler_set);
    }
  }

  vector<Trace> traces_to_write;

  for (const auto &t : traces) {
    traces_to_write.push_back(t.second);
  }

  if (db_path.empty()) {
    // Raw output (primarily used for tests)
    for (const auto &t : traces_to_write) {
      t.raw(OS) << endl;

      // Write pre- and post-actions (intra-procedural context) for this handler
      if (prepost_context) {
	const auto &pre_iter = pre_actions.find(t.stack_id);
	if (pre_iter == pre_actions.end()) {
	  abort();
	}
	const PreActionTrace &pre_trace = pre_iter->second;
	pre_trace.raw(OS) << endl;

	const auto &post_iter = post_actions.find(t.stack_id);
	if (post_iter == post_actions.end()) {
	  abort();
	}
	const PostActionTrace &post_trace = post_iter->second;
	post_trace.raw(OS) << endl;
      }
    }
  } else {
    TraceDatabase TD(db_path);
    
    map<string, sqlite3_int64> handler_row_ids;

    for (const auto &t : traces_to_write) {
      sqlite3_int64 handler_id = TD.addHandlerTrace(t);
      handler_row_ids[t.stack_id] = handler_id;

      // Write pre-actions (intra-procedural context) for this handler
      if (prepost_context) {
	const auto &pre_iter = pre_actions.find(t.stack_id);
	if (pre_iter == pre_actions.end()) {
	  abort();
	}
	const PreActionTrace &pre_trace = pre_iter->second;
	TD.addPreActionTrace(handler_id, pre_trace);

	// Write post-actions for this handler
	const auto &post_iter = post_actions.find(t.stack_id);
	if (post_iter == post_actions.end()) {
	  abort();
	}
	const PostActionTrace &post_trace = post_iter->second;
	TD.addPostActionTrace(handler_id, post_trace);
      }
    }

    for (pair<string, string> p : nesting_pairs) {
      TD.addNested(handler_row_ids[p.first], handler_row_ids[p.second]);
    }
  }

  return OS;
}

Trace Traces::collectHandlerActions(flow_vertex_t start, string stop) {
  FlowVertex start_vtx = FG.G[start];

  Trace trace(start_vtx.stack);
  trace.location = handler2pred[start_vtx.stack];

  StandardActionMapper mapper(names, "HNDL");
  HandlerActionsVisitor cav(mapper, trace.items, stop, handlers, nesting_pairs);
  DepthFirstVisitor<_FlowGraph> visitor(cav);
  visitor.visit(start, FG.G);

  // Add function context to handler trace if enabled
  if (fcp) {
    for (string fn_name : fn_contexts[start_vtx.stack]) {
      Item fn_context = Item(Item::Type::CALL, Location(), fn_name);
      fn_context.tactic = "FN";
      trace.contexts.push_back(fn_context);
    }
  }

  if (ec_context) {
    set<string> passes_through;
    set<string> limited_context;
    set<string> reaches_handler = error_codes[trace.location];
    if (error_contexts.find(start_vtx.stack) != error_contexts.end()) {
      passes_through = error_contexts.at(start_vtx.stack);

      set_intersection(reaches_handler.begin(), reaches_handler.end(),
		   passes_through.begin(), passes_through.end(),
		   inserter(limited_context, limited_context.begin()));
    } else {
      limited_context = reaches_handler;
    }

    for (string ec : limited_context) {
      Item ec_context = Item(Item::Type::EC, trace.location, ec);
      ec_context.tactic = "EC";
      trace.contexts.push_back(ec_context);
    }
  }

  return trace;
}

std::pair<PreActionTrace, PostActionTrace> Traces::collectPrePostActions(flow_vertex_t handler) {
  llvm::Function *F = FG.G[handler].F;
  StandardActionMapper pre_mapper(names, "PRE");
  StandardActionMapper post_mapper(names, "POST");

  string handler_stack = boost::get(&FlowVertex::stack, FG.G, handler);
  PreActionTrace pre_trace(handler_stack);
  PreActionVisitor pre_vis(pre_mapper, pre_trace.contexts);
  DepthFirstVisitor<_FlowGraph> pre_dfs(pre_vis);
  flow_vertex_t fnVertex = control_flow->getFunctionVertex(F);

  pre_dfs.visit(fnVertex, handler, FG.G);

  FlowVertex start_vtx = FG.G[handler];

  PostActionTrace post_trace(handler_stack);
  PostActionVisitor post_vis(post_mapper, post_trace.items, pre_vis.get_discovered());
  DepthFirstVisitor<_FlowGraph> post_dfs(post_vis);
  post_dfs.visit(handler, FG.G);

  return std::make_pair(pre_trace, post_trace);
}


// This function takes the predicates read from the predicates file
// and populates the handler maps, including the function context map.
void Traces::resolveHandlers() {
  flow_vertex_iter vi, vi_end;

  for (tie(vi, vi_end) = vertices(FG.G); vi != vi_end; ++vi) {
    FlowVertex v = FG.G[*vi];
    if (predicates.find(v.loc) == predicates.end()) {
      continue;
    }
  
    if (BranchInst *branch = dyn_cast<BranchInst>(v.I)) {
      resolveBranch(branch, v);
    } 
  }  

  // The icmp resolver needs the results from the branch resolver,

  // So for now we just go through twice.
  for (tie(vi, vi_end) = vertices(FG.G); vi != vi_end; ++vi) {
    FlowVertex v = FG.G[*vi];
    if (predicates.find(v.loc) == predicates.end()) {
      continue;
    }
  
    if (ICmpInst *icmp = dyn_cast<ICmpInst>(v.I)) {
      resolveICmp(icmp);
    } 
  }  

  for (auto H : handler2pred) {
    handlers.insert(H.first);
  }
}

void Traces::resolveICmp(ICmpInst *icmp) {
  // Function contexts turned off
  if (!fcp) return;

  string handler_bbe;

  // Go through each use of the icmp to find branch instruction
  for (User *U : icmp->users()) {
    if (BranchInst *branch = dyn_cast<BranchInst>(U)) {
      if (branch_handlers.find(branch) != branch_handlers.end()) {
	handler_bbe = branch_handlers[branch];
	break;
      }
    }
  }

  if (handler_bbe.empty()) {
    return;
  }

  // icmp_in_facts has the funcion context for all variables
  Facts icmp_in_facts = fcp->facts[icmp].first;

  for (auto var_key : icmp_in_facts) {
    // Is this variable tested by this icmp instruction?
    VarName var = var_key.first;
    bool tested_here = safety->tested_at(var, icmp);
    if (!tested_here) continue;

    // Does this variable hold error codes?
    set<string> ec_vars = pred_vars[getSource(icmp)];
    bool holds_ec = ec_vars.find(var.name()) != ec_vars.end();
    if (!holds_ec) continue;
    
    for (string fn_call : var_key.second) {
      fn_contexts[handler_bbe].insert(fn_call);
    }
  }

  for (VarName en : safety->codesThroughPred(icmp)) {
    string str_ec = en.name();

    // We use non-tentative form of EC names in traces
    if (str_ec.find("TENTATIVE") == 0) {
	str_ec = str_ec.substr(10, string::npos);
    }

    error_contexts[handler_bbe].insert(str_ec);
  }
}

void Traces::resolveBranch(BranchInst *branch, FlowVertex V) {
  std::pair<BasicBlock*, BasicBlock*> branch_blocks = safety->getBranchBlocks(branch);

  BasicBlock *handler_block = branch_blocks.second;
  BasicBlock *not_handler_block = branch_blocks.first;

  if (!handler_block) return;

  // Handler is the empty else branch - do nothing
  postdom->runOnFunction(*handler_block->getParent());
  if (postdom->dominates(handler_block, not_handler_block)) return;

  // Find where control flow merges again
  BasicBlock *join_block = postdom->findNearestCommonDominator(handler_block, not_handler_block);
    
  string handler_stack;
  tie(handler_stack, std::ignore) = names->getBBNames(*handler_block);

  // No common post-dominator. Something funky going on.
  // We have to skip this error-handling block
  if (!join_block) return;

  string handler_off_stack;
  tie(handler_off_stack, std::ignore) = names->getBBNames(*join_block);

  handlers_stop[handler_stack] = handler_off_stack; 
  handler2pred[handler_stack] = V.loc;
  branch_handlers[branch] = handler_stack;
}

void Traces::read_predicates(string preds_path) {
  ifstream f_predicates(preds_path);
  string line;

  while(getline(f_predicates, line)) {
    string predicate_file, var, str_error_codes;
    int predicate_line;
    vector<string> vec_error_codes;

    istringstream line_iss(line);

    line_iss >> predicate_file;
    line_iss >> predicate_line;
    line_iss >> var;
    
    Location predicate_loc(predicate_file, predicate_line);
    predicates[predicate_loc].insert(Location());

    copy(istream_iterator<string>(line_iss),
	 istream_iterator<string>(),
	 back_inserter(vec_error_codes));
 
    pred_vars[predicate_loc].insert(var);

    for (string ec : vec_error_codes) {
      if (ec.find("TENTATIVE") == 0) {
	ec = ec.substr(10, string::npos);
      }
      error_codes[predicate_loc].insert(ec);
    }
  }

  f_predicates.close();

  initialize();
}

void Traces::read_predicates(DataflowResult dflow_res) {
  dflow_stack_range stack_range = dflow_res.getStackRange();
  dflow_stack_map::const_iterator s_it = get<0>(stack_range);
  dflow_stack_map::const_iterator s_e  = get<1>(stack_range);

  for (; s_it != s_e; ++s_it) {
    string stack = s_it->first;

    FRule rule = dflow_res.getRule(stack);

    if (!rule.pred_op1.empty() || !rule.pred_op2.empty()) {
      var_map vars = s_it->second;
      var_map::const_iterator v_it = vars.begin(), v_e = vars.end();
      for (; v_it != v_e; ++v_it) {
	std::set<VarName>::const_iterator ec_it = v_it->second.begin(), ec_e = v_it->second.end();
	for (; ec_it != ec_e; ++ec_it) {
	  
	  string full_ec = ec_it->name();
	  if (full_ec.find("!") != string::npos) {
	    string instance = full_ec.substr(full_ec.find("!"));
	    string code = full_ec.substr(0, full_ec.find("!"));
	    instance.erase(0, 1);
	    string file = instance.substr(0, instance.find(":"));
	    string line = instance.substr(instance.find(":"));
	    line.erase(0, 1);

	    Location predicate_loc = rule.location;
	    Location instance_loc(file, std::stoi(line));

	    predicates[predicate_loc].insert(instance_loc);
	    error_codes[predicate_loc].insert(code);
	    pred_vars[predicate_loc].insert(v_it->first.name());
	  }
	}
      }
    }
  }

  initialize();
}
