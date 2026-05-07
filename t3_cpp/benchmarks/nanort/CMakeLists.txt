cmake_minimum_required(VERSION 3.10)
project(nanort_benchmark CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
add_library(nanort_functions STATIC
    src/nanort_functions.cpp
)

target_include_directories(nanort_functions PUBLIC src)

# Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Individual test executables
add_executable(test_vneg tests/test_vneg.cpp)
target_link_libraries(test_vneg nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_operator_multiply tests/test_operator_multiply.cpp)
target_link_libraries(test_operator_multiply nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_vlength tests/test_vlength.cpp)
target_link_libraries(test_vlength nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_vnormalize tests/test_vnormalize.cpp)
target_link_libraries(test_vnormalize nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_vcross tests/test_vcross.cpp)
target_link_libraries(test_vcross nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_vdot tests/test_vdot.cpp)
target_link_libraries(test_vdot nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_vsafe_inverse tests/test_vsafe_inverse.cpp)
target_link_libraries(test_vsafe_inverse nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_calculate_surface_area tests/test_calculate_surface_area.cpp)
target_link_libraries(test_calculate_surface_area nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_get_bounding_box_of_triangle tests/test_get_bounding_box_of_triangle.cpp)
target_link_libraries(test_get_bounding_box_of_triangle nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_safemin tests/test_safemin.cpp)
target_link_libraries(test_safemin nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_safemax tests/test_safemax.cpp)
target_link_libraries(test_safemax nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_intersect_ray_aabb tests/test_intersect_ray_aabb.cpp)
target_link_libraries(test_intersect_ray_aabb nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_sah tests/test_sah.cpp)
target_link_libraries(test_sah nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_find_cut_from_bin_buffer tests/test_find_cut_from_bin_buffer.cpp)
target_link_libraries(test_find_cut_from_bin_buffer nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_safe_minmax tests/test_safe_minmax.cpp)
target_link_libraries(test_safe_minmax nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(test_scalar_multiply tests/test_scalar_multiply.cpp)
target_link_libraries(test_scalar_multiply nanort_functions ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

enable_testing()
add_test(NAME test_vneg COMMAND test_vneg)
add_test(NAME test_operator_multiply COMMAND test_operator_multiply)
add_test(NAME test_vlength COMMAND test_vlength)
add_test(NAME test_vnormalize COMMAND test_vnormalize)
add_test(NAME test_vcross COMMAND test_vcross)
add_test(NAME test_vdot COMMAND test_vdot)
add_test(NAME test_vsafe_inverse COMMAND test_vsafe_inverse)
add_test(NAME test_calculate_surface_area COMMAND test_calculate_surface_area)
add_test(NAME test_get_bounding_box_of_triangle COMMAND test_get_bounding_box_of_triangle)
add_test(NAME test_safemin COMMAND test_safemin)
add_test(NAME test_safemax COMMAND test_safemax)
add_test(NAME test_intersect_ray_aabb COMMAND test_intersect_ray_aabb)
add_test(NAME test_sah COMMAND test_sah)
add_test(NAME test_find_cut_from_bin_buffer COMMAND test_find_cut_from_bin_buffer)
add_test(NAME test_safe_minmax COMMAND test_safe_minmax)
add_test(NAME test_scalar_multiply COMMAND test_scalar_multiply)
