#include "ErrorPropagationHandler.hpp"
#include "Main.hpp"
#include "bdd.h"
#include "WeightInfo.hpp"
#include <map>
#include <queue>
#include <set>
#include <vector>
#include "ProgramPoint.hpp"
#include "Assignment.hpp"
#include "AssignmentInfo.hpp"
#include "Return.hpp"
#include "Dereference.hpp"
#include "IsErr.hpp"
#include "Handled.hpp"
#include "Operand.hpp"
#include "Hexastore.hpp"
#include "Input.hpp"
#include "Output.hpp"
#include "MergeError.hpp"
#include "Predicate.hpp"
#include <xercesc/sax2/Attributes.hpp>

using namespace std;

// Some Bdds
bdd identity;

bdd localsToIdentity;
bdd localsToUninitialized;

bdd globalsToUniverse;
bdd globalsToIdentity;
bdd globalsToOK;
bdd globalsToUninitialized;

bdd constantsToIdentity;

bdd bddErrors;
bdd bddTentativeErrors;
bdd bddNonTentativeErrors;

map<int, int> mapTentativeToReal;
map<int, int> mapRealToTentative;

vector<string> globalVars;
vector<string> constants;

queue<WeightInfo>  weightInfos;
map<string, string> targets;
set<string> derefVars;
set<string> isErrVars;
set<string> handledVars;
set<string> operands;
set<string> inputs;
set<string> outputs;
set<string> predVars;

void populatingErrorDS();
void createBdds();

ProgramPoints programPoints;
set<wali::Key> setProgramPoints;

static enum {
  Elsewhere,
  InGlobals,
  InLocals,
  InPointers,
} whereAmI = Elsewhere;

bool ErrorPropagationHandler::inSet = false;
bool ErrorPropagationHandler::inReturn = false;
bool ErrorPropagationHandler::inDereference = false;
bool ErrorPropagationHandler::inIsErr = false;
bool ErrorPropagationHandler::inHandled = false;
bool ErrorPropagationHandler::inOperand = false;
bool ErrorPropagationHandler::inInput = false;
bool ErrorPropagationHandler::inOutput = false;
bool ErrorPropagationHandler::inPred = false;


ErrorPropagationHandler::ErrorPropagationHandler()
  : setToID("to"),
    setFromID("from"),
    trustedID("trusted"),
    lineID("line"),
    fileID("file"),
    localsID("locals"),
    globalsID("globals"),
    pointersID("pointers"),
    basisID("basis"),
    varIdID("id"),
    varNameID("name"),
    valueID("value"),
    predOp1ID("op1"),
    predOp2ID("op2")
{
}

bool ErrorPropagationHandler::handlesElement( std::string tag) {
  return (
	  (tag == SemElem::XMLTag) ||
	  (tag == MergeFn::XMLTag) || 
	  (tag == "Variables") ||
	  (tag == "Globals") ||
	  (tag == "Locals") ||
	  (tag == "Pointers") ||
	  (tag == "identity") ||
	  (tag == "one") ||
	  (tag == "Prologue") ||
	  (tag == "set") ||
	  (tag == "source") ||
	  (tag == "unimplemented") ||
	  (tag == "var") ||
	  (tag == "Weight") ||
	  (tag == "zero") ||
	  (tag == "return") ||
	  (tag == "dereference") ||
	  (tag == "iserr") ||
	  (tag == "handled") ||
	  (tag == "operand") ||
	  (tag == "input") ||
	  (tag == "output") ||
          (tag == "pred")
	  );

}


