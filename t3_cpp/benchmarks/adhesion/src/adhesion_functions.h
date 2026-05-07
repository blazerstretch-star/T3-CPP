#pragma once
#include "cgal_base.hh"
#include "mesh.hh"
#include <array>
#include <vector>
#include <map>
#include <cstdint>

namespace Adhesion {

enum NodeType : uint32_t {
  VOID, KURTOPARABOLIC, WALL, FILAMENT, CLUSTER, UNDEFINED_NODE_TYPE
};

struct Node {
  std::array<double, 3> position;
  std::array<double, 3> velocity;
  double    mass;
  NodeType  node_type;
};

// Function declarations
int edge_count(RT const &rt, RT::Cell_handle h, double threshold);
Vector calculate_velocity(RT const &rt, RT::Cell_handle c, double time);
std::vector<Node> extract_nodes(RT const &rt, double threshold, double time);
Mesh<Point, double> extract_walls(RT const &rt, double threshold);
Mesh<Point, double> extract_filaments(RT const &rt, double threshold);
Mesh<Point, double> power_diagram_faces(RT const &rt, double threshold);
Mesh<Point, double> power_diagram_edges(RT const &rt, double threshold);

}
