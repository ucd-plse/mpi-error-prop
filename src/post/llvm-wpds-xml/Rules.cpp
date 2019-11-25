#include "AssignmentsReturnBlock.hpp"
#include "CollectVariables.hpp"
#include "Conditionals.hpp"
#include "ExchangeAssignments.hpp"
#include "ExchangeFormals.hpp"
#include "ExchangeReturns.hpp"
#include "FilterVariables.hpp" // error-code file name
#include "llvm-version.hpp"
#include "PrintPrologue.hpp"
#include "Rules.hpp"
#include <sstream>

#if LLVM_VERSION >= 30500
#include <llvm/IR/CFG.h>
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#include <llvm/Support/CFG.h>
#endif

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

#include <fstream>

using namespace llvm;

cl::opt<string> LocationsFileName("locations", cl::value_desc("locations"), cl::desc("Intermediate file for --locations"));

bool Rules::runOnModule(Module &M) {

  // opening file
#if LLVM_VERSION >= 30600
  error_code errorCode;
  outfile = make_unique<raw_fd_ostream>(OutputFileName, errorCode, llvm::sys::fs::F_Append);
  if (errorCode) {
    errs() << "Unable to open " << OutputFileName << ": " << errorCode.message() << '\n';
    exit(1);
  }
  if (! LocationsFileName.empty()) {
    locfile = make_unique<raw_fd_ostream>(LocationsFileName, errorCode, llvm::sys::fs::F_RW);
    if (errorCode) {
      errs() << "Unable to open " << LocationsFileName << ": " << errorCode.message() << '\n';
      exit(1);
    }
  }
#else
  string errorInfo;
  outfile.reset(new raw_fd_ostream(OutputFileName.c_str(), errorInfo, llvm::sys::fs::F_Append));
  if (!errorInfo.empty()) {
    errs() << "Unable to open " << OutputFileName << ": " << errorInfo << '\n';
    exit(1);
  }
#endif

  // populating errorNames map
  ifstream inFile(ErrorCodesFileName.c_str());
  string name;
  int value;

  if (!inFile) {
    errs() << "Unable to open " << ErrorCodesFileName << '\n';
    exit(1);
  }

  while(inFile >> name >> value) {
    errorNames[value] = "TENTATIVE_" + name;
  }

  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    if (!f->isDeclaration()) {
      runOnFunction(*f);
    }
  }

  *outfile << "\t</FWPDS>\n";
  *outfile << "\t<WFA query='INORDER'>\n";
  *outfile << "\t\t<State Name='p' initial='true'><Weight basis='identity'><zero/></Weight></State>\n";
  *outfile << "\t\t<State Name='accept' final='true'><Weight basis='identity'><zero/></Weight></State>\n";
  *outfile << "\t\t<Trans from='p' stack='main.0' to='accept'><Weight basis='identity'><one/></Weight></Trans>\n";
  *outfile << "\t</WFA>\n";
  *outfile << "</Query>\n";

  return false;
}


bool Rules::runOnFunction(Function &F) {
  
  //CollectVariables &vars = getAnalysis<CollectVariables>();
  FilterVariables &vars = getAnalysis<FilterVariables>();
  locals = vars.getLocals();
  globals = vars.getGlobals();

  // Assigning instruction IDs
  unsigned int count = 0;
  InstructionID.clear();

  for(Function::iterator b = F.begin(), be = F.end(); b != be; b++) {    
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
      Instruction *ins = i;
      InstructionID[ins] = ++count; 
    }
  }


  // Printing the first intraprocedural rule
  printIntraRuleOpen(0, 1, &F);
  if (F.getName().str() == "main") {
    *outfile << "\t\t\t<Weight basis='uninitialized'>\n";
  }
  else {
    *outfile << "\t\t\t<Weight basis='identityGlobals'>\n";
  }
  *outfile << "\t\t\t</Weight>\n";
  printRuleClose();


  // Printing the rest of the rules
  for(Function::iterator b = F.begin(), be = F.end(); b != be; b++) {
    printBlockRules(b, &F);
  }
  
  return false;
}



void Rules::printIntraRuleOpen(unsigned int fromStack, unsigned int toStack, Function *function) {

  *outfile << "\t\t<Rule from='p' fromStack='" << function->getName() << '.'
         << fromStack << "' to='p' toStack1='" << function->getName() << '.'
         << toStack << "'>\n";
  return;
}


void Rules::printRuleClose() {
  *outfile << "\t\t</Rule>\n";
  return;
}


