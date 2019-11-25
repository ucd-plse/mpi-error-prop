#include "InitializeCounters.hpp"
#include "PrintRules.hpp"
#include "StrX.hpp"
#include <xercesc/sax2/Attributes.hpp>
#include <map>

using namespace std;


PrintRules::PrintRules(ostream &out)
  : oss(out),
    depth(0),
    print(false),
    fromID("from"),
    fromStackID("fromStack"),
    toID("to"),
    toStack1ID("toStack1"),
    toStack2ID("toStack2")
{
}


void PrintRules::printIndent()
{
  for (unsigned indent = depth; indent--; )
    oss << '\t';
}


void PrintRules::printElementPrefix(const string &name, const Attributes &attributes)
{
  printIndent();
  oss << '<' << name;

  // warning: does no escaping of attribute values!
  for (XMLSize_t slot = 0, count = attributes.getLength(); slot < count; ++slot) {
    const StrX qname = attributes.getQName(slot);
    const StrX value = attributes.getValue(slot);
    oss << ' ' << qname << '=' << '\'' << value << '\'';
  }
}


void PrintRules::printLeafElement(const string &name, const Attributes &attributes)
{
  printElementPrefix(name, attributes);
  oss << "/>\n";
}


void PrintRules::printStartElement(const string &name, const Attributes &attributes)
{
  printElementPrefix(name, attributes);
  oss << ">\n";
}


void PrintRules::printEndElement(const string &name)
{
  printIndent();
  oss << "</" << name << ">\n";
}


void PrintRules::printRuleElement(const string &name, const Attributes& attributes) {

  const StrX from = attributes.getValue(fromID);
  const StrX fromStack = attributes.getValue(fromStackID);
  const StrX to = attributes.getValue(toID);
  const StrX toStack1 = attributes.getValue(toStack1ID);
  const StrX toStack2 = attributes.getValue(toStack2ID);

  printIndent();
  oss << '<' << name << " from='" << from << "' fromStack='" << fromStack << "' to='" << to << '\'';

  if (!toStack1.get().empty())
    oss << " toStack1='" << keys[toStack1.get()] << '\'';

  if (!toStack2.get().empty())
    oss << " toStack2='" << keys[toStack2.get()] << '\'';
  
  oss << ">\n";
}


bool PrintRules::isIntraRule(const Attributes& attributes) {

  StrX toStack1 = attributes.getValue(toStack1ID);
  StrX toStack2 = attributes.getValue(toStack2ID);
  
  return (!toStack1.get().empty()) && (toStack2.get().empty());
}


string PrintRules::getFromStack(const Attributes& attributes) {
  StrX fromStack = attributes.getValue(fromStackID);
  return fromStack.get();
}



void PrintRules::startElement(
        const   XMLCh* const    uri ATTR_UNUSED,
        const   XMLCh* const    localname ATTR_UNUSED,
        const   XMLCh* const    qname ATTR_UNUSED,
        const   Attributes&     attributes ATTR_UNUSED) {

  const string who(StrX(localname).get());

  if (who == "Rule") {
    print = true;
    if (isIntraRule(attributes)) { //print
      string currentFromStack = getFromStack(attributes);
      if (counters[currentFromStack] == 0) {
	print = false;	
      }
    }
    if (print) {     
      printRuleElement(who, attributes);
    }

  } else if (who == "one" || who == "output" || who == "set" || who == "return" || who == "var" || who == "zero")
    printLeafElement(who, attributes);
  else if (who == "FWPDS" || who == "Globals" || who == "Locals" || who == "Pointers" || who == "Prologue" || who == "State" || who == "Trans" || who == "Variables")
    printStartElement(who, attributes);
  else if (who == "Query")
    printStartElement(who + " xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'", attributes);
  else if (who == "WFA") {
    printStartElement(who, attributes);
    print = true;

  } else if (print) {
    if (who == "Weight")
      printStartElement(who, attributes);
    else if (who == "dereference" || who == "handled" || who == "input" || who == "iserr" || who == "operand" || who == "source" || who == "unimplemented" || who == "pred")
      printLeafElement(who, attributes);
  }

  ++depth;
}


void PrintRules::endElement( 
	 const XMLCh* const uri ATTR_UNUSED
	 , const XMLCh* const localname ATTR_UNUSED
	 , const XMLCh* const qname ATTR_UNUSED) {

  --depth;

  const string who(StrX(localname).get());

  if (who == "FWPDS" || who == "Globals" || who == "Locals" || who == "Pointers" || who == "Prologue" || who == "Query" || who == "State" || who == "Trans" || who == "Variables" || who == "WFA")
    printEndElement(who);
  else if (print) {
    if (who == "Rule") {
      printEndElement(who);
      print = false;
    } else if (who == "Weight")
      printEndElement(who);
  }
}


void PrintRules::characters(
	const XMLCh* const chars ATTR_UNUSED
	, const unsigned int length ATTR_UNUSED)
{
}
