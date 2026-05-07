cmake_minimum_required(VERSION 3.10)
project(littlefs_benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(lfs_functions src/lfs_functions.cpp)

enable_testing()

add_executable(test_lfs_crc tests/test_lfs_crc.cpp)
target_link_libraries(test_lfs_crc lfs_functions)
add_test(NAME test_lfs_crc COMMAND test_lfs_crc)

add_executable(test_lfs_ctz_index tests/test_lfs_ctz_index.cpp)
target_link_libraries(test_lfs_ctz_index lfs_functions)
add_test(NAME test_lfs_ctz_index COMMAND test_lfs_ctz_index)

add_executable(test_lfs_bd_cmp tests/test_lfs_bd_cmp.cpp)
target_link_libraries(test_lfs_bd_cmp lfs_functions)
add_test(NAME test_lfs_bd_cmp COMMAND test_lfs_bd_cmp)

add_executable(test_lfs_alloc tests/test_lfs_alloc.cpp)
target_link_libraries(test_lfs_alloc lfs_functions)
add_test(NAME test_lfs_alloc COMMAND test_lfs_alloc)

add_executable(test_lfs_min tests/test_lfs_min.cpp)
target_link_libraries(test_lfs_min lfs_functions)
add_test(NAME test_lfs_min COMMAND test_lfs_min)

add_executable(test_lfs_aligndown tests/test_lfs_aligndown.cpp)
target_link_libraries(test_lfs_aligndown lfs_functions)
add_test(NAME test_lfs_aligndown COMMAND test_lfs_aligndown)

add_executable(test_lfs_alignup tests/test_lfs_alignup.cpp)
target_link_libraries(test_lfs_alignup lfs_functions)
add_test(NAME test_lfs_alignup COMMAND test_lfs_alignup)

add_executable(test_lfs_pair_swap tests/test_lfs_pair_swap.cpp)
target_link_libraries(test_lfs_pair_swap lfs_functions)
add_test(NAME test_lfs_pair_swap COMMAND test_lfs_pair_swap)

add_executable(test_lfs_pair_isnull tests/test_lfs_pair_isnull.cpp)
target_link_libraries(test_lfs_pair_isnull lfs_functions)
add_test(NAME test_lfs_pair_isnull COMMAND test_lfs_pair_isnull)

add_executable(test_lfs_pair_cmp tests/test_lfs_pair_cmp.cpp)
target_link_libraries(test_lfs_pair_cmp lfs_functions)
add_test(NAME test_lfs_pair_cmp COMMAND test_lfs_pair_cmp)

add_executable(test_lfs_tag_type3 tests/test_lfs_tag_type3.cpp)
target_link_libraries(test_lfs_tag_type3 lfs_functions)
add_test(NAME test_lfs_tag_type3 COMMAND test_lfs_tag_type3)

add_executable(test_lfs_gstate_xor tests/test_lfs_gstate_xor.cpp)
target_link_libraries(test_lfs_gstate_xor lfs_functions)
add_test(NAME test_lfs_gstate_xor COMMAND test_lfs_gstate_xor)