void Rules::printPushRule(BasicBlock *block, Function *caller, Function *callee, CallInst* callInst) {

  Instruction *terminator = block->getTerminator();
  if (const BranchInst * const branchInst = dyn_cast<BranchInst>(terminator)) {
    BasicBlock *successor = branchInst->getSuccessor(0); // this should be an unconditional branch

    unsigned int fromStack = InstructionID[&block->front()];
    unsigned int toStack2 = InstructionID[&successor->front()];
      
    *outfile << "\t\t<Rule from='p' fromStack='" << caller->getName() << '.' << fromStack 
	     << "' to='p' toStack1='" << callee->getName() << ".0'"
	     << " toStack2='" << caller->getName() << '.' << toStack2 << "'>\n";

    *outfile << "\t\t\t<Weight basis='identity'>\n";
  
    Instruction *currentInst = NULL;
    bool source = true;
  
    // there should be at least call and branch instructions
    for(BasicBlock::iterator i = block->begin(), ie = block->end(); i != ie; i++) {
      currentInst = i;
      printInstructionWeight(currentInst, source);
    }

    *outfile << "\t\t\t</Weight>\n";  
    printSource(callInst); // we always print source for push rules
    printRuleClose();

  }
  else {
    assert(false);
  }

  return;
}


void Rules::printPopRule(BasicBlock *block, Function *function) {

  unsigned int fromStack = InstructionID[&block->front()];
  bool source = true;

  *outfile << "\t\t<Rule from='p' fromStack='" << function->getName() << '.' <<  fromStack << "' to='p'>\n";
  
  *outfile << "\t\t\t<Weight basis='identity'>\n";
  
  Instruction *currentInst = NULL;
  
  // there should be at least the return instruction
  for(BasicBlock::iterator i = block->begin(), ie = block->end(); i != ie; i++) {
    currentInst = i;
    printInstructionWeight(currentInst, source);
  }

  *outfile << "\t\t\t</Weight>\n";  
  printSource(currentInst); // we always print source for push rules
  printRuleClose();

  return;
}


