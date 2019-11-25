#include "ActionTest.hpp"
#include "../VarName.hpp"
#include "../Traces.hpp"
#include "../TraceVisitors.hpp"

using namespace std;
using namespace ep;

using ::testing::_;
using ::testing::Return;
using ::testing::ContainerEq;

TEST_F(ActionTest, PreActions) {
  MockNames names;
  MockActionMapper mapper(&names);

  // 1 is dummy instruction pointer for a vertex
  EXPECT_CALL(mapper, map((llvm::Instruction*) 1, Location()))
    .WillOnce(Return(actions_a));
  EXPECT_CALL(mapper, map((llvm::Instruction*) 2, Location()))
    .WillOnce(Return(actions_b));
  EXPECT_CALL(mapper, map((llvm::Instruction*) 3, Location()));
  EXPECT_CALL(mapper, map((llvm::Instruction*) 5, Location()))
    .WillOnce(Return(actions_e));

  PreActionTrace trace("PreActionsTest.0");
  PreActionVisitor pav(mapper, trace.items);
  DepthFirstVisitor<_FlowGraph> visitor(pav);
  visitor.visit(entry_vtx, handler_vtx, FG.G);

  vector<Item> expected = { action_a, action_b, action_e } ;
  ASSERT_THAT(trace.items, ContainerEq(expected));
};

TEST_F(ActionTest, PostActions) {
  // PostActionVisitor requires pre-actions to be marked already
  MockNames names;
  MockActionMapper pre_mapper(&names);

  EXPECT_CALL(pre_mapper, map((llvm::Instruction*) 1, Location()))
    .WillOnce(Return(actions_a));
  EXPECT_CALL(pre_mapper, map((llvm::Instruction*) 2, Location()))
    .WillOnce(Return(actions_b));
  EXPECT_CALL(pre_mapper, map((llvm::Instruction*) 3, Location()));
  EXPECT_CALL(pre_mapper, map((llvm::Instruction*) 5, Location()))
    .WillOnce(Return(actions_e));

  PreActionTrace pre_trace("PostActionsTest.0");
  PreActionVisitor pre_vis(pre_mapper, pre_trace.items);
  DepthFirstVisitor<_FlowGraph> pre_dfs(pre_vis);
  pre_dfs.visit(entry_vtx, handler_vtx, FG.G);

  MockActionMapper post_mapper(&names);
  EXPECT_CALL(post_mapper, map((llvm::Instruction *) 5, Location()))
    .WillRepeatedly(Return(actions_e));
  EXPECT_CALL(post_mapper, map((llvm::Instruction *) 4, Location()));
  EXPECT_CALL(post_mapper, map((llvm::Instruction *) 6, Location()))
    .WillRepeatedly(Return(actions_f));

  PostActionTrace post_trace("PostActionsTest.0");
  PostActionVisitor post_vis(post_mapper, post_trace.items, pre_vis.get_discovered());
  DepthFirstVisitor<_FlowGraph> post_dfs(post_vis);
  post_dfs.visit(handler_vtx, FG.G);

  vector<Item> expected = { action_e, action_f };
  ASSERT_THAT(post_trace.items, ContainerEq(expected));
};

TEST_F(ActionTest, HandlerActions) {
  MockNames names;
  MockActionMapper mapper(&names);

  EXPECT_CALL(mapper, map((llvm::Instruction*) 5, Location()))
    .WillOnce(Return(actions_e));

  Trace trace("HandlerActionsTest.0"); 

  unordered_set<string> dummy_handler_set;
  vector<pair<string, string>> dummy_nesting_pairs;
  HandlerActionsVisitor handle_vis(mapper, trace.items, "d", dummy_handler_set, dummy_nesting_pairs); 

  DepthFirstVisitor<_FlowGraph> dfs_visitor(handle_vis);
  dfs_visitor.visit(handler_vtx, FG.G);

  vector<Item> expected = { action_e };
  ASSERT_THAT(trace.items, ContainerEq(expected));
};

// TODO: Prevent double-discovers in simple DFS
