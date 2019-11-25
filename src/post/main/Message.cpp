//===- Message.cpp - Superclass of Message Formatters ---------------------===//
//
// All of the message formatters should inherit from this class.
// Use Message::factory() to get a message object of the correct type.
//
//===----------------------------------------------------------------------===//

#include <memory>
#include "Message.hpp"
#include "MessageGeoff.hpp"
#include "MessageStandard.hpp"
#include "MessageTraces.hpp"
#include "Main.hpp"

// selectd_formatter is a global set by command line parser in Main
MsgRef Message::factory() {
  MsgRef ret;

  if (selected_formatter == Message::Formatters::STANDARD) {
    ret = MsgRef(new MessageStandard);
  } else if (selected_formatter == Message::Formatters::GEOFF) {
    ret = MsgRef(new MessageGeoff);
  } else if (selected_formatter == Message::Formatters::TRACES) {
    ret = MsgRef(new MessageTraces);
  }

  return ret;
}

bool Message::operator==(const Message& RHS) {
  if (debug == RHS.debug &&
      line_number == RHS.line_number &&
      error == RHS.error &&
      file_name == RHS.file_name &&
      target == RHS.target &&
      function == RHS.function &&
      source == RHS.source &&
      error_codes == RHS.error_codes &&
      precision == RHS.precision) {
    return true;
  } else {
    return false;
  }
}

bool Message::operator!=(const Message& RHS) {
  return !operator==(RHS);
}
