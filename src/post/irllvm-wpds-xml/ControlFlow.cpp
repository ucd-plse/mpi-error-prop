#include "ControlFlow.hpp"
#include "Utility.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;
using namespace std;
using namespace ep;

static llvm::RegisterPass<ControlFlowPass> C("control-flow", "Build CFG", false, false);
static cl::opt<string> WriteDot("dot-epcfg", cl::desc("Write dot file for EP CFG"));
static cl::opt<string> DotStart("dot-start", cl::desc("(Optional) function to start dot file at"));

// TODO: Strip disconnected components (have main function, never called)

bool ControlFlowPass::runOnModule(Module &M) {
  names = &getAnalysis<NamesPass>();

  Function *main = M.getFunction("main");
  FlowVertex main_v("main.0", main);
  FG.add(main_v);

  if (main) {
    BasicBlock &entry = main->getEntryBlock();
    string entry_name;
    tie(entry_name, std::ignore) = names->getBBNames(entry);
    FlowVertex entry_v(entry_name, main);
    FlowGraph::add_t added = FG.add(main_v, entry_v);
    fn2vtx[main] = std::get<0>(added);
  }

  for (Module::iterator f = M.begin(), e = M.end(); f != e; ++f) {
    if (f->isIntrinsic() || f->isDeclaration()) {
      continue;
    }

    FlowVertex fn_v(names->getCallName(*f), &*f);
    if (!main) {
      FlowGraph::add_t added = FG.add(main_v, fn_v);

      flow_edge_t edge_from_main = std::get<3>(added);
      FG.G[edge_from_main].main = true;
      
      flow_vertex_t fn_entry = std::get<1>(added);
      fn2vtx[&*f] = fn_entry;
    }

    // Connect function to first basic block
    BasicBlock &entry = f->getEntryBlock();
    string bbe;
    tie(bbe, std::ignore) = names->getBBNames(entry);
    FlowVertex entry_v(bbe, &*f);
    FG.add(fn_v, entry_v);

    runOnFunction(f);
  }

  for (Module::iterator f = M.begin(), e = M.end(); f != e; ++f) {
    addMayReturnEdges(*f);
  }

  if (!WriteDot.empty()) {
    write_dot(WriteDot);
  }

  return false;
}

flow_vertex_t ControlFlowPass::getFunctionVertex(const llvm::Function *F) const {
  return fn2vtx.at(F);
}

void ControlFlowPass::write_dot(const string path) {
  ofstream out(path);

  if (!DotStart.empty()) {
    string start_stack = DotStart + ".0";
    FG.write_graphviz(out, start_stack);
  } else {
    FG.write_graphviz(out);
  }

  out.close();
}

// We are building with -fno-exceptions, which means we have to do *something*
// if BOOST throws an exception. We cannot handle it gracefully.
void boost::throw_exception(std::exception const &e) {
  errs() << e.what() << "\n";
  abort();
}

void ControlFlowPass::runOnFunction(Function *F) {
  for (auto bi = F->begin(), be = F->end(); bi != be; ++bi) {
    string bb_enter, bb_exit;
    tie(bb_enter, bb_exit) = names->getBBNames(*bi);

    // Connect exit of each predecessor block to entry of this block
    FlowVertex bbe_v(bb_enter, F);
    for (auto pi = pred_begin(bi), pe = pred_end(be); pi != pe; ++pi) {
      BasicBlock *pred = *pi;
      string pred_exit; 
      tie(std::ignore, pred_exit) = names->getBBNames(*pred);
      FlowVertex predx_v(pred_exit, F);
      FG.add(predx_v, bbe_v);
    }
    
    BasicBlock *bb = &*bi;
    FlowVertex prev = bbe_v;
    for (auto ii = bb->begin(), ie = bb->end(); ii != ie; ++ii) {
      Instruction *i = &*ii;
      prev = visitInstruction(i, prev);

      FlowGraph::add_t added;
      if (i == bb->getTerminator()) {
        FlowVertex bbx_v(bb_exit, F);
        added = FG.add(prev, bbx_v);
      }

      // Populate fn2ret
      if (isa<ReturnInst>(i)) {
        flow_vertex_t ret_vtx = std::get<1>(added);
        if (!ret_vtx) {
          cerr << "FATAL ERROR: ControlFlowPass::visitInstruction received null return vertex\n";
          abort();
        }
        fn2ret[F] = ret_vtx;
      }
    }
  }
}

