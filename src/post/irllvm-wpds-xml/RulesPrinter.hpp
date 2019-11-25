#include "Names.hpp"
#include "Rule.hpp"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstVisitor.h"
#include <fstream>
#include <set>

#ifndef RULESPRINTER_H
#define RULESPRINTER_H

using namespace std;

class RulesPrinter {
public:
  RulesPrinter(string schema_path, vector<FRule> rules);

  static char ID;

  string formatRules(bool use_locations);

  // For --locations intermediate file
  string locations();

private:
  // Singleton
  RulesPrinter() {}
  RulesPrinter(const RulesPrinter&);
  void operator=(const RulesPrinter&);

  string schema_path;

  const vector<FRule> rules;
  set<string> local_names;
  set<string> global_names;

  string prologue();
  string epilogue();
};

#endif
