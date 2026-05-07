#include "adhesion_functions.h"
#include <limits>
#include <CGAL/Cartesian_d.h>
#include <CGAL/Kernel_d/Hyperplane_d.h>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


typedef CGAL::Cartesian_d<double>    LiftedK;
typedef CGAL::Point_d<LiftedK>       LiftedPoint;
typedef CGAL::Hyperplane_d<LiftedK>  HyperPlane;

namespace Adhesion {

constexpr double infinity = std::numeric_limits<double>::infinity();

inline LiftedPoint lifted_point(double x, double y, double z, double w) {
  double p[4] = { x, y, z, w };
  return LiftedPoint(4, p, p + 4);
}

inline NodeType type_from_edge_count(int n) {
  switch (n) {
    case 0: return VOID;
    case 1:
    case 2: return KURTOPARABOLIC;
    case 3:
    case 4: return WALL;
    case 5: return FILAMENT;
    case 6: return CLUSTER;
  }
  return UNDEFINED_NODE_TYPE;
}

inline bool is_small_edge(RT const &rt, RT::Edge const &e, double threshold) {
  double l = rt.segment(e).squared_length();
  return l < threshold;
}

inline bool is_large_facet(RT const &rt, RT::Facet const &f, double threshold) {
  RT::Cell_handle c = f.first;
  unsigned        k = f.second;

  for (unsigned i = 0; i < 4; ++i) {
    if (i == k) continue;
    for (unsigned j = 0; j < i; ++j) {
      if (j == k) continue;
      if (rt.segment(c, i, j).squared_length() < threshold) {
        return false;
      }
    }
  }
  return true;
}

int edge_count(RT const &rt, RT::Cell_handle h, double threshold) {
  // FUNCTION_ID: adhesion_func001 - START
  int count = 0;
  for (unsigned i = 1; i < 4; ++i) {
    for (unsigned j = 0; j < i; ++j) {
      auto segment = rt.segment(h, i, j);
      double l = segment.squared_length();
      if (l > threshold) {
        ++count;
      }
    }
  }
  return count;
  // FUNCTION_ID: adhesion_func001 - END
}

Vector calculate_velocity(RT const &rt, RT::Cell_handle c, double time) {
  // FUNCTION_ID: adhesion_func002 - START
  LiftedPoint points[4];
  for (unsigned i = 0; i < 4; ++i) {
    Weighted_point wp = rt.point(c, i);
    auto p = wp.point();
    auto w = wp.weight();
    points[i] = lifted_point(p.x(), p.y(), p.z(), w);
  }
  
  auto guide = lifted_point(0, 0, 0, -infinity);
  HyperPlane h(points, points + 4, guide, CGAL::ON_NEGATIVE_SIDE);
  
  auto normal = h.orthogonal_vector();
  auto v  = normal / (2 * time * normal[3]);
  return Vector(v[0], v[1], v[2]);
  // FUNCTION_ID: adhesion_func002 - END
}

std::vector<Node> extract_nodes(RT const &rt, double threshold, double time) {
  // FUNCTION_ID: adhesion_func003 - START
  std::vector<Node> result;
  for (auto c  = rt.finite_cells_begin(); c != rt.finite_cells_end(); ++c) {
    std::array<double, 3>  ps, vs;
    Point    p = rt.dual(c);
    Vector   v = calculate_velocity(rt, c, time);
    for (unsigned i = 0; i < 3; ++i) {
      ps[i] = p[i];
      vs[i] = v[i];
    }
    NodeType n = type_from_edge_count(edge_count(rt, c, threshold));
    double   m = rt.tetrahedron(c).volume();
    result.push_back(Node({ps, vs, m, n}));
  }
  return result;
  // FUNCTION_ID: adhesion_func003 - END
}

Mesh<Point, double> extract_walls(RT const &rt, double threshold) {
  // FUNCTION_ID: adhesion_func004 - START
  return power_diagram_faces(rt, threshold);
  // FUNCTION_ID: adhesion_func004 - END
}

Mesh<Point, double> extract_filaments(RT const &rt, double threshold) {
  // FUNCTION_ID: adhesion_func005 - START
  return power_diagram_edges(rt, threshold);
  // FUNCTION_ID: adhesion_func005 - END
}

Mesh<Point, double> power_diagram_faces(RT const &rt, double threshold) {
  // FUNCTION_ID: adhesion_func006 - START
  Mesh<Point, double> mesh;
  std::map<RT::Cell_handle, unsigned> cell_index;
  
  auto get_dual_vertex = [&rt, &cell_index, &mesh] (RT::Cell_handle const &h) -> unsigned {
    if (cell_index.count(h) == 0) {
      cell_index[h] = mesh.vertices.size();
      mesh.vertices.push_back(rt.dual(h));
    }
    return cell_index[h];
  };
  
  for (auto e = rt.finite_edges_begin(); e != rt.finite_edges_end(); ++e) {
    if (is_small_edge(rt, *e, threshold)) {
      continue;
    }
    std::vector<unsigned> polygon;
    auto first = rt.incident_cells(*e), c = first;
    bool ok = true;
    do {
      if (rt.is_infinite(++c)) {
          ok = false;
          break;
      }
      polygon.push_back(get_dual_vertex(c));
    } while (c != first);
    
    if (ok) {
      double l = sqrt(rt.segment(*e).squared_length());
      mesh.push_back(polygon, l);
    }
  }
  return mesh;
  // FUNCTION_ID: adhesion_func006 - END
}

Mesh<Point, double> power_diagram_edges(RT const &rt, double threshold) {
  // FUNCTION_ID: adhesion_func007 - START
  Mesh<Point, double> mesh;
  std::map<RT::Cell_handle, unsigned> cell_index;
  
  auto get_dual_vertex = [&rt, &cell_index, &mesh] (RT::Cell_handle const &h) -> unsigned {
    if (cell_index.count(h) == 0) {
      cell_index[h] = mesh.vertices.size();
      mesh.vertices.push_back(rt.dual(h));
    }
    return cell_index[h];
  };
  
  for (auto f = rt.finite_facets_begin(); f != rt.finite_facets_end(); ++f) {
    if (!is_large_facet(rt, *f, threshold)) {
      continue;
    }
    double area = sqrt(rt.triangle(*f).squared_area());
    auto mirror = rt.mirror_facet(*f);
    if (rt.is_infinite(f->first) || rt.is_infinite(mirror.first)) {
      continue;
    }
    std::vector<unsigned> polygon;
    polygon.push_back(get_dual_vertex(f->first));
    polygon.push_back(get_dual_vertex(mirror.first));
    mesh.push_back(polygon, area);
  }
  return mesh;
  // FUNCTION_ID: adhesion_func007 - END
}

}
