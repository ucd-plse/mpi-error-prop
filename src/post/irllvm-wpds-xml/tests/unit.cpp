#include "unit.hpp"
#include "../FlowGraph.hpp"

using namespace std;
using namespace ep;

using ::testing::_;
using ::testing::Return;
using ::testing::ContainerEq;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(FlowGraphTest, AddReturnValue) {
  FlowGraph FG;

  FlowGraph::add_t added = FG.add(FlowVertex("test.stack", Location(), nullptr));
  flow_vertex_t new_v_desc = get<0>(added);
  FlowVertex new_v = FG.G[new_v_desc];
  ASSERT_EQ(new_v.stack, "test.stack");
};

TEST(MockTest, NamesMock) {
  MockNames names;
  vn_t mock_vn = make_shared<VarName>("mock.0", VarScope::GLOBAL, VarType::FUNCTION);
  EXPECT_CALL(names, getVarName(_))
    .Times(1)
    .WillOnce(Return(mock_vn));

  vn_t vn = names.getVarName(nullptr);
  ASSERT_NE(vn, nullptr);

  ASSERT_EQ(vn->name(), "mock.0");
};



