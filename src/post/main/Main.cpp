/*
 * @author Nick Kidd, Modified by Cindy Rubio Gonzalez
 */

#include <wali/QueryHandler.hpp>
#include "wali/wpds/WpdsHandler.hpp"
#include "wali/wfa/WfaHandler.hpp"
#include "ErrorPropagationHandler.hpp"
#include "ParseArgv.hpp"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include "fdd.h"
#include "wali/wpds/ewpds/EWpdsHandler.hpp"
#include "wali/wpds/ewpds/EWPDS.hpp"
#include "wali/wpds/fwpds/FWPDS.hpp"
#include "wali/witness/WitnessWrapper.hpp"
#include "wali/wfa/State.hpp"
#include "wali/witness/Witness.hpp"
#include "wali/util/Timer.hpp"
#include "Main.hpp"
#include "ProgramPoint.hpp"
#include "MessageGeoff.hpp"
#include <time.h>
#include <iostream>

//#include "llvm/Support/raw_ostream.h"
//#include "llvm/Support/SourceMgr.h"
//#include "llvm/IR/LegacyPassManager.h"
//#include "llvm/IR/LLVMContext.h"
//#include "llvm/IRReader/IRReader.h"
//#include "llvm/IR/Module.h"

//#include "../irllvm-wpds-xml/Rules.hpp"

XERCES_CPP_NAMESPACE_USE

PrintDirection print = Backward; /*Default printing style*/

bool printdot = false;
string dotfile;
string option_irllvm;
//RulesPass* llvm_pass;
//llvm::legacy::PassManager PM;

bool verbose = false;
bool debugging = false;
bool report_tentative = false;
bool report_temps = false;
bool return_values = false;
bool info = false;
bool hexastore = false;
bool csv = false;
bool dereference = false;
bool iserr = false;
bool handled = false;
bool input_output = false;
bool predicates = false;
bool option_assignment = true;

Message::Formatters selected_formatter = Message::Formatters::STANDARD;

int parseWpds(std::string& xmlFile);
int parseWfa(std::string& xmlFile);
int parseQuery(std::string& xmlFile);
int parseFile(DefaultHandler&, std::string& xmlFile);
void openFiles(ofstream& foutAll, ofstream& fout);
void closeFiles(ofstream& foutAll, ofstream& fout);

int main( int argc, char** argv ) {

  k::ParseArgv parser(argc,argv);
  std::string fname;
  std::string outname;

  if (parser.get("--error-codes", fname))
    variables.reset(new Variables(fname));
  else {
    cerr << "missing required \"--error-codes\" option\n";
    return 2;
  }

  if (parser.get("--print", fname)) {
    if (fname == "forward") {
      print = Forward;
    }
    else if (fname == "none") {
      print = PrintDirection::None;
    }
  }

  if (parser.get("--printdot", dotfile)) {
    printdot = true;
  }

  if (parser.exists("--verbose")) {
    verbose = true;
    wali::util::Timer::measureAndReport = verbose;
  }

  if (parser.exists("--debugging")) {
    debugging = true;
  }

  if (parser.exists("--tentative")) {
    report_tentative = true;
  }

  if (parser.exists("--temps")) {
    report_temps = true;
  }

  if (parser.exists("--return")) {
    return_values = true;
  }

  if (parser.exists("--info")) {
    info = true;
  }

  if (parser.exists("--hexastore")) {
    hexastore = true;
  }

  if (parser.exists("--csv")) {
    csv = true;
    info = true;
  }

  if (parser.exists("--dereference")) {
    dereference = true;
  }

  if (parser.exists("--iserr")) {
    iserr = true;
  }

  if (parser.exists("--handled")) {
    handled = true;
  }

  if (parser.exists("--input-output")) {
    input_output = true;
  }

  if (parser.exists("--geoff")) {
    print = Forward;
    selected_formatter = Message::Formatters::GEOFF;
  }

  if (parser.exists("--traces")) {
    selected_formatter = Message::Formatters::TRACES;
  }

  if (parser.exists("--predicates")) {
    predicates = true;
  }

  if (parser.exists("--no-assignment")) {
    option_assignment = false;
  }

  int rc=0;

  // Initialize the XML4C2 system
  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch) {
    XERCES_STD_QUALIFIER cerr << "Error during initialization! :\n" << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
    return 1;
  }

  bdd_cpp_init(1000000, 100000);
  bdd_setcacheratio(32);
  bdd_gbc_hook(NULL);

  if( parser.get("--wpds",fname) ) {
    rc |= parseWpds(fname);
  }
  if( parser.get("--wfa",fname) ) {
    rc |= parseWfa(fname);
  }
  if( parser.get("--query",fname) ) {
    rc |= parseQuery(fname);
  }

  bdd_done();
  XMLPlatformUtils::Terminate();

  return rc;
}


