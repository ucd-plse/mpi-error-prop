#include "Dereference.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"

void Dereference::printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors) {
  
  MsgRef msg = Message::factory();
  msg->type = Message::Types::DEREFERENCE;
  msg->target = var;
  msg->precision = precision;
  msg->error = variables->getId(error);
  msg->error_codes = errors;
  msg->file_name = file;
  msg->line_number = line;

  // finding sample path
  Path path(msg);
  bool stop = false;
  int idxVariable = variables->getIndex(var);
  FindPathVisitor visitor(idxVariable, error, error, path, stop);
  witness->accept(visitor);
  
  // printing path
  path.printReport(out);
  
  return;
}
