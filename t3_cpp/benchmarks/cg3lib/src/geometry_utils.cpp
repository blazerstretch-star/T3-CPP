#include "geometry_utils.h"
#include <cmath>
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


double pointDistance(const Point3d& p1, const Point3d& p2) {
    // FUNCTION_ID: cg3lib_func015 - START
    double dx = p1.x() - p2.x();
    double dy = p1.y() - p2.y();
    double dz = p1.z() - p2.z();
    return std::sqrt(dx*dx + dy*dy + dz*dz);
    // FUNCTION_ID: cg3lib_func015 - END
}

Point3d pointMidpoint(const Point3d& p1, const Point3d& p2) {
    // FUNCTION_ID: cg3lib_func016 - START
    double mx = (p1.x() + p2.x()) / 2.0;
    double my = (p1.y() + p2.y()) / 2.0;
    double mz = (p1.z() + p2.z()) / 2.0;
    return Point3d(mx, my, mz);
    // FUNCTION_ID: cg3lib_func016 - END
}

Vector3d vectorNormalize(const Vector3d& v) {
    // FUNCTION_ID: cg3lib_func017 - START
    double len = std::sqrt(v.x()*v.x() + v.y()*v.y() + v.z()*v.z());
    return Vector3d(v.x()/len, v.y()/len, v.z()/len);
    // FUNCTION_ID: cg3lib_func017 - END
}

double vectorLength(const Vector3d& v) {
    // FUNCTION_ID: cg3lib_func018 - START
    return std::sqrt(v.x()*v.x() + v.y()*v.y() + v.z()*v.z());
    // FUNCTION_ID: cg3lib_func018 - END
}

BBox3d boundingBox(const std::vector<Point3d>& points) {
    // FUNCTION_ID: cg3lib_func019 - START
    BBox3d bbox = points[0].bbox();
    for(size_t i = 1; i < points.size(); i++) {
        bbox += points[i].bbox();
    }
    return bbox;
    // FUNCTION_ID: cg3lib_func019 - END
}
