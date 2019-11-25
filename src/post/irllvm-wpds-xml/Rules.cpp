#include "Rules.hpp"
#include "Names.hpp"
#include "VarName.hpp"
#include "Location.hpp"
#include "Utility.hpp"
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/Pass.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/InstrTypes.h>
#include <fstream>
#include <sstream>
#include <stdint.h>

#include <boost/graph/graph_utility.hpp>

// An instruction ID identifies the program point immediately after that instruction is executed
// Each basic block as an entry and an exit point added

using namespace llvm;
using namespace std;
using namespace ep;

char RulesPass::ID = 0;
static llvm::RegisterPass<RulesPass> X("rules", "Create WPDS Rules from LLVM IR", false, false);

bool RulesPass::runOnModule(Module &M) {
  names = &getAnalysis<NamesPass>();
  safety = &getAnalysis<BranchSafetyPass>();

  // Create first rule
  Function *main = M.getFunction("main");
  if (main) {
    BasicBlock &entry = main->getEntryBlock();
    string entry_name;
    tie(entry_name, std::ignore) = names->getBBNames(entry);
    FRule r("main.0", entry_name);
    r.basis = "uninitialized";
    addRule(r);
    prev = names->getStackName(entry.front());
  }

  // Functions in module
  for (auto fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
    if (fi->isIntrinsic() || fi->isDeclaration()) {
      continue;
    }

    // Map local variables to uninitialized for each function
    // Rules are foo.0 -> bb_entry, basis identityGlobals
    if (&*fi != main) {
      BasicBlock &entry = fi->getEntryBlock();
      string call_bb;
      tie(call_bb, std::ignore) = names->getBBNames(entry);
      FRule r_ident(names->getCallName(*fi), call_bb);
      r_ident.basis = "identityGlobals";
      addRule(r_ident);
    }

    // No main function, we need to explicitly create a rule to this function
    // Rules are main.0 -> foo.0
    if (!main) {
      FRule lib_f("main.0", names->getCallName(*fi));
      addRule(lib_f);
    }

    // Basicblocks in function
    for (auto bi = fi->begin(), be = fi->end(); bi != be; ++bi) {
      // Each basic block has enter and exit point
      string bb_enter, bb_exit;
      tie(bb_enter, bb_exit) = names->getBBNames(*bi);

      // Connect exit of each predecessor block to entry of this block
      for (auto pi = pred_begin(bi), pe = pred_end(be); pi != pe; ++pi) {
        BasicBlock *pred = *pi;
        string pred_exit;
        tie(std::ignore, pred_exit) = names->getBBNames(*pred);
        FRule r(pred_exit, bb_enter);
        addRule(r);
      }

      // Add rule clearing safe names for this block
      string safe_stack = names->getDummyName(&bi->front());
      FRule safe(bb_enter, safe_stack);
      set<VarName> safe_names = safety->getSafeNames(*bi);
      VarName OK = ErrorName("OK");
      for(set<VarName>::iterator si = safe_names.begin(), se = safe_names.end(); si != se; ++si) {
        safe.addAssignment(OK, *si, true);
      }
      addRule(safe);

      // Visit each instruction in this block
      // Each visitor function will connect from and reset prev
      prev = safe_stack;
      visit(*bi);

      // Connect the last instruction visited with the exit of the block
      // If prev is empty then that was the last block, so the function has returned
      if (!prev.empty()) {
        FRule r(prev, bb_exit);
        addRule(r);
        // Adding rule for exit
        FRule r2(bb_exit,"exit.0");
        addRule(r2);
        //fprintf(stderr, "\nAdded rule to exit.0");
      }
    }
  }

  // Filter out variables that cannot possibly hold an error code
  // This also collects names that will go in prologue
  // RulesPrinter does not print assignments with names not in prologue
  NameGraph NG(rules);
  NG.filter();

  RulesPrinter RP(schema_path, rules);

  // Print the WPDS file
  ofstream outfile;
  outfile.open(wpds_out_path, std::ofstream::out);
  outfile << RP.formatRules(false);
  outfile.close();

  return false;
}

// Adds the rule to the RulesPrinter and constructs NameGraph of assignments
// The NameGraph is used to filter out unused variable names
void RulesPass::addRule(FRule &R) {
  if (R.from_stack.empty()) {
    errs() << "FATAL ERROR: RulePass::addRule called with empty from_stack.\n";
    abort();
  }

  rules.push_back(R);
}

