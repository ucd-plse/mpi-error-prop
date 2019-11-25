//===- Message.hpp - Superclass of Message Formatters ---------------------===//
//
// All of the message formatters should inherit from this class.
// Use Message::factory() to get a message object of the correct type.
//
//===----------------------------------------------------------------------===//

#ifndef MESSAGE_GUARD
#define MESSAGE_GUARD 1

#include <string>
#include <vector>
#include <memory>

using namespace std;

class Message;

// Shared because messages go into two lists in Path
typedef std::shared_ptr<Message> MsgRef;

class Message {
public:

  enum class Types {
    EMPTY, MAY_HAVE,
    RECEIVES_FUNCTION, RECEIVES_MAYBE, RECEIVES_IS, RECEIVES_FROM, RECEIVES_ARGUMENT,
    OVERWRITE, OVERWRITE_POINTER, OVERWRITE_NT, OVERWRITE_NT_POINTER,
    UNCHECK_NSAVE, UNCHECK_NT_NSAVE,
    UNCHECK_OUT, UNCHECK_OUT_POINTER, UNCHECK_NT_OUT, UNCHECK_NT_OUT_POINTER,
    RETURN_FN, DEREFERENCE, HANDLED, ISERR, ISERRWARN, OPERAND,
    PREDICATE 
  };

  enum class Formatters {
    STANDARD, GEOFF, TRACES
  };

  // This returns a message object of the type appropriate for selected formatter
  static MsgRef factory();
  virtual ~Message() {};

  Message::Types type = Message::Types::EMPTY;
  bool debug = false;           // "Tracking code: "
  int line_number = 0;
  string error;
  string file_name;
  string target;
  string function;              // "from function"
  string source;                // "from source"
  vector<string> error_codes;
  string precision;

  // Use this to get the message text out
  virtual string format() = 0;

  // For printing "Error codes:" header in report
  virtual string getErrorCodeStr(bool starBefore=false) = 0;
 
  bool operator==(const Message& RHS);
  bool operator!=(const Message& RHS);
};

#endif

