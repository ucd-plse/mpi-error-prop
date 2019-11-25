#include "Operand.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"

void Operand::printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors) {

  MsgRef msg = Message::factory();
  msg->type = Message::Types::OPERAND;
  msg->file_name = file;
  msg->line_number = line;
  msg->error = variables->getId(error);
  msg->error_codes = errors;
  msg->target = var;
  msg->precision = precision;

  Path path(msg);
  bool stop = false;
  int idxVariable = variables->getIndex(var);
  FindPathVisitor visitor(idxVariable, error, error, path, stop);
  witness->accept(visitor);
  
  // printing path
  path.printReport(out);

  return;
}  
