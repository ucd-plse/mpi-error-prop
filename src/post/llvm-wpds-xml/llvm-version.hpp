#ifndef LLVM_VERSION_GUARD
#define LLVM_VERSION_GUARD

#include <llvm/Config/config.h>


#define LLVM_VERSION (LLVM_VERSION_MAJOR * 10000 \
                   + LLVM_VERSION_MINOR * 100 \
                   + LLVM_VERSION_PATCH)


#endif	// !LLVM_VERSION_GUARD