void ErrorPropagationHandler::startElement(
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attributes)
{
  
  StrX who(localname);
  parsedWeight = NULL;
  
  if (who.get() == "Variables") {
    // do nothing
  } else if (who.get() == "Globals") {
    whereAmI = InGlobals;
  }
  else if (who.get() == "Locals") {
    whereAmI = InLocals;
  }
  else if (who.get() == "Pointers") {
    whereAmI = InPointers;
  }
  else if (who.get() == "identity") {
    // do nothing
  }
  else if (who.get() == "one") {
    // do nothing
    parsedWeight = factory.one();
  }
  else if (who.get() == "Prologue") {
    // do nothing
  }
  else if (who.get() == "set") {
    inSet = true;
    handleSet(uri, localname, qname, attributes);
  }
  else if (who.get() == "source") {
    handleSource(uri, localname, qname, attributes);
  }
  else if (who.get() == "unimplemented") {
    // do nothing
  }
  else if (who.get() == "var") {
    handleVar(uri, localname, qname, attributes);
  }
  else if (who.get() == "Weight") {
    handleWeight(uri, localname, qname, attributes);
  }
  else if (who.get() == "zero") {
    // do nothing
    parsedWeight = factory.zero();
  }
  else if (who.get() == "return") {
    inReturn = true;
    handleReturn(uri, localname, qname, attributes);
  }
  else if (who.get() == "dereference") {
    inDereference = true;
    handleDereference(uri, localname, qname, attributes);
  }
  else if (who.get() == "iserr") {
    inIsErr = true;
    handleIsErr(uri, localname, qname, attributes);
  }
  else if (who.get() == "handled") {
    inHandled = true;
    handleHandled(uri, localname, qname, attributes);
  }
  else if (who.get() == "operand") {
    inOperand = true;
    handleOperand(uri, localname, qname, attributes);
  }
  else if (who.get() == "input") {
    inInput = true;
    handleInput(uri, localname, qname, attributes);
  }
  else if (who.get() == "output") {
    inOutput = true;
    handleOutput(uri, localname, qname, attributes);
  }
  else if (who.get() == "pred") {
    inPred = true;
    handlePred(uri, localname, qname, attributes);
  }
  else
    abort();
}



void ErrorPropagationHandler::endElement( 
	 const XMLCh* const uri ATTR_UNUSED
	 , const XMLCh* const localname
	 , const XMLCh* const qname ATTR_UNUSED) 
{
  
  StrX who(localname);
  
  if (who.get() == "one") {
    parsedWeight = factory.one();
  }
  else if (who.get() == "set") {
    weightString1 = setTo.get();
    weightString2 = setFrom.get();
    
    trustedString = trusted.get();
    useWeight = false;
    
    bool isTrusted = false;
    if (strcmp(trustedString.c_str(), "true") == 0) {
      isTrusted = true;
    }
    else {
      targets[weightString1] = weightString2;
    }
    addWeight(weightString1, weightString2, isTrusted);
    // parsedWeight set at the end of the rule

  }
  else if (who.get() == "Prologue") {
    parsedWeight = factory.one();
    createBdds();
  }
  else if (who.get() == "Globals") {
    whereAmI = Elsewhere;
  }
  else if (who.get() == "Locals") {
    whereAmI = Elsewhere;
  }
  else if (who.get() == "Pointers") {
    whereAmI = Elsewhere;
  }
  else if ((who.get() == "identity") || (who.get() == "unimplemented")) {
    weightString1 = weightString2 = "identity";
    addWeight(weightString1, weightString2, true);
    // parsedWeight set at the end of the rule
  }
  else if (who.get() == "identityGlobals") {
    weightString1 = weightString2 = "identityGlobals";
    addWeight(weightString1, weightString2, true);
    // parsedWeight set at the end of the rule
  }
  else if (who.get() == "uninitialized") {
    weightString1 = weightString2 = "uninitialized";
    addWeight(weightString1, weightString2, true);
    // parsedWeight set at the end of rule
  }
  else if (who.get() == "zero") {
    parsedWeight = factory.zero();
  }
}



void ErrorPropagationHandler::characters(
	const XMLCh* const chars ATTR_UNUSED
	, const unsigned int length ATTR_UNUSED)
{
}


void ErrorPropagationHandler::handleSet(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  setTo = attributes.getValue(setToID);
  setFrom = attributes.getValue(setFromID);
  trusted = attributes.getValue(trustedID);
}


void ErrorPropagationHandler::handleSource(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX sline = attributes.getValue(lineID);
  sscanf(sline.get().c_str(), "%d", &line);
  
  const StrX sfile = attributes.getValue(fileID);
  file = sfile.get();
}

void ErrorPropagationHandler::handlePred(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes)
{
  const StrX sOp1 = attributes.getValue(predOp1ID);
  if (!sOp1.get().empty()) {
    predVars.insert(sOp1.get());
  }
  const StrX sOp2 = attributes.getValue(predOp2ID);
  if (!sOp2.get().empty()) {
    predVars.insert(sOp2.get());
  }
}

void ErrorPropagationHandler::handleVar(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const string varId(StrX(attributes.getValue(varIdID)).get());
  const string varName(StrX(attributes.getValue(varNameID)).get());
  variables->add(varId, varName); // all variables

  switch (whereAmI) {
  case InGlobals:
    globalVars.push_back(varId);
    variables->addGlobal(varId); // only global variables
    break;

  case InLocals:
    variables->addLocal(varId); // only local variables
    break;
  
  case InPointers:
    variables->addPointer(varId); // only pointer variables
    break;

  default:
    abort();
  }
}
	

