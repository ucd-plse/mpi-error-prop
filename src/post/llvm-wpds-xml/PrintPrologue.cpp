#include "AssignmentsReturnBlock.hpp"
#include "CollectVariables.hpp"
#include "Conditionals.hpp"
#include "ExchangeAssignments.hpp"
#include "ExchangeFormals.hpp"
#include "ExchangeReturns.hpp"
#include "FilterVariables.hpp"
#include "llvm-version.hpp"
#include "PrintPrologue.hpp"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <sstream>
#include <map>
#include <set>

using namespace llvm;

cl::opt<string> SchemaFileName("schema", cl::value_desc("filename"), cl::desc("Schema file"), cl::init("error-prop.xsd"));
cl::opt<string> OutputFileName("output", cl::value_desc("filename"), cl::desc("Output XML file"));


StringRef
PrintPrologue::findVariableName(const GlobalVariable &gvar)
{
#if LLVM_VERSION >= 30500
  for (const DIGlobalVariable globalDescriptor : debugInfo.global_variables())
    if (globalDescriptor.getGlobal() == &gvar)
      {
	const StringRef displayName = globalDescriptor.getDisplayName();
	return displayName.empty() ? gvar.getName() : displayName;
      }
#else
  for (DebugInfoFinder::iterator global = debugInfo.global_variable_begin(), end = debugInfo.global_variable_end(); global != end; ++global)
    {
      const DIGlobalVariable globalDescriptor(*global);
      if (globalDescriptor.getGlobal() == &gvar)
	{
	  const StringRef displayName = globalDescriptor.getDisplayName();
	  return displayName.empty() ? gvar.getName() : displayName;
	}
    }
#endif

  return gvar.getName();
}


static StringRef
findVariableName(const Value &value, const Function &function)
{
  for (Function::const_iterator block = function.begin(), end = function.end(); block != end; ++block)
    for (BasicBlock::const_iterator instruction = block->begin(), end = block->end(); instruction != end; ++instruction)
      if (const DbgDeclareInst * const declaration = dyn_cast<DbgDeclareInst>(instruction))
	if (declaration->getAddress() == &value)
	  {
	    DIVariable info(declaration->getVariable());
	    const StringRef debugName = info.getName();
	    return debugName.empty() ? value.getName() : debugName;
	  }

  return value.getName();
}


static StringRef
findVariableName(const Instruction &instruction)
{
  return findVariableName(instruction, *instruction.getParent()->getParent());
}



static StringRef
findVariableName(const Argument &argument)
{
  return findVariableName(argument, *argument.getParent());
}



void
PrintPrologue::printVarElement(const StringRef unique, const string &original) {
  *outfile << "\t\t\t\t\t<var id='" << unique << '\'';
  if (!original.empty() && original != unique) {
    *outfile << " name='" << original << '\'';
  }
  *outfile << "/>\n";
}



void
PrintPrologue::printTmpVarElement(Value &value, const Function &function, int counter) {
  ostringstream formatter;
  formatter << function.getName().str() << "#cabs2cil_" << counter;
  value.setName(formatter.str());
  printVarElement(value.getName(), formatter.str());
}




void
PrintPrologue::renameAndPrint(Value &value, const Function &function, const StringRef original)
{
  if (value.getName().str().find('#') == string::npos && !value.getName().str().empty()) {
    const string prefix(function.getName().str() + '#');
    const string prefixedOriginal(prefix + original.str());
    const string prefixedUnique(prefix + value.getName().str());
    value.setName(prefixedUnique);
    printVarElement(prefixedUnique, prefixedOriginal);
  }
}


bool PrintPrologue::runOnFunction(Function &F) {

  Variables printed;
  Function *f = &F;

  // printing arguments
  for(Function::arg_iterator a = f->arg_begin(), ae = f->arg_end(); a != ae; a++) {
    map<Value*, Value*>::iterator it = argToAlloca.find(a);
    if (it != argToAlloca.end()) {
      const StringRef original = ::findVariableName(*a);
      renameAndPrint(*a, *f, original);
      printed.insert(it->second);
    }
  }

  // printing original local variables
  for(Function::iterator b = f->begin(), be = f->end(); b != be; b++)
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++)
      if (DbgDeclareInst * const declareInst = dyn_cast<DbgDeclareInst>(i))
	if (AllocaInst * const allocaInst = dyn_cast<AllocaInst>(declareInst->getAddress()))
	  if (locals[f].find(allocaInst) != locals[f].end()) {
	    const StringRef original = ::findVariableName(*allocaInst);
	    renameAndPrint(*allocaInst, *f, original);
	    printed.insert(allocaInst);
	  }

  // printing introduced locals, including temporaries
  int tmpCounter = 1;
  for(Variables::iterator it = locals[f].begin(), ite = locals[f].end(); it != ite; it++) {

    if (printed.find(*it) == printed.end()) {

      if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(*it)) {
	const StringRef prefix("__cil_tmp");

	if (allocaInst->getName().startswith(prefix)) {
	  const StringRef original = ::findVariableName(*allocaInst);
	  renameAndPrint(*allocaInst, *f, original);
	}
	else {
	  allocaInst->setName("");
	  printTmpVarElement(*allocaInst, *f, tmpCounter++);
	}
	printed.insert(*it);
      }
    }
  }

  return false;
}


bool PrintPrologue::runOnModule(Module &M) {

  debugInfo.processModule(M);

  FilterVariables &vars = getAnalysis<FilterVariables>();
  const Variables &globals = vars.getGlobals();
  locals = vars.getLocals();

  CollectVariables &allVars = getAnalysis<CollectVariables>();
  argToAlloca = allVars.getArgToAlloca();

#if LLVM_VERSION >= 30600
  error_code errorCode;
  outfile = make_unique<raw_fd_ostream>(OutputFileName, errorCode, llvm::sys::fs::F_None);
#else
  string errorInfo;
  outfile.reset(new raw_fd_ostream(OutputFileName.c_str(), errorInfo, llvm::sys::fs::F_None));
#endif

  // printing prologue header

  *outfile << "<Query type='poststar' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='file://"
	   << SchemaFileName << "'>\n"
	   << "\t<FWPDS>\n"
	   << "\t\t<Prologue>\n"
	   << "\t\t\t<Variables>\n";

  // printing globals

  *outfile << "\t\t\t\t<Globals>\n";

  for(Variables::iterator it = globals.begin(), ite = globals.end(); it != ite; it++) {
    if (GlobalVariable *gvar = dyn_cast<GlobalVariable>(*it)) {
      const StringRef original(findVariableName(*gvar));
      const StringRef unique(gvar->getName());
      printVarElement(unique, original);
    }
  }
  *outfile << "\t\t\t\t</Globals>\n";


  *outfile << "\t\t\t\t<Locals>\n";

  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    if (!f->isDeclaration()) {
      runOnFunction(*f);
    }
  }

  *outfile << "\t\t\t\t</Locals>\n"
	   << "\t\t\t\t<Pointers/>\n"
	   << "\t\t\t</Variables>\n"
	   << "\t\t</Prologue>\n";
  outfile->flush();

  return false;
}


// same as current Rules
void PrintPrologue::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<CollectVariables>();
  AU.addRequired<FilterVariables>();
}

char PrintPrologue::ID = 0;
static const RegisterPass<PrintPrologue> registration("print-prologue", "Prints WPDS XML prologue");