void RulesPass::visitStoreInst(StoreInst &I) {
  string iid = names->getStackName(I);

  Value *sender = I.getOperand(0);
  Value *receiver = I.getOperand(1);

  vn_t senderName   = names->getVarName(sender);
  vn_t receiverName = names->getVarName(receiver);
  set<vn_t> senders;

  if (!senderName || !receiverName) {
    // Always add a rule for a store, even if it is meaningless (test169)
    // Need this to get all of the source locations. Isn't converting back to source fun?
    FRule r(prev, iid);
    r.location = getSource(&I);
    addRule(r);
    prev = iid;
    return;
  }

  if (senderName->type == VarType::MULTI) {
    mul_t multi_sender = static_pointer_cast<MultiName>(senderName);
    for (vn_t s : multi_sender->names()) {
      senders.insert(s);
    }
  } else {
    senders.insert(senderName);
  }

  if (senders.size() == 0) {
    //cerr << "No senders\n";
    FRule r(prev, iid);
    r.location = getSource(&I);
    addRule(r);
    prev = iid;
    return;
  }

  for (vn_t s : senders) {
    FRule r(prev, iid);
    r.addAssignment(*s, *receiverName);
    r.location = getSource(&I);
    addRule(r);

    // Hack to make output match up with old pass.
    // It prints the source location twice upon assigment from return.
    if (s->name().find("$return") != string::npos) {
      string dummy_name = names->getDummyName(&I);
      FRule dummy(iid, dummy_name);
      dummy.location = getSource(&I);
      addRule(dummy);
      iid = dummy_name;
    }
  }

  prev = iid;
}

void RulesPass::visitReturnInst(ReturnInst &I) {
  //  string iid = names->getStackName(I);

  Function *f = I.getParent()->getParent();
  Value *ret_value = I.getReturnValue();

  vector<VarName> ret_names;
  if (ret_value != nullptr) {
    vn_t ret_varname = names->getVarName(ret_value);
    if (ret_varname && ret_varname->type != VarType::MULTI) {
      ret_names.push_back(*ret_varname);
    } else if (ret_varname && ret_varname->type == VarType::MULTI) {
      mul_t ret_varnames = static_pointer_cast<MultiName>(ret_varname);
      for (vn_t rn : ret_varnames->names()) {
        ret_names.push_back(*rn);
      }
    }
  }

  // Out of scope assignments
  // Do not include the value being returned
  FRule r(prev);
  set<Value*> locals = names->getLocalValues(*f);
  for (set<Value*>::iterator li = locals.begin(), le = locals.end(); li != le; ++li) {
    Value *lookup = *li;
    vn_t local = names->getVarName(lookup);
    if (local && find(ret_names.begin(), ret_names.end(), *local) == ret_names.end()) {
      r.addAssignment(ErrorName("OK"), *local);
    }
  }

  // Unsaved assignments
  for (auto s : tmp_names[f]) {
    r.addAssignment(ErrorName("OK"), s);
  }

  if (!ret_names.empty() && f->getName() != "main") {
    VarName exchange_name = IntName(string(f->getName()) + "$return");
    // We need an extra rule to pass test191
    for (VarName ret_var : ret_names) {
      r.addAssignment(ret_var, exchange_name, true);
      r.return_value = ret_var;
    }
  }

  r.location = getSource(&I);
  addRule(r);

  // Indicates this is the last instruction in the block
  prev = "";
}

void RulesPass::visitICmpInst(ICmpInst &I) {
  string iid = names->getStackName(I);

  vn_t op1 = names->getVarName(I.getOperand(0));
  vn_t op2 = names->getVarName(I.getOperand(1));

  FRule r(prev, iid);
  r.location = getSource(&I);

  if (op1 && op1->type != VarType::MULTI) {
    r.pred_op1 = op1->name();
  }
  if (op2 && op2->type != VarType::MULTI) {
    r.pred_op2 = op2->name();
  }

  addRule(r);

  prev = iid;
}

void RulesPass::visitBranchInst(BranchInst &I) {
      string iid = names->getStackName(I);
      FRule r(prev, iid);
      addRule(r);
      prev = iid;
}

