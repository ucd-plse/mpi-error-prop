#include "InitializeCounters.hpp"
#include "StrX.hpp"
#include <xercesc/sax2/Attributes.hpp>
#include <map>
#include <string>

using namespace std;

map<string,int> counters;
map<string,int> countersFrom;
map<string,string> keys;

InitializeCounters::InitializeCounters()
  : fromStackID("fromStack"),
    toStack1ID("toStack1"),
    toStack2ID("toStack2")
{
}


void InitializeCounters::inspectRuleAttrs(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) {

  StrX fromStack = attributes.getValue(fromStackID);
  StrX toStack1 = attributes.getValue(toStack1ID);
  StrX toStack2 = attributes.getValue(toStack2ID);

  counters[fromStack.get()] = 0;
  counters[toStack1.get()] = 0;
  counters[toStack2.get()] = 0;

  countersFrom[fromStack.get()] = 0;
  countersFrom[toStack1.get()] = 0;
  countersFrom[toStack2.get()] = 0;

  keys[fromStack.get()] = fromStack.get();
  if (!toStack1.get().empty()) {
    keys[toStack1.get()] = toStack1.get();
  }
  
  if (!toStack2.get().empty()) {
    keys[toStack2.get()] = toStack2.get();
  }

  return;
}


void InitializeCounters::startElement(
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



void InitializeCounters::endElement( 
	 const XMLCh* const uri ATTR_UNUSED
	 , const XMLCh* const localname ATTR_UNUSED
	 , const XMLCh* const qname ATTR_UNUSED) {

  StrX who(localname);

  if (who.get() == "Rule") {
  }
  return;
}



void InitializeCounters::characters(
	const XMLCh* const chars ATTR_UNUSED
	, const unsigned int length ATTR_UNUSED)
{
}


