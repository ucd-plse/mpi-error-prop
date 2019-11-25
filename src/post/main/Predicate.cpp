#include "FindPathVisitor.hpp"
#include "Predicate.hpp"
#include "Path.hpp"


void Predicate::printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors) {
  MsgRef msg = Message::factory();
  msg->type = Message::Types::PREDICATE;
  msg->error = variables->getId(error);
  msg->file_name = file;
  msg->line_number = line;
  msg->error_codes = errors;
  msg->precision = precision;
  msg->target = var;

  Path path(msg);
  bool stop = false;
  int idxVariable = variables->getIndex(var);
  FindPathVisitor visitor(idxVariable, error, error, path, stop);
  witness->accept(visitor);
  
  // printing path
  path.printReport(out);
 
  return;
}