void ErrorPropagationHandler::handleWeight(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
		
  line = 0;
  file = "na";
  const StrX basis = attributes.getValue(basisID);

  if (strcmp(basis.get().c_str(), "identity") == 0) {
    weightString1 = weightString2 = "identity";
    addWeight(weightString1, weightString2, true);
  }
  else if (strcmp(basis.get().c_str(), "identityGlobals") == 0) {
    weightString1 = weightString2 = "identityGlobals";
    addWeight(weightString1, weightString2, true);
  }
  else if (strcmp(basis.get().c_str(), "uninitialized") == 0) {
    weightString1 = weightString2 = "uninitialized";
    addWeight(weightString1, weightString2, true);
  }
}


void ErrorPropagationHandler::handleReturn(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX svalue = attributes.getValue(valueID);
  value = svalue.get();
}


void ErrorPropagationHandler::handleDereference(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX sname = attributes.getValue(varNameID);
  name = sname.get();
  derefVars.insert(name);
}


void ErrorPropagationHandler::handleIsErr(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX sname = attributes.getValue(varNameID);
  name = sname.get();
  isErrVars.insert(name);
}


void ErrorPropagationHandler::handleHandled(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX sname = attributes.getValue(varNameID);
  name = sname.get();
  handledVars.insert(name);
}


void ErrorPropagationHandler::handleOperand(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX sname = attributes.getValue(varNameID);
  name = sname.get();
  operands.insert(name);
}

void ErrorPropagationHandler::handleInput(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  
  const StrX sname = attributes.getValue(varNameID);
  name = sname.get();
  inputs.insert(name);
}

void ErrorPropagationHandler::handleOutput(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes) 
{
  const StrX svalue = attributes.getValue(valueID);
  value = svalue.get();
  outputs.insert(value);
}


void ErrorPropagationHandler::setCaller(std::string c) {
  caller = c;
}

void ErrorPropagationHandler::setCallee(std::string c) {
  callee = c;
}


void createBdds() {

  // initialize the table of named constants
  for(unsigned int i = 0; i < errorNames.size(); i++)
    constants.push_back(errorNames[i]);
  constants.push_back("OK");
  constants.push_back("UNINITIALIZED");
  
  // Needed before creating any BDD
  const int numberVars = constants.size() + globalVars.size() + variables->numLocals();
  int domain[] = {numberVars, numberVars, numberVars};
  
  fdd_extdomain(domain, sizeof(domain)/sizeof(domain[0]));
  ErrorPropagation::initialize();
  
  // Creating identity
  identity = bddfalse;
  for(int i = 0; i < numberVars; i++) {
    identity |= (fdd_ithvar(0, i) & fdd_ithvar(1, i));
  }
  
  // Creating bddErrors
  populatingErrorDS();

  // Computing globals -> universe, locals -> identity to be used in apply_f MergeError
  // Computing all -> uninitialized
  
  // G -> U and G -> I            
  bdd globalsBDD0 = bddfalse;
  bdd constantsBDD0 = bddfalse;
          
  globalsToIdentity = bddfalse;
  for(size_t i = 0; i < globalVars.size(); i++) {
        
    int index = variables->getIndex(globalVars[i]);
        
    globalsBDD0 |= fdd_ithvar(0, index);
    globalsToIdentity |= (fdd_ithvar(0, index) & fdd_ithvar(1, index));
  }
  
  constantsToIdentity = bddfalse;
  for(size_t i = 0; i < constants.size(); i++) {
        
    int index = variables->getIndex(constants[i]);
        
    constantsBDD0 |= fdd_ithvar(0, index);
    constantsToIdentity |= (fdd_ithvar(0, index) & fdd_ithvar(1, index));
  }
  
  bdd allBDD = bddtrue;
  
  int indexUninitialized = variables->getIndex("UNINITIALIZED");
  bdd uninitializedBDD = fdd_ithvar(1, indexUninitialized);
  
  int indexOK = variables->getIndex("OK");
  bdd okBDD = fdd_ithvar(1, indexOK);
  
  
  // rest of global BDDs
  globalsToUniverse = globalsBDD0 & allBDD;
  globalsToOK = globalsBDD0 & okBDD;
  globalsToUninitialized = globalsBDD0 & uninitializedBDD;
  
  // rest of constant BDDs
  bdd constantsToUninitialized = constantsBDD0 & uninitializedBDD;
  
  //rest of local BDDS
  localsToIdentity = identity - globalsToIdentity - constantsToIdentity;
  
  bdd allToUninitialized = allBDD & uninitializedBDD;
  localsToUninitialized = allToUninitialized - globalsToUninitialized - constantsToUninitialized;
  
}


