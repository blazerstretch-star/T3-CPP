// ~/~ begin <<adhesion_example.md#src/adhesion/edge_count.cc>>[init]
#include "adhesion.hh"

int Adhesion::edge_count(RT::Cell_handle h, double threshold) const
{
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
// ~/~ end