void RulesPass::visitCallInst(CallInst &I) {
  // Ignore llvm.dbg calls
  if (isa<IntrinsicInst>(I)) {
    return;
  }

  string iid = names->getStackName(I);

  Value *cv = I.getCalledValue();
  if (!cv) return;

  Function *f = I.getCalledFunction();
  if (f && f->isDeclaration()) {
    // If this is just a declaration then there will be no basic blocks
    // This happens for system functions like printf
    // We still need to add a rule over this location to match
    // the output of previous front end, but do not go into function
    FRule r(prev, iid);
    r.location = getSource(&I);
    addRule(r);
    prev = iid;
    return;
  }

  vn_t callee_name = names->getVarName(cv);
  mul_t callees;
  if (!callee_name) {
    return;
  }

  if (callee_name->type == VarType::MULTI) {
    callees = static_pointer_cast<MultiName>(callee_name);
  } else {
    // Create a fake MultiName to loop over to simplify cases below
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

    if (callee->isDeclaration()) {
      FRule r(prev, iid);
      r.location = getSource(&I);
      addRule(r);
    } else {

      // Push rule for call
      // We use the function name so that bb name is free
      // for identityGlobals rule.
      FRule r_call(prev, names->getCallName(*callee), iid);
      r_call.location = getSource(&I);

      // Copy parameters to global exchange vars
      int op = 0;
      Function::ArgumentListType &args = callee->getArgumentList();
      for (auto a = args.begin(), ae = args.end(); a != ae; ++a) {
        Value *param = I.getArgOperand(op);
        vn_t param_name = names->getVarName(param);
        vn_t formal_arg = names->getVarName(a);
        string fname = I.getParent()->getParent()->getName();
        if (param_name && formal_arg) {
          if (param_name->type == VarType::MULTI) {
            r_call.addMultiAssignment(param_name, *formal_arg);
          } else {
            r_call.addAssignment(*param_name, *formal_arg);
          }
        }
        op++;
      }

      addRule(r_call);

      // Check to see if this value is ever saved
      bool saved = false;
      for (User *U : I.users()) {
        if (isa<StoreInst>(U) || isa<ReturnInst>(U)) {
          saved = true;
          break;
        }
      }

      // This return value has not been saved yet
      // We generate tmp names here because they are not tied to a value
      Type *t = callee->getReturnType();
      if (!saved && names->filter(t))  {
        string dummy_stack = names->getDummyName(&I);
        Function *f = I.getParent()->getParent();
        string fname = f->getName();

        IntName tmp_name(fname + "#__cil_tmp" + to_string(tmp_cnt++), f);
        IntName ret_exchange(callee_vn->name() + "$return");

        FRule no_save(iid, dummy_stack);
        no_save.location = getSource(&I);
        no_save.addAssignment(ret_exchange, tmp_name);
        addRule(no_save);

        // So return instructions will generate OK assignments for this tmp
        tmp_names[f].push_back(tmp_name);

        iid = dummy_stack;
      }
    }
  }

  prev = iid;
}

// Selects chose one of two values based on a condition without branching.
// We just assume it could be either value by creating two assignments.
void RulesPass::visitSelectInst(SelectInst &I) {
  string iid = names->getStackName(I);

  vn_t gen = names->getVarName(&I);
  vn_t T = names->getVarName(I.getTrueValue());
  vn_t F = names->getVarName(I.getFalseValue());

  if (!gen || !T || !F) {
    return;
  }

  FRule r(prev, iid);
  r.location = getSource(&I);
  r.addAssignment(*T, *gen);
  r.addAssignment(*F, *gen);
  addRule(r);

  prev = iid;
}

void RulesPass::visitLoadInst(LoadInst &I) {
  // Do nothing. We don't want rules printed for load instructions.
  // These could have source locs so default would print rule
  // The name, however, will be of a variable not a proper stack location
  (void)I;
}

void RulesPass::visitSwitchInst(SwitchInst &I) {
  // Do nothing. To match output from previous LLVM pass
  (void)I;
}

void RulesPass::visitBinaryOperator(BinaryOperator &I) {
  // Do nothing. To match output from previous LLVM pass
  (void)I;
}


// Default case, do nothing but add rule if there is source loc
// This is required to meet previous pass output,
// and it makes the witnesses a little more readable
void RulesPass::visitInstruction(Instruction &I) {
  Location source = getSource(&I);

  if (! source.file.empty()) {
    string iid = names->getStackName(I);
    FRule r(prev, iid);
    r.location = source;
    addRule(r);
    prev = iid;
  }
}

void RulesPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<NamesPass>();
  AU.addRequired<BranchSafetyPass>();
}