void ControlFlowPass::addMayReturnEdges(Function &F) {
  if (F.isIntrinsic() || F.isDeclaration()) {
    return;
  }

  string stack = names->getCallName(F);  
  flow_vertex_t vtx = FG.getVertex(stack);

  if (!vtx) {
    cerr << "FATAL ERROR: ControlFlowPass::addMayReturnEdges unable to find vertex for stack\n";
    abort();
  }

  vector<flow_vertex_t> return_sites;
  flow_in_edge_iter iei, iei_end;
  for (tie(iei, iei_end) = in_edges(vtx, FG.G); iei != iei_end; ++iei) {
    FlowEdge e = FG.G[*iei];
    if (e.call) {
      flow_vertex_t call_site = source(*iei, FG.G);

      // We add an edge to the vertex immediately after call site
      flow_out_edge_iter oei, oei_end;
      for (tie(oei, oei_end) = out_edges(call_site, FG.G); oei != oei_end; ++oei) {
        if (FG.G[*oei].ret) {
          flow_vertex_t ret_to = target(*oei, FG.G);
          return_sites.push_back(ret_to);
          break;
        }
      }
    }
  }

  // Goto return inst for function
  auto ret_vtx_it = fn2ret.find(&F);
  if (ret_vtx_it == fn2ret.end()) {
    cerr << "FATAL ERROR: ControlFlowPass::addMayReturnEdges unable to find return vertex\n";
    abort();
  }
  flow_vertex_t ret_from = ret_vtx_it->second;

  for (flow_vertex_t ret_to : return_sites) {
    bool success;
    flow_edge_t may_ret; 
    tie(may_ret, success) = boost::add_edge(ret_from, ret_to, FG.G);
    FG.G[may_ret].may_ret = true;
    if (!success) {
      cerr << "FATAL ERROR: ContolFlowPass::addMayReturnEdges failed to add return edge\n";
      abort();
    }
  }
}

FlowVertex ControlFlowPass::visitInstruction(Instruction *I, FlowVertex prev) {
  if (!I) {
    cerr << "FATAL ERROR: ControlFlowPass::visitInstruction called with null instruction\n";
    abort();
  }

  if (isa<IntrinsicInst>(I)) {
    return prev;
  }

  string iid = names->getStackName(*I);

  FlowVertex i_v(iid, getSource(I), I);   
  FlowGraph::add_t added = FG.add(prev, i_v);
  flow_vertex_t from = std::get<0>(added);

  // CallInst are not terminators, so all calls are guaranteed to be visited as prev
  if (prev.I && isa<CallInst>(prev.I)) {
      addCalls(from, i_v);  
  }

  return i_v;
}

void ControlFlowPass::addCalls(flow_vertex_t call_desc, FlowVertex ret_v) {
  FlowVertex call_v = FG.G[call_desc];
  CallInst *call = dyn_cast<CallInst>(call_v.I);
  if (!call) abort();

  Value *cv = call->getCalledValue();
  if (!cv) return;

  Function *f = call->getCalledFunction();
  if (f && f->isDeclaration()) {
    return;
  }

  vn_t callee_name = names->getVarName(cv);
  if (!callee_name) return;

  mul_t callees;
  if (callee_name->type == VarType::MULTI) {
    callees = static_pointer_cast<MultiName>(callee_name);
  } else {
    callees = make_shared<MultiName>();
    callees->insert(callee_name);
  }

  for (vn_t callee_vn : callees->names()) {
    if (callee_vn->type != VarType::FUNCTION) {
      // See test102, could be function pointer declaration we don't have
      // Nothing we can do
      return;
    }

    fn_t callee_fn = static_pointer_cast<FunctionName>(callee_vn);
    Function *callee = callee_fn->function;
    if (callee->isDeclaration()) continue;

    string call_name = names->getCallName(*callee);
    FlowVertex callee_v(call_name, call->getParent()->getParent());
    FG.add(call_v, callee_v, ret_v);
  }
}

void ControlFlowPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<NamesPass>();
  AU.setPreservesAll();
}

char ControlFlowPass::ID = 0;
