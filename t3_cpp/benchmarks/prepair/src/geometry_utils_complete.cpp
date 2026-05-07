#include "geometry_utils.h"
#include <cmath>
#include <algorithm>

namespace GeometryUtils {
    
    double calculate_distance(double x1, double y1, double x2, double y2) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    std::pair<double, double> calculate_midpoint(double x1, double y1, double x2, double y2) {
        return std::make_pair((x1 + x2) / 2.0, (y1 + y2) / 2.0);
    }
    
    double calculate_polygon_area(const std::vector<std::pair<double, double>>& points) {
        if (points.size() < 3) return 0.0;
        
        double area = 0.0;
        size_t n = points.size();
        
        for (size_t i = 0; i < n; ++i) {
            size_t j = (i + 1) % n;
            area += points[i].first * points[j].second;
            area -= points[j].first * points[i].second;
        }
        
        return std::abs(area) / 2.0;
    }
    
    bool is_point_inside_polygon(double px, double py, const std::vector<std::pair<double, double>>& polygon) {
        if (polygon.size() < 3) return false;
        
        bool inside = false;
        size_t n = polygon.size();
        
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            double xi = polygon[i].first, yi = polygon[i].second;
            double xj = polygon[j].first, yj = polygon[j].second;
            
            if (((yi > py) != (yj > py)) && 
                (px < (xj - xi) * (py - yi) / (yj - yi) + xi)) {
                inside = !inside;
            }
        }
        
        return inside;
    }
    
    bool is_polygon_clockwise(const std::vector<std::pair<double, double>>& points) {
        if (points.size() < 3) return false;
        
        double sum = 0.0;
        size_t n = points.size();
        
        for (size_t i = 0; i < n; ++i) {
            size_t j = (i + 1) % n;
            sum += (points[j].first - points[i].first) * (points[j].second + points[i].second);
        }
        
        return sum > 0.0;
    }
    
    bool is_polygon_closed(const std::vector<std::pair<double, double>>& points) {
        if (points.size() < 2) return false;
        
        const double epsilon = 1e-9;
        const auto& first = points.front();
        const auto& last = points.back();
        
        return (std::abs(first.first - last.first) < epsilon && 
                std::abs(first.second - last.second) < epsilon);
    }
    
    std::vector<std::pair<double, double>> close_polygon(const std::vector<std::pair<double, double>>& points) {
        if (points.empty()) return points;
        
        std::vector<std::pair<double, double>> result = points;
        
        // Check if closed inline to avoid dependency
        const double epsilon = 1e-9;
        const auto& first = points.front();
        const auto& last = points.back();
        bool is_closed = (std::abs(first.first - last.first) < epsilon && 
                         std::abs(first.second - last.second) < epsilon);
        
        if (!is_closed) {
            result.push_back(points.front());
        }
        
        return result;
    }
    
    std::vector<std::pair<double, double>> reverse_polygon_orientation(const std::vector<std::pair<double, double>>& points) {
        std::vector<std::pair<double, double>> result = points;
        std::reverse(result.begin(), result.end());
        return result;
    }
}