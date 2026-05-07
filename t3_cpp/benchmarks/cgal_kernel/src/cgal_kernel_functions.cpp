#include "cgal_kernel_functions.h"
#include <algorithm>
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


namespace CGALKernel {

// ============================================================================
// SIMPLE FUNCTIONS (10) - API Wrappers
// ============================================================================

Point_2 midpoint(const Point_2& p, const Point_2& q) {
    // FUNCTION_ID: cgal_kernel_func001 - START
    return CGAL::midpoint(p, q);
    // FUNCTION_ID: cgal_kernel_func001 - END
}

bool collinear(const Point_2& p, const Point_2& q, const Point_2& r) {
    // FUNCTION_ID: cgal_kernel_func002 - START
    return CGAL::collinear(p, q, r);
    // FUNCTION_ID: cgal_kernel_func002 - END
}

bool left_turn(const Point_2& p, const Point_2& q, const Point_2& r) {
    // FUNCTION_ID: cgal_kernel_func003 - START
    return CGAL::left_turn(p, q, r);
    // FUNCTION_ID: cgal_kernel_func003 - END
}

bool right_turn(const Point_2& p, const Point_2& q, const Point_2& r) {
    // FUNCTION_ID: cgal_kernel_func004 - START
    return CGAL::right_turn(p, q, r);
    // FUNCTION_ID: cgal_kernel_func004 - END
}

FT triangle_area(const Point_2& p, const Point_2& q, const Point_2& r) {
    // FUNCTION_ID: cgal_kernel_func005 - START
    return CGAL::area(p, q, r);
    // FUNCTION_ID: cgal_kernel_func005 - END
}

CGAL::Orientation orientation(const Point_2& p, const Point_2& q, const Point_2& r) {
    // FUNCTION_ID: cgal_kernel_func006 - START
    return CGAL::orientation(p, q, r);
    // FUNCTION_ID: cgal_kernel_func006 - END
}

FT squared_distance(const Point_2& p, const Point_2& q) {
    // FUNCTION_ID: cgal_kernel_func007 - START
    return CGAL::squared_distance(p, q);
    // FUNCTION_ID: cgal_kernel_func007 - END
}

FT determinant(const Vector_2& u, const Vector_2& v) {
    // FUNCTION_ID: cgal_kernel_func008 - START
    return u.x() * v.y() - u.y() * v.x();
    // FUNCTION_ID: cgal_kernel_func008 - END
}

CGAL::Comparison_result compare_x(const Point_2& p, const Point_2& q) {
    // FUNCTION_ID: cgal_kernel_func009 - START
    return CGAL::compare_x(p, q);
    // FUNCTION_ID: cgal_kernel_func009 - END
}

CGAL::Comparison_result compare_y(const Point_2& p, const Point_2& q) {
    // FUNCTION_ID: cgal_kernel_func010 - START
    return CGAL::compare_y(p, q);
    // FUNCTION_ID: cgal_kernel_func010 - END
}

// ============================================================================
// COMPLEX FUNCTIONS (5) - Algorithmic Implementations (EXACT CGAL)
// ============================================================================

// Adapted from CGAL Polygon_2_algorithms.h
FT polygon_area(const std::vector<Point_2>& polygon) {
    // FUNCTION_ID: cgal_kernel_func011 - START
    FT result = FT(0);
    if (polygon.empty()) return result;
    
    auto first = polygon.begin();
    auto last = polygon.end();
    auto second = first; ++second;
    if (second == last) return result;
    
    auto third = second;
    while (++third != last) {
        result = result + CGAL::area(*first, *second, *third);
        second = third;
    }
    return result;
    // FUNCTION_ID: cgal_kernel_func011 - END
}

// Adapted from CGAL Polygon_2_algorithms_impl.h (is_convex_2)
bool is_polygon_convex(const std::vector<Point_2>& polygon) {
    // FUNCTION_ID: cgal_kernel_func012 - START
    if (polygon.size() < 3) return true;
    
    auto first = polygon.begin();
    auto last = polygon.end();
    
    auto previous = first;
    auto current = previous; ++current;
    if (current == last) return true;
    
    auto next = current; ++next;
    if (next == last) return true;
    
    while (previous != last && current != last && *previous == *current) {
        current = next;
        ++next;
        if (next == last) return true;
    }
    
    auto less_xy_2 = [](const Point_2& p1, const Point_2& p2) {
        return CGAL::compare_xy(p1, p2) == CGAL::SMALLER;
    };
    
    bool HasClockwiseTriples = false;
    bool HasCounterClockwiseTriples = false;
    bool Order = less_xy_2(*previous, *current);
    int NumOrderChanges = 0;
    
    do {
    switch_orient:
        CGAL::Orientation orient = CGAL::orientation(*previous, *current, *next);
        switch (orient) {
            case CGAL::CLOCKWISE:
                HasClockwiseTriples = true;
                break;
            case CGAL::COUNTERCLOCKWISE:
                HasCounterClockwiseTriples = true;
                break;
            case CGAL::COLLINEAR:
                if (*current == *next) {
                    if (next == first) {
                        first = current;
                    }
                    ++next;
                    if (next == last)
                        next = first;
                    goto switch_orient;
                }
                break;
        }
        
        bool NewOrder = less_xy_2(*current, *next);
        if (Order != NewOrder) NumOrderChanges++;
        
        if (NumOrderChanges > 2) {
            return false;
        }
        
        if (HasClockwiseTriples && HasCounterClockwiseTriples) {
            return false;
        }
        
        previous = current;
        current = next;
        ++next;
        if (next == last) next = first;
        Order = NewOrder;
    }
    while (previous != first);
    
    return true;
    // FUNCTION_ID: cgal_kernel_func012 - END
}

// Adapted from CGAL Polygon_2_algorithms_impl.h (which_side_in_slab)
namespace internal {
    int which_side_in_slab(const Point_2& point, const Point_2& low, const Point_2& high) {
        CGAL::Comparison_result low_x_comp_res = CGAL::compare_x(point, low);
        CGAL::Comparison_result high_x_comp_res = CGAL::compare_x(point, high);
        if (low_x_comp_res == CGAL::SMALLER) {
            if (high_x_comp_res == CGAL::SMALLER)
                return -1;
        } else {
            switch (high_x_comp_res) {
                case CGAL::LARGER: return 1;
                case CGAL::SMALLER: break;
                case CGAL::EQUAL: return (low_x_comp_res == CGAL::EQUAL) ? 0 : 1;
            }
        }
        switch (CGAL::orientation(low, point, high)) {
            case CGAL::LEFT_TURN: return 1;
            case CGAL::RIGHT_TURN: return -1;
            default: return 0;
        }
    }
}

// Adapted from CGAL Polygon_2_algorithms_impl.h (bounded_side_2)
CGAL::Bounded_side point_in_polygon(const std::vector<Point_2>& polygon, const Point_2& point) {
    // FUNCTION_ID: cgal_kernel_func013 - START
    if (polygon.size() < 2) return CGAL::ON_UNBOUNDED_SIDE;
    
    auto first = polygon.begin();
    auto last = polygon.end();
    auto current = first;
    auto next = current; ++next;
    if (next == last) return CGAL::ON_UNBOUNDED_SIDE;
    
    bool IsInside = false;
    CGAL::Comparison_result cur_y_comp_res = CGAL::compare_y(*current, point);
    
    do {
        CGAL::Comparison_result next_y_comp_res = CGAL::compare_y(*next, point);
        
        switch (cur_y_comp_res) {
            case CGAL::SMALLER:
                switch (next_y_comp_res) {
                    case CGAL::SMALLER:
                        break;
                    case CGAL::EQUAL:
                        switch (CGAL::compare_x(point, *next)) {
                            case CGAL::SMALLER: IsInside = !IsInside; break;
                            case CGAL::EQUAL:   return CGAL::ON_BOUNDARY;
                            case CGAL::LARGER:  break;
                        }
                        break;
                    case CGAL::LARGER:
                        switch (internal::which_side_in_slab(point, *current, *next)) {
                            case -1: IsInside = !IsInside; break;
                            case  0: return CGAL::ON_BOUNDARY;
                        }
                        break;
                }
                break;
            case CGAL::EQUAL:
                switch (next_y_comp_res) {
                    case CGAL::SMALLER:
                        switch (CGAL::compare_x(point, *current)) {
                            case CGAL::SMALLER: IsInside = !IsInside; break;
                            case CGAL::EQUAL:   return CGAL::ON_BOUNDARY;
                            case CGAL::LARGER:  break;
                        }
                        break;
                    case CGAL::EQUAL:
                        switch (CGAL::compare_x(point, *current)) {
                            case CGAL::SMALLER:
                                if (CGAL::compare_x(point, *next) != CGAL::SMALLER)
                                    return CGAL::ON_BOUNDARY;
                                break;
                            case CGAL::EQUAL: return CGAL::ON_BOUNDARY;
                            case CGAL::LARGER:
                                if (CGAL::compare_x(point, *next) != CGAL::LARGER)
                                    return CGAL::ON_BOUNDARY;
                                break;
                        }
                        break;
                    case CGAL::LARGER:
                        if (CGAL::compare_x(point, *current) == CGAL::EQUAL) {
                            return CGAL::ON_BOUNDARY;
                        }
                        break;
                }
                break;
            case CGAL::LARGER:
                switch (next_y_comp_res) {
                    case CGAL::SMALLER:
                        switch (internal::which_side_in_slab(point, *next, *current)) {
                            case -1: IsInside = !IsInside; break;
                            case  0: return CGAL::ON_BOUNDARY;
                        }
                        break;
                    case CGAL::EQUAL:
                        if (CGAL::compare_x(point, *next) == CGAL::EQUAL) {
                            return CGAL::ON_BOUNDARY;
                        }
                        break;
                    case CGAL::LARGER:
                        break;
                }
                break;
        }
        
        current = next;
        cur_y_comp_res = next_y_comp_res;
        ++next;
        if (next == last) next = first;
    }
    while (current != first);
    
    return IsInside ? CGAL::ON_BOUNDED_SIDE : CGAL::ON_UNBOUNDED_SIDE;
    // FUNCTION_ID: cgal_kernel_func013 - END
}

// Adapted from CGAL Polygon_2_algorithms_impl.h (left_vertex_2)
Point_2 leftmost_point(const std::vector<Point_2>& points) {
    // FUNCTION_ID: cgal_kernel_func014 - START
    if (points.empty()) return Point_2(0, 0);
    
    auto less_xy = [](const Point_2& p1, const Point_2& p2) {
        return CGAL::compare_xy(p1, p2) == CGAL::SMALLER;
    };
    return *std::min_element(points.begin(), points.end(), less_xy);
    // FUNCTION_ID: cgal_kernel_func014 - END
}

// Adapted from CGAL ch_graham_andrew_impl.h (ch_graham_andrew_scan)
std::vector<Point_2> convex_hull_graham(std::vector<Point_2> points) {
    // FUNCTION_ID: cgal_kernel_func015 - START
    if (points.empty()) return points;
    
    auto less_xy = [](const Point_2& p1, const Point_2& p2) {
        return CGAL::compare_xy(p1, p2) == CGAL::SMALLER;
    };
    
    std::sort(points.begin(), points.end(), less_xy);
    
    if (points.size() < 2) return points;
    if (points.front() == points.back()) {
        return std::vector<Point_2>{points.front()};
    }
    
    std::vector<size_t> S;
    S.push_back(points.size() - 1);
    S.push_back(0);
    
    size_t iter = 0;
    do {
        ++iter;
    }
    while (iter < points.size() - 1 && !CGAL::left_turn(points.back(), points[0], points[iter]));
    
    if (iter < points.size() - 1) {
        S.push_back(iter);
        size_t alpha = iter;
        size_t beta = S[S.size() - 2];
        
        for (++iter; iter < points.size() - 1; ++iter) {
            if (CGAL::left_turn(points[alpha], points[iter], points.back())) {
                while (!CGAL::left_turn(points[beta], points[alpha], points[iter])) {
                    S.pop_back();
                    alpha = beta;
                    beta = S[S.size() - 2];
                }
                S.push_back(iter);
                beta = alpha;
                alpha = iter;
            }
        }
    }
    
    std::vector<Point_2> result;
    for (size_t i = 1; i < S.size(); ++i) {
        result.push_back(points[S[i]]);
    }
    return result;
    // FUNCTION_ID: cgal_kernel_func015 - END
}

}
