#include "RulesPrinter.hpp"
#include <llvm/Support/CommandLine.h>
#include <sstream>

using namespace llvm;

RulesPrinter::RulesPrinter(string schema_path, vector<FRule> rules) : schema_path(schema_path), rules(rules) {
  for (FRule r : rules) {
    for (Assignment a : r.assignments) {

      if (a.from.scope == VarScope::LOCAL) {
        local_names.insert(a.from.name());
      } else if (a.from.scope == VarScope::GLOBAL) {
        global_names.insert(a.from.name());
      }

      if (a.to.scope == VarScope::LOCAL) {
        local_names.insert(a.to.name());
      } else if (a.to.scope == VarScope::GLOBAL) {
        global_names.insert(a.to.name());
      }

    }
  }
}

// TODO: parameterize xsd
string RulesPrinter::prologue() {

  ostringstream oss;
  oss << "<Query xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' type='poststar' xsi:noNamespaceSchemaLocation='file://";
  oss << schema_path << "'>\n";
  oss << "\t<FWPDS>\n";
  oss << "\t\t<Prologue>\n";
  oss << "\t\t\t<Variables>\n";
  oss << "\t\t\t\t<Globals>\n";

  for (auto i = global_names.begin(), e = global_names.end(); i != e; ++i) {
    oss << "\t\t\t\t\t<var id='" << *i << "'/>\n";
  }

  oss << "\t\t\t\t</Globals>\n";
  oss << "\t\t\t\t<Locals>\n";

  for (auto i = local_names.begin(), e = local_names.end(); i != e; ++i) {
    oss << "\t\t\t\t\t<var id='" << *i << "'/>\n";
  }

  oss << "\t\t\t\t</Locals>\n";
  oss << "\t\t\t\t<Pointers>\n";
  oss << "\t\t\t\t</Pointers>\n";
  oss << "\t\t\t</Variables>\n";
  oss << "\t\t</Prologue>\n";

  return oss.str();
}

string RulesPrinter::epilogue() {
  ostringstream oss;

  oss << "\t</FWPDS>\n";
  oss << "\t<WFA query='INORDER'>\n";
  oss << "\t\t<State Name='p' initial='true' final='false'>\n";
  oss << "\t\t\t<Weight basis='identity'>\n";
  oss << "\t\t\t\t<zero/>\n";
  oss << "\t\t\t</Weight>\n";
  oss << "\t\t</State>\n";
  oss << "\t\t<State Name='accept' initial='false' final='true'>\n";
  oss << "\t\t\t<Weight basis='identity'>\n";
  oss << "\t\t\t\t<zero/>\n";
  oss << "\t\t\t</Weight>\n";
  oss << "\t\t</State>\n";
  oss << "\t\t<Trans from='p' stack='main.0' to='accept'>\n";
  oss << "\t\t\t<Weight basis='identity'>\n";
  oss << "\t\t\t\t<one/>\n";
  oss << "\t\t\t</Weight>\n";
  oss << "\t\t</Trans>\n";
  oss << "\t</WFA>\n";
  oss << "</Query>\n";

  return oss.str();
}

string RulesPrinter::formatRules(bool use_locations) {
  ostringstream oss;

  oss << prologue();

  for (vector<FRule>::const_iterator i = rules.begin(), e = rules.end(); i != e; ++i) {
    oss << i->format(use_locations);
  }

  oss << epilogue();

  return oss.str();
}

string RulesPrinter::locations() {
  ostringstream oss;

  for (auto r = rules.begin(), re = rules.end(); r != re; ++r) {
    for (auto a = r->assignments.begin(), ae = r->assignments.end(); a != ae; ++a) {
      string from = a->from.name();
      if (from.find("TENTATIVE_") != string::npos) {
        string stripped = from.substr(10, string::npos);
        oss << stripped + "$" + r->location.file + ":" + to_string(r->location.line) << " 0" << "\n";
      }
    }
  }

  return oss.str();
}

