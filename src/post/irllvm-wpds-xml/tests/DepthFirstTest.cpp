#include "DepthFirstTest.hpp"

using namespace std;
using namespace ep;

using ::testing::_;
using ::testing::Return;
using ::testing::ContainerEq;

TEST_F(DepthFirstTest, FromRoot) {
  StackVisitor sv;
  DepthFirstVisitor<_FlowGraph> visitor(sv);

  visitor.visit(a_vtx, FG.G);
  set<string> expected_a = {"a", "b", "c", "d", "g", "h", "i"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_a));
  sv.clear();

  visitor.visit(e_vtx, FG.G);
  set<string> expected_e = {"e", "d", "f"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_e));
};

TEST_F(DepthFirstTest, Between) {
  StackVisitor sv;
  DepthFirstVisitor<_FlowGraph> visitor(sv);

  visitor.visit(a_vtx, c_vtx, FG.G);
  set<string> expected_ac = {"a", "b", "c"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ac));

  sv.clear();
  visitor.visit(e_vtx, d_vtx, FG.G);
  set<string> expected_ed = {"e", "d"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ed));

  sv.clear();
  visitor.visit(a_vtx, e_vtx, FG.G);
  set<string> expected_ae;
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ae));

  sv.clear();
  visitor.visit(a_vtx, d_vtx, FG.G);
  set<string> expected_ad = {"a", "b", "d"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ad));

  sv.clear();
  visitor.visit(a_vtx, b_vtx, FG.G);
  set<string> expected_ab = {"a", "b"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ab));

  sv.clear();
  visitor.visit(a_vtx, a_vtx, FG.G);
  set<string> expected_aa = {"a"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_aa));
};

TEST_F(DepthFirstTest, BetweenWithFollowEdge) {
  StackVisitorIntra sv;
  DepthFirstVisitor<_FlowGraph> visitor(sv);

  visitor.visit(a_vtx, d_vtx, FG.G);
  set<string> expected_ad;
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ad));

  sv.clear();
  visitor.visit(a_vtx, c_vtx, FG.G);
  set<string> expected_ac = {"a", "b", "c"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ac));
};

TEST_F(DepthFirstTest, BetweenAllPaths) {
  StackVisitorIntra sv;
  DepthFirstVisitor<_FlowGraph> visitor(sv);

  visitor.visit(a_vtx, i_vtx, FG.G);
  set<string> expected_ai = {"a", "b", "c", "g", "h", "i"};
  ASSERT_THAT(sv.get_visited(), ContainerEq(expected_ai));
};
