cmake_minimum_required(VERSION 3.13.0)
project(SPH_Benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GLM
find_package(glm REQUIRED)

# Core SPH library
add_library(sph_lib STATIC
    src/sph_functions.cpp
    src/sph_types.cpp
)

target_include_directories(sph_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLM_INCLUDE_DIRS}
)

target_link_libraries(sph_lib PUBLIC glm::glm)

# Enable testing
enable_testing()

# List of all test files
set(TEST_SOURCES
    test_getHash
    test_getCell
    test_createNeighborTable
    test_calculateKernelPoly6
    test_calculateKernelSpikyGrad
    test_calculateKernelSpikyLap
    test_calculateSelfDensity
    test_calculateDensityContribution
    test_calculatePressure
    test_accumulateDensityFromNeighbor
    test_calculatePressureForce
    test_calculateViscosityForce
    test_accumulateForcesFromNeighbor
    test_integrateVelocity
    test_integratePosition
    test_handleGroundCollision
    test_handleWallCollisionX
    test_handleWallCollisionZ
    test_applyBoundaryConditions
    test_isWithinSmoothingRadius
    test_calculateDistanceSquared
    test_normalizeDirection
    test_computeAcceleration
    test_parallelCalculateHashes
    test_parallelDensityAndPressures
    test_parallelForces
    test_parallelUpdateParticlePositions
)

# Create test executables
foreach(test_name ${TEST_SOURCES})
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE sph_lib)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
