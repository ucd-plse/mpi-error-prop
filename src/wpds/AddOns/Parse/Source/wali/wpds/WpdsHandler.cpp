/**
 * @author Nicholas Kidd
 *
 * @version $Id: WpdsHandler.cpp 522 2009-05-28 20:17:28Z kidd $
 */

#include "StrX.hpp"

#include "WeightInfo.hpp"

#include "wali/Common.hpp"
#include "wali/Key.hpp"
#include "wali/IUserHandler.hpp"

#include "wali/wpds/Rule.hpp"
#include "wali/wpds/WPDS.hpp"
#include "wali/wpds/DebugWPDS.hpp"
#include "wali/wpds/WpdsHandler.hpp"

#include <xercesc/sax2/Attributes.hpp>

namespace wali
{
  namespace wpds
  {

    const std::string WpdsHandler::FunctionXMLTag("Function");

    WpdsHandler::WpdsHandler( IUserHandler& user, WPDS* thePds ) :
      pds(thePds), fUserHandler(user)
    {
      if( NULL == pds )
        pds = new WPDS();

      // Initialize the XML4C2 system
      // Necessary for using transcode
      XMLPlatformUtils::Initialize();
      fromStackID = XMLString::transcode(Rule::XMLFromStackTag.c_str());
      fromID      = XMLString::transcode(Rule::XMLFromTag.c_str());
      toID        = XMLString::transcode(Rule::XMLToTag.c_str());
      toStack1ID  = XMLString::transcode(Rule::XMLToStack1Tag.c_str());
      toStack2ID  = XMLString::transcode(Rule::XMLToStack2Tag.c_str());

      // For the function information
      nameID  = XMLString::transcode("name");
      entryID = XMLString::transcode("entry");
      exitID  = XMLString::transcode("exit");
    }

    WpdsHandler::~WpdsHandler()
    {
      // Rule tags
      XMLString::release(&fromID);
      XMLString::release(&fromStackID);
      XMLString::release(&toID);
      XMLString::release(&toStack1ID);
      XMLString::release(&toStack2ID);
      // Function tags
      XMLString::release(&nameID);
      XMLString::release(&entryID);
      XMLString::release(&exitID);

      XMLPlatformUtils::Terminate();
      if( NULL == pds ) {
        assert(false);
      }
      delete pds;
    }

    bool WpdsHandler::handlesElement(std::string tag)
    {
      return (
          (tag == Rule::XMLTag) ||
          (tag == WPDS::XMLTag) ||
          (tag == WpdsHandler::FunctionXMLTag)
          );
    }

    WPDS& WpdsHandler::get() {
      return *pds;
    }

    //////////////////////////////////////////////////
    // Parsing handlers
    //////////////////////////////////////////////////

    void WpdsHandler::startElement(  
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attributes)
    {
      StrX who(localname);
      if (WPDS::XMLTag == who.get()) {
        // do nothing
      }
      else if (DebugWPDS::XMLTag == who.get())
      {
        // do nothing
        *waliErr << "[INFO] Begin parsing DebugWPDS." << std::endl;
      }
      else if (Rule::XMLTag == who.get()) {
        handleRule(uri,localname,qname,attributes);
      }
      else if (WpdsHandler::FunctionXMLTag == who.get()) {
        StrX fname = attributes.getValue(nameID);
        StrX e = attributes.getValue(entryID);
        StrX x = attributes.getValue(exitID);

        std::string sfun(fname.get());
        std::string se(e.get());
        metaEntry[sfun] = se;
        std::string sx(x.get());
        metaExit[sfun] = sx;
      }
      else if (fUserHandler.handlesElement(who.get())) {
        fUserHandler.startElement(uri,localname,qname,attributes);
      }
      else {
        *waliErr << "[ERROR] WpdsHandler::startElement - unrecognized element.\n";
        *waliErr << "  " << who << "\n";
        for( unsigned i = 0 ; i < attributes.getLength() ; i++ ) {
          StrX lname(attributes.getLocalName(i));
          StrX val(attributes.getValue(i));
          *waliErr << "    " << lname << "\t->\t" << val << "\n";
        }
        throw who.get();
      }
    }

    void WpdsHandler::endElement( 
        const XMLCh* const uri ATTR_UNUSED,
        const XMLCh* const localname,
        const XMLCh* const qname ATTR_UNUSED)
    {
      using wali::Key;
      StrX who(localname);
      if (WPDS::XMLTag == who.get()) {
        // do nothing
      }
      else if (DebugWPDS::XMLTag == who.get())
      {
        // do nothing
        *waliErr << "[INFO] End parsing DebugWPDS." << std::endl; 
      }
      else if (WpdsHandler::FunctionXMLTag == who.get()) {
        // do nothing
      }
      else if( Rule::XMLTag == who.get() ) {
        Key fromKey    = getKey(from.get());
        Key fromStkKey = getKey(fromStack.get());
        Key toKey      = getKey(to.get());
        Key toStk1Key  = getKey(toStack1.get());
        Key toStk2Key  = getKey(toStack2.get());

#if 0
        *waliErr << "(Rule\n";
        *waliErr << "\t" << from << std::endl;
        *waliErr << "\t" << fromStack << std::endl;
        *waliErr << "\t" << to << std::endl;
        if( toStack1.get() )
          *waliErr << "\t" << toStack1 << std::endl;
        if( toStack2.get() )
          *waliErr << "\t" << toStack2 << std::endl;
        *waliErr << "Weight >>  " << weightString << std::endl;
        *waliErr << ")" << std::endl;
        wali::sem_elem_t se = weightFactory.getWeight(weightString);
        se->print( *waliErr ) << std::endl;
#endif

        pds->add_rule(
            fromKey, fromStkKey, 
            toKey, toStk1Key, toStk2Key, 
            fUserHandler.getWeight("overwrite", fromStkKey)); // added parameters
      }
      else if (fUserHandler.handlesElement(who.get())) {
        fUserHandler.endElement(uri,localname,qname);
      }
      else {
        *waliErr << "[ERROR] WpdsHandler::endElement - unrecognized element.\n";
        *waliErr << "  " << who << "\n";
        throw who.get();
      }
    }

    void WpdsHandler::characters(const XMLCh *const chars, const XMLSize_t length)
    {
      fUserHandler.characters(chars,length);
    }

    //////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////
    void WpdsHandler::handleRule(
        const XMLCh* const uri ATTR_UNUSED, 
        const XMLCh* const localname ATTR_UNUSED, 
        const XMLCh* const qname ATTR_UNUSED, 
        const Attributes& attributes)
    {
      from      = attributes.getValue(fromID);
      fromStack = attributes.getValue(fromStackID);
      to        = attributes.getValue(toID);
      toStack1  = attributes.getValue(toStack1ID);
      toStack2  = attributes.getValue(toStack2ID);

      // Error Propagation code, move later
      std::string caller, callee = "";

      caller = fromStack.get();
      string::size_type loc = caller.find(".", 0);
      if (loc != string::npos) {
	caller = caller.substr(0, loc);
      }
      
      Key toStk1Key = getKey(toStack1.get());
      Key toStk2Key = getKey(toStack2.get());
      
      if ( (toStk1Key != WALI_EPSILON) && (toStk2Key != WALI_EPSILON)) {
	callee = toStack1.get();
	string::size_type loc = callee.find(".", 0);
	if (loc != string::npos) {
	  callee = callee.substr(0, loc);
	}
      }
      fUserHandler.setCaller(caller);
      fUserHandler.setCallee(callee);
    }

  } // namespace wpds

} // namespace wali

