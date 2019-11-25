#include "CountRules.hpp"
#include "InitializeCounters.hpp"
#include "StrX.hpp"
#include <xercesc/sax2/Attributes.hpp>
#include <map>
#include <string>

using namespace std;

CountRules::CountRules()
  : fromStackID("fromStack"),
    toStack1ID("toStack1"),
    toStack2ID("toStack2")
{
  counters["main.0"] = 1;
}


void CountRules::inspectRuleAttrs(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) {

  StrX fromStack = attributes.getValue(fromStackID);
  StrX toStack1 = attributes.getValue(toStack1ID);
  StrX toStack2 = attributes.getValue(toStack2ID);

  countersFrom[fromStack.get()]++;
  counters[toStack1.get()]++;
  counters[toStack2.get()]++;
  

  return;
}


void CountRules::startElement(
        const   XMLCh* const    uri ATTR_UNUSED,
        const   XMLCh* const    localname ATTR_UNUSED,
        const   XMLCh* const    qname ATTR_UNUSED,
        const   Attributes&     attributes ATTR_UNUSED) {

  StrX who(localname);

  if (who.get() == "Rule") {
    inspectRuleAttrs(uri, localname, qname, attributes);
  }
  return;
}



void CountRules::endElement( 
	 const XMLCh* const uri ATTR_UNUSED
	 , const XMLCh* const localname ATTR_UNUSED
	 , const XMLCh* const qname ATTR_UNUSED) {

  StrX who(localname);

  if (who.get() == "Rule") {
  }
  return;
}



void CountRules::characters(
	const XMLCh* const chars ATTR_UNUSED
	, const unsigned int length ATTR_UNUSED)
{
}


