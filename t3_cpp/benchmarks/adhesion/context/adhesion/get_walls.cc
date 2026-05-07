// ~/~ begin <<adhesion_example.md#src/adhesion/get_walls.cc>>[init]
#include "adhesion.hh"
#include "power_diagram.hh"

Mesh<Point, double> Adhesion::get_walls(
    double threshold) const
{
  // FUNCTION_ID: adhesion_func004 - START
  return power_diagram_faces(rt, threshold);
  // FUNCTION_ID: adhesion_func004 - END
}

Mesh<Point, double> Adhesion::get_filaments(
    double threshold) const
{
  // FUNCTION_ID: adhesion_func005 - START
  return power_diagram_edges(rt, threshold);
  // FUNCTION_ID: adhesion_func005 - END
}
// ~/~ end