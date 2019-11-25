#ifndef DATAFLOWTEST_HPP
#define DATAFLOWTEST_HPP

#include "unit.hpp"
#include "../DataflowWali.hpp"
#include "../Rule.hpp"
#include "../VarName.hpp"

class DataFlowTest : public ::testing::Test {
protected:
  DataFlowTest() : 
    tentative_eio("TENTATIVE_EIO"), eio("EIO"), 
    tentative_enomem("TENTATIVE_ENOMEM"), enomem("ENOMEM"), 
    x_var("x"), y_var("y") {}

  DataflowWali solver;
  ErrorName tentative_eio, eio, tentative_enomem, enomem;
  IntName x_var, y_var;
};

#endif
