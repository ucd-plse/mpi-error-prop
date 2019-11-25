#ifndef NAMEGRAPH_H
#define NAMEGRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include "RulesPrinter.hpp"

struct NameVertex;
struct NameEdge;

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS,
  NameVertex, NameEdge> _NameGraph;

typedef boost::graph_traits<_NameGraph>::vertex_descriptor name_vertex_t;
typedef boost::graph_traits<_NameGraph>::edge_descriptor   name_edge_t;

typedef boost::graph_traits<_NameGraph>::edge_iterator   name_edge_iter;
typedef boost::graph_traits<_NameGraph>::vertex_iterator name_vertex_iter;

struct NameVertex {
  VarName vn;
  bool may_contain_ec;
};

struct NameEdge {
};

// Prunes away assignments that cannot have error codes (reachability)
template<class T>
class FilterVisitor : public boost::default_bfs_visitor {
public:
  FilterVisitor(set<string> &out_locals,
                set<string> &out_globals,
                set<name_vertex_t> &out_reachable,
                bool save_vertices) :
    local_names(out_locals),
    global_names(out_globals),
    reachable(out_reachable),
    save_vertices(save_vertices) {}


  void discover_vertex(name_vertex_t v, const T &G) {
    VarName vn = G[v].vn;
    if (vn.scope == VarScope::LOCAL) {
      local_names.insert(vn.name());
    } else if (vn.scope == VarScope::GLOBAL) {
      global_names.insert(vn.name());
    }

    if (save_vertices) {
      reachable.insert(v);
    }
  }

private:
  set<string> &local_names;
  set<string> &global_names;
  set<name_vertex_t> &reachable;
  bool save_vertices = true;
};

class NameGraph {
public:
  NameGraph(vector<FRule> &rules) : rules(rules) {

    for (FRule &R : rules) {
      // Create two vertices for source/target of assignment
      for (Assignment a : R.assignments) {
        name_vertex_t vertex_from; 
        name_vertex_t vertex_to; 
        if (name_vertex_map.find(a.from) != name_vertex_map.end()) {
          vertex_from = name_vertex_map[a.from];
        } else {
          vertex_from = boost::add_vertex(G);
          G[vertex_from].vn = a.from;
          name_vertex_map[a.from] = vertex_from; 
        }
        if (name_vertex_map.find(a.to) != name_vertex_map.end()) {
          vertex_to = name_vertex_map[a.to];
        } else {
          vertex_to = boost::add_vertex(G);
          G[vertex_to].vn = a.to;
          name_vertex_map[a.to] = vertex_to;
        }

        name_edge_t edge;
        bool b;
        tie(edge, b) = boost::add_edge(vertex_from, vertex_to, G);
        if (!b) abort();

        if (a.from.type == VarType::EC && a.from.name() != "OK") {
          ec_sources.insert(vertex_from);
        }
      }
    }
  }

  void filter() {
    set<name_vertex_t> new_sources;

    FilterVisitor<_NameGraph> vis(local_names, global_names, new_sources, true);
    for (name_vertex_t ec_source : ec_sources) {
      boost::breadth_first_search(G, ec_source, boost::visitor(vis));
    }

    for (FRule &r : rules) {
      std::vector<Assignment>::iterator it = r.assignments.begin();

      if (local_names.find(r.pred_op1) == local_names.end() &&
          global_names.find(r.pred_op1) == global_names.end()) {
        r.pred_op1 = "";
      }
      if (local_names.find(r.pred_op2) == local_names.end() &&
          global_names.find(r.pred_op2) == global_names.end()) {
        r.pred_op2 = "";
      }
      if (local_names.find(r.return_value.name()) == local_names.end() &&
          global_names.find(r.return_value.name()) == global_names.end() &&
          r.return_value.type != VarType::EC) {     // keep direct assignment of ret vars, test248
        r.return_value = VarName();
      }

      while (it != r.assignments.end()) {
        if (local_names.find(it->to.name()) == local_names.end() &&
            global_names.find(it->to.name()) == global_names.end()) {

          it = r.assignments.erase(it);
        } else {
          ++it;
        }

      } // each assignment
    } // each rule
  } //filter

private:
  _NameGraph G;
  map<VarName, name_vertex_t> name_vertex_map;
  set<name_vertex_t> ec_sources;
  set<string> local_names;
  set<string> global_names;
  vector<FRule> &rules;
};

#endif
