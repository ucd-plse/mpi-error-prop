#include "Names.hpp"
#include "ControlFlow.hpp"
#include "BranchSafety.hpp"
#include "Rules.hpp"
#include <iostream>
#include <unistd.h>
#include <libconfig.h++>
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

using namespace std;
using namespace libconfig;
using namespace llvm;

void usage() {
  cerr << "Usage : " << "frontend "
       << "-c <config file> "
       << "-b <bitcode file "
       << "-o <wpds output> "
    //       << "[-t] (branch safety test output instead of wpds) "
    //       << "[-s] (schema file, usually just used by tests)"
       << endl;
}

int main(int argc, char **argv) {
  string config_path;
  string bitcode_path;
  string wpds_out_path;
  string loc_out_path;
  string schema_path;
  bool test = false;

  int c;

  while ((c = getopt(argc, argv, "e:c:b:o:t:s:")) != EOF) {
    switch(c) {
    case 'c':
      config_path = optarg;
      break;
    case 'b':
      bitcode_path = optarg;
      break;
    case 'o':
      wpds_out_path = optarg;
      break;
    case 't':
      test = true;
      break;
    case 's':
      schema_path = optarg;
      break;
    }
  }

  if (config_path.empty() || bitcode_path.empty() || wpds_out_path.empty()) {
    usage();
    return 1;
  }

  SMDiagnostic Err;

  std::unique_ptr<Module> Mod(parseIRFile(bitcode_path, Err, getGlobalContext()));  
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  Config cfg;
  cfg.readFile(config_path.c_str());

  if (schema_path.empty()) {
    cfg.lookupValue("schema", schema_path);
  }
  string ec_path = cfg.lookup("ecfile");
  
  legacy::PassManager PM;  
  NamesPass *names = new NamesPass(ec_path);
  PM.add(names);

  RulesPass *rules_pass = new RulesPass(wpds_out_path, schema_path);
  PM.add(rules_pass);

  BranchSafetyPass *safety;
  if (test) {
    safety = new BranchSafetyPass(wpds_out_path);
  } else {
    safety = new BranchSafetyPass();
  }

  PM.add(safety);

  PM.run(*Mod);
}
