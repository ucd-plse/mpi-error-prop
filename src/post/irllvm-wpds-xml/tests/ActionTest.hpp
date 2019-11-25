#ifndef ACTIONTEST_HPP
#define ACTIONTEST_HPP

#include "unit.hpp"
#include "../FlowGraph.hpp"
#include "../Item.hpp"
#include "../TraceVisitors.hpp"

class MockActionMapper : public StandardActionMapper {
public:
  MockActionMapper(NamesPass *names) : StandardActionMapper(names, "test") {}
  MOCK_METHOD2(map, std::vector<Item>(const llvm::Instruction *I, const Location &loc));
};

class ActionTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    FlowVertex a = FlowVertex("a", Location(), nullptr);
    FlowVertex b = FlowVertex("b", Location(), nullptr);
    FlowVertex c = FlowVertex("c", Location(), nullptr);
    FlowVertex d = FlowVertex("d", Location(), nullptr);
    FlowVertex e = FlowVertex("e", Location(), nullptr);
    FlowVertex f = FlowVertex("f", Location(), nullptr);

    a_vtx = get<0>(FG.add(a));
    b_vtx = get<1>(FG.add(a, b));
    c_vtx = get<1>(FG.add(b, c));
    d_vtx = get<1>(FG.add(c, d));
    e_vtx = get<0>(FG.add(e, d));
    f_vtx = get<1>(FG.add(d, f));
    FG.add(c, e);
    FG.add(f, b);

    // Allows us to return different values from mock action mapper
    FG.G[a_vtx].I = (llvm::Instruction*) 1;
    FG.G[b_vtx].I = (llvm::Instruction*) 2;
    FG.G[c_vtx].I = (llvm::Instruction*) 3;
    FG.G[d_vtx].I = (llvm::Instruction*) 4;
    FG.G[e_vtx].I = (llvm::Instruction*) 5;
    FG.G[f_vtx].I = (llvm::Instruction*) 6;
    
    entry_vtx = a_vtx;
    handler_vtx = e_vtx;
  };

  FlowGraph FG;
  flow_vertex_t a_vtx, b_vtx, c_vtx, d_vtx, e_vtx, f_vtx;
  flow_vertex_t entry_vtx, handler_vtx;

  Item action_a = Item(Item::Type::LOAD, Location(), "a");
  Item action_b = Item(Item::Type::CALL, Location(), "b");
  Item action_e = Item(Item::Type::CALL, Location(), "e");
  Item action_f = Item(Item::Type::CALL, Location(), "f");
  vector<Item> actions_a = { action_a };
  vector<Item> actions_b = { action_b };
  vector<Item> actions_e = { action_e };
  vector<Item> actions_f = { action_f };
};

#endif

