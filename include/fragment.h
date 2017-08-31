//
// Created by M. Engler on 06/12/16.
//

#ifndef MOGLI_FRAGMENT_H
#define MOGLI_FRAGMENT_H


#include <deque>
#include <boost/ptr_container/ptr_vector.hpp>
#include <cstring>
#include "product.h"

namespace mogli {

  typedef typename lemon::FilterNodes<const Graph, const NodeToBoolMap> FilterNodes;
  typedef typename FilterNodes::NodeIt FilteredNodeIt;

  class Fragment : public Molecule {

  private:
    
    typedef std::deque<Node> NodeDeque;
    
    NodeToBoolMap _is_core;
    int _shell_size;
    int _core_node_count;

  public:

    Fragment() :
        Molecule(),
        _is_core(_g, false),
        _shell_size(0),
        _core_node_count(0) {}

    Fragment(const Product &product, const NodeVector &clique, IntToIntMap &g_to_mol1, IntToIntMap &g_to_mol2) :
        Molecule(product.get_mol1().get_perdiodic_table()),
        _is_core(_g, false),
        _shell_size(product.get_shell()),
        _core_node_count(0) {

      const Molecule &mol1 = product.get_mol1();
      const Molecule &mol2 = product.get_mol2();

      NodeToNodeMap shell_g_to_mol(_g);
      NodeToNodeMap shell_mol_to_g(mol1.get_graph());
      NodeToNodeMap g_to_product(_g);
      NodeToBoolMap mol1_core_nodes(mol1.get_graph(), false);
      NodeToIntMap shell_min_depth(_g, 0);

      int core_nodes = 0;
      for (NodeVector::const_iterator it = clique.begin(), end = clique.end(); it != end; ++it) {
        Node u = product.get_mol1_node(*it);
        Node uv = add_atom(mol1.get_color(u));

        g_to_product[uv] = *it;
        g_to_mol1[_node_to_id[uv]] = mol1.get_id(u);
        g_to_mol2[_node_to_id[uv]] = mol2.get_id(product.get_mol2_node(*it));
        _is_core[uv] = true;
        mol1_core_nodes[u] = true;
        _core_node_count++;

        const NodePairVector &reductions = product.get_reductions(*it);
        for (NodePairVector::const_iterator it2 = reductions.begin(), end2 = reductions.end(); it2 != end2; ++it2) {
          Node _u = it2->first;
          Node _uv = add_atom(mol1.get_color(_u));

          g_to_product[_uv] = *it;
          g_to_mol1[_node_to_id[_uv]] = mol1.get_id(_u);
          g_to_mol2[_node_to_id[_uv]] = mol2.get_id(it2->second);
          _is_core[_uv] = true;
          mol1_core_nodes[_u] = true;
          _core_node_count++;
        }
      }

      IntSet shell_ids;
      for (NodeIt u(_g); u != lemon::INVALID; ++u) {
        bfs_shell(mol1, mol1.get_node_by_id(g_to_mol1.at(_node_to_id[u])), shell_g_to_mol, shell_mol_to_g,
                  mol1_core_nodes, shell_ids, shell_min_depth,
                  product.get_mol1_canon(g_to_product[u]), product.get_mol2_canon(g_to_product[u]), g_to_mol1, g_to_mol2);
      }

      lemon::ArcLookUp<Graph> arcLookUp(mol1.get_graph());

      for (NodeIt uv1(_g); uv1 != lemon::INVALID; ++uv1) {
        for (NodeIt uv2 = uv1; uv2 != lemon::INVALID; ++uv2) {
          if (uv1 == uv2)
            continue;
          // if both nodes are shell nodes with maximal depth, there can't be an edge between them
          if (!_is_core[uv1] && !_is_core[uv2] &&
              shell_min_depth[uv1] == _shell_size && shell_min_depth[uv2] == _shell_size)
            continue;
          Node u1 = _is_core[uv1] ? mol1.get_node_by_id(g_to_mol1.at(_node_to_id[uv1])) : shell_g_to_mol[uv1];
          Node u2 = _is_core[uv2] ? mol1.get_node_by_id(g_to_mol1.at(_node_to_id[uv2])) : shell_g_to_mol[uv2];
          bool is_edge = arcLookUp(u1, u2) != lemon::INVALID;
          if (is_edge) {
            _g.addEdge(uv1, uv2);
          }
        }
      }

    }

