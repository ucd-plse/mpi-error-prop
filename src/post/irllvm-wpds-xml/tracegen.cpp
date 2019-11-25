 #include "ControlFlow.hpp"
 #include "BranchSafety.hpp"
 #include "Rules.hpp"
 #include "Traces.hpp"
 #include "DataflowWali.hpp"
 #include "FunctionContext.hpp"
 #include "llvm/IR/LegacyPassManager.h"
 #include "llvm/IR/LLVMContext.h"
 #include "llvm/IR/Module.h"
 #include "llvm/IRReader/IRReader.h"
 #include "llvm/Pass.h"
 #include "llvm/Support/raw_ostream.h"
 #include "llvm/Support/SourceMgr.h"
 #include "llvm/Analysis/PostDominators.h"
 #include "llvm/Analysis/MemoryDependenceAnalysis.h"
 #include <unistd.h>
 #include <libconfig.h++>

 using namespace llvm;
 using namespace std;
 using namespace libconfig;

 void usage() {
   cerr << "Usage: " << "tracegen -c <config file> -b <bitcode file> [-p predsfile ] [-e ecfile] [-d dbfile] [-c]\n";
   cerr << "-c path to config file\n";
   cerr << "-e path to error codes file\n";
   cerr << "-p path to preds file\n";
   cerr << "-d to write results to sqlite database\n";
 }

 int main(int argc, char **argv) {
   string bitcode_path;
   string db_path;
   string ec_path;
   string config_path;
   string preds_path;
   bool prepost_context  = false;
   bool function_context = false;
   bool ec_context       = false;

   int c;

   while ((c = getopt(argc, argv, "e:c:b:d:p:")) != EOF) {
     switch (c) {
     case 'e':
       ec_path = optarg;
       break;
     case 'c':
       config_path = optarg;
       break;
     case 'b':
       bitcode_path = optarg;
       break;
     case 'd':
       db_path = optarg;
       break;
     case 'p':
       preds_path = optarg;
       break;
     case ':':
     case '?':
       usage();
       return 1;
     }
   }

   if (bitcode_path.empty()) {
     usage();
     return 1;
   }

   Config cfg;
   if (! config_path.empty()) {
     cfg.readFile(config_path.c_str());
   }

   if (ec_path.empty() && cfg.exists("ecfile")) {
     cfg.lookupValue("ecfile", ec_path);    
   }
   if (ec_path.empty()) {
     usage();
     return 1;
   }

   if (cfg.exists("contexts")) {
     const Setting &contexts = cfg.getRoot()["contexts"];
     for (int i = 0; i < contexts.getLength(); ++i) {
       string context = contexts[i];
       if (context == "prepost") {
	 prepost_context = true;
       } else if (context == "function") {
	 function_context = true;
       } else if (context == "ec") {
	 ec_context = true;
       }
     }
  }
    
  if (preds_path.empty() && cfg.exists("preds_file")) {
    cfg.lookupValue("preds_file", preds_path);
  }

  bool test_nesting = false;
  cfg.lookupValue("test_nesting", test_nesting);
  
  SMDiagnostic Err;
  std::unique_ptr<Module> Mod(parseIRFile(bitcode_path, Err, getGlobalContext()));

  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  legacy::PassManager PM;
  NamesPass *names = new NamesPass(ec_path);
  PM.add(names);

  RulesPass *rules_pass = new RulesPass();
  PM.add(rules_pass);

  BranchSafetyPass *safety = new BranchSafetyPass();
  PM.add(safety);

  ControlFlowPass *cfp = new ControlFlowPass();
  PM.add(cfp);

  PostDominatorTree *postdom = new PostDominatorTree();
  PM.add(postdom);

  MemoryDependenceAnalysis *mda = new MemoryDependenceAnalysis();
  PM.add(mda);

  FunctionContextPass *fcp = new FunctionContextPass();
  if (function_context) {   
    PM.add(fcp);
  }
  
  cerr << "Running frontend passes...\n";
  PM.run(*Mod);

  vector<FRule> rules = rules_pass->get_rules();

  Traces traces(cfp, db_path, safety, names, postdom);
  traces.prepost_context = prepost_context;
  traces.ec_context = ec_context;
  if (fcp) {
    traces.setFunctionContextPass(fcp);
  }
  
  if (preds_path.empty()) {
    // bool argument indicates to only inspect predicate points
    DataflowWali solver(true);
    cerr << "Computing dataflow results...\n";
    DataflowResult dflow_res = solver.solve(rules);
    traces.read_predicates(dflow_res);
  } else {
    traces.read_predicates(preds_path);
  }

  cerr << "Writing traces...\n";
  if (test_nesting) {
    traces.test_nesting(cout);
  } else {
    traces.generate(cout);
  }

  return 0;
}
