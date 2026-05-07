cmake_minimum_required(VERSION 3.10)
project(pbrt_benchmark CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Source files
add_library(pbrt_functions STATIC src/pbrt_functions.cpp)
target_include_directories(pbrt_functions PUBLIC src)

# Enable testing
enable_testing()

# Individual test executables
add_executable(test_dot tests/test_dot.cpp)
target_link_libraries(test_dot pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_dot COMMAND test_dot)

add_executable(test_cross tests/test_cross.cpp)
target_link_libraries(test_cross pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_cross COMMAND test_cross)

add_executable(test_normalize tests/test_normalize.cpp)
target_link_libraries(test_normalize pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_normalize COMMAND test_normalize)

add_executable(test_coordinate_system tests/test_coordinate_system.cpp)
target_link_libraries(test_coordinate_system pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_coordinate_system COMMAND test_coordinate_system)

add_executable(test_distance tests/test_distance.cpp)
target_link_libraries(test_distance pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_distance COMMAND test_distance)

add_executable(test_uniform_sample_sphere tests/test_uniform_sample_sphere.cpp)
target_link_libraries(test_uniform_sample_sphere pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_uniform_sample_sphere COMMAND test_uniform_sample_sphere)

add_executable(test_uniform_sphere_pdf tests/test_uniform_sphere_pdf.cpp)
target_link_libraries(test_uniform_sphere_pdf pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_uniform_sphere_pdf COMMAND test_uniform_sphere_pdf)

add_executable(test_uniform_sample_hemisphere tests/test_uniform_sample_hemisphere.cpp)
target_link_libraries(test_uniform_sample_hemisphere pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_uniform_sample_hemisphere COMMAND test_uniform_sample_hemisphere)

add_executable(test_uniform_hemisphere_pdf tests/test_uniform_hemisphere_pdf.cpp)
target_link_libraries(test_uniform_hemisphere_pdf pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_uniform_hemisphere_pdf COMMAND test_uniform_hemisphere_pdf)

add_executable(test_concentric_sample_disk tests/test_concentric_sample_disk.cpp)
target_link_libraries(test_concentric_sample_disk pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_concentric_sample_disk COMMAND test_concentric_sample_disk)

add_executable(test_cosine_sample_hemisphere tests/test_cosine_sample_hemisphere.cpp)
target_link_libraries(test_cosine_sample_hemisphere pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_cosine_sample_hemisphere COMMAND test_cosine_sample_hemisphere)

add_executable(test_cosine_hemisphere_pdf tests/test_cosine_hemisphere_pdf.cpp)
target_link_libraries(test_cosine_hemisphere_pdf pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_cosine_hemisphere_pdf COMMAND test_cosine_hemisphere_pdf)

add_executable(test_spherical_direction tests/test_spherical_direction.cpp)
target_link_libraries(test_spherical_direction pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_spherical_direction COMMAND test_spherical_direction)

add_executable(test_balance_heuristic tests/test_balance_heuristic.cpp)
target_link_libraries(test_balance_heuristic pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_balance_heuristic COMMAND test_balance_heuristic)

add_executable(test_power_heuristic tests/test_power_heuristic.cpp)
target_link_libraries(test_power_heuristic pbrt_functions ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME test_power_heuristic COMMAND test_power_heuristic)