void ErrorPropagationHandler::addWeight(string w1, string w2, bool t) {
  // Creating weightInfo
  WeightInfo wi;
  
  wi.weight1 = w1;
  wi.weight2 = w2;
  wi.function = caller;
  wi.callee = callee;
  wi.trusted = t;
  
  weightInfos.push(wi);
}



void populatingErrorDS() {
  
  for(unsigned int i = 0; i < errorNames.size(); i++) {
    int index = variables->getIndex(errorNames[i]);
    bddErrors |= fdd_ithvar(1, index);

    if (i < errorNames.size()/2) { // number of errors (non-tentative + tentative / 2)
      int indexTentative = variables->getIndex("TENTATIVE_" + errorNames[i]);
      bddTentativeErrors |= fdd_ithvar(1, indexTentative);
      
      mapTentativeToReal[indexTentative] = index;
      mapRealToTentative[index] = indexTentative;
    }
  }
  
  bddNonTentativeErrors = bddErrors - bddTentativeErrors;
}


sem_elem_t ErrorPropagationHandler::getWeight() {
  // weights were grouped by calling addWeight
  targets.clear();
  return factory.getWeight(weightInfos, line, file);
}


// Called per rule, thus we group all inputs (for example) in order to consider them altogether.
void ErrorPropagationHandler::createPoint(std::string msgcode, Key fromStkKey) {

  // The first boolean var is a global command line option
  // The second boolean var matches the rule
  if (return_values && inReturn) {
    shared_ptr<ProgramPoint> point(new Return(file, line, caller, msgcode, fromStkKey, value));
    inReturn = false;
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);
  }
  else if (iserr && inIsErr) {
    shared_ptr<ProgramPoint> point(new IsErr(file, line, caller, msgcode, fromStkKey, isErrVars));
    inIsErr = false;
    isErrVars.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);
  }
  else if (dereference && inDereference) {
    shared_ptr<ProgramPoint> point(new Dereference(file, line, caller, msgcode, fromStkKey, derefVars));
    inDereference = false;
    derefVars.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);
  }
  else if (dereference && inOperand) {
    shared_ptr<ProgramPoint> point(new Operand(file, line, caller, msgcode, fromStkKey, operands));
    inOperand = false;
    operands.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);
  }
  else if (handled && inHandled) {
    shared_ptr<ProgramPoint> point(new Handled(file, line, caller, msgcode, fromStkKey, handledVars));
    inHandled = false;
    handledVars.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);
  }
  else if (input_output && inInput) {
    shared_ptr<ProgramPoint> point(new Input(file, line, caller, msgcode, fromStkKey, inputs));
    inInput = false;
    inputs.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);    
  }
  else if (input_output && inOutput) {
    shared_ptr<ProgramPoint> point(new Output(file, line, caller, msgcode, fromStkKey, outputs));
    inOutput = false;
    outputs.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);
  }
  else if (predicates && inPred) {
    shared_ptr<ProgramPoint> point(new Predicate(file, line, caller, msgcode, fromStkKey, predVars));
    inPred = false;
    predVars.clear();
    programPoints.push_back(point);
    setProgramPoints.insert(fromStkKey);    
  }
  else if (!return_values && !dereference && !iserr && !handled && !input_output && inSet && targets.size() > 0) {

    if (hexastore) {
      shared_ptr<ProgramPoint> point(new Hexastore(file, line, caller, msgcode, fromStkKey, targets));
      programPoints.push_back(point);
    }
    else if (info) {
      shared_ptr<ProgramPoint> point(new AssignmentInfo(file, line, caller, msgcode, fromStkKey, targets));
      programPoints.push_back(point);
    }
    else if (option_assignment) { // original error-prop analysis
      shared_ptr<ProgramPoint> point(new Assignment(file, line, caller, msgcode, fromStkKey, targets));
      programPoints.push_back(point);
    }

    // independent of analysis
    inSet = false;
    setProgramPoints.insert(fromStkKey);
  }
  return;
}


sem_elem_t ErrorPropagationHandler::getWeight(std::string msgcode, Key fromStkKey) {
  createPoint(msgcode, fromStkKey);
  return getWeight();
}


bool ErrorPropagationHandler::hasMergeFn() {
  return true;
}

merge_fn_t ErrorPropagationHandler::getMergeFn(sem_elem_t se, Key fromStkKey) {
  createPoint("", fromStkKey);
  return new MergeError(callee, se);
}


ref_ptr<wali::wpds::Wrapper> ErrorPropagationHandler::getWrapper()
  {
    return new WitnessWrapper();
  }
