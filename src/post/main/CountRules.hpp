#ifndef PARSER2_GUARD
#define PARSER2_GUARD 1

#include "TranscodedString.hpp"
#include <xercesc/sax2/DefaultHandler.hpp>
#include <fstream>

#define ATTR_UNUSED __attribute__((__unused__))

using namespace std;

XERCES_CPP_NAMESPACE_USE


class CountRules : public DefaultHandler {

public:
  CountRules();

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


void inspectRuleAttrs(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes);

  const TranscodedString fromStackID;
  const TranscodedString toStack1ID;
  const TranscodedString toStack2ID;
};

#endif // PARSER2_GUARD