void Rules::printInstructionWeight(Instruction *currentInst, bool &source) {

  if (const StoreInst * const storeInst = dyn_cast<StoreInst>(currentInst)) {
    Value* sender = storeInst->getOperand(0);
    Value* receiver = storeInst->getOperand(1);
    
    Function* function = currentInst->getParent()->getParent();
    
    if (globals.find(receiver) != globals.end() ||
	locals[function].find(receiver) != locals[function].end()) {

      string receiverName;

      const CollectVariables &vars = getAnalysis<CollectVariables>();
      const map<Value*, Value*> &allocaToArg = vars.getAllocaToArg();
      if (allocaToArg.find(receiver) != allocaToArg.end()) {
	receiverName = allocaToArg.find(receiver)->second->getName().str();
      }
      else if (receiver->hasName()) {
	receiverName = receiver->getName().str();
      } 
      
      if (!receiverName.empty()) {
	
	string trusted = "false";
	if (receiverName.find('$') != string::npos) {
	  trusted = "true";
	}

	if (const ConstantInt * const constantInt = dyn_cast<ConstantInt>(sender)) {
	  
	  string constantIntName = "OK";
	  int numberValue = constantInt->getLimitedValue();
	  
	  if (errorNames.find(numberValue) != errorNames.end()) {
	    constantIntName = errorNames[numberValue];
	  }

          // Use --locations=path to turn on or off tracking of unique error instances
          if (! LocationsFileName.empty()) {
              // We ignore OK because we only care about unique instances of error generation points
              if (constantIntName != "OK") {
                DILocation loc = getSource(currentInst, true);
                if (loc.Verify()) {
                  string file = loc.getFilename();
                  string line = std::to_string(loc.getLineNumber());
            
                  // Modify the EC name used in WPDS rule
                  constantIntName += "$" + file + ":" + line;

                  // Need to strip off "TENTATIVE" to match error codes file format
                  // At this point the EC is always tentative
                  string ecName = constantIntName.substr(10, string::npos);
                  *locfile << ecName << " " << "0\n";
                }
              }
          }
	  
	  trusted = "false";
	  if (numberValue == -67737868) {
	    trusted = "true";
	  }
	  
	  *outfile << "\t\t\t\t<set to='" << receiverName
		   << "' from='" << constantIntName 
		   << "' trusted='" << trusted << "'/>\n";
	}
	else {
	  
	  // sender is a register
	  if (const LoadInst *loadInst = dyn_cast<LoadInst>(sender)) {
	    Value* realSender = loadInst->getOperand(0);

	    if (globals.find(realSender) != globals.end() ||
		locals[function].find(realSender) != locals[function].end()) {
	      
	      string senderName;
	      if (allocaToArg.find(realSender) != allocaToArg.end()) {
		senderName = allocaToArg.find(realSender)->second->getName().str();
	      }
	      else {
		senderName = realSender->getName().str();
	      } 	
      
	      if ((!loadInst->isVolatile()) && !senderName.empty()) { //????
		*outfile << "\t\t\t\t<set to='" << receiverName
			 << "' from='" << senderName 
			 << "' trusted='" << trusted << "'/>\n";
	      }
	    }
	    else {
	      // sender could be anything, including a non-relevant variable

	      *outfile << "\t\t\t\t<set to='" << receiverName
		       << "' from='" << "OK"
		       << "' trusted='" << trusted << "'/>\n";

	    }
	  }
	  else if (const Instruction *senderInst = dyn_cast<Instruction>(sender)) {
	    switch(senderInst->getOpcode()) {
	    case Instruction::Add:
	    case Instruction::FAdd:
	    case Instruction::Sub:
	    case Instruction::FSub:
	    case Instruction::Mul:
	    case Instruction::FMul:
	    case Instruction::UDiv:
	    case Instruction::SDiv:
	    case Instruction::FDiv:
	    case Instruction::URem:
	    case Instruction::SRem:
	    case Instruction::FRem:
	    case Instruction::And:
	    case Instruction::Or:
	    case Instruction::Xor:
	      *outfile << "\t\t\t\t<set to='" << receiverName
		       << "' from='OK' trusted='" << trusted << "'/>\n";
	      break;

	    default:	    
	    // do nothing
	      break;	    
	    
	    }
	  }
	  
	}
      }
      else {
	// receiver has no name
      
      }
    }
  }
  else if (isa<AllocaInst>(currentInst)) {
    // do nothing
    source = false;

  }
  else if (CallInst * const callInst = dyn_cast<CallInst>(currentInst)) {
    // do nothing, but only for now!!!!!!
    ;
    if (isLLVMFunction(callInst)) {
      source = false;
    }
    else {
      source = true; // was false
    }
  }
  else if (isa<ICmpInst>(currentInst)) {
    // do nothing
  }
  else if (isa<BranchInst>(currentInst)) {
    // do nothing
    source = false;
  }
  else if (isa<LoadInst>(currentInst)) {
    // do nothing, taking care of when processing StoreInst
    source = false;
  }
  else if (isa<ReturnInst>(currentInst)) {
    // printing pop rule instead
  }
  else if (isa<SExtInst>(currentInst)) {
    // do nothing
    source = false;
  }
  else if (isa<BitCastInst>(currentInst) ||
	   isa<FCmpInst>(currentInst) ||
	   isa<FPToSIInst>(currentInst) ||
	   isa<GetElementPtrInst>(currentInst) ||
	   isa<IntToPtrInst>(currentInst) ||
	   isa<PtrToIntInst>(currentInst) ||
	   isa<SelectInst>(currentInst) ||
	   isa<SIToFPInst>(currentInst) ||
	   isa<TruncInst>(currentInst) ||
	   isa<ZExtInst>(currentInst)) {
    // do nothing
    source = false;
  }
  else {

    switch(currentInst->getOpcode()) {
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Sub:
    case Instruction::FSub:
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::FDiv:
    case Instruction::URem:
    case Instruction::SRem:
    case Instruction::FRem:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
    case Instruction::AShr:
    case Instruction::Shl:
    case Instruction::LShr: 
      // do nothing
      source = false;
      break;

    default:
      *outfile << "\t\t\t\t<unimplemented reason='" << currentInst->getOpcodeName() << " instruction'/>\n";
      break;
    }
  }
  
  return;
}


void Rules::printWeight(Instruction *currentInst, bool &source) {
  
  *outfile << "\t\t\t<Weight basis='identity'>\n";
  printInstructionWeight(currentInst, source);
  *outfile << "\t\t\t</Weight>\n"; 
  
  return;
}

// Returns the DILocation representing the source location
// Use tryHarder if you want to try and find the location from another instruction
// tryHarder is required for --locations to work
// The biggest reason for not always using tryHarder is compatibility with CIL/old tests
// Caller should use DILocation::Verify() to see if we actually got the location
DILocation Rules::getSource(Instruction *inst, bool tryharder) {

  DILocation ret(inst->getMetadata("dbg"));

  // If we don't have dbg metadata at this point, it could be because of a transformation.
  // For example reg2mem does not preserve metadata
  // Our best guess is that the last instruction in this basic block has the metadata
  // If that fails then give up.
  if (!ret.Verify() && tryharder) {
    inst  = inst->getParent()->getTerminator();
    ret = DILocation(inst->getMetadata("dbg"));
  }

  return ret;
}

