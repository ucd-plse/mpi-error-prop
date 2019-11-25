#include "Rule.hpp"
#include <sstream>

using namespace std;

string FRule::format(bool use_locations) const {
  ostringstream oss;

  oss << "\t\t<Rule from='p' "
      << "fromStack='" << from_stack << "' "
      << "to='p' ";

  if (!to_stack1.empty()) {
    oss << "toStack1='" << to_stack1 << "' ";
  }
  if (!to_stack2.empty()) {
    oss << "toStack2='" << to_stack2 << "' ";
  }
  oss << ">\n";

  oss << "\t\t\t<Weight basis='" << basis << "'>\n";
  for (auto i = assignments.begin(), e = assignments.end(); i != e; ++i) {
    // Right now we only support MultiName assignments in the FROM field
    if (!i->keep) {
      continue;
    }

    string from_name = i->from.name();
    string to_name   = i->to.name();

    if (use_locations) {
      if (from_name != "OK" && from_name.find("TENTATIVE_") != string::npos) {
        from_name += "$" + location.file + ":" + to_string(location.line);
      }
    }

    oss << "\t\t\t\t<set to='" << to_name << "' " << "from='" << from_name << "' trusted='";
    if (i->trusted) {
      oss << "true";
    } else {
      oss << "false";
    }
    oss << "'/>\n";
  }
  oss << "\t\t\t</Weight>\n";

  if (location.line != 0) {
    oss << "\t\t\t<source line='" << location.line << "' file='" << location.file << "'/>\n";
  }

  if (!pred_op1.empty() || !pred_op2.empty()) {
    oss << "\t\t\t<pred";
    if (!pred_op1.empty()) {
      oss << " op1='" << pred_op1 << "'";
    }
    if (!pred_op2.empty()) {
      oss << " op2='" << pred_op2 << "'";
    }
    oss << "/>\n";
  }

  if (!return_value.name().empty()) {
    oss << "\t\t\t<return value='" << return_value.name() << "'/>\n";
    oss << "\t\t\t<output value='" << return_value.name() << "'/>\n";
  }

  oss << "\t\t</Rule>\n";

  return oss.str();
}

void FRule::addAssignment(VarName from, VarName to, bool trusted) {
  if (from.name().empty() || to.name().empty())
    return;

  Assignment A(from, to, trusted);
  assignments.push_back(A);
}

void FRule::addMultiAssignment(vn_t from, VarName to, bool trusted) {
  if (!from || to.name().empty())
    return;

  // We do not support MultiNames as the receiver of assignments
  mul_t froms;
  if (from->type == VarType::MULTI) {
    froms = static_pointer_cast<MultiName>(from);
  } else {
    froms->insert(from);
  }

  for (vn_t subname: froms->names()) {
    if (!subname) {
      continue;
    }
    Assignment A(*subname, to, trusted);
    assignments.push_back(A);
  }
}