    int get_core_atom_count() const {
      return _core_node_count;
    }

    bool is_core(const Node node) const {
      return _is_core[node];
    }

    void set_core(const Node &u, const bool core) {
      if (_is_core[u] && !core) {
        _core_node_count--;
      } else if (!_is_core[u] && core) {
        _core_node_count++;
      }
      _is_core[u] = core;
    }

    int get_shell_size() const {
      return _shell_size;
    }

    void set_shell_size(int shell_size) {
      _shell_size = shell_size;
    }

    const std::string print_dot() const {
      std::stringstream buffer;
      print_dot(buffer);
      return buffer.str();
    }

    const void print_dot(std::ostream &out) const {
      // header
      out << "graph G {" << std::endl
          << "\toverlap=scale" << std::endl;

      // nodes
      for (NodeIt v(_g); v != lemon::INVALID; ++v) {
        out << "\t" << _g.id(v);
        if (_is_core[v]) {
          out << "[style=\"filled,bold\",fillcolor=" << _perdiodic_table.get_color(_colors[v]);
        } else {
          out << "[style=\"filled,dashed\",fillcolor=" << _perdiodic_table.get_color(_colors[v]);
        }
        out << ",label=\"" << _node_to_id[v] << " (" << _perdiodic_table.get_element(_colors[v]) << ")\"]" << std::endl;
      }

      // edges
      for (EdgeIt e(_g); e != lemon::INVALID; ++e) {
        out << _g.id(_g.u(e)) << " -- " << _g.id(_g.v(e)) << std::endl;
      }

      out << "}" << std::endl;
    }

  private:
    
    void bfs_shell(const Molecule &mol, const Node &v, NodeToNodeMap &shell_g_to_mol, NodeToNodeMap &shell_mol_to_g,
                   NodeToBoolMap &core_nodes, IntSet &shell_ids, NodeToIntMap &shell_min_depth,
                   const Canonization &canon1, const Canonization &canon2, IntToIntMap &g_to_mol1, IntToIntMap &g_to_mol2) {
      NodeToIntMap depth(mol.get_graph(), 0);
      NodeToBoolMap visited(mol.get_graph(), false);
      NodeDeque queue;

      queue.push_back(v);
      visited[v] = true;
      while (queue.size() > 0) {
        Node &current = queue.front();
        // if current node is not a core
        if (!core_nodes[current]) {
          // have we seen the node before?
          if (shell_ids.count(mol.get_id(current)) == 0) {
            Node uv = add_atom(mol.get_color(current));
            shell_g_to_mol[uv] = current;
            shell_mol_to_g[current] = uv;
            int id = mol.get_id(current);
            shell_ids.insert(id);

            const ShortVector &order1 = canon1.get_node_order();
            const ShortVector &order2 = canon2.get_node_order();
            for (int i = 0; i < order1.size(); ++i) {
              if (order1[i] == id) {
                g_to_mol1[_node_to_id[uv]] = id;
                g_to_mol2[_node_to_id[uv]] = order2[i];
              }
            }
          }
          // minimal distance of this shell node to the nearest core node
          if (shell_min_depth[shell_mol_to_g[current]] > 0) {
            shell_min_depth[shell_mol_to_g[current]] = std::min(shell_min_depth[shell_mol_to_g[current]], depth[current]);
          } else {
            shell_min_depth[shell_mol_to_g[current]] = depth[current];
          }

        }

        // breadth-first-search
        if (depth[current] < _shell_size) {
          for (IncEdgeIt e = mol.get_inc_edge_iter(current); e != lemon::INVALID; ++e) {
            Node w = mol.get_opposite_node(current, e);
            if (!visited[w]) {
              visited[w] = true;
              depth[w] = depth[current]+1;
              queue.push_back(w);
            }
          }
        }
        queue.pop_front();
      }
    }

  };

}

#endif //MOGLI_FRAGMENT_H
