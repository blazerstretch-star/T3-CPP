#pragma once
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_3.h>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using RT = CGAL::Regular_triangulation_3<K>;

typedef K::Vector_3           Vector;
typedef RT::Bare_point        Point;
typedef RT::Edge              Edge;
typedef RT::Weighted_point    Weighted_point;
typedef RT::Segment           Segment;
typedef RT::Tetrahedron       Tetrahedron;
