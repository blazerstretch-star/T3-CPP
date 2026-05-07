cmake_minimum_required(VERSION 3.10)
project(cgal_kernel_benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -mtune=generic")

find_package(CGAL REQUIRED)

include_directories(${CGAL_INCLUDE_DIRS})
include_directories(src)

# Build the library
add_library(cgal_kernel_lib src/cgal_kernel_functions.cpp)
target_link_libraries(cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

# Simple functions (10)
add_executable(test_midpoint tests/test_midpoint.cpp)
target_link_libraries(test_midpoint cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_collinear tests/test_collinear.cpp)
target_link_libraries(test_collinear cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_left_turn tests/test_left_turn.cpp)
target_link_libraries(test_left_turn cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_right_turn tests/test_right_turn.cpp)
target_link_libraries(test_right_turn cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_triangle_area tests/test_triangle_area.cpp)
target_link_libraries(test_triangle_area cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_orientation tests/test_orientation.cpp)
target_link_libraries(test_orientation cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_squared_distance tests/test_squared_distance.cpp)
target_link_libraries(test_squared_distance cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_determinant tests/test_determinant.cpp)
target_link_libraries(test_determinant cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_compare_x tests/test_compare_x.cpp)
target_link_libraries(test_compare_x cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_compare_y tests/test_compare_y.cpp)
target_link_libraries(test_compare_y cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

# Complex functions (5)
add_executable(test_polygon_area tests/test_polygon_area.cpp)
target_link_libraries(test_polygon_area cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_is_polygon_convex tests/test_is_polygon_convex.cpp)
target_link_libraries(test_is_polygon_convex cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_point_in_polygon tests/test_point_in_polygon.cpp)
target_link_libraries(test_point_in_polygon cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_leftmost_point tests/test_leftmost_point.cpp)
target_link_libraries(test_leftmost_point cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

add_executable(test_convex_hull_graham tests/test_convex_hull_graham.cpp)
target_link_libraries(test_convex_hull_graham cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)

# Register all tests with CTest
enable_testing()
add_test(NAME test_midpoint COMMAND test_midpoint)
add_test(NAME test_collinear COMMAND test_collinear)
add_test(NAME test_left_turn COMMAND test_left_turn)
add_test(NAME test_right_turn COMMAND test_right_turn)
add_test(NAME test_triangle_area COMMAND test_triangle_area)
add_test(NAME test_orientation COMMAND test_orientation)
add_test(NAME test_squared_distance COMMAND test_squared_distance)
add_test(NAME test_determinant COMMAND test_determinant)
add_test(NAME test_compare_x COMMAND test_compare_x)
add_test(NAME test_compare_y COMMAND test_compare_y)
add_test(NAME test_polygon_area COMMAND test_polygon_area)
add_test(NAME test_is_polygon_convex COMMAND test_is_polygon_convex)
add_test(NAME test_point_in_polygon COMMAND test_point_in_polygon)
add_test(NAME test_leftmost_point COMMAND test_leftmost_point)
add_test(NAME test_convex_hull_graham COMMAND test_convex_hull_graham)

# Strict tests (enhanced property-based tests)
add_executable(test_midpoint_strict tests/test_midpoint_strict.cpp)
target_link_libraries(test_midpoint_strict cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)
add_test(NAME test_midpoint_strict COMMAND test_midpoint_strict)

add_executable(test_collinear_strict tests/test_collinear_strict.cpp)
target_link_libraries(test_collinear_strict cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)
add_test(NAME test_collinear_strict COMMAND test_collinear_strict)

add_executable(test_orientation_strict tests/test_orientation_strict.cpp)
target_link_libraries(test_orientation_strict cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)
add_test(NAME test_orientation_strict COMMAND test_orientation_strict)

add_executable(test_polygon_area_strict tests/test_polygon_area_strict.cpp)
target_link_libraries(test_polygon_area_strict cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)
add_test(NAME test_polygon_area_strict COMMAND test_polygon_area_strict)

add_executable(test_convex_hull_graham_strict tests/test_convex_hull_graham_strict.cpp)
target_link_libraries(test_convex_hull_graham_strict cgal_kernel_lib ${CGAL_LIBRARIES} gmp mpfr)
add_test(NAME test_convex_hull_graham_strict COMMAND test_convex_hull_graham_strict)
