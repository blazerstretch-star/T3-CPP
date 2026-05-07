#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <vector>
#include <utility>

namespace GeometryUtils {
    // Point operations
    double calculate_distance(double x1, double y1, double x2, double y2);
    std::pair<double, double> calculate_midpoint(double x1, double y1, double x2, double y2);
    
    // Polygon operations
    double calculate_polygon_area(const std::vector<std::pair<double, double>>& points);
    bool is_point_inside_polygon(double px, double py, const std::vector<std::pair<double, double>>& polygon);
    
    // Validation functions
    bool is_polygon_clockwise(const std::vector<std::pair<double, double>>& points);
    bool is_polygon_closed(const std::vector<std::pair<double, double>>& points);
    
    // Repair utilities
    std::vector<std::pair<double, double>> close_polygon(const std::vector<std::pair<double, double>>& points);
    std::vector<std::pair<double, double>> reverse_polygon_orientation(const std::vector<std::pair<double, double>>& points);
}

#endif