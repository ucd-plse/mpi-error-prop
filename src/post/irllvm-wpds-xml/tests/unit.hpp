#ifndef UNIT_HPP
#define UNIT_HPP

#include "../Names.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MockNames : public NamesPass {
public:
  MOCK_METHOD1(getVarName, vn_t(llvm::Value *V));
};

#endif
