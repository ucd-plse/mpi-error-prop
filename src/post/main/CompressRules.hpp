#ifndef PARSER3_GUARD
#define PARSER3_GUARD 1

#include "TranscodedString.hpp"
#include <xercesc/sax2/DefaultHandler.hpp>
#include <fstream>

#define ATTR_UNUSED __attribute__((__unused__))

using namespace std;

XERCES_CPP_NAMESPACE_USE


class CompressRules : public DefaultHandler {

public:
  CompressRules();

  virtual void startElement(  
			    const   XMLCh* const    uri,
			    const   XMLCh* const    localname,
			    const   XMLCh* const    qname,
			    const   Attributes&     attributes);

  virtual void endElement( 
			  const XMLCh* const uri
			  , const XMLCh* const localname
			  , const XMLCh* const qname);

  virtual void characters(
			  const XMLCh* const chars
			  , const unsigned int length);

private:
  
  bool isIntraRule(const Attributes& attributes);
  string getToStack1(const Attributes& attributes);
  string getFromStack(const Attributes& attributes);

  bool sets;
  bool source;
  bool unimplemented;
  bool identity;
  bool intra;
  string oldKey;
  string newKey;
  string currentToStack1;
  string previousToStack1;
  string currentFromStack;

  const TranscodedString toStack1ID;
  const TranscodedString toStack2ID;
  const TranscodedString fromStackID;
  const TranscodedString basisID;
};

#endif // PARSER3_GUARD
