cmake_minimum_required(VERSION 3.10)
project(adhesion_benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(adhesion_functions src/adhesion_functions.cpp)
target_link_libraries(adhesion_functions CGAL::CGAL)

enable_testing()

add_executable(test_edge_count tests/test_edge_count.cpp)
target_link_libraries(test_edge_count adhesion_functions CGAL::CGAL)
add_test(NAME test_edge_count COMMAND test_edge_count)

add_executable(test_calculate_velocity tests/test_calculate_velocity.cpp)
target_link_libraries(test_calculate_velocity adhesion_functions CGAL::CGAL)
add_test(NAME test_calculate_velocity COMMAND test_calculate_velocity)

add_executable(test_extract_nodes tests/test_extract_nodes.cpp)
target_link_libraries(test_extract_nodes adhesion_functions CGAL::CGAL)
add_test(NAME test_extract_nodes COMMAND test_extract_nodes)

add_executable(test_extract_walls tests/test_extract_walls.cpp)
target_link_libraries(test_extract_walls adhesion_functions CGAL::CGAL)
add_test(NAME test_extract_walls COMMAND test_extract_walls)

add_executable(test_extract_filaments tests/test_extract_filaments.cpp)
target_link_libraries(test_extract_filaments adhesion_functions CGAL::CGAL)
add_test(NAME test_extract_filaments COMMAND test_extract_filaments)

add_executable(test_power_diagram_faces tests/test_power_diagram_faces.cpp)
target_link_libraries(test_power_diagram_faces adhesion_functions CGAL::CGAL)
add_test(NAME test_power_diagram_faces COMMAND test_power_diagram_faces)

add_executable(test_power_diagram_edges tests/test_power_diagram_edges.cpp)
target_link_libraries(test_power_diagram_edges adhesion_functions CGAL::CGAL)
add_test(NAME test_power_diagram_edges COMMAND test_power_diagram_edges)