void Rules::printSource(Instruction *currentInst) {

  DILocation loc = getSource(currentInst, false);

  if (loc.Verify()) {
    *outfile << "\t\t\t<source line='" << loc.getLineNumber() << "' file='" << loc.getFilename() << "'/>\n";
  }

  return;
}

// TODO: When 207 passes make sure this is picking up the name
void Rules::printPred(Instruction *currentInst) {
  const ICmpInst *icmp = dyn_cast<ICmpInst>(currentInst);
  if (!icmp) {
    return;
  }

  // TODO: This is the same test done in printBlockRules() for store
  // We get the name of the actual allocation by passing through load instructions
  // Are there other instructions that we need to handle?
  // If so, perhaps we should create a generic function that gets the name of an instruction
  Value *op1 = icmp->getOperand(0);
  Value *op2 = icmp->getOperand(1);
  if (const LoadInst *li = dyn_cast<LoadInst>(op1)) {
    op1 = li->getOperand(0);
  }
  if (const LoadInst *li = dyn_cast<LoadInst>(op2)) {
    op2 = li->getOperand(0);
  }

  // We use op1 and op2 to match the icmp syntax in IR reference
  // We must filter against globals locals[function] to avoid predicates that ref vars not in prologue
  StringRef nameOp1;
  StringRef nameOp2;
  Function *function = currentInst->getParent()->getParent();
  if (globals.find(op1) != globals.end() || locals[function].find(op1) != locals[function].end()) {
    nameOp1 = op1->getName();
  }
  if (globals.find(op2) != globals.end() || locals[function].find(op2) != locals[function].end()) {
    nameOp2 = op2->getName();
  }
  if (nameOp1.empty() && nameOp2.empty()) {
    return;
  }

  *outfile << "\t\t\t<pred";
  if (!nameOp1.empty()) {
    *outfile << " op1='" << nameOp1 << "'"; 
  }
  if (!nameOp2.empty()) {
    *outfile << " op2='" << nameOp2 << "'"; 
  }
  *outfile << "/>\n";
}


void Rules::printBlockRules(BasicBlock *block, Function *function) {

  Instruction *terminator = block->getTerminator();

  if (isa<ReturnInst>(terminator)) {
    printPopRule(block, function);
  }
  else if (CallInst *callInst = getCallInst(block)) {
    Function *callee = callInst->getCalledFunction();
    printPushRule(block, function, callee, callInst);
  }
  else {
    
    for(BasicBlock::iterator i = block->begin(), ie = block->end(); i != ie; i++) {
      Instruction *currentInst = i;
      Instruction *nextInst = getNextInstruction(currentInst);
      bool source = true;
      
      if (nextInst) {

	printIntraRuleOpen(InstructionID[currentInst], InstructionID[nextInst], function);
	printWeight(currentInst, source);
	if (source) {
	  printSource(currentInst);
	}
        printPred(currentInst);
	printRuleClose();
      }
      else {
	
	for(succ_iterator s = succ_begin(block), se = succ_end(block); s != se; s++) {
	  Instruction *nextInst = &(s->front());

	  printIntraRuleOpen(InstructionID[currentInst], InstructionID[nextInst], function);
	  printWeight(currentInst, source);
	  if (source) {
	    printSource(currentInst);
	  }
	  printRuleClose();
	}
      }
    } 
  }

  return;
}


bool Rules::isLLVMFunction(CallInst *callInst) {
  Function* callee = callInst->getCalledFunction();
    
  if (!callee) { // not available
    return true;
  }
  else if (callee->getName().str() == "llvm.dbg.declare") {
    return true;
  }
  return false;
}


// Returns true if the block contains a callInst
CallInst* Rules::getCallInst(BasicBlock* block) {
  
  for(BasicBlock::iterator i = block->begin(), ie = block->end(); i != ie; i++) {
    if (CallInst * const callInst = dyn_cast<CallInst>(i)) {
      if (!isLLVMFunction(callInst)) {
	Function *callee = callInst->getCalledFunction();
	if (callee && !callee->isDeclaration())
	  return callInst;
      }
    }
  }
  return NULL;

}


// Returns the next instruction in the current block or NULL if terminator 
Instruction* Rules::getNextInstruction(Instruction* targetInst)
{
  if (targetInst->isTerminator())
    return NULL;
  
  BasicBlock::iterator iterator(targetInst);
  ++iterator;
  return iterator;
}



// We don't modify the program, so we preserve all analyses
void Rules::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<CollectVariables>();
  AU.addRequired<FilterVariables>();
  AU.addRequired<PrintPrologue>();
  return;
}


char Rules::ID = 0;
static const RegisterPass<Rules> registration("rules", "Prints WPDS XML rules");
