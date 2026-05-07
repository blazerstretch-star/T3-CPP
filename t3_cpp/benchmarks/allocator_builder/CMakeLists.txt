cmake_minimum_required(VERSION 3.10)
project(AllocatorBuilder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(allocator_functions src/allocator_functions.cpp)

enable_testing()

add_executable(test_stack_owns tests/test_stack_owns.cpp)
target_link_libraries(test_stack_owns allocator_functions)
add_test(NAME test_stack_owns COMMAND test_stack_owns)

add_executable(test_stack_allocate tests/test_stack_allocate.cpp)
target_link_libraries(test_stack_allocate allocator_functions)
add_test(NAME test_stack_allocate COMMAND test_stack_allocate)

add_executable(test_stack_deallocate tests/test_stack_deallocate.cpp)
target_link_libraries(test_stack_deallocate allocator_functions)
add_test(NAME test_stack_deallocate COMMAND test_stack_deallocate)

add_executable(test_stack_reallocate tests/test_stack_reallocate.cpp)
target_link_libraries(test_stack_reallocate allocator_functions)
add_test(NAME test_stack_reallocate COMMAND test_stack_reallocate)

add_executable(test_owns tests/test_owns.cpp)
target_link_libraries(test_owns allocator_functions)
add_test(NAME test_owns COMMAND test_owns)

add_executable(test_allocate tests/test_allocate.cpp)
target_link_libraries(test_allocate allocator_functions)
add_test(NAME test_allocate COMMAND test_allocate)

add_executable(test_deallocate tests/test_deallocate.cpp)
target_link_libraries(test_deallocate allocator_functions)
add_test(NAME test_deallocate COMMAND test_deallocate)

add_executable(test_reallocate tests/test_reallocate.cpp)
target_link_libraries(test_reallocate allocator_functions)
add_test(NAME test_reallocate COMMAND test_reallocate)
