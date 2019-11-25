#ifndef DEPTHFIRSTTEST_HPP
#define DEPTHFIRSTTEST_HPP

#include "unit.hpp"
#include "../FlowGraph.hpp"

// Test fixture for DFS tests
class DepthFirstTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    FlowVertex a = FlowVertex("a", Location(), nullptr);
    FlowVertex b = FlowVertex("b", Location(), nullptr);
    FlowVertex c = FlowVertex("c", Location(), nullptr);
    FlowVertex d = FlowVertex("d", Location(), nullptr);
    FlowVertex e = FlowVertex("e", Location(), nullptr);
    FlowVertex f = FlowVertex("f", Location(), nullptr);
    FlowVertex g = FlowVertex("g", Location(), nullptr);
    FlowVertex h = FlowVertex("h", Location(), nullptr);
    FlowVertex i = FlowVertex("i", Location(), nullptr);
  
    a_vtx = get<0>(FG.add(a));
    b_vtx = get<1>(FG.add(a, b));
    c_vtx = get<1>(FG.add(b, c));

    FlowGraph::add_t bd_added = FG.add(b, d);
    d_vtx = get<1>(bd_added);
    flow_edge_t bd_edge = get<3>(bd_added);
    FG.G[bd_edge].call = true;

    e_vtx = get<0>(FG.add(e, d));
    f_vtx = get<1>(FG.add(e, f));
    FG.add(f, e);

    g_vtx = get<1>(FG.add(c, g));
    h_vtx = get<1>(FG.add(c, h));
    i_vtx = get<1>(FG.add(g, i));
    FG.add(h, i);
  };

  FlowGraph FG;
  flow_vertex_t a_vtx, b_vtx, c_vtx, d_vtx, e_vtx, f_vtx, g_vtx, h_vtx, i_vtx;
};

class StackVisitor : public ep::DFSVisitorInterface<_FlowGraph> {
public:
  virtual void discover_vertex(flow_vertex_t vtx, _FlowGraph &G) {
    visited.insert(G[vtx].stack);
  }
  std::set<std::string> get_visited() const {
    return visited;
  }

  void clear() {
    visited.clear();
  }

  virtual bool follow_edge(const flow_edge_t edge, const _FlowGraph &G) const {
    (void) edge; (void) G;
    return true;
  }

private:
  std::set<std::string> visited;
};

class StackVisitorIntra : public StackVisitor {
  virtual bool follow_edge(const flow_edge_t edge, const _FlowGraph &G) const {
    bool may_ret = boost::get(&FlowEdge::may_ret, G, edge);
    bool call    = boost::get(&FlowEdge::call, G, edge);
    
    if (may_ret || call) {      
      return false;
    }
    return true;
  }
};

#endif
