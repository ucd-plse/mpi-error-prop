//===- MessageGeoff.cpp - GEOFF Message Formatter -------------------------===//
// 
// This is the implementation of the GEOFF message formatter. It is enabled by
// passing the --geoff switch. It is used for importing the traces into Neo4j.
//
//===----------------------------------------------------------------------===//

#include "Message.hpp"
#include "MessageGeoff.hpp"
#include "Variables.hpp"    // For errorNames global used by getGeoffErorrHeader
#include <sstream>

// For holding a copy of previous node so relationships can be connected
MessageGeoff::Node MessageGeoff::prev;

string MessageGeoff::getErrorCodeStr(bool starBefore) {
  ostringstream oss ;
  for(unsigned int i = 0; i < error_codes.size(); i++) {
    if (error_codes[i] == error && starBefore) {
      oss << '*' << error_codes[i];
    } else if (error_codes[i] == error) {
      oss << error_codes[i] << '*';
    } else {
      oss << error_codes[i];
    }
    oss << ' ';
  }
 
  return oss.str();
}

// Prints out all of the error codes in GEOFF format
// Put at the beginning of the report
// errorNames is a global from Variables.hpp, defined in Variables.cpp
string MessageGeoff::getGeoffErrorHeader() {
  std::ostringstream oss;
  for (auto e : errorNames) {
    oss << "(\"" << e << "\":EC { \"name\": \"" << e << "\"})" << endl;
  }
  return oss.str();
}

// Return node struct for an error code
MessageGeoff::Node MessageGeoff::getECNode(string code) {
  Node ecNode;
  propV properties {
    std::make_pair("name", code)
  };

  ecNode.name = code;
  ecNode.type = "EC";
  ecNode.properties = properties;

  return ecNode;
}

MessageGeoff::Node MessageGeoff::getSourceNode(string file, int line) {
  std::ostringstream key;
  key << file << ":" << line;

  Node sourceNode;
  propV properties {
    std::make_pair("location", key.str())
  };

  sourceNode.name = key.str();
  sourceNode.type = "SourceLocation";
  sourceNode.properties = properties;
  sourceNode.unique = "location";
  
  return sourceNode;
}

string MessageGeoff::Node::format() const {
  std::ostringstream oss;
  string propStr = "{";

  bool first = true;
  for (const auto kp : properties) {
    if (!first)
      propStr.append(", ");
    else
      first = false;
    propStr.append("\"" + kp.first + "\":\"" + kp.second + "\"");
  }
  propStr.append("}");

  oss << "(\"" << name << "\":" << type;
  if (! unique.empty()) {
    oss << "!" << unique;
  }
  oss << propStr << ")";

  return oss.str();
}

// TODO: create function for property strings to remove redundancy
string MessageGeoff::Relationship::format() const {
  std::ostringstream oss;
  oss << "-[:\"" << type;

  string propStr = "{";
  bool first = true;
  for (const auto kp : properties) {
    if (!first)
      propStr.append(", ");
    else
      first = false;
    propStr.append("\"" + kp.first + "\":\"" + kp.second + "\"");
  }
  propStr.append("}");

  oss << "\"" << propStr << "]->";

  return oss.str();
}

// TODO: stream operators for node and relationship
string MessageGeoff::format() {
  // If prev is not set, then this is the first node
  // we can't print this, but we still want to set prev for the next message
  bool output = (! prev.name.empty());

  std::ostringstream oss; 
  switch (type) {
    case MessageGeoff::Types::EMPTY:
    case MessageGeoff::Types::MAY_HAVE: {
        Node sourceNode = getSourceNode(file_name, line_number);
        oss << MessageGeoff::prev.format() << Relationship("witness").format() << sourceNode.format();
        MessageGeoff::prev = sourceNode;
      break;
    }
    case MessageGeoff::Types::RECEIVES_FUNCTION:
    case MessageGeoff::Types::RECEIVES_MAYBE:
    case MessageGeoff::Types::RECEIVES_IS: {
      Node sourceNode = getSourceNode(file_name, line_number);
      Node from;
      from = MessageGeoff::prev;
      oss << from.format() << Relationship("witness").format() << sourceNode.format();
      MessageGeoff::prev = sourceNode;
      break;
    }
    case MessageGeoff::Types::RECEIVES_FROM: {
      Node sourceNode = getSourceNode(file_name, line_number);
      Node from;
      if (source == error) {
        from = getECNode(source);
      } else {
        from = MessageGeoff::prev;
      }
      oss << from.format() << Relationship("witness").format() << sourceNode.format();
      MessageGeoff::prev = sourceNode;
      break;
    }
    case MessageGeoff::Types::OVERWRITE_POINTER:
    case MessageGeoff::Types::OVERWRITE:
    case MessageGeoff::Types::OVERWRITE_NT_POINTER:
    case MessageGeoff::Types::OVERWRITE_NT: {
      Node sourceNode = getSourceNode(file_name, line_number);
      Relationship witness("witness");
      propV properties {
        std::make_pair("overwrite", target)
      };
      witness.properties = properties;
      oss << MessageGeoff::prev.format() << witness.format() << sourceNode.format();
      MessageGeoff::prev = sourceNode;
      break;
    }
    case MessageGeoff::Types::UNCHECK_NSAVE:
    case MessageGeoff::Types::UNCHECK_NT_NSAVE:
    case MessageGeoff::Types::UNCHECK_OUT_POINTER:
    case MessageGeoff::Types::UNCHECK_OUT:
    case MessageGeoff::Types::UNCHECK_NT_OUT:
    case MessageGeoff::Types::UNCHECK_NT_OUT_POINTER: {
      Node sourceNode = getSourceNode(file_name, line_number);
      Relationship witness("witness");
      propV properties {
        std::make_pair("uncheck", target)
      };
      witness.properties = properties;
      oss << MessageGeoff::prev.format() << witness.format() << sourceNode.format();
      MessageGeoff::prev = sourceNode;
      break;
    }
    case MessageGeoff::Types::RECEIVES_ARGUMENT:
      oss << "RECEIVES_ARGUMENT not implemented";
      break;
    case MessageGeoff::Types::RETURN_FN:
      oss << "RETURN_FN not implemented";
      break;
    case MessageGeoff::Types::DEREFERENCE:
      oss << "DEREFERENCE not implemented";
      break;
    case MessageGeoff::Types::HANDLED:
      oss << "HANDLED not implemented";
      break;
    case MessageGeoff::Types::ISERR:
      oss << "ISERR not implemented";
      break;
    case MessageGeoff::Types::ISERRWARN:
      oss << "ISERRWARN not implemented";
      break;
    case MessageGeoff::Types::OPERAND:
      oss << "OPERAND not implemented";
      break;
    case MessageGeoff::Types::PREDICATE: {
      Node predNode = getSourceNode(file_name, line_number);
      predNode.properties.push_back(std::make_pair("pred", target));
      predNode.properties.push_back(std::make_pair("codes", getErrorCodeStr()));
      oss << predNode.format();
      break;
    }
  }

  return output ? oss.str() : "";
}

