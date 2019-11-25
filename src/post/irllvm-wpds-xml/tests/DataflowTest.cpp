#include "DataflowTest.hpp"

using namespace std;

using ::testing::ContainerEq;

TEST_F(DataFlowTest, trivial_assignment) {
  FRule main0to1("main.0", "main.1");
  FRule main1to2("main.1", "main.2");

  main0to1.addAssignment(eio, x_var, false);
  main1to2.addAssignment(enomem, y_var, false);
    
  vector<FRule> rules;
  rules.push_back(main0to1);
  rules.push_back(main1to2);

  var_map expected;
  set<VarName> x_values = { eio };
  set<VarName> y_values = { enomem };
  expected[x_var] = x_values;
  expected[y_var] = y_values;
  
  DataflowResult res = solver.solve(rules);  
  
  ASSERT_THAT(res.getVars("main.2"), ContainerEq(expected));
};

TEST_F(DataFlowTest, overwrite) {
  FRule main0to1("main.0", "main.1");
  FRule main1to2("main.1", "main.2");

  main0to1.addAssignment(eio, x_var, false);
  main1to2.addAssignment(enomem, x_var, false);

  vector<FRule> rules = { main0to1, main1to2 };
  DataflowResult res = solver.solve(rules);

  var_map expected;
  set<VarName> x_values = { enomem };
  expected[x_var] = x_values;
  
  ASSERT_THAT(res.getVars("main.2"), ContainerEq(expected));
};

TEST_F(DataFlowTest, may) {
  FRule main0to1("main.0", "main.1");
  FRule main0to2("main.0", "main.2");
  FRule main1to3("main.1", "main.3");
  FRule main2to3("main.2", "main.3");

  main0to1.addAssignment(eio, x_var, false);
  main0to2.addAssignment(enomem, x_var, false);
  
  vector<FRule> rules = { main0to1, main0to2, main1to3, main2to3 };  
  DataflowResult res = solver.solve(rules);

  var_map expected;
  set<VarName> x_values = { eio, enomem };
  expected[x_var] = x_values;  

  ASSERT_THAT(res.getVars("main.3"), ContainerEq(expected));
};

TEST_F(DataFlowTest, return_value) {
  FRule main0to1("main.0", "main.1");
  FRule main1foo("main.1", "foo.0", "main.2");
  FRule main2ret("main.2");
  FRule foo0to1("foo.0", "foo.1");
  FRule foo1ret("foo.1");

  foo0to1.addAssignment(eio, x_var, false);

  vector<FRule> rules = { main0to1, main1foo, main2ret, foo0to1, foo1ret }; 
  DataflowResult res = solver.solve(rules);

  var_map expected;
  set<VarName> x_values = { eio };
  expected[x_var] = x_values;  

  ASSERT_THAT(res.getVars("main.2"), ContainerEq(expected));
};

TEST_F(DataFlowTest, extend) {
  FRule main0to1("main.0", "main.1");
  FRule main1to2("main.1", "main.2");

  main0to1.addAssignment(eio, x_var, false);
  main1to2.addAssignment(x_var, y_var, false);

  vector<FRule> rules = { main0to1, main1to2 };
  DataflowResult res = solver.solve(rules);

  var_map expected;
  set<VarName> x_values = { eio };
  set<VarName> y_values = { eio };
  expected[x_var] = x_values;
  expected[y_var] = y_values;

  ASSERT_THAT(res.getVars("main.2"), ContainerEq(expected));
};


// Tests that per-instance name mangling is working correctly.
TEST_F(DataFlowTest, tentative) {
   FRule main0to1("main.0", "main.1");
   FRule main1to2("main.1", "main.2");

   main0to1.addAssignment(tentative_eio, x_var, false);
   main0to1.location = Location("test.c", 4);
  
   vector<FRule> rules = { main0to1, main1to2 };
   DataflowResult res = solver.solve(rules);

   var_map expected;
   VarName x_rewrite = x_var;
   x_rewrite.setName("TENTATIVE_EIO!test.c:4");
   set<VarName> x_values = { x_rewrite };
   expected[x_var] = x_values;

   ASSERT_THAT(res.getVars("main.2"), ContainerEq(expected));
};
