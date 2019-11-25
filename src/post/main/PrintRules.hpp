#ifndef PARSER4_GUARD
#define PARSER4_GUARD 1

#include "TranscodedString.hpp"
#include <xercesc/sax2/DefaultHandler.hpp>
#include <fstream>

#define ATTR_UNUSED __attribute__((__unused__))

using namespace std;

XERCES_CPP_NAMESPACE_USE


class PrintRules : public DefaultHandler {

public:
  PrintRules(const string &filename);
  PrintRules(ostream &out);

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

  string getFromStack(const Attributes& attributes);

  void printIndent();
  void printElementPrefix(const string &, const Attributes &);
  void printLeafElement(const string &, const Attributes &);
  void printStartElement(const string &, const Attributes &);
  void printEndElement(const string &);
  void printRuleElement(const string &, const Attributes &);

  ostream &oss;
  unsigned depth;
  bool print;
  
  const TranscodedString fromID;
  const TranscodedString fromStackID;
  const TranscodedString toID;
  const TranscodedString toStack1ID;
  const TranscodedString toStack2ID;
};

#endif // PARSER4_GUARD
