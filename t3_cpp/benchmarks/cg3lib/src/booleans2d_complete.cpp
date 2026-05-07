#include "booleans2d.h"
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_set_2.h>
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


std::vector<PolygonWithHoles2d> difference2D(const Polygon2d& P1, const Polygon2d& P2) {
    // FUNCTION_ID: cg3lib_func021 - START
    std::vector<PolygonWithHoles2d> result;
    CGAL::difference(P1, P2, std::back_inserter(result));
    return result;
    // FUNCTION_ID: cg3lib_func021 - END
}

std::vector<PolygonWithHoles2d> intersection2D(const Polygon2d& P1, const Polygon2d& P2) {
    // FUNCTION_ID: cg3lib_func022 - START
    std::vector<PolygonWithHoles2d> result;
    CGAL::intersection(P1, P2, std::back_inserter(result));
    return result;
    // FUNCTION_ID: cg3lib_func022 - END
}

std::vector<PolygonWithHoles2d> union2D(const Polygon2d& P1, const Polygon2d& P2) {
    // FUNCTION_ID: cg3lib_func023 - START
    std::vector<PolygonWithHoles2d> result;
    PolygonWithHoles2d pwh;
    if (CGAL::join(P1, P2, pwh)) {
        result.push_back(pwh);
    }
    return result;
    // FUNCTION_ID: cg3lib_func023 - END
}

bool doIntersect2D(const Polygon2d& P1, const Polygon2d& P2) {
    // FUNCTION_ID: cg3lib_func024 - START
    return CGAL::do_intersect(P1, P2);
    // FUNCTION_ID: cg3lib_func024 - END
}

std::vector<Point2d> segmentIntersections(const std::vector<Segment2d>& segments) {
    // FUNCTION_ID: cg3lib_func025 - START
    std::vector<Point2d> intersections;
    for (size_t i = 0; i < segments.size(); ++i) {
        for (size_t j = i + 1; j < segments.size(); ++j) {
            auto result = CGAL::intersection(segments[i], segments[j]);
            if (result) {
                if (const Point2d* p = boost::get<Point2d>(&*result)) {
                    intersections.push_back(*p);
                }
            }
        }
    }
    return intersections;
    // FUNCTION_ID: cg3lib_func025 - END
}

Arrangement2 computeArrangement(const std::vector<Segment2d>& segments) {
    // FUNCTION_ID: cg3lib_func026 - START
    Arrangement2 arr;
    CGAL::insert(arr, segments.begin(), segments.end());
    return arr;
    // FUNCTION_ID: cg3lib_func026 - END
}

std::vector<Polygon2d> extractArrangementFaces(const Arrangement2& arr) {
    // FUNCTION_ID: cg3lib_func027 - START
    std::vector<Polygon2d> faces;
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        if (!fit->is_unbounded()) {
            Polygon2d poly;
            auto ccb = fit->outer_ccb();
            auto curr = ccb;
            do {
                poly.push_back(curr->source()->point());
                ++curr;
            } while (curr != ccb);
            faces.push_back(poly);
        }
    }
    return faces;
    // FUNCTION_ID: cg3lib_func027 - END
}
