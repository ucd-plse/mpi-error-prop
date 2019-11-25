#include "Utility.hpp"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace ep {

Location getSource(Instruction *I) {
  if (I == nullptr) {
    errs() << "getSource requested for nullptr\n";
    abort();
  }

  Location location;
  if (MDNode *node = I->getMetadata("dbg")) {
    DILocation loc(node);
    location = Location(loc.getFilename(), loc.getLineNumber());
  }
  return location;
}

}
