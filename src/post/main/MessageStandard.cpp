//===- MessageStandard.cpp - Standard Message Formatter -------------------===//
// 
// This is the implementation of the "standard" message formatter. It outputs
// messages in the same format as was used in error-prop prior to centralizing
// the formatters. It is supposed to be 100% backwards compatible.
//
//===----------------------------------------------------------------------===//

#include "Message.hpp"
#include "MessageStandard.hpp"
#include "Variables.hpp"    // For errorNames global used by getGeoffErorrHeader
#include <sstream>

// *TENTATIVE_EIO vs. TENTATIVE_EIO* controlled by starBefore
string MessageStandard::getErrorCodeStr(bool starBefore) {
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

string MessageStandard::getLocationStr() {
  std::ostringstream oss;
  oss << file_name << ":" << line_number;
  return oss.str();
}

string MessageStandard::format() {
  std::ostringstream oss;

  if (debug) {
    oss << "Tracking code: " << error << ", ";
  }

  switch (type) {
    case MessageStandard::Types::EMPTY:
      break;
    case MessageStandard::Types::MAY_HAVE:
      oss << file_name << ":" << line_number << ":\"" << target << '\"'
          << " may have an unchecked error";
      break;
    case MessageStandard::Types::RECEIVES_FUNCTION:
	    oss << file_name << ':' << line_number << ":\"" << target
	        << '\"' << " receives an error from function \"" << function << "\"";
      break;
    case MessageStandard::Types::RECEIVES_MAYBE:
	    oss << file_name << ':' << line_number;
      oss << ": an unchecked error may be returned";
      break;
    case MessageStandard::Types::RECEIVES_IS:
      oss << file_name << ':' << line_number;
      oss << ": unchecked error \"" << error << "\" is returned";
      break;
    case MessageStandard::Types::RECEIVES_FROM:
      oss << file_name << ":" << line_number << ":\"" << target
          << "\" receives an error from \"" << source << "\"";
      break;
    case MessageStandard::Types::RECEIVES_ARGUMENT:
      oss << file_name << ":" << line_number  
          << ":\"" << error << "\" error is passed as argument";
      break;
    case MessageStandard::Types::OVERWRITE_POINTER:
      oss << getLocationStr();
      oss << ": overwriting potential unchecked error in pointer variable \"" + target + '\"';
      break;
    case MessageStandard::Types::OVERWRITE:
      oss << getLocationStr();
      oss << ": overwriting potential unchecked error in variable \"" + target + '\"';
      break;
    case MessageStandard::Types::OVERWRITE_NT_POINTER:
      oss << getLocationStr();
      oss << ": overwriting potential non-tentative unchecked error in pointer variable \"" + target + '\"';
      break;
    case MessageStandard::Types::OVERWRITE_NT:
      oss << getLocationStr();
      oss << ": overwriting potential non-tentative unchecked error in variable \"" + target + '\"';
      break;
    case MessageStandard::Types::UNCHECK_NSAVE:
      oss << getLocationStr();
      oss << ": potential unchecked error is not saved";
      break;
    case MessageStandard::Types::UNCHECK_NT_NSAVE:
      oss << getLocationStr();
      oss << ": potential non-tentative unchecked error is not saved";
      break;
    case MessageStandard::Types::UNCHECK_OUT_POINTER:
      oss << getLocationStr();
      oss << ": potential unchecked error in out of scope pointer variable \"" + target + '\"';
      break;
    case MessageStandard::Types::UNCHECK_OUT:
      oss << getLocationStr();
      oss << ": potential unchecked error in out of scope variable \"" + target + '\"';
      break;
    case MessageStandard::Types::UNCHECK_NT_OUT_POINTER:
      oss << getLocationStr();
      oss << ": potential non-tentative unchecked error in out of scope pointer variable \"" + target + '\"';
      break;
    case MessageStandard::Types::UNCHECK_NT_OUT:
      oss << getLocationStr();
      oss << ": potential non-tentative unchecked error in out of scope variable \"" + target + '\"';
      break;
    case MessageStandard::Types::RETURN_FN:
      oss << getLocationStr();
      oss << ":" << getErrorCodeStr();
      oss << "returned from function " << target;
      break;
    case MessageStandard::Types::DEREFERENCE:
      oss << getLocationStr();
      oss << ":";
      oss << " Dereferencing variable " << target << ", which " << precision
          << " contain one of the following error codes: ";
      oss << getErrorCodeStr();
      break;
    case MessageStandard::Types::HANDLED:
      oss << getLocationStr();
      oss << ": Handling error in variable " << target << ", which " << precision 
          <<  " contain one of the following error codes: ";
      oss << getErrorCodeStr();
      break;
    case MessageStandard::Types::ISERR:
      oss << getLocationStr();
      oss << ": Checking variable " << target + ", which " << precision
          <<  " contain one of the following error codes: ";
      oss << getErrorCodeStr();
      break;
    case MessageStandard::Types::ISERRWARN:
      oss << getLocationStr();
      oss <<  ": Checking variable " << target <<  ", which cannot possibly contain any error codes\n";
      break;
    case MessageStandard::Types::OPERAND:
      oss << getLocationStr();
      oss << ": Using variable " << target << " in pointer arithmetic, which " << precision
          <<  " contain one of the following error codes: ";
      oss << getErrorCodeStr();
      break;
    case MessageStandard::Types::PREDICATE:
      oss << getLocationStr();
      oss << ": predicate depends on " << target << ", which " << precision
          << " contain one of the following error codes: ";
      oss << getErrorCodeStr();
      break;
  }

  return oss.str();
}

