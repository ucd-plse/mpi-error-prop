#include "CompressRules.hpp"
#include "InitializeCounters.hpp"
#include "StrX.hpp"
#include <xercesc/sax2/Attributes.hpp>
#include <map>
#include <string>

using namespace std;

CompressRules::CompressRules()
  : sets(false),
    source(false),
    unimplemented(false),
    identity(false),
    intra(false),
    toStack1ID("toStack1"),
    toStack2ID("toStack2"),
    fromStackID("fromStack"),
    basisID("basis")
{
}


bool CompressRules::isIntraRule(const Attributes& attributes) {
  StrX toStack1 = attributes.getValue(toStack1ID);
  StrX toStack2 = attributes.getValue(toStack2ID);
  
  return (!toStack1.get().empty()) && (toStack2.get().empty());
}


string CompressRules::getToStack1(const Attributes& attributes) {
  StrX toStack1 = attributes.getValue(toStack1ID);
  return toStack1.get();
}


string CompressRules::getFromStack(const Attributes& attributes) {
  StrX fromStack = attributes.getValue(fromStackID);
  return fromStack.get();
}


void CompressRules::startElement(
        const   XMLCh* const    uri ATTR_UNUSED,
        const   XMLCh* const    localname ATTR_UNUSED,
        const   XMLCh* const    qname ATTR_UNUSED,
        const   Attributes&     attributes) {

  StrX who(localname);

  if (who.get() == "Rule") {

    // previous toStack1 or empty if first rule
    previousToStack1 = currentToStack1;

    if (isIntraRule(attributes)) {
      intra = true;

      // retrieving data for current rule
      currentToStack1 = getToStack1(attributes);
      currentFromStack = getFromStack(attributes);

      if (oldKey.empty()) { // if first rule
	oldKey = currentToStack1;
      }
    }
  }
  else if (who.get() == "set") {
    sets = true;
  }
  else if (who.get() == "source") {
    source = true;
  }
  else if (who.get() == "unimplemented") {
    unimplemented = true;
  }
  else if (who.get() == "Weight") {
    StrX basis = attributes.getValue(basisID);
    if (basis.get() == "identity") {
      identity = true;
    }
  }
  return;
}


void CompressRules::endElement( 
	 const XMLCh* const uri ATTR_UNUSED
	 , const XMLCh* const localname ATTR_UNUSED
	 , const XMLCh* const qname ATTR_UNUSED) {

  StrX who(localname);

  if (who.get() == "Rule") {

    if (intra && (previousToStack1.empty() || currentFromStack == previousToStack1) && 
	counters[currentToStack1] == 1 && countersFrom[currentFromStack] == 1 && 
	!sets && !source && !unimplemented && identity) {
      
      // we are removing this rule
      newKey = currentToStack1;

      // decrementing counter for potential next rule
      // we associate rules with fromStack not toStack1
      counters[currentToStack1]--;
      
    }
    else {

      // stop and do compress
      if (!newKey.empty()) {

	// restablish counter for potential next rule
	counters[previousToStack1]++;

	// oldKey is to be replaced by newKey when printing
	keys[oldKey] = newKey;
      }

      // starting new iteration
      oldKey.clear();
      newKey.clear();
      currentToStack1.clear();	
    }
    
    // exploring new rule
    sets = false;
    source = false;
    unimplemented = false;
    identity = false;
    intra = false;
  }

  return;
}



void CompressRules::characters(
	const XMLCh* const chars ATTR_UNUSED
	, const unsigned int length ATTR_UNUSED)
{
}