int parseWpds( std::string& xmlFile )
{

  wali::wpds::fwpds::FWPDS *fwpds = new wali::wpds::fwpds::FWPDS();
  ErrorPropagationHandler eph;
  wali::wpds::ewpds::EWpdsHandler handler(eph, fwpds);

  int rc = parseFile(handler,xmlFile);
  handler.get().print( std::cout );
  return rc;
}

int parseWfa( std::string& xmlFile )
{
  ErrorPropagation wf(bddfalse);
  ErrorPropagationHandler eph;
  wali::wfa::WfaHandler handler(eph);
  int rc = parseFile(handler,xmlFile);
  handler.get().print( std::cout );
  return rc;
}

int parseQuery( std::string& xmlFile )
{

  time_t start, end;
  ErrorPropagationHandler eph;
  wali::QueryHandler handler(eph);

  if (verbose) clog << "Phase 1: Parsing XML file and creating rules\n";
  
  time(&start);
  
  int rc = parseFile(handler, xmlFile);
  
  time(&end);
  
  if (verbose) clog << "\tTotal time: " << difftime(end, start) << " seconds\n";


  if( 0 == rc ) {

    if (verbose) clog << "Phase 2: Constructing the WFA\n";

    time(&start);
    // Constructing the WFA
    wali::wfa::WFA & result = handler.run();

    // filtering uninteresting program points
    result.filter(setProgramPoints);

    // calculating weights from start of program to particular program points
    result.path_summary();

    // Populating map from trans to key so that we can retrieve any trans
    // given a key without the need to iterate through all transtitions
    wali::wfa::State* st = result.getState(result.getInitialState());

    map<wali::Key, wali::wfa::ITrans*> mapTrans;
    wali::wfa::State::iterator it = st->begin();
    for(; it != st->end(); it++) {
      wali::wfa::ITrans* t = *it;
      wali::Key s = t->stack();
      mapTrans[s] = t;
    }

    time(&end);

    if (verbose) clog << "\tTotal time: " << difftime(end, start) << " seconds\n";


    if (verbose) {
      clog << "Phase 3: Inspecting interesting program points\n";
      clog << "\tTotal # of points " << programPoints.size() << endl;
    }

    time(&start);

    // GEOFF reports need a one-time header with the error codes
    if (selected_formatter == Message::Formatters::GEOFF) {
      std::cout << MessageGeoff::getGeoffErrorHeader();
    }

    // Iterating through potential interesting program points
    for(unsigned int i = 0; i < programPoints.size(); i++) {
      if (debugging) {
	clog << "Processing point #" << i+1 << " out of " << programPoints.size() << endl;
      }
      programPoints[i]->calculateWeight(result, mapTrans);
      programPoints[i]->process();
    }

    time(&end);

    if (verbose) clog << "\tTotal time: " << difftime(end, start) << " seconds\n";
  }
  return rc;
}


int parseFile(DefaultHandler& handler, std::string& xmlFile) {
    int errorCode = 0;

    auto_ptr<SAX2XMLReader> parser(XMLReaderFactory::createXMLReader());

    try {
	parser->setContentHandler(&handler);
	parser->setErrorHandler(&handler);
	parser->parse(xmlFile.c_str());
    }
    catch( const OutOfMemoryException&)
    {
	XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
	errorCode = 2;
    }
    catch (const XMLException& toCatch)
    {
	XERCES_STD_QUALIFIER cerr << "\nAn error occurred\n  Error: "
	    << StrX(toCatch.getMessage())
	    << '\n' << XERCES_STD_QUALIFIER endl;
	errorCode = 3;
    }
    catch( const SAXException& toCatch ) {
	std::cerr << "Error in Main: " << StrX(toCatch.getMessage()) << std::endl;
	errorCode = 4;
    }

    return errorCode;
}


void openFiles(ofstream& foutAll, ofstream& fout) {
    string fileAll = "all-" + dotfile;
    foutAll.open(fileAll.c_str());

    foutAll << "digraph G {" << endl;
    foutAll << "  rankdir=\"TB\";" << endl;
    foutAll << "  page=\"8.5,11\";" << endl;
    foutAll << "  orientation=\"landscape\";" << endl;
    foutAll << "  size=\"20,20\";" << endl;

    fout.open(dotfile.c_str());
}


void closeFiles(ofstream& foutAll, ofstream& fout) {
  foutAll << "}\n";
  foutAll.close();
  fout.close();
}